#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

/*==========================================================================
   README
   This app has 3 main behaviors: Test Mode (run a test sequence),
     Flair mode (run the flashy show-off sequence),
   and Pit Game mode (reaction time game).
   
    The lights are housed in to a retangular frame shape and have to be 
  wired a particular way for this to work. Test mode  will help validate 
  that the lights are wired correctly; that we know where the left and right
  sides are, the corners, and that all code is properly configured for the 
  number of pixels actually showing in the frame. 
  
  The Pit Game needs to know where the uprights are and their orientation
  so that it can use them as an indicator of game start (eg ready, steady go)
  as well as a indicator of how fast the player is and who won (player 1 or 2)

    Pixel wiring: loading the pixels in the frame needs to be done "in order".
  The fram2 is 2 columns wide on the sides and 2 columns tall on top and bottom.
  The sequence should follow the wiring pattern below, extended for the number
  of pixels in the frame.
  
  10 11 12 14 16 17  
  8  9  13 15 18 19  
  6  7        20 21
  4  5        22 23
  2  3  30 28 24 25 
  0  1  31 29 27 26

  Of note: 27 and 26 in the above example are "reversed" because the length 
  of the wires between the lights isn't long enough to stretch from the hole
  at position 26 to the hole at position 28.
*/
   
//==========================================================================
// Defines

#define PIXEL_PIN 6          // which data pin drives the pixels
#define NUM_LEDS 70          // how many lights are in the display
#define TEST_MODE_PIN 12     // ground this pin to run in test mode

// the first pixel in the sequence at the bottom of the left column. 
// In the example above, this is pixel 0
#define BOT_LEFT_CORNER 0   

// the last pixel in the sequence at the top of the left column 
// In the example above, this is pixel 11 
#define TOP_LEFT_CORNER 34  
 
// the first pixel in the sequence at the top of the right column
// In the example above, this is pixel 16 
#define TOP_RIGHT_CORNER 37  

// the last pixel in the sequence at the bottom of the right column
// In the example above, this is pixel 27 
#define BOT_RIGHT_CORNER 70  

// the last pixel in the sequence that is the outer 2 columns of the frame.
// this is currently a bit of over-engineering, in case additional pixels at
// the end of the strand are rolled into some other part of the display
// In the example above, this is pixel 27 
#define FRAME_END 70  


// effectively creating an enum for the frame sides
#define LEFT_SIDE 1
#define TOP_SIDE 2
#define RIGHT_SIDE 3
#define BOTTOM_SIDE 4



//==========================================================================
// Initialize the Pixel Library
//
// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, PIXEL_PIN, NEO_RGB + NEO_KHZ800);

//==========================================================================
void setup() {
  pinMode(13, OUTPUT);  // for the onboard LED, in case we need it for debugging

  // pick a pin to ground to start testMode
  pinMode(TEST_MODE_PIN, INPUT_PULLUP);
  
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
}


//==========================================================================
void loop() {

  // if we grounded the test mode pin, run the test sequence
  if(LOW == digitalRead(TEST_MODE_PIN)) {
     runTestSequence();
  } 
  else {

  theaterChase(strip.Color(127, 127, 127), 50); // Light White
  theaterChase(strip.Color(127, 0, 0), 50);     // Light Red
  theaterChase(strip.Color(0, 0, 127), 50);     // Light Blue
  delay(1000);
  
  rainbow(20);
  rainbowCycle(20);
  theaterChaseRainbow(50); 
  delay(1000);
  
    
  } // end if / else testMode
} // end loop



/*==========================================================================
   runTestSequence: runs a predefined test pattern to make sure that the
   pixels are wired properly in the frame and that we know that all the key
   points and edges are located where we expect them.
  ==========================================================================
*/
void runTestSequence(){
  // left column bottom row from left to right
  simpleCyclePixel(BOT_LEFT_CORNER);
  simpleCyclePixel(BOT_LEFT_CORNER+1);

  // left column top row from left to right
  simpleCyclePixel(TOP_LEFT_CORNER-1);
  simpleCyclePixel(BOT_LEFT_CORNER);

  // right column top row from left to right
  simpleCyclePixel(TOP_RIGHT_CORNER);
  simpleCyclePixel(TOP_RIGHT_CORNER+1);
 
  // right column bottom row from left to right
  simpleCyclePixel(BOT_RIGHT_CORNER);
  simpleCyclePixel(BOT_RIGHT_CORNER-1);

  lightsOff();
  
  // Light the frame sides: left and right, then top and bottom
  lightFrameSide(LEFT_SIDE, 255,0,0);  delay(1000);
  lightFrameSide(RIGHT_SIDE, 0,255,0); delay(1000);
  lightsOff();
  
  lightFrameSide(TOP_SIDE, 0,0,255);        delay(1000);
  lightFrameSide(BOTTOM_SIDE, 255,255,255); delay(1000);
  
  lightsOff();


 // Slowly light up the strand, 1 pixel at a time
//  colorWipe(strip.Color(255, 0, 0), 500); // Red
//  colorWipe(strip.Color(0, 255, 0), 500); // Green
//  colorWipe(strip.Color(0, 0, 255), 500); // Blue

} // end testSequence

/*==========================================================================
   cycles a pixel through Red, Green, and Blue with a short wait between.
   pixel: which pixel to light up 
  ==========================================================================
*/
void simpleCyclePixel(int pixel) {
  strip.setPixelColor(pixel,strip.Color(255,0,0)); strip.show(); delay(500);
  strip.setPixelColor(pixel,strip.Color(0,255,0)); strip.show(); delay(500);
  strip.setPixelColor(pixel,strip.Color(0,0,255)); strip.show(); delay(500);
  strip.setPixelColor(pixel,strip.Color(0,0,0));   strip.show(); delay(500);
} // end simpleCyclePixel

/*==========================================================================
   lightFrameSide: Lights a side of the frame. This function may  not live 
   to production once I get a feeling for how this  "interface" should work
   side: a DEFINE that denotes which side to light 
   colorR: the R of RGB color
   colorG: the G of RGB color
   colorB: the B of RGB color
  ==========================================================================
*/
void lightFrameSide(int side, int colorR, int colorG, int colorB) {
  int i;
  switch (side) {
     case LEFT_SIDE:
       for(i=BOT_LEFT_CORNER; i<=TOP_LEFT_CORNER; i++) {
       strip.setPixelColor(i,strip.Color(colorR,colorG,colorB));
     }
       break;
     
     case RIGHT_SIDE:
       for(i=TOP_RIGHT_CORNER; i<=BOT_RIGHT_CORNER; i++) {
       strip.setPixelColor(i,strip.Color(colorR,colorG,colorB));
     }
       break;
     
     case TOP_SIDE:
     // since the top side is really two rows and the DEFINES mark the locations
     // on the top row, we need to offset appropriately. The README at top explains.
       for(i=TOP_LEFT_CORNER-2; i<=TOP_RIGHT_CORNER+3; i++) {
       strip.setPixelColor(i,strip.Color(colorR,colorG,colorB));
     }
     break;
   
     case BOTTOM_SIDE:
     // since the lights are wired clockwise, the bottom row is pretty complicated.
     // we do assume that our frame is 2 lights wide so the corners are squares of 4
     // The README at top explains and has an example.
       
     // light first 4 LEDs to that make the first corner. Also note that this
     // loop uses < 4 to make the mental model easier even though the others use <=
     for(i=BOT_LEFT_CORNER; i<BOT_LEFT_CORNER+4; i++) {
       strip.setPixelColor(i,strip.Color(colorR,colorG,colorB));
     }
     // now go get the rest of the bottom row which is at the end of the light strand
     // because of the wiring order, the numbers here will look really weird. See the
     // README at the top for clarity
     for(i=BOT_RIGHT_CORNER-3; i<NUM_LEDS; i++) {
       strip.setPixelColor(i,strip.Color(colorR,colorG,colorB));
     }
       break;
     
     default: 
       // if nothing else matches turn off all the lights
       for(i=BOT_RIGHT_CORNER; i<FRAME_END; i++) {
       strip.setPixelColor(i,strip.Color(0,0,0));
     }
       break;
   } // end switch
   // turn on the lights
   strip.show();
} // lightColumn 

/*==========================================================================
   turn all the lights off 
  ==========================================================================
*/
void lightsOff() {
  for(int i=0; i<NUM_LEDS; i++) {
    strip.setPixelColor(i,strip.Color(0,0,0));
  }
  strip.show();
} // end lightsOff;

//==========================================================================
// Fill the dots one after the other with a color
//==========================================================================
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
} // end colorWipe

//==========================================================================
void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
} // end rainbow

//==========================================================================
// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
} // end rainbowCycle

//==========================================================================
//Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait) {
  for (int j=0; j<10; j++) {  //do 10 cycles of chasing
    for (int q=0; q < 3; q++) {
      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, c);    //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    } // end q loop
  } // end j loop
} // end theaterChase

//==========================================================================
//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
  for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
    for (int q=0; q < 3; q++) {
      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, Wheel( (i+j) % 255));    //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    } // end q loop
  } // end j loop
} // end theaterChaseRainbow

//==========================================================================
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
} // end Wheel

/*==========================================================================
    blinkLED: blinks the onboard LED nTimes
*/
void blinkLED(int nTimes) {
  digitalWrite(13, LOW);
  for(int i=0; i<nTimes; i++) {
    digitalWrite(13, HIGH); 
    delay(300);
    digitalWrite(13, LOW);
    delay(300); 
  }
    
} // end blinkLED


