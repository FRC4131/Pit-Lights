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
	wired a particular way for this to work. Test mode 	will help validate 
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

*/
   
//==========================================================================
// Defines

#define PIXEL_PIN 6          // which data pin drives the pixels
#define NUM_LEDS 68          // how many lights are in the display
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
  pinMode(13, OUTPUT);  // for the onboard LED

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
  } else {

  theaterChase(strip.Color(127, 127, 127), 50); // Light White
  theaterChase(strip.Color(127, 0, 0), 50);     // Light Red
  theaterChase(strip.Color(0, 0, 127), 50);     // Light Blue
  delay(1000);
  
  rainbow(20);
  rainbowCycle(20);
  theaterChaseRainbow(50); 
  delay(1000);
  
    
  } // end if/else testMode
} // end loop


//==========================================================================
void blinkLED(int pin, int nTimes) {
  digitalWrite(pin, LOW);
  for(int i=0; i<nTimes; i++) {
    digitalWrite(13, HIGH); 
    delay(300);
    digitalWrite(13, LOW);
    delay(300); 
  }
    
} // end blinkLED


//==========================================================================
// runTestSequence: runs a predefined test pattern to make sure that the
// pixels are wired properly in the frame and that we know that all the key
// points and edges are located where we expect them.

void runRestSequence(){
  // left column bottom row from left to righ
  simpleCyclePixel(BOT_LEFT_CORNER);
  simpleCyclePixel(BOT_LEFT_CORNER+1);

  // left column top top row from left to right
  simpleCyclePixel(TOP_LEFT_CORNER-1);
  simpleCyclePixel(BOT_LEFT_CORNER);

  
  // Slowly light up the strand, 1 pixel at a time
  colorWipe(strip.Color(255, 0, 0), 500); // Red
  colorWipe(strip.Color(0, 255, 0), 500); // Green
  colorWipe(strip.Color(0, 0, 255), 500); // Blue

} // end testSequence

//==========================================================================
// cycles a pixel through Red, Green, and Blue with a short wait in between.

void simpleCyclePixel(int pixel) {
  strip.setPixelColor(pixel,strip.Color(255,0,0)); strip.show(); delay(500);
  strip.setPixelColor(pixel,strip.Color(0,255,0)); strip.show(); delay(500);
  strip.setPixelColor(pixel,strip.Color(0,0,255)); strip.show(); delay(500);
  strip.setPixelColor(pixel,strip.Color(0,0,0));   strip.show(); delay(500);
} // end simplecCyclePixel


//===============================================================
// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
} // end colorWipe

//===============================================================
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

//===============================================================
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
}

//===============================================================
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
}

//===============================================================
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
}

//===============================================================
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
