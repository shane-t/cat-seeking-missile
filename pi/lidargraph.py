from time import sleep
import matplotlib
from threading import Thread
import io
from rplidar import RPLidar
import os 
import RPi.GPIO as GPIO
import atexit
from breezyslam.algorithms import RMHC_SLAM
from breezyslam.sensors import RPLidarA1 as LaserModel
import numpy as np

MAP_SIZE_PIXELS         = 500
MAP_SIZE_METERS         = 20
MIN_SAMPLES = 180

GPIO.setmode(GPIO.BCM)
GPIO.setup(23, GPIO.OUT)

@atexit.register
def shutdown_lidar():
  GPIO.output(23, GPIO.LOW)

sleep(1)

matplotlib.use('Agg')


DMAX = 4000
IMIN = 0
IMAX = 50

class LidarGraph(Thread):

  def __init__(self, port):
    Thread.__init__(self)
    self.scan = None
    self.running = False
    self.port = port

  def setup(self):
    GPIO.output(23, GPIO.HIGH)
    sleep(1)
    self.lidar = RPLidar(self.port)
    self.lidar.connect()
    self.lidar.start_motor()

    self.slam = RMHC_SLAM(LaserModel(), MAP_SIZE_PIXELS, MAP_SIZE_METERS, map_quality=1, max_search_iter=50)
    self.mapbytes = bytearray(MAP_SIZE_PIXELS * MAP_SIZE_PIXELS)

    success = False
    while success is not True:
      try:
        self.iterator = self.lidar.iter_scans()
        next(self.iterator) #first scan is shit
        success=True
      except Exception as e:
        print('retrying')
        print(e)
        sleep(0.5)
     

  def restart(self):
    self.stop()
    self.go()

  def go(self):
    self.setup()
    self.running = True

  def stop(self):
    print('Stopping')
    self.running = False
    self.lidar.stop_motor()
    self.lidar.disconnect()
    del self.lidar
    GPIO.output(23, GPIO.LOW) # turn off lidar relay
    sleep(1)

  def update(self):
    self.scan = next(self.iterator)
    self.offsets = np.array([(np.radians(meas[1]), meas[2]) for meas in self.scan])
    self.intens = np.array([meas[0] for meas in self.scan])

    # BreezySLAM
    previous_distances = None
    previous_angles    = None

    items = [item for item in self.scan]
    distances = [item[2] for item in items]
    angles    = [item[1] for item in items]

    print(str(len(distances)) + ' distances')

    # Update SLAM with current Lidar scan and scan angles if adequate
    if len(distances) > MIN_SAMPLES:
        print('updating slam')
        self.slam.update(distances, scan_angles_degrees=angles)
        previous_distances = distances.copy()
        previous_angles    = angles.copy()

    # If not adequate, use previous
    elif previous_distances is not None:
        self.slam.update(previous_distances, scan_angles_degrees=previous_angles)

    # Get current robot position
    x, y, theta = self.slam.getpos()

    print('x ' + str(x) + 'y ' + str(y) + 'theta ' + str(theta))

    # Get current map bytes as grayscale
    self.slam.getmap(self.mapbytes)
    sleep(0.05) #gather more samples?

  def data(self):
    if self.running != True:
      return {
        'intens': [],
        'offsets': []
      }


    return {
      'intens': self.intens.tolist(),
      'offsets': self.offsets.tolist()
    }

  def get_map(self):
    # slam map
    return self.mapbytes

  def run(self):
    print('run')

    iterator = None

    while True:

      if self.running:
        try:
          self.update()
          print('Updated')
        except Exception as e:
          print('Exception during update')
          print(e)
          #self.restart()
          #os._exit(1)
