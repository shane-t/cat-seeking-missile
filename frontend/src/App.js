import React from 'react';
import logo from './logo.svg';
import './App.css';

import LidarGraph from './LidarGraph'

class App extends React.Component {


  componentDidMount() {
    this.updateState()
    document.querySelectorAll('.camera-image')[0].addEventListener('load', this.updateImage.bind(this))
    window.addEventListener("keydown", this.onKeyDown.bind(this), false);
    window.addEventListener("keyup", this.onKeyUp.bind(this), false);
    document.addEventListener('load', () => console.log('hello'))
    this.getSounds()

    window.addEventListener("gamepadconnected", this.gamepadSetup.bind(this));
  }

  gamepadSetup (e) {
    console.log("Gamepad connected at index %d: %s. %d buttons, %d axes.",
    e.gamepad.index, e.gamepad.id,
    e.gamepad.buttons.length, e.gamepad.axes.length);
    
    window.requestAnimationFrame(this.pollGamepad.bind(this))
  }

  count = 0

  lawyaw = 0
  lastpitch = 0

  map( x,  in_min,  in_max,  out_min,  out_max){
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
  }

  pollGamepad () {
    window.requestAnimationFrame(this.pollGamepad.bind(this))

    const gamepad = navigator.getGamepads()[0]
    try {

      // Don't always poll buttons
      if (this.count > 5) {
        this.count = 0
      } else {
        this.count++;
        return
      }


      const { axes } = gamepad;

      const maxyaw = 410
      const midyaw = 320
      const minyaw = 230  

      const minpitch = 440;
      const midpitch = 320;
      const maxpitch = 210;

      let yaw = 0

      if (axes[2] > 0.01) {
        yaw = this.map(axes[2], 0, 1, midyaw, minyaw)
      }

      if (axes[2] < -0.01) {
        yaw = this.map(Math.abs(axes[2]), 0, 1, midyaw, maxyaw)
      }

      if (yaw && Math.floor(yaw) !== this.lastyaw) {
        this.send('yaw', yaw)
        this.lastyaw = Math.floor(yaw)
      }
      
      let pitch = 0

      if (axes[3] > 0.01) {
        pitch = this.map(axes[3], 0, 1, midpitch, minpitch)
      }

      if (axes[3] < -0.01) {
        pitch = this.map(Math.abs(axes[3]), 0, 1, midpitch, maxpitch)
      }

      if (pitch && Math.floor(pitch) !== this.lastpitch) {
        this.send('pitch', pitch)
        this.lastpitch = Math.floor(pitch)
      }

      const buttonState = gamepad.buttons.map(button => button.pressed)
      
      let stop = true;

      for (let i = 0; i < gamepad.buttons.length; i++) {
        if(gamepad.buttons[i].pressed) {
          // 12 is forward
          // 14 left
          // 15 right
          // 13 back
          console.log(i)

          switch(i) {
            case 12:
              stop = false
              this.stopCount = 0
              this.forward();
              break;
            case 15:
              stop = false
              this.stopCount = 0
              this.right();
              break;
            case 13:
              stop = false
              this.stopCount = 0
              this.back();
              break;
            case 14:
              stop = false
              this.stopCount = 0
              this.left();
              break;
            case 0:
              fetch('/playsound?wav=squidgy-2.wav')
              this.count = -15
              break;
            case 1:
              fetch('/playsound?wav=eris-2.wav')
              this.count = -15
              break;
            case 7:
              this.centreCamera();
              break
            case 3:
              this.lightsOn()
              break
            case 2:
              this.lightsOff()
              break
        }
        }

      }
      if (stop && this.stopCount < 5) {
        this.stop()
        this.stopCount++
      }
    } catch(e) {
      
    }

  }

  stopCount = 0

  // Key trickery
  releaseTimes = {

  }

  pressed = {

  }

  MAX_KEY_DELAY = 100

  MAX_FPS = 10


  state = {
    yaw : 0,
    pitch: 0,
    speed: 0,
    move: 0,
    pitchInterval : 10,
    yawInterval : 10,
    speedInterval: 10,
    timestamp : new Date().getTime(),
    sound : {
      files : [],
      current : null
    },
    cameraPath : 'camera'
    
  }

  async updateImage() {

    await new Promise((resolve, reject) => {
      setTimeout(resolve, 1000 / this.MAX_FPS)
    })

    this.setState({ timestamp : new Date().getTime() })
    
  }

  setNormalCamera() {
    this.setState({cameraPath : 'camera'})
  }

  setDetectingCamera() {
    this.setState({cameraPath : 'camera_detect'})
  }

  centreCamera() {
    this.send('yaw', 320)
    this.send('pitch', 320)
  }

  async updateState() {
    const state = await (await fetch('/state')).json()

    this.setState(state)
  }

  wait(ms) {
    return m => new Promise(r => setTimeout(r, ms))
  }

  async send(action, parameter) {
    console.log(`Sending ${action} : ${parameter}`)
    await fetch(`/${action}?value=${parameter}`)
    setTimeout(this.updateState.bind(this), 100)
  }

  yawLeft(value) {
    const newYaw = this.state.yaw + (parseInt(value) || this.state.yawInterval)
    this.send('yaw', newYaw)
  }

  yawRight(value = this.state.yawInterval) {
    const newYaw = this.state.yaw - (parseInt(value) || this.state.yawInterval)
    this.send('yaw', newYaw)
  }

  pitchUp(value = this.state.pitchInterval) {
    const newPitch = this.state.pitch - (parseInt(value) || this.state.pitchInterval)
    this.send('pitch', newPitch)
  }

  pitchDown(value = this.state.pitchInterval) {
    const newPitch = this.state.pitch + (parseInt(value) || this.state.pitchInterval)
    this.send('pitch', newPitch)
  }

  speedUp(value = this.state.speedInterval) {
    console.log(value)
    const newSpeed = this.state.speed + (parseInt(value) || this.state.speedInterval)
    this.send('speed', newSpeed)
  }

  speedDown(value = this.state.speedInterval) {
    const newSpeed = this.state.speed - (parseInt(value) || this.state.speedInterval)
    this.send('speed', newSpeed)
  }

  forward() {
    this.send('forward')
  }

  back() {
    this.send('back')
  }

  left() {
    this.send('left')
  }

  right() {
    this.send('right')
  }

  stop() {
    this.send('stop')
  }

  stopLidar() {
    this.setState({ lidarStarted : false })
    this.send('lidarstop')
  }

  startLidar() {
    this.setState({ lidarStarted : true })
    this.send('lidarstart')
  }

  async getSounds() {
    const sounds = await fetch('/sounds').then(r => r.json())
    this.setState({
      sound: {
        ...this.state.sound,
        files : sounds
      }
    })
  }

  async playSound() {
    if (this.state.sound.current == null) {
      return
    }
    await fetch('playsound?wav=' + this.state.sound.current)
  }

  lightsOn() {
    return fetch('/lights/white')
  }

  lightsOff() {
    return fetch('/lights/off')
  }

  laserOn() {
    return fetch('/laser/on')
  }

  laserOff() {
    return fetch('/laser/off')
  }

  selectSound(e) {
    this.setState({
      sound: {
        ...this.state.sound,
        current : e.target.value
      }
    })
  }

  onKeyDown(event) {
    
    const {  keyCode } = event

    const time = new Date().getTime()

    if (this.releaseTimes[keyCode] && time < this.releaseTimes[keyCode] + this.MAX_KEY_DELAY) {
      console.log('Ignored keyup')
      return false
    }

    switch (keyCode) {
      case 68: //d
        this.right()
        break;
      case 83: //s
        this.back()
        break;
      case 65: //a
        this.left()
        break;
      case 87: //w
        this.forward()
        break;
      case 81: //q
        this.yawLeft(5)
        break;
      case 69: //e
        this.yawRight(5)
        break;
      case 90: //z
        this.pitchDown(5)
        break;
      case 88: //x
        this.pitchUp(5)
        break;
    }
  }
  
  onKeyUp(event) {
    const { keyCode } = event

    this.releaseTimes[keyCode] = new Date().getTime()
    delete this.pressed[keyCode]

    switch (keyCode) {
      case 68: //d
        this.stop()
        break;
      case 83: //s
        this.stop()
        break;
      case 65: //a
        this.stop()
        break;
      case 87: //w
        this.stop()
        break;
    }
  }

  render() {
  return (
    <div className="App">
      <div class="lidar">
        <LidarGraph yaw={this.state.yaw} started={this.state.lidarStarted} />
      </div>
      <div class="camera">
        <img  className='camera-image' src={ `/${this.state.cameraPath}?time=${this.state.timestamp}`} />
      </div>
      <button disabled >yaw: { this.state.yaw } </button> | 
      <button disabled>pitch: { this.state.pitch }</button>
      <button disabled>speed: { this.state.speed }</button> | 
      <button disabled>move: {this.state.move } </button>
      <table className='controls'>
        <tr>
          <td><button onClick={this.yawLeft.bind(this)}>yaw left+</button></td>
          <td><button onClick={this.forward.bind(this)}>⬆️</button></td>
          <td><button onClick={this.yawRight.bind(this)}>yaw right-</button></td>
        </tr>
        <tr>
          <td><button onClick={this.left.bind(this)}>⬅️</button></td>
          <td><button onClick={this.stop.bind(this)}>🛑</button></td>
          <td><button onClick={this.right.bind(this)}>➡</button></td>
        </tr>
        <tr>
          <td><button onClick={this.pitchUp.bind(this)}>pitch up-</button></td>
          <td><button onClick={this.back.bind(this)}>⬇️</button></td>
          <td><button onClick={this.pitchDown.bind(this)}>pitch down+</button></td>
        </tr>
        <tr>
          <td><button onClick={this.speedDown.bind(this)}>speed -</button></td>
          <td>
            <button  className="half" onClick={this.startLidar.bind(this)}>Lidar On</button>
            <button  className="half" onClick={this.stopLidar.bind(this)}>Lidar Off</button>
          </td>
          <td><button onClick={this.speedUp.bind(this)}>speed+</button></td>
        </tr>
        <tr>
          <td>
            <div className="action half" >
              Select
            <select onChange={this.selectSound.bind(this)}>

            <option value={null}></option>
              {
                this.state.sound.files.map(sound => <option value={sound}>{sound}</option>)
              }
            </select>
            </div>
            <button className="half" onClick={this.playSound.bind(this)}>
            Play
          </button>

          </td>
          <td>

          </td>
          <td>
          <button className="half"  onClick={this.lightsOn.bind(this)}>
            Lights On
          </button>
          <button className="half" onClick={this.lightsOff.bind(this)}>
            Lights Off
          </button>
          </td>
        </tr>
        <tr>
          <td>
          <button className="half"  onClick={this.setNormalCamera.bind(this)}>
            Normal
          </button>
          <button className="half" onClick={this.setDetectingCamera.bind(this)}>
            Detection
          </button>
          </td>
            <td><button onClick={this.centreCamera.bind(this)}>Centre</button></td>
          <td>
          <button className="half"  onClick={this.laserOn.bind(this)}>
            Laser On
          </button>
          <button className="half" onClick={this.laserOff.bind(this)}>
            Laser Off
          </button>
          </td>
        </tr>
      </table>
    </div>

  );
  }
}

export default App;
