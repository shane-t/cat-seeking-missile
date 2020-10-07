import json

class RobotState():
  def __init__(self):
    self.yaw = 320
    self.pitch = 320
    self.speed = 255
    self.next_move = ''  

  def handle_data(self, data):
    print(data)
    values = data.split('|')
    for value in values:
      key = value.split(':')[0]
      val = value.split(':')[1]

      if key == 'y':
        self.yaw = int(val)
      if key == 'p':
        self.pitch = int(val)
      if key == 's':
        self.speed = int(val)
      if key == 'm':
        self.next_move = val # not really a number

  def json(self):
    state = {
      'pitch' : self.pitch,
      'yaw' : self.yaw,
      'speed' : self.speed,
      'next_move' : self.next_move
    }

    return json.dumps(state)
  
  def __repr__(self):
    return f'Speed: {self.speed} Pitch: {self.pitch} Yaw: {self.yaw} Next move: {self.next_move}'


