
# Lint as: python3
# Copyright 2019 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
"""Example using TF Lite to detect objects in a given image."""

import argparse
import time

from PIL import Image
from PIL import ImageDraw
from PIL import ImageFont
from PIL import ImageOps

import detect
import tflite_runtime.interpreter as tflite
import platform

import io

EDGETPU_SHARED_LIB = {
  'Linux': 'libedgetpu.so.1',
  'Darwin': 'libedgetpu.1.dylib',
  'Windows': 'edgetpu.dll'
}[platform.system()]


model = 'models/mobilenet_ssd_v2_coco_quant_postprocess_edgetpu.tflite'
labels_file = 'models/coco_labels.txt'
threshold = 0.4

def load_labels(path, encoding='utf-8'):
  """Loads labels from file (with or without index numbers).

  Args:
    path: path to label file.
    encoding: label file encoding.
  Returns:
    Dictionary mapping indices to labels.
  """
  with open(path, 'r', encoding=encoding) as f:
    lines = f.readlines()
    if not lines:
      return {}

    if lines[0].split(' ', maxsplit=1)[0].isdigit():
      pairs = [line.split(' ', maxsplit=1) for line in lines]
      return {int(index): label.strip() for index, label in pairs}
    else:
      return {index: line.strip() for index, line in enumerate(lines)}


def make_interpreter(model_file):
  model_file, *device = model_file.split('@')
  return tflite.Interpreter(
      model_path=model_file,
      experimental_delegates=[
          tflite.load_delegate(EDGETPU_SHARED_LIB,
                               {'device': device[0]} if device else {})
      ])


def draw_objects(draw, objs, labels):
  """Draws the bounding box and label for each object."""
  for obj in objs:
    bbox = obj.bbox
    draw.rectangle([(bbox.xmin, bbox.ymin), (bbox.xmax, bbox.ymax)],
                   outline='red')
    font = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 16)
    draw.text((bbox.xmin + 10, bbox.ymin + 10),
              '%s\n%.2f' % (labels.get(obj.id, obj.id), obj.score),
              fill='white', font=font)


labels = load_labels(labels_file)
interpreter = make_interpreter(model)
interpreter.allocate_tensors()

def run(input_image):

  image = Image.open(input_image)
  scale = detect.set_input(interpreter, image.size,
                           lambda size: image.resize(size, Image.ANTIALIAS))
  
  interpreter.invoke()
  objs = detect.get_output(interpreter, threshold, scale)

  if not objs:
    print('No objects detected')

  for obj in objs:
    print(labels.get(obj.id, obj.id))
    print('  id:    ', obj.id)
    print('  score: ', obj.score)
    print('  bbox:  ', obj.bbox)

  image = image.convert('RGB')
  draw_objects(ImageDraw.Draw(image), objs, labels)
  image_byte_array = io.BytesIO()
  image.save(image_byte_array, format='JPEG') 
  image_byte_array.seek(0)
  return image_byte_array.read()

