import os
import threading
import queue

sound_file_dir = 'sound'

class Sound(threading.Thread):

  def __init__(self, q, *args, **kwargs):
    self.q = q
    super().__init__(*args, **kwargs)

  def listfiles(self):
    return os.listdir(sound_file_dir)

  def play(self, file_name):
    os.system('aplay ' + os.path.join(sound_file_dir, file_name))

  def run(self):
    print('Audio thread running')
    while True:
      try:
        print(self.q)
        work = self.q.get()  # 3s timeout
      
      except queue.Empty:
          return

      self.play(work)
      self.q.task_done()
  
  
