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
  This is super important to get right and to validate first (and often).
  The frame is 2 columns wide on the sides and 2 columns tall on top and bottom.
  The sequence should follow the wiring pattern below, extended for the number
  of pixels in the frame.
  
  10 11 12 14 16 17  
  8  9  13 15 18 19  
  6  7        20 21
  4  5        22 23
  2  3  30 28 24 25 
  0  1  31 29 26 27

*/
   
//==========================================================================
// Defines

#define PIXEL_PIN 6          // which data pin drives the pixels
#define NUM_LEDS 72          // how many lights are in the display
#define TEST_MODE_PIN 12     // ground this pin to run in test mode

// the first pixel in the sequence at the bottom of the left column. 
// In the example above, this is pixel 0
#define BOT_LEFT_CORNER 0   

// the last pixel in the sequence at the top of the left column 
// In the example above, this is pixel 11 
#define TOP_LEFT_CORNER 33
 
// the first pixel in the sequence at the top of the right column
// In the example above, this is pixel 16 
#define TOP_RIGHT_CORNER 36  

// the last pixel in the sequence at the bottom of the right column
// In the example above, this is pixel 26 
#define BOT_RIGHT_CORNER 68

// the last pixel in the sequence that is the outer 2 columns of the frame.
// this is currently a bit of over-engineering, in case additional pixels at
// the end of the strand are rolled into some other part of the display
// In the example above, this is pixel 27 
#define FRAME_END 72  

// effectively creating an enum for the frame sides
#define LEFT_SIDE 1
#define TOP_SIDE 2
#define RIGHT_SIDE 3
#define BOTTOM_SIDE 4
#define WHOLE_FRAME 5

// pit game buttons
#define BUTTON_1_PIN 2
#define BUTTON_2_PIN 3


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
// global variables

// Lets us know if flair mode has been interrupted by a button press.
// (has to be volatile because of the way interrupts work)
volatile boolean interruptButtonPressed;

// Lets us know which button was pressed by way of interrupts 
// (has to be volatile because of the way interrupts work)
//volatile boolean buttonOnePressed;
//volatile boolean buttonTwoPressed;

// Interrupt Service Routine (ISR)
void isrButtonOne () {
Serial.println("isrButtonOne: Button 1 pressed");
  interruptButtonPressed = true;
//  buttonOnePressed = true;
}  // end of isr

void isrButtonTwo () {
Serial.println("isrButtonTwo: Button 2 pressed");
  interruptButtonPressed = true;
//  buttonTwoPressed = true;
}  // end of isr


//==========================================================================
void setup() {
   pinMode(13, OUTPUT);  // for the onboard LED, in case we need it for debugging
   
   Serial.begin(9600);   // set up serial port for console logging
   
   // pick a pin to ground to start testMode
   pinMode(TEST_MODE_PIN, INPUT_PULLUP);

   // game mode buttons
   pinMode(BUTTON_1_PIN, INPUT_PULLUP);
   pinMode(BUTTON_2_PIN, INPUT_PULLUP);

   // registering the intertupt to stop flair mode and go into game mode
   interruptButtonPressed = false;
   buttonOnePressed = false;
   buttonTwoPressed = false;
   attachInterrupt(digitalPinToInterrupt(BUTTON_1_PIN), isrButtonOne, FALLING);
   attachInterrupt(digitalPinToInterrupt(BUTTON_2_PIN), isrButtonTwo, FALLING);
  
   strip.begin();
   strip.show(); // Initialize all pixels to 'off'
} // end setup


//==========================================================================
void loop() {

  lightsOff(); delay(1000);
   
  // if we grounded the test mode pin, run the test sequence
  if(LOW == digitalRead(TEST_MODE_PIN)) {
     runTestSequence();
  } else {

  theaterChase(strip.Color(127, 127, 127), 50); // Light White
  if(interruptButtonPressed) { gameMode(); }    // we do this often to reduce wait time

  theaterChase(strip.Color(127, 0, 0), 50);     // Light Red
  if(interruptButtonPressed) { gameMode(); }

  theaterChase(strip.Color(0, 0, 127), 50);     // Light Blue
  if(interruptButtonPressed) { gameMode(); }

  delay(1000);
  if(interruptButtonPressed) { gameMode(); }

  rainbow(20);
  if(interruptButtonPressed) { gameMode(); }

  rainbowCycle(20);
  if(interruptButtonPressed) { gameMode(); }

  theaterChaseRainbow(20); 
  if(interruptButtonPressed) { gameMode(); }

  delay(1000);
  if(interruptButtonPressed) { gameMode(); }
  
    
  } // end if / else testMode
} // end loop

/*==========================================================================
   gameMode: starts a reaction time game 
   How the game plays: While in flair mode, when 1 button is pressed, the lights
    will go out and then the game will start. The whole frame will flash red,
	yellow, green to denote that game mode is starting and hint that the count
	down will also be red, yellow, geen (like a traffic light). Then the left
	and right columns will light red, yellow, then green, and then go out. 
	Think ready, steady, go. After the lights go out, the first person to hit
	their button wins. If no button is pressed, the game will end after 1 second.
	The lights in each column will light showing how fast each player was. The
    lights will light up white from the top down where the shorter white column
	is better (less time from lights out to button press). However, in order to
	give the players a feeling of success, the empty space in the column will be
	filled in by green lights which will result in a taller column of green being
	considered doing the best.
	
   How the code works: We don't get hung on strict time. Instead we make some broad
    assumptions to life easier. We use interrupts to determine when buttons are
	pressed. Meanwhile, we loop for a second or 2 waiting 1 ms each iteration
	checking to see if a button was pressed each time. Each iteration, we add 1 ms
	to each players reaction time if they haven't pressed the button yet. This 
	works under the assumption that we keep the code in the loop super small.
	
   Additional ideas on the UI are to fill the remaining area with green on the winning
	side red on the losing side OR to provide a relative weighting where the
	column is broken up into 3rds where the poorer parts are red, medium are yellow 
	and better section is green. The idea is that besides beating one another,
	players would want to be fast enough to get lights in the green segment.
	This will take some tuning on the scale of how fast people tend to be and 
	may be more trouble than it is worth.
   
   ==========================================================================
*/
void gameMode(){
   noInterrupts();  // once we are in game mode, don't listen to interrupts

   // lights off to alert the players that the game is about to start and 
   // keep them off for 1 second to make it obvious
   lightsOff(); delay(1000);
   
   // !! READY !!
   // cycle the whole frame through red, yellow, green
   lightFrame(WHOLE_FRAME, 255,0,0);   delay(1000);  // Red 
   lightFrame(WHOLE_FRAME, 255,255,0); delay(1000);  // Yellow
   lightFrame(WHOLE_FRAME, 0,255,0);   delay(1000);  // Green 

   lightsOff(); delay(500);

   // !! STEADY !!
   // cycle the left and right columns through red, yellow, green  
   lightFrame(LEFT_SIDE, 255,0,0); lightFrame(RIGHT_SIDE, 255,0,0);     delay(1000); // Red 
   lightFrame(LEFT_SIDE, 255,255,0); lightFrame(RIGHT_SIDE, 255,255,0); delay(1000); // Yellow 
   lightFrame(LEFT_SIDE, 0,255,0); lightFrame(RIGHT_SIDE, 0,255,0);     delay(1000); // Green
   
   // preparing to start the game
   int playerOneTime = 0;
   int playerTwoTime = 0;
   int numMS = 0;

   // re-enable interrupts because we'll use those to catch the button press
   interruptButtonPressed = false;
   
   boolean buttonOnePressed = false;
   boolean buttonTwoPressed = false;
   //interrupts(); 

   // !! GO !!
   // Game starts when the lights go out!
   lightsOff();   
   
   Serial.println("entering wait for button press loop");
   // loop until 2 seconds or both buttons pressed (via interrupt)
   while( (numMS < 2000) || ( buttonOnePressed && buttonTwoPressed ) ) {
      
      // check the buttons
      if(LOW == digitalRead(BUTTON_1_PIN)) {
         buttonOnePressed = true;
         Serial.println("Game: Button 1 pressed");
      }
      if(LOW == digitalRead(BUTTON_2_PIN)) {
         buttonTwoPressed = true;
         Serial.println("Game: Button 2 pressed");
      }
      
      // if button wasn't pressed increment the time
      if( !buttonOnePressed ) {
	     playerOneTime++; 
      }
      if( !buttonTwoPressed ) {
	 	 playerTwoTime++; 
      }
      
      // delaying 1 ms is a proxy to count/track time in the loop to see who is fastest
	  delay(1);   
	  numMS++;
   } // end while

   Serial.println("Game: Finished loop. determining winner.");
   
   // figure out who wins and the show results
   if(playerOneTime < playerTwoTime) {
      Serial.println("Game: Player 1 wins");
      // player 1 won!
      // for testing, we'll just flash the winning side and then we'll come back
	   lightFrame(LEFT_SIDE, 0,255,0); delay(500); lightsOff(); delay(500);
      lightFrame(LEFT_SIDE, 0,255,0); delay(500); lightsOff(); delay(500);
      lightFrame(LEFT_SIDE, 0,255,0); delay(500); lightsOff(); delay(500);
      lightFrame(LEFT_SIDE, 0,255,0);
   } else if (playerTwoTime < playerOneTime ) {
      Serial.println("Game: Player 2 wins");
      // player 2 won!
	   lightFrame(RIGHT_SIDE, 0,255,0); delay(500); lightsOff(); delay(500);
      lightFrame(RIGHT_SIDE, 0,255,0); delay(500); lightsOff(); delay(500);
      lightFrame(RIGHT_SIDE, 0,255,0); delay(500); lightsOff(); delay(500);
      lightFrame(RIGHT_SIDE, 0,255,0);
   } else {
      Serial.println("Game: TIE");
      // Tie
	  	lightFrame(LEFT_SIDE, 0,255,0); lightFrame(RIGHT_SIDE, 0,255,0); delay(500); lightsOff(); delay(500);
	  	lightFrame(LEFT_SIDE, 0,255,0); lightFrame(RIGHT_SIDE, 0,255,0); delay(500); lightsOff(); delay(500);
	  	lightFrame(LEFT_SIDE, 0,255,0); lightFrame(RIGHT_SIDE, 0,255,0); delay(500); lightsOff(); delay(500);
	  	lightFrame(LEFT_SIDE, 0,255,0); lightFrame(RIGHT_SIDE, 0,255,0);  
   } // end if else
   
Serial.println("Game: reenable interrupts etc before exit");
   // re-enable interrupts and reset flags
   interruptButtonPressed = false;
//   buttonOnePressed = false;
//   buttonTwoPressed = false;
   interrupts();
   
   lightsOff();  delay(500); // turn off the lights and go back to regularly scheduled programming
} // end gameMode

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
  simpleCyclePixel(TOP_LEFT_CORNER);

  // right column top row from left to right
  simpleCyclePixel(TOP_RIGHT_CORNER);
  simpleCyclePixel(TOP_RIGHT_CORNER+1);
 
  // right column bottom row from left to right
  simpleCyclePixel(BOT_RIGHT_CORNER);
  simpleCyclePixel(BOT_RIGHT_CORNER+1);

  lightsOff();
  
  // Light the frame sides: left and right, then top and bottom
  lightFrame(LEFT_SIDE, 255,0,0);  delay(1000);
  lightFrame(RIGHT_SIDE, 0,255,0); delay(1000);
  lightsOff();
  
  lightFrame(TOP_SIDE, 0,0,255);        delay(1000);
  lightFrame(BOTTOM_SIDE, 255,255,255); delay(1000);
  
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
   lightFrame: Lights some parts of the frame. This function may  not live 
   to production once I get a feeling for how this  "interface" should work
   side: a DEFINE that denotes which side to light 
   colorR: the R of RGB color
   colorG: the G of RGB color
   colorB: the B of RGB color
  ==========================================================================
*/
void lightFrame(int side, int colorR, int colorG, int colorB) {
  int i;
  switch (side) {
     case LEFT_SIDE:
       for(i=BOT_LEFT_CORNER; i<=TOP_LEFT_CORNER; i++) {
       strip.setPixelColor(i,strip.Color(colorR,colorG,colorB));
     }
       break;
     
     case RIGHT_SIDE:
       for(i=TOP_RIGHT_CORNER; i<=BOT_RIGHT_CORNER+1; i++) {
       strip.setPixelColor(i,strip.Color(colorR,colorG,colorB));
     }
       break;
     
     case TOP_SIDE:
       // since the top side is really two rows and the DEFINES mark the locations
       // on the top row, we need to offset appropriately. The README at top explains.
       for(i=TOP_LEFT_CORNER-3; i<=TOP_RIGHT_CORNER+3; i++) {
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
       for(i=BOT_RIGHT_CORNER-2; i<NUM_LEDS; i++) {
         strip.setPixelColor(i,strip.Color(colorR,colorG,colorB));
       }
       break;
	   
     case WHOLE_FRAME:
       for(i=BOT_LEFT_CORNER; i<FRAME_END; i++) {
         strip.setPixelColor(i,strip.Color(colorR,colorG,colorB));
       }
       break;
	 
     default: 
       // if nothing else matches turn off all the lights
       for(i=BOT_LEFT_CORNER; i<FRAME_END; i++) {
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


