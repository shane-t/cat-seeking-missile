import serial

class Light():

  def __init__(self, port):
    self.port = port

    self.connect()
    self.rainbow()

  def connect(self):
    self.ser  = serial.Serial(self.port, 9600, timeout=1)

  def write(self, data):
    print('Writing ' + data.decode('utf-8'))
    self.ser.write(data + '\n'.encode())

  def white(self):
    self.write(b'white')

  def off(self):
    self.write(b'off')

  def brightness(self, num):
    self.write(b'bri:' + str(num).encode('ascii'))

  def laser_on(self):
    self.write(b'laser:on')

  def laser_off(self):
    self.write(b'laser:off')

  def rainbow(self):
    self.write(b'rainbow')
