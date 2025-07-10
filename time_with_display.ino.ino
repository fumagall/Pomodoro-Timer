/*********************************************************************
This is an example sketch for our Monochrome Nokia 5110 LCD Displays

  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/products/338

These displays use SPI to communicate, 4 or 5 pins are required to
interface

Adafruit invests time and resources providing this open source code,
please support Adafruit and open-source hardware by purchasing
products from Adafruit!

Written by Limor Fried/Ladyada  for Adafruit Industries.
BSD license, check license.txt for more information
All text above, and the splash screen must be included in any redistribution
*********************************************************************/

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>

#include <Pushbutton.h>

const int buttonPin = 8;
const int lightPin = 7;

Pushbutton MyButton(buttonPin);
unsigned long doublePressThreshold = 400;  // Threshold for double press in milliseconds (500ms)


// Software SPI (slower updates, more flexible pin options):
// pin 7 - Serial clock out (SCLK)
// pin 6 - Serial data out (DIN)
// pin 5 - Data/Command select (D/C)
// pin 4 - LCD chip select (CS)
// pin 3 - LCD reset (RST)
Adafruit_PCD8544 display = Adafruit_PCD8544(6, 5, 4, 3, 2);

// Hardware SPI (faster, but must use certain hardware pins):
// SCK is LCD serial clock (SCLK) - this is pin 13 on Arduino Uno
// MOSI is LCD DIN - this is pin 11 on an Arduino Uno
// pin 5 - Data/Command select (D/C)
// pin 4 - LCD chip select (CS)
// pin 3 - LCD reset (RST)
// Adafruit_PCD8544 display = Adafruit_PCD8544(5, 4, 3);
// Note with hardware SPI MISO and SS pins aren't used but will still be read
// and written to during SPI transfer.  Be careful sharing these pins!

#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2


#define LOGO16_GLCD_HEIGHT 16
#define LOGO16_GLCD_WIDTH  16

enum State : byte {OVERTIME, COUNTING, PAUSE};
enum Duration : byte {LONG, SHORT};

void writeString(String str) {
  display.setTextSize(3);
  display.setTextColor(BLACK);
  display.setCursor(14,14);
  uint8_t len = str.length();
  if (len > 3) {
    display.write("OVT");
  } else {
    for (uint8_t i = 0; i < 3 - len; i++) {
      str = " " + str;
    }
    for (uint8_t i=0; i < 3; i++) {
      display.write(str[i]);
    }    
  }
  display.display();
}


void setup()   {
  Serial.begin(9600);
  pinMode(lightPin, OUTPUT);
  digitalWrite(lightPin, HIGH);
Serial.println("PCD test");
  display.begin();
  // init done

  // you can change the contrast around to adapt the display
  // for the best viewing!
  display.setContrast(56);
  display.clearDisplay();   // clears the screen and buffer

  display.display(); // show splashscreen
  delay(2000);

  writeString("Hi");
  delay(1000);
}

class Counter  //ClassName (DigitalIO) is declared/created using class keyword
{
  private:
    unsigned long start_time;  //variable can only be accessed by the functions under public: specifier
    unsigned long pause_time;
    bool counting;
  public:
    Counter();
    void start();
    void stop();
    void reset();
    unsigned long get_time();
};

void Counter::reset() {
  unsigned long time = millis();
  start_time = time;
  pause_time = time;
}

Counter::Counter() {
  reset();
  counting = false;
}

void Counter::stop() {
  if (counting) {
    unsigned long time = millis();
    pause_time = time;
    counting = false;
  }
}

void Counter::start() {
  if (!counting) {
    unsigned long time = millis();
    start_time += time - pause_time;
    counting = true;
  }
}

unsigned long Counter::get_time() {
  if (counting) {
    return millis() - start_time;
  } else {
    return pause_time - start_time;
  }
}

State current_state = PAUSE;
Duration current_duration = LONG;

Counter counter;

void loop() {
  int presses = 0;
  if (MyButton.getSingleDebouncedPress()) {
    unsigned long lastPressTime = millis();
    delay(100);
    presses++;
    while (millis() - lastPressTime < doublePressThreshold) {
      if (MyButton.getSingleDebouncedPress()) {
        presses++;
        lastPressTime = millis();
        delay(100);
      }
    } 
  }

  if (presses == 1) {
    // Serial.print("Start: ");
    switch (current_state) {
      case OVERTIME:
        counter.reset();
        switch (current_duration) {
          case SHORT:
            current_duration = LONG;
            break;
          case LONG:
            current_duration = SHORT;
            break;
        }
        current_state = COUNTING;
        break;
      case COUNTING:
        counter.stop();
        current_state = PAUSE;
        break;
      case PAUSE:
        counter.start();
        current_state = COUNTING;
        break;
    }
  }

  if (presses == 2) {
    switch (current_state) {
      case OVERTIME:
        counter.reset();
        current_state = COUNTING;
        break;
      case COUNTING:
        counter.reset();
        break;
      case PAUSE:
        counter.reset();
        switch (current_duration) {
          case SHORT:
            current_duration = LONG;
            break;
          case LONG:
            current_duration = SHORT;
            break;
        }
        break;
    }
  }

  Serial.print("Time:");
  Serial.println(counter.get_time());

  long duration;

  display.clearDisplay();   // clears the screen and buffer
  if (current_duration == SHORT) {
    duration = 5*60;
  } else {
    duration = 30*60;
  }

  long time = duration - (long) counter.get_time() / 1000;
  writeString(String(time / 60));

  if (duration < counter.get_time() / 1000) {
    current_state = OVERTIME;
    if (counter.get_time() / 1000 / 1 % 2 == 0) {
      digitalWrite(lightPin, HIGH);
      display.invertDisplay(false);
    } else {
      digitalWrite(lightPin, LOW);
      display.invertDisplay(true);
    }
  } else {
    digitalWrite(lightPin, LOW);
    if (current_state == PAUSE) {
      display.invertDisplay(true);
    } else {
      display.invertDisplay(false);
    }
  }

  display.display(); // show splashscreen
  
}
