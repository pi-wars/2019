// PiWars BadgeOS
// 5x5 LED array
// 4 buttons
// Light Sensor
// External connections
//
//////////////////////////////////////////////////
//
// PiWars19 Badge Control Software
//
// 4tronix 2018
//
//////////////////////////////////////////////////

/*
 BadgeOS provides a few functions to make the continuously updated LED display on the badge work without the user having to worry overmuch about it
 The main arduino loop updates the display regularly, checks for button presses and calls the user's code when appropriate
 Users should place all their code in the area provided and use the functions provided.
 It is encouraged to add new characters at the end of the character generator - don't forget to increase the charEnd variable appropriately
*/

#include "FastLED.h"

#define DEBUG         false
//#define REV4

// LED Display 
#define NUM_ROWS  5
#define NUM_COLS  5

#ifdef REV4
byte rows[NUM_ROWS] = {2, 3, 4, 5, 6};  // Rows are active HIGH
byte cols[NUM_COLS] = {11, 10, 9, 8, 7};  // Cols are active LOW
#else
byte rows[NUM_ROWS] = {2, 3, 4, 5, 6};  // Rows are active HIGH
byte cols[NUM_COLS] = {13, 12, 9, 8, 7};  // Cols are active LOW
#endif
bool matrix[NUM_COLS][NUM_ROWS];  // this is mapped to the LEDs for displaying

// Display scanning definitions
int idx=0;
int scan=0;   // Variable to count scan rate for each character ie. time between columns being activated
#define SCANRATE 50 // times round the loop before changing displayed column

// Buttons and Light Sensor
#define NUM_BUTTONS  4 // number of Button inputs
#ifdef REV4
byte buttons[NUM_BUTTONS] = {A0, A1, A2, A3};
#else
byte buttons[NUM_BUTTONS] = {A0, A1, A2, A6}; // Button 3 is analog read only...
#endif
bool lastButton[NUM_BUTTONS] = {false, false, false, false};
#define LIGHTSENSOR A7


// Character generator. First byte is column0 (left of character), then column1
// bit 0 is Row 0 (bottom of character)
byte charStart = 0x2e;
byte chargen[][5] = {
{0x00,0x00,0x00,0x00,0x00},  // space
{0x1e,0x0e,0x1f,0x0e,0x1e},  // 2f  //
{0x0e,0x11,0x11,0x0e,0x00},  // 0
{0x00,0x12,0x1f,0x10,0x00},  // 1
{0x12,0x19,0x15,0x12,0x00},  // 2
{0x11,0x15,0x15,0x0a,0x00},  // 3
//{0x0f,0x08,0x1c,0x08,0x00},  // 4 alternative font
{0x07,0x04,0x1f,0x04,0x00},  // 4
{0x17,0x15,0x15,0x09,0x00},  // 5
{0x0e,0x15,0x15,0x08,0x00},  // 6
{0x11,0x09,0x05,0x03,0x00},  // 7
{0x0a,0x15,0x15,0x0a,0x00},  // 8
{0x02,0x15,0x15,0x0e,0x00},  // 9
{0x0e,0x11,0x11,0x0e,0x00},  // 0x3a
{0x0e,0x11,0x11,0x0e,0x00},  // 0x3b
{0x0e,0x11,0x11,0x0e,0x00},  // 0x3c
{0x0e,0x11,0x11,0x0e,0x00},  // 0x3d
{0x0e,0x11,0x11,0x0e,0x00},  // 0x3e
{0x0e,0x11,0x11,0x0e,0x00},  // 0x3f
{0x0e,0x11,0x17,0x17,0x02},  // @
{0x1e,0x05,0x05,0x1e,0x00},  // A
{0x1f,0x15,0x15,0x0a,0x00},  // B
{0x0e,0x11,0x11,0x11,0x00},  // C
{0x1f,0x11,0x11,0x0e,0x00},  // D
{0x1f,0x15,0x15,0x11,0x00},  // E
{0x1f,0x05,0x05,0x01,0x00},  // F
{0x0e,0x11,0x15,0x1d,0x00},  // G
{0x1f,0x04,0x04,0x1f,0x00},  // H
{0x00,0x11,0x1f,0x11,0x00},  // I
{0x08,0x11,0x1f,0x01,0x00},  // J
{0x1f,0x04,0x0a,0x11,0x00},  // K
{0x1f,0x10,0x10,0x10,0x00},  // L
{0x1f,0x02,0x04,0x02,0x1f},  // M
{0x1f,0x02,0x04,0x08,0x1f},  // N
{0x0e,0x11,0x11,0x0e,0x00},  // O
{0x1f,0x05,0x05,0x02,0x00},  // P
{0x0e,0x11,0x19,0x1e,0x00},  // Q
{0x1f,0x05,0x0d,0x12,0x00},  // R
{0x12,0x15,0x15,0x09,0x00},  // S
{0x00,0x01,0x1f,0x01,0x00},  // T
{0x0f,0x10,0x10,0x0f,0x00},  // U
{0x07,0x08,0x10,0x0f,0x00},  // V
{0x1f,0x08,0x04,0x08,0x1f},  // W
{0x11,0x0a,0x04,0x0a,0x11},  // X
{0x01,0x02,0x1c,0x02,0x01},  // Y
{0x19,0x15,0x15,0x13,0x00},  // Z
{0x0e,0x11,0x15,0x11,0x0e},  // 0x5B  // user character for static display
{0x1f,0x11,0x11,0x11,0x1f},  // 0x5C  // 3 user characters for animation
{0x00,0x0e,0x0A,0x0e,0x00},  // 0x5D
{0x00,0x00,0x04,0x00,0x00},  // 0x5E
};
byte charEnd = 0x5E;

// ======== USER CODE =============================================
//
// Put your user code here. The main Arduino setup() and loop() functions handle the 5x5 display, buttons and sensors
//   progSetup() is called once at the start of the program, at the end of the Ardino setup() function, so all I/O is already setup for you
//   progMain() is called every SCANRATE times around the main Arduino loop(). Use this to handle any dynamic functions like scrolling or animation
//   evButtons(int ID, bool state) is called when any button changes state. ID contains the button number (0..3) and state contains a boolean. true for pressed, false for released
//
// There various support functions that you can call to help develop animations:
//   copyChar(character) copies the data for that chareacter from the character generator to the display matrix. eg. copyChar('A')
//   copy2Chars(character1, character2, offset) copies a window on the 2 characters next to each other, starting at offset and finishing after 5 columns. This allows scrolling. eg. cop2Chars('A', 'B', 3) will display the last 2 columns of 'A' and the first 3 of 'B'
//   bool readButton(ID) will read the current state of button ID. eg. readButton(2) will return true if button 2 is currently pressed, else false
//

// ----------------------------------------
// User variables and defines
byte mode; // Defines the display mode: Static, Scrolling characters, animation, dynamic
int cycle=0;  // Variable to count rate of scrolling
#define CYCLERATE 300
#define SCROLLRATE 100
#define ANIRATE 100
#define PIXELS 1
#define NUM_LEDS 14
#define BRIGHT 80
CRGB leds[NUM_LEDS];
String text1 = "0123456789";
String text2 = " PiWars 2019 ";
int charIDX=0;
int offset=0; // offset from start of character. Used in scrolling

int L1 = A3, L2 = 10; // Left Motor Drive pins
int R1 = 11, R2 = A4; // Right Motor Drive pins
int mCount=0;
int mState=0;

// ----------------------------------------

// progSetup is called once at the start of the program, after all I/O etc has been setup and initialisation is complete
void progSetup()
{
  evButtons(0, true); // setup initial mode
  FastLED.addLeds<WS2812B, PIXELS, RGB>(leds, NUM_LEDS);  // always have WS2812B enabled on output 5
  FastLED.setBrightness(BRIGHT);  // sets the maximum brightness level. All values are scaled to fit in this range
  rainbow();
  pinMode(L1, OUTPUT);     
  pinMode(L2, OUTPUT);     
  pinMode(R1, OUTPUT);     
  pinMode(R2, OUTPUT);
}

// progMain is called each time column address is changed in display
void progMain()
{
  switch (mode)
  {
    case 0: // scrolls each character in string
      if (++cycle >= SCROLLRATE)
      {
        int IDX2;
        cycle = 0;
        if (++offset >= NUM_COLS)
        {
          offset = 0;
          charIDX = ++charIDX % text2.length();
        }
        IDX2 = (charIDX+1) % text2.length();
        osCopy2Chars(text2[charIDX], text2[IDX2], offset);
      }
      break;
    case 1: // prints each character in string one at a time
      if (++cycle >= CYCLERATE)
      {
        cycle = 0;
        charIDX = ++charIDX % text1.length();
        osCopyChar(text1[charIDX]);
      }
      break;
    case 2: // read and print light sensor
      if (++cycle > ANIRATE)
      {
        cycle = 0;
        int i = osReadLight();
        osCopyChar(i / 100 + '0');
        if (DEBUG)
          Serial.println(i);
      }
      break;
    case 3: // random animation - done in same way as mode 1 
      if (++cycle >= ANIRATE)
      {
        if (++charIDX > 0x5E)
          charIDX = 0x5C;
        cycle = 0;
        osCopyChar(charIDX);
        rotateLEDs(0);
        if(++mCount > 20)
        {
          mCount = 0;
          mState = (mState+1) % 5;
          switch (mState)
          {
            case 0: motorStop(); break;
            case 1: forward(); break;
            case 2: reverse(); break;
            case 3: spinLeft(); break;
            case 4: spinRight(); break;
          }
        }
      }
      break;
  }
}

// evButtons is called every time a button changes state
void evButtons(int ID, bool state)
{
  if (state)
  {
    mode = ID;
    switch (mode)
    {
      case 0: osCopyChar(0x5B); break;
      case 1: charIDX = 0; break;
      case 2: charIDX = 0; break;
      case 3: charIDX = 0x5C; break;
    }
    motorStop(); // always stop motors when any button pressed
  }
}


// User Helper Functions for Roundabot
// Motors
// Neopixels

// Set single LED value
void setLED(int ID, int red, int green, int blue)
{
  leds[ID] = CRGB(green,red,blue);
  FastLED.show();
}

// Turns all the LEDs to OFF
void allOff()
{
  for (int i=0; i<NUM_LEDS; i++)
    leds[i] = 0;
  FastLED.show();
}

// Sets all the LEDs to the same colour
void setAll(int red, int green, int blue)
{
  for (int i=0; i<NUM_LEDS; i++)
  {
    leds[i].g = red;
    leds[i].r = green;
    leds[i].b = blue;
  }
  FastLED.show();
}

// Set rainbow colour scheme on LEDs
void rainbow()
{
  int pos;
  for (int i=0; i<NUM_LEDS; i++)
  {
    pos = i;
    if (i<5)
      leds[i] = CRGB(pos*51, 255 - pos*51, 0);
    else if (i<10)
    {
      pos -= 5;
      leds[i] = CRGB(255 - pos*51, 0, pos*51);
    }
    else
    {
      pos -= 10;
      leds[i] = CRGB(0, pos*51, 255 - pos*51);
    }
  }
  FastLED.show();
}


// Rotates LEDs 0=clockwise, 1=anticlockwise
void rotateLEDs(int direction)
{
  CRGB t;
  if (direction)
  {
    t = leds[0];
    for (int i=0; i<NUM_LEDS-1; i++)
      leds[i] = leds[i+1];
    leds[13] = t;
  }
  else
  {
    t = leds[13];
    for (int i=NUM_LEDS-1; i>0; i--)
      leds[i] = leds[i-1];
    leds[0] = t;
  }
  FastLED.show();
}

// Stop motors
void motorStop()
{
  digitalWrite(L1, 0);
  digitalWrite(L2, 0);
  digitalWrite(R1, 0);
  digitalWrite(R2, 0);
}

// Motors Forward
void forward()
{
  digitalWrite(L1, 1);
  digitalWrite(L2, 0);
  digitalWrite(R1, 1);
  digitalWrite(R2, 0);
}

// Motors Reverse
void reverse()
{
  digitalWrite(L1, 0);
  digitalWrite(L2, 1);
  digitalWrite(R1, 0);
  digitalWrite(R2, 1);
}

// Spin Left
void spinLeft()
{
  digitalWrite(L1, 0);
  digitalWrite(L2, 1);
  digitalWrite(R1, 1);
  digitalWrite(R2, 0);
}

// Spin Right
void spinRight()
{
  digitalWrite(L1, 1);
  digitalWrite(L2, 0);
  digitalWrite(R1, 0);
  digitalWrite(R2, 1);
}



// ======== END OF USER CODE ======================================
// You shouldn't have to touch any code from here on


// ======== BadgeOS ===============================================

int osReadLight()
{
  return analogRead(LIGHTSENSOR);
}

void osCopyChar (char myChar)
{
  myChar = toupper(myChar);
  if (myChar>= charStart and myChar<= charEnd)
    myChar -= charStart;
  else
    myChar = 0;
  for (int i=0; i<NUM_COLS; i++)
    for (int j=0; j<NUM_ROWS; j++)
    {
      matrix[i][j] = bitRead(chargen[myChar][i], j);
    }
}

void osCopy2Chars (char myChar1, char myChar2, int offset)
{
  myChar1 = toupper(myChar1);
  if (myChar1>= charStart and myChar1<= charEnd)
    myChar1 -= charStart;
  else
    myChar1 = 0;
  myChar2 = toupper(myChar2);
  if (myChar2>= charStart and myChar2<= charEnd)
    myChar2 -= charStart;
  else
    myChar2 = 0;
  for (int i=0; i<NUM_COLS; i++)
    for (int j=0; j<NUM_ROWS; j++)
    {
      if ((i+offset) < NUM_COLS)
        matrix[i][j] = bitRead(chargen[myChar1][i+offset], j);
      else
        matrix[i][j] = bitRead(chargen[myChar2][i+offset-NUM_COLS], j);
    }
}

bool osReadButton (int ID)
{
  if (buttons[ID] == A6)
    return (analogRead(buttons[ID]) < 512);
  else
    return !digitalRead(buttons[ID]);
}

void setup()
{
  for (int i=0; i<NUM_ROWS; i++)
  {
    pinMode(rows[i], OUTPUT);
    digitalWrite(rows[i], LOW); // disable all rows
  }
  for (int i=0; i<NUM_COLS; i++)
  {
    pinMode(cols[i], OUTPUT);
    digitalWrite(cols[i], HIGH);  // disable all columns
  }
  for (int i=0; i<NUM_BUTTONS; i++)
    pinMode(buttons[i], INPUT_PULLUP);
  if (DEBUG)
  {
    Serial.begin(115200);
    Serial.println("Starting...");
    delay(1000);
  }
  osCopyChar(0x2f);
  progSetup();  // call user's setup function
}

// The main loop continuously displays whatever is in the 5x5 buffer "screen"
// Every 50 (TBC) times around the loop, buttons and sensor(s) are checked and relevant flags or data stored
// After checking buttons and sensors the "progMain" function is called to handle things like changing or scrolling characters, reacting to buttons etc.
// The progMain function is the main user function in which the user's work is done. Try to consume as little time as possible in the function or you may see the display start to flicker
void loop()
{
  bool button;
  delayMicroseconds(10);
  if (++scan > 99)  // change column every hundred loops
  {
    digitalWrite(cols[idx], HIGH);  // disable whatever column we're on right now
    idx = (++idx) % NUM_COLS;
    setCol(idx);
    progMain(); // call user's main function
    scan = 0;
    // check for button changes
    for (int i=0; i<NUM_BUTTONS; i++)
    {
      button = osReadButton(i);
      if (button != lastButton[i])
      {
        lastButton[i] = button;
        evButtons(i, button);
      }
    }
  }
}

void setCol(int col)
{
  digitalWrite(cols[col], HIGH);  // switch off the column, then change the data
  for (int i=0; i<NUM_ROWS; i++)
    digitalWrite(rows[i], matrix[col][i]);
  digitalWrite(cols[col], LOW);   // switch ON the column  
}

void flashCol1()
{
  digitalWrite(cols[0], HIGH);  // switch off the column
  digitalWrite(rows[0], HIGH);  // change the data
  digitalWrite(rows[1], LOW);
  digitalWrite(rows[2], HIGH);
  digitalWrite(rows[3], LOW);
  digitalWrite(rows[4], HIGH);
  digitalWrite(cols[0], LOW);   // switch ON the column
  delay(1000);
  digitalWrite(cols[0], HIGH);  // switch off the column
  digitalWrite(rows[0], LOW);  // change the data
  digitalWrite(rows[1], HIGH);
  digitalWrite(rows[2], LOW);
  digitalWrite(rows[3], HIGH);
  digitalWrite(rows[4], LOW);
  digitalWrite(cols[0], LOW);   // switch ON the column
  delay(1000);
}
