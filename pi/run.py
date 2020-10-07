import serial

from time import sleep
from lidargraph import LidarGraph
from robot import Robot
from robotstate import RobotState
from light import Light

import detect_image

from sound import Sound

import traceback

from flask import Flask, request, make_response, send_file, jsonify
from requests import get

app = Flask(__name__, 
  static_url_path='/',
  static_folder='build')

from gevent.pywsgi import WSGIServer

import queue

port = '/dev/ttyS0'
light_port = '/dev/ttyUSB0'
lidar_port = '/dev/ttyUSB1'


@app.route('/forward')
def forward():
  robot.forward()
  return 'ok'
  
@app.route('/back')
def back():
  robot.back()
  return 'ok'

@app.route('/left')
def left():
  robot.left()
  return 'ok'

@app.route('/right')
def right():
  robot.right()
  return 'ok'

@app.route('/stop')
def stop():
  robot.stop()
  return 'ok'

@app.route('/speed')
def speed():
  value = request.args.get('value')
  robot.set_speed(value)
  return 'ok'

@app.route('/pitch')
def pitch():
  value = request.args.get('value')
  robot.pitch(value)
  return 'ok'


@app.route('/yaw')
def yaw():
  value = request.args.get('value')
  robot.yaw(value)
  return 'ok'

@app.route('/state')
def state():
  return state.json()


@app.route('/lights/white')
def white():
  light.white()
  return 'ok'

@app.route('/lights/off')
def off():
  light.off()
  return 'ok'

@app.route('/lights/rainbow')
def rainbow():
  light.rainbow()
  return 'ok'

@app.route('/lights/brightness')
def brightness():
  light.brightness(request.args.get('value'))
  return 'ok'

@app.route('/laser/on')
def laser_on():
  light.laser_on()
  return 'ok'

@app.route('/laser/off')
def laser_off():
  light.laser_off()
  return 'ok'

@app.route('/laser/beam')
def laser_beam():
  light.laser_on()
  wav = 'TIE-Fire.wav'
  s.play(wav)
  light.laser_off()
  return 'ok'


@app.route('/camera')
def image():
  image = open('/dev/shm/mjpeg/cam.jpg', 'rb').read()
  response = make_response(image)
  response.headers.set('Content-Type', 'image/jpeg')
  return response

@app.route('/camera_detect')
def image_detect():
  image = detect_image.run('/dev/shm/mjpeg/cam.jpg')
  response = make_response(image)
  response.headers.set('Content-Type', 'image/jpeg')
  return response

@app.route('/lidarimage')
def lidarimage():
  img = lidargraph.get_figure()
  response = make_response(img)
  response.headers.set('Content-Type', 'image/png')
  return response

@app.route('/lidardata')
def lidardata():
  obj = lidargraph.data()

  return make_response(jsonify(obj), 200)

@app.route('/lidarstop')
def lidarstop():
  lidargraph.stop()
  return 'ok'

@app.route('/lidarstart')
def lidarstart():
  lidargraph.go()
  return 'ok'

@app.route('/slam/map')
def slammap():
  map_data = lidargraph.get_map()
  response = make_response(map_data)
  return response

@app.route('/sounds')
def list_sounds():
  sounds = s.listfiles()
  return make_response(jsonify(sounds), 200)

@app.route('/playsound')
def play_sound():
  wav = request.args.get('wav')
  sound_q.put(wav)
  return 'ok'


sound_q = queue.Queue()
s = Sound(sound_q)
s.start()

lidargraph = LidarGraph(lidar_port)
lidargraph.start()

light = Light(light_port)

state = RobotState()
robot = Robot(port, state)
robot.start()

print('serving')
http_server = WSGIServer(('', 5000), app)
http_server.serve_forever()
