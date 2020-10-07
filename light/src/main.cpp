#include <Adafruit_NeoPixel.h>
#include <Arduino.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define NEOPIXEL_PIN D3
#define LASER_PIN D1


#define NUM_LEDS 8


Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

void setup() {

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LASER_PIN, OUTPUT);
  Serial.begin(9600);
  // This is for Trinket 5V 16MHz, you can remove these three lines if you are not using a Trinket
  #if defined (__AVR_ATtiny85__)
    if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
  #endif
  // End of trinket special code

  strip.begin();
  strip.setBrightness(50);
  strip.show(); // Initialize all pixels to 'off'

  digitalWrite(LED_BUILTIN, HIGH); // Turn LED off

}

void off() {
  Serial.println("Off");
  strip.fill(strip.Color(0,0,0));
  strip.show();
}

void white() {
  Serial.println("White");
  strip.fill(strip.Color(255,255,255));
  strip.show();
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
  //Check if string starts with number (for LED number)

  String ledNumber = getValue(filterString, ':', 0);
  int ledNumberInt = ledNumber.toInt();

  // To cheat we send "1" for 0

  if (ledNumberInt > 0 && ledNumberInt <= NUM_LEDS) {
    int realLedNumber = ledNumberInt - 1;
    String hexColor = getValue(filterString, ':', 1);
    if (hexColor.length() > 0) {
      long number = (long) strtol( &hexColor[0], NULL, 16);
      int r = number >> 16;
      int g = number >> 8 & 0xFF;
      int b = number & 0xFF;
      Serial.println(r);
      Serial.println(g);
      Serial.println(b);

      strip.setPixelColor(realLedNumber, strip.Color(r, g, b));
      strip.show();
    }
  } else {
    if (filterString.equals("white")) {
      white();
    } else if (filterString.equals("off")) {
      off();
    } else if (filterString.equals("rainbow")) {
      rainbowCycle(5);
      off();
    } else if (filterString.startsWith("bri")) {
      Serial.println("Brightness");
      String brightnessValue = getValue(filterString, ':', 1);
      int brightnessValueInt = brightnessValue.toInt();
      if (brightnessValueInt > 0 && brightnessValueInt < 255) {
        Serial.println(brightnessValueInt);
        strip.setBrightness(brightnessValueInt);
        strip.show();
      }
    } else if (filterString.equals("led:on")) {
      digitalWrite(LED_BUILTIN, LOW);
    } else if (filterString.equals("led:off")) {
      digitalWrite(LED_BUILTIN, HIGH);
    }else if (filterString.equals("laser:on")) {
      digitalWrite(LASER_PIN, HIGH);
    } else if (filterString.equals("laser:off")) {
      digitalWrite(LASER_PIN, LOW);
    }
  }
}

char input;

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
  while(Serial.available()) {
    processIncomingByte(Serial.read());
  }
}

