///////////////////////////////////////////////////////////////////////////////////////////
//
// Pi Wars 2019 Trophy
// Hardware: Gareth Davies, 4tronix
// Software: Michael Horne, @recantha
//    
///////////////////////////////////////////////////////////////////////////////////////////

// Include the FastLED library so we can control the LEDs
#include "FastLED.h"

// Include the OLED library so we can control the OLED
#include <U8x8lib.h>

// Set-up the OLED
U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE);

// Character buffer for the OLED
char buf[20];

// Define stuff for the LEDs
#define NUM_LEDS 20
#define DATA_PIN 2
CRGB leds[NUM_LEDS];

// Define brightness values and other variables
int maxBrightness = 20; // This can go MUCH higher for a brighter display, but watch your eyes!
int newBrightness = 10;
int wait = 200; // This is the delay in ms between LED colour changes for power up test

// Define touch buttons
#define NUM_BUTTONS 4
int buttons[NUM_BUTTONS] = {6,5,4,3};

// Define the light sensor and light level variable
#define lightSensor A7
int lightLevel=0;

// Define something to blank an OLED line out
String blankLine = "                ";

void setup()
{
  // Start up the OLED and clear it
  u8x8.begin();
  u8x8.clearDisplay();
  // You need this next line to display anything on the OLED
  u8x8.setPowerSave(0);
  // Choice of fonts - plustand... is thicker
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  //u8x8.setFont(u8x8_font_pxplustandynewtv_f);
  displayHeader();

  // Define how many LEDs we have and set initial brightness
  FastLED.addLeds<WS2812B, DATA_PIN, RGB>(leds, NUM_LEDS);
  FastLED.setBrightness(maxBrightness);  // sets the maximum brightness level. All values are scaled to fit in this range
  allOff();

  // After upload, we'll flash the LEDs different colours as a test
  setAll(255, 0, 0);
  delay(wait);
  setAll(0, 255, 0);
  delay(wait);
  setAll(0, 0, 255);
  delay(wait);
  setAll(255, 255, 255);
  delay(wait);
  allOff();

  // Initial display of "No button pressed"
  printMode("-1", "None");
}

// This function runs over and over, and is where you do the magic to light your leds.
void loop()
{
  // Read the light sensor and print out the level on the OLED
  lightLevel = analogRead(lightSensor);
  printLightLevel();

  // Work out what brightness we want based on the inverse of the light level
  newBrightness = maxBrightness - lightLevel/10;

  // Set the brightness and show the LEDs
  FastLED.setBrightness(newBrightness);
  FastLED.show();

  // If buttons 0 and 1 are pressed together...
  if (digitalRead(buttons[0])==1 && digitalRead(buttons[1])==1)
  {
    // Set all LEDs to purple
    setAll(255,0,255);
    printMode("0 and 1", "All purple");

    // This prints out the character list, but it also messes up the display. Re-instate if you'd like!
    /*
    for (int line=0; line<8; line++)
    {
      for (int i=0; i<16; i++)
      {
        buf[i] = line*16 + i;
      }
      buf[16] = 0;
      u8x8.setCursor(0,line);
      u8x8.print(buf);
    }
    */
  }
  // Other buttons pressed
  else if (digitalRead(buttons[0]) == 1)
  {
    // Clear before switching to Larson scanner
    setAll(0,0,0);
    printMode("0", "Larson");
    // Trigger the Larson scanner
    larson(255, 0, 0, 3, 75, 50);
    // Clear on exit from Larson
    setAll(0,0,0);
  }
  else if (digitalRead(buttons[1]) == 1)
  {
    // Clear before switchign to Broadway
    setAll(0,0,0);
    printMode("1", "Broadway");
    // Trigger Broadway mode
    broadway(255, 255, 255, 200);
    // Clear on exit from Broadway
    setAll(0,0,0);
  }
  else if (digitalRead(buttons[2]) == 1)
  {
    // Set all to blue
    setAll(0,0,255);
    printMode("2", "All blue");
  }
  else if (digitalRead(buttons[3]) == 1)
  {
    // Clear all before Rainbow
    setAll(0,0,0);
    printMode("3", "Rainbow");

    // Set some variables to configure the rainbow cycle
    int wait = 5;
    int dim = 3;
    printStatus("R'bow start");
    // Start rainbow
    rainbowCycle(wait, dim);
    printStatus("R'bow finish");
    // Clear on exit from rainbow
    setAll(0,0,0);
  }
  delay(200);
}

// This just prints out three lines at the top of the OLED
void displayHeader(void)
{
  u8x8.setCursor(0,0);
  u8x8.setFont(u8x8_font_pxplustandynewtv_f);
  u8x8.print("Pi Wars 2019");
  u8x8.setCursor(0,1);
  u8x8.print("Trophy");
  u8x8.setCursor(0,2);
  u8x8.print("================");
  u8x8.setFont(u8x8_font_chroma48medium8_r);
}

// Print out the light level, formatted
void printLightLevel()
{
  u8x8.setCursor(0,3);
  u8x8.print("Light: " + String(lightLevel) + "    ");
}

// Print out what button is pressed and which colour the LEDs are
void printMode(String button, String mode)
{
  // Print first status line - which button has been pressed
  u8x8.setCursor(0,4);
  u8x8.print(blankLine);
  u8x8.setCursor(0,4);
  if (button == "-1")
    u8x8.print("Button: None");
  else
    u8x8.print("Button: " + button);

  // Print second status line - which mode we are in
  u8x8.setCursor(0,5);
  u8x8.print(blankLine);
  u8x8.setCursor(0,5);
  u8x8.print("Mode: " + mode);
}

// Print a message on the 8th line
void printStatus(String msg)
{
  u8x8.setCursor(0,7);
  u8x8.print(blankLine);
  u8x8.setCursor(0,7);
  u8x8.print(msg);  
}

// Turns all the LEDs to OFF
void allOff()
{
  for (int i=0; i<NUM_LEDS; i++)
    leds[i] = 0;
  FastLED.show();
}

// Sets all the LEDs to the same colour
// This is CORRECT - red, green, blue and r,g,b do not equate! I (Mike) have no idea why!
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

void broadway(int red, int green, int blue, int speed) {
  printStatus("B'way start");
  setAll(0,0,0);

  while (1) {
    for (int j=0; j<NUM_LEDS; j=j+2) {
      // DEBUG printStatus(String(j));
      setPixel(j, red, green, blue);
    }
    FastLED.show();

    delay(speed);
    setAll(0,0,0);

    for (int j=1; j<NUM_LEDS; j=j+2) {
      // DEBUG printStatus(String(j));
      setPixel(j, red, green, blue);
    }
    FastLED.show();

    delay(speed);
    setAll(0,0,0);

    if (digitalRead(buttons[3]) == 1) {
      printStatus("B'way exit");
      return;
    }
  }
}

// Creates a Larson/Cylon scanner which scans along the top, then along the bottom, then it rebounds and reverses
void larson(int red, int green, int blue, int EyeSize, int SpeedDelay, int ReturnDelay) {
  printStatus("Larson start");

  while(1) {
    // Go one way...
    for(int i = 0; i < NUM_LEDS-EyeSize-2; i++) {
      setAll(0,0,0);
      setPixel(i, red/10, green/10, blue/10);
      for(int j = 1; j <= EyeSize; j++) {
        setPixel(i+j, red, green, blue); 
      }
      setPixel(i+EyeSize+1, red/10, green/10, blue/10);
      FastLED.show();
      delay(SpeedDelay);

      // If button 4 is pressed, exit from function
      if (digitalRead(buttons[3]) == 1) {
        printStatus("Larson exit");
        return;
      }
    }

    // Pause
    delay(ReturnDelay);

    // Go the other way
    for(int i = NUM_LEDS-EyeSize-2; i > 0; i--) {
      setAll(0,0,0);
      setPixel(i, red/10, green/10, blue/10);
      for(int j = 1; j <= EyeSize; j++) {
        setPixel(i+j, red, green, blue); 
      }
      setPixel(i+EyeSize+1, red/10, green/10, blue/10);
      FastLED.show();
      delay(SpeedDelay);

      // If button 4 is pressed, exit from function
      if (digitalRead(buttons[3]) == 1) {
        printStatus("Larson exit");
        return;
      }
    }
    
    delay(ReturnDelay);
  }
  printStatus("Larson fin'd");
}

// Set a specific pixel to a specific colour. For some reason, the colour specification is messed up
void setPixel(int Pixel, byte red, byte green, byte blue) {
  leds[Pixel].g = red;
  leds[Pixel].r = green;
  leds[Pixel].b = blue;
}

// Do a rainbow pattern along the two rows of LEDs
void rainbowCycle(int wait, int dim) {
  //loop several times with same configurations and same delay
  while(1) {
  
    byte dir=random(0,2);
    int k=255;
    
    //loop through all colors in the wheel
    for (int j=0; j < 256; j++,k--) { 
      
      if(k<0) {
        k=255;
      }
      
      //Set RGB color of each LED
      for(int i=0; i<NUM_LEDS; i++) {        
        CRGB ledColor = wheel(((i * 256 / NUM_LEDS) + (dir==0?j:k)) % 256,dim);        
        leds[i]=ledColor;

        // If button 3 is pressed, exit from the function
        if (digitalRead(buttons[2]) == 1) {
          return;
        }
      }
      
      FastLED.show();
      FastLED.delay(wait);
    }
  }
}

// Part of the rainbow code which calculates the r,g,b values based on a position on the "colour wheel"
CRGB wheel(int WheelPos, int dim) {
  CRGB color;
  if (85 > WheelPos) {
   color.r=0;
   color.g=WheelPos * 3/dim;
   color.b=(255 - WheelPos * 3)/dim;;
  } 
  else if (170 > WheelPos) {
   color.r=WheelPos * 3/dim;
   color.g=(255 - WheelPos * 3)/dim;
   color.b=0;
  }
  else {
   color.r=(255 - WheelPos * 3)/dim;
   color.g=0;
   color.b=WheelPos * 3/dim;
  }

  //printStatus("Status: " + String(color.r));

  return color;
}
