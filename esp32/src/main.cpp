#include <Arduino.h>

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#include <Wire.h>
#include <Adafruit_MotorShield_MOD_CB.h>
#include <Adafruit_MS_PWMServoDriver.h>

// Create the motor shield object 
Adafruit_MotorShield AFMS = Adafruit_MotorShield(0x40); 

// Select which 'port' M1, M2, M3 or M4. In this case, M1
Adafruit_DCMotor *rightMotor = AFMS.getMotor(1);
// Make another motor on port M2
Adafruit_DCMotor *leftMotor = AFMS.getMotor(2);


#define MAX_SPEED 255

//  main speed
uint8_t mSpeed = MAX_SPEED;

// low speed for turning etc
uint8_t lSpeed = 90;


Adafruit_MS_PWMServoDriver pwm = Adafruit_MS_PWMServoDriver(0x40);

#define MIN_YAW 220
#define MAX_YAW 420

#define MIN_PITCH 190
#define MAX_PITCH 450

//  main speed
volatile int yawPWM = 320;
volatile int pitchPWM = 320;

// For reporting
char writeTemplate[] =  "p:%i|y:%i|s:%i|m:%i";
volatile bool servoChanged = true;
volatile bool motorChanged = false;

BLECharacteristic *txCharacteristic;
BLECharacteristic *rxCharacteristic;

bool deviceConnected = false;
uint8_t txValue = 0;
String filterString;

#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"


volatile int nextMove = 0;

#define STOP 0
#define FORWARD 1
#define LEFT 2
#define RIGHT 3
#define BACK 4
#define SET_SPEED 5

#define RXD2 16
#define TXD2 17


void forward() {
  Serial.println("forward!");
  leftMotor->setSpeed(mSpeed);
  rightMotor->setSpeed(mSpeed);
  leftMotor->run(FORWARD);
  rightMotor->run(FORWARD);
}

void left() {
  Serial.println("left!");
  //leftMotor->setSpeed(lSpeed/2);  
  leftMotor->setSpeed(lSpeed);
  rightMotor->setSpeed(lSpeed);
  leftMotor->run(BACKWARD);
  rightMotor->run(FORWARD);
}

void right() {
  Serial.println("right!");
  leftMotor->setSpeed(lSpeed);  
  //rightMotor->setSpeed(lSpeed/2);
  rightMotor->setSpeed(lSpeed);
  leftMotor->run(FORWARD);
  rightMotor->run(BACKWARD);
}
void backward() {
  Serial.println("back!");
  leftMotor->setSpeed(mSpeed);
  rightMotor->setSpeed(mSpeed);
  leftMotor->run(BACKWARD);
  rightMotor->run(BACKWARD);
}

void stop() {
  Serial.println("stop!");
  leftMotor->run(RELEASE);
  rightMotor->run(RELEASE);
}

void yaw(int yawValue) {
  Serial.println("yaw!");
  Serial.println(yawValue);
  yawPWM = yawValue;
  servoChanged = true;
}

void pitch(int pitchValue) {
  Serial.println("pitch!");
  Serial.println(pitchValue);
  pitchPWM = pitchValue;
  servoChanged = true;
}

void setSpeed() {
  leftMotor->setSpeed(mSpeed);
  rightMotor->setSpeed(mSpeed);
}


String getValue(String data, char separator, int index)
{
    int found = 0;
    int strIndex[] = { 0, -1 };
    int maxIndex = data.length() - 1;

    for (int i = 0; i <= maxIndex && found <= index; i++) {
        if (data.charAt(i) == separator || i == maxIndex) {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i+1 : i;
        }
    }
    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void handleCommand(String filterString) {
  if (filterString.equals("forward")) {
    nextMove = FORWARD;
    motorChanged = true;
  } else if (filterString.equals("back")) {
    nextMove = BACK;
    motorChanged = true;
  } else if (filterString.equals("left")) {
    nextMove = LEFT;
    motorChanged = true;
  } else if (filterString.equals("right")) {
    nextMove = RIGHT;
    motorChanged = true;
  } else if (filterString.equals("stop")) {
    nextMove = STOP;
    motorChanged = true;
  } else if (filterString.startsWith("speed")) {
    String speedValue = getValue(filterString, ':', 1);
    int speedValueInt = speedValue.toInt();
    if (speedValueInt > 0 && speedValueInt < MAX_SPEED) {
      mSpeed = speedValueInt;  // convert string to integer
      lSpeed = mSpeed * .6;  // set low speed to 60%
      nextMove = SET_SPEED;
      motorChanged = true;
    }
    
  } else if (filterString.startsWith("yaw")) {
    String yawValue = getValue(filterString, ':', 1);
    int yawValueInt = yawValue.toInt();
    if (yawValueInt > MIN_YAW && yawValueInt < MAX_YAW) {
      yaw(yawValueInt);
    }
  } else if (filterString.startsWith("pitch")) {
    String pitchValue = getValue(filterString, ':', 1);
    int pitchValueInt = pitchValue.toInt();
    if (pitchValueInt > MIN_PITCH && pitchValueInt < MAX_PITCH) {
      pitch(pitchValueInt);
    }
  }
}

class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
  deviceConnected = true;
  };
  void onDisconnect(BLEServer* pServer) {
  deviceConnected = false;
  }
};
class Callbacks: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    std::string rxValue = pCharacteristic->getValue();
  
    filterString = rxValue.c_str(); // Convert to standard c string format
    Serial.println(filterString);
    handleCommand(filterString);
  }
};
void setup() {
  Serial.begin(9600);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);


  AFMS.begin();  // create with the default frequency 1.6KHz
  //AFMS.begin(1000);  // OR with a different frequency, say 1KHz

  // Set the speed to start, from 0 (off) to 255 (max speed)
  leftMotor->setSpeed(mSpeed);
  leftMotor->run(FORWARD);
  // turn on motor
  leftMotor->run(RELEASE);


  rightMotor->setSpeed(mSpeed);
  rightMotor->run(FORWARD);
  // turn on motor
  rightMotor->run(RELEASE);

  // Servos

  pwm.begin();
  
  pwm.setPWMFreq(60);  // Analog servos run at ~60 Hz updates

  // Create the BLE Device
  BLEDevice::init("ESP32bot");
  // Create the BLE Server
  BLEServer *server = BLEDevice::createServer();
  
  server->setCallbacks(new MyServerCallbacks());
  // Create the BLE Service
  BLEService *service = server->createService(SERVICE_UUID);

  // TX Characteristic
  txCharacteristic = service->createCharacteristic(
    CHARACTERISTIC_UUID_TX,
    BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ
  );

  txCharacteristic->addDescriptor(new BLE2902());

  // RX Characteristic
  BLECharacteristic *rxCharacteristic = service->createCharacteristic(
    CHARACTERISTIC_UUID_RX,
    BLECharacteristic::PROPERTY_WRITE
  );
  rxCharacteristic->setCallbacks(new Callbacks());
  // Start the service
  service->start();
  // Start advertising
  server->getAdvertising()->start();
  Serial.println("Waiting a client connection to notify...");
}

void writeCurrentState() {
  char writeValue[80];

  sprintf(writeValue, writeTemplate, pitchPWM, yawPWM, mSpeed, nextMove);

  txCharacteristic->setValue(writeValue);
  txCharacteristic->notify();

  Serial.println(writeValue);
  Serial2.println(writeValue);
}

void processIncomingByte (const byte inByte)  {
  static char input_line [10];
  static unsigned int input_pos = 0;

  switch (inByte) {
    case '\n':   // end of text
      input_line [input_pos] = 0;  // terminating null byte
      // terminator reached! process input_line here ...
      handleCommand(input_line);
      // reset buffer for next time
      input_pos = 0;  
      break;
    case '\r':   // discard carriage return
      break;
    default:
      // keep adding if not full ... allow for terminating null byte
      if (input_pos < (10 - 1))
        input_line [input_pos++] = inByte;
      break;
    }
} 

void loop() {

  while(Serial2.available()) {
    processIncomingByte(Serial2.read());
  }

  if (servoChanged) {
    writeCurrentState();

    pwm.setPWM(1, 0, yawPWM);
    pwm.setPWM(0, 0, pitchPWM);

    servoChanged = false;
  } else if (motorChanged) {
    writeCurrentState();
    motorChanged = false;
    switch (nextMove) {
      case STOP:
        stop();
        break;
      case FORWARD:
        forward();
        break;
      case LEFT:
        left();
        break;
      case RIGHT:
        right();
        break;
      case BACK:
        backward();
        break;
      case SET_SPEED:
        setSpeed();
        break;
    }
  }
}
