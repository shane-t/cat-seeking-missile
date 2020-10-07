
from threading import Thread
import serial
import  queue

class Robot(Thread):

  def __init__(self, port, robotstate):
    Thread.__init__(self)

    self.port = port
    self.robotstate = robotstate
    self.out_q = queue.Queue(maxsize=5)

    self.connect()

  def connect(self):
    print('Connecting to ' + self.port)
    self.ser  = serial.Serial(self.port, 9600, timeout=0.01)

  def write(self, data):
    print('Queueing ' + data.decode('utf-8'))
    self.out_q.put(data)
    
  def set_speed(self, speed):
    speed = str(speed).encode()
    self.write(b'speed:'+speed)


  def forward(self):
    self.write(b'forward')

  def back(self):
    self.write(b'back')

  def left(self):
    self.write(b'left')

  def right(self):
    self.write(b'right')

  def stop(self):
    self.write(b'stop')

  def yaw(self, amount):
    amount = str(amount).encode()
    self.write(b'yaw:'+amount)

  def pitch(self, amount):
    amount = str(amount).encode()
    self.write(b'pitch:'+amount)


  def run(self):
    print('Serial Robot control thread Running')

    while True:
      line = self.ser.readline()
      if len(line):
        self.robotstate.handle_data(line.decode())
      try:
        item = self.out_q.get(False)  # or whatever
        print('Writing ' + item.decode('utf-8'))
        self.ser.write(item + '\n'.encode())
      except queue.Empty:
        continue

