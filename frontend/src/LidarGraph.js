import React from 'react';


export default class LidarGraph extends React.PureComponent {
  
    state = {
      offsets : []
    }

    constructor(props) {
      super(props)
      this.canvas = React.createRef()
    }

    componentDidMount() {
      this.ctx = this.canvas.current.getContext('2d')
      setInterval(this.getData.bind(this), 100)
    }
    
    // componentWillUnmount() {
    //   Chart.destroy(this.el);
    // }

    async getData() {
      if (this.props.started) {
        const data = await fetch('/lidardata')
        .then(r => r.json ())

        this.setState({...data})
      }
    }
    map( x,  in_min,  in_max,  out_min,  out_max){
      return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    }
    componentDidUpdate(args) {

      const centerX = this.canvas.current.width/2;
      const centerY = this.canvas.current.height/2;
      const cameraDistance = 200
      const raydistance = 500
      const multiplier = 0.1
      const fieldOfView = 1.1
      const centreAngle = Math.PI
      const extremeLeft = centreAngle - 0.2
      const extremeRight = centreAngle + 0.1

      const maxleftYaw = 415
      const centreYaw = 320
      const maxRightYaw = 230

      let cameraAngle = Math.PI
      if (this.props.yaw >= 320) {
        cameraAngle = this.map(this.props.yaw, centreYaw, maxleftYaw, cameraAngle, (extremeLeft - fieldOfView/2))
      } else {
        cameraAngle = this.map(this.props.yaw, maxRightYaw, centreYaw, (extremeRight + fieldOfView/2), cameraAngle)
      }

      // reset
      this.ctx.fillStyle = '#000'
      this.ctx.fillRect(0, 0, this.canvas.current.width, this.canvas.current.height)

      // Center
      this.ctx.fillStyle = '#FF0000'
      this.ctx.fillRect(centerX-4, centerY-4, 16, 16)


      this.ctx.beginPath()
      this.ctx.moveTo(centerX - (cameraDistance * multiplier), centerY) // Go to camera location
      const leftRayAngle = cameraAngle - fieldOfView / 2
      this.ctx.lineTo((centerX - (cameraDistance * multiplier)) + (raydistance * Math.cos(leftRayAngle)), centerY + raydistance * Math.sin(leftRayAngle))
      //this.ctx.lineTo(centerX , centerY )
      this.ctx.strokeStyle = '#00FF00'
      this.ctx.stroke()


      this.ctx.beginPath()
      this.ctx.moveTo(centerX - (cameraDistance * multiplier), centerY) // Go to camera location
      const rightRayAngle = cameraAngle + fieldOfView / 2
      this.ctx.lineTo((centerX - (cameraDistance * multiplier)) + (raydistance * Math.cos(rightRayAngle)), centerY + raydistance * Math.sin(rightRayAngle))
      //this.ctx.lineTo(centerX , centerY )
      this.ctx.strokeStyle = '#00FF00'
      this.ctx.stroke()
      

      for (const angle of this.state.offsets) {
        this.ctx.fillStyle = '#FFF'
        const theta =  angle[0]
        const distance = angle[1] * multiplier

        const x = centerX + distance * Math.cos(theta)
        const y = centerY + distance * Math.sin(theta)

        this.ctx.fillRect(x,y, 2, 2)
      }

      // const theta3 = 1
      // const xDot3 = centerX + (100  * Math.cos(theta3))
      // const yDot3 = centerY + (100 * Math.sin(theta3))
      // this.ctx.fillRect(xDot3, yDot3, 2, 2)
    }
  
    //  update(args) {
    //   console.log(args)
    // }

    render() {
        const style = { transform: "rotate(90deg)"}
        //const style = {}
        return <canvas style={style} width={600} height={600} ref={this.canvas}></canvas>
    }
}