# Simple test for NeoPixels on Raspberry Pi
import time
import board
import neopixel
from flask import Flask

app = Flask(__name__)


import threading
 

class Lights(threading.Thread):
  def __init__(self):
    threading.Thread.__init__(self)

    # Choose an open pin connected to the Data In of the NeoPixel strip, i.e. board.D18
    # NeoPixels must be connected to D10, D12, D18 or D21 to work.
    pixel_pin = board.D18
     
    # The number of NeoPixels
    self.num_pixels = 8
     
    # The order of the pixel colors - RGB or GRB. Some NeoPixels have red and green reversed!
    # For RGBW NeoPixels, simply change the ORDER to RGBW or GRBW.
    self.ORDER = neopixel.GRB
     
    self.pixels = neopixel.NeoPixel(
        pixel_pin, self.num_pixels, brightness=0.2, auto_write=False, pixel_order=self.ORDER
    )
 

  
  def rainbow_cycle(self, wait):
    for j in range(255):
      for i in range(self.num_pixels):
          pixel_index = (i * 256 // self.num_pixels) + j
          self.pixels[i] = self.wheel(pixel_index & 255)
      self.pixels.show()
      time.sleep(wait)

  def wheel(self, pos):
      # Input a value 0 to 255 to get a color value.
      # The colours are a transition r - g - b - back to r.
      if pos < 0 or pos > 255:
          r = g = b = 0
      elif pos < 85:
          r = int(pos * 3)
          g = int(255 - pos * 3)
          b = 0
      elif pos < 170:
          pos -= 85
          r = int(255 - pos * 3)
          g = 0
          b = int(pos * 3)
      else:
          pos -= 170
          r = 0
          g = int(pos * 3)
          b = int(255 - pos * 3)
      return (r, g, b) if self.ORDER in (neopixel.RGB, neopixel.GRB) else (r, g, b, 0)
 

  def white(self):
    self.pixels.fill((255,255,255))
    self.pixels.show()

  def off(self):
    self.pixels.fill((0,0,0))
    self.pixels.show()

lights = Lights()

@app.route('/white')
def set_white():
  lights.white()
  return 'ok'

@app.route('/off')
def set_off():
  lights.off()
  return 'ok'

@app.route('/rainbow')
def set_rainbow():
  lights.rainbow_cycle(0.001)
  lights.rainbow_cycle(0.001)
  lights.rainbow_cycle(0.001)
  lights.rainbow_cycle(0.001)
  lights.off()
  return 'ok'

if __name__ == '__main__':
  lights.rainbow_cycle(0.001)
  lights.off()
  app.run("0.0.0.0", port=5001)


