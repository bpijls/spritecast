// A basic everyday NeoPixel strip test program.

// NEOPIXEL BEST PRACTICES for most reliable operation:
// - Add 1000 uF CAPACITOR between NeoPixel strip's + and - connections.
// - MINIMIZE WIRING LENGTH between microcontroller board and first pixel.
// - NeoPixel strip's DATA-IN should pass through a 300-500 OHM RESISTOR.
// - AVOID connecting NeoPixels on a LIVE CIRCUIT. If you must, ALWAYS
//   connect GROUND (-) first, then +, then data.
// - When using a 3.3V microcontroller with a 5V-powered NeoPixel strip,
//   a LOGIC-LEVEL CONVERTER on the data line is STRONGLY RECOMMENDED.
// (Skipping these may work OK on your workbench but can fail in the field)

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1:
#define LED_PIN    4
#define BUTTON_NEXT_PIN 0
#define BUTTON_PREV_PIN 1

// How many NeoPixels are attached to the Arduino?
#define LED_COUNT 64

// Declare our NeoPixel strip object:
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
// Argument 1 = Number of pixels in NeoPixel strip
// Argument 2 = Arduino pin number (most are valid)
// Argument 3 = Pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)

// Enumerate the demo states
enum DemoState {
  COLOR_WIPE_STATE,
  THEATER_CHASE_STATE,
  RAINBOW_STATE,
  THEATER_CHASE_RAINBOW_STATE,
  NUM_DEMO_STATES // Keep this last for easy counting
};

DemoState currentDemoState = COLOR_WIPE_STATE;

// Button debounce variables
// unsigned long lastDebounceTimeNext = 0; // Removed
// unsigned long lastDebounceTimePrev = 0; // Removed
unsigned long debounceDelay = 100; // 50 milliseconds

int lastButtonStateNext = HIGH;
int lastButtonStatePrev = HIGH;
// int buttonStateNext; // Will be local in loop
// int buttonStatePrev; // Will be local in loop

// --- Non-Blocking Animation State Variables ---
unsigned long animationLastUpdateTime = 0;
int animationStep = 0;      // General purpose step counter (current pixel, current frame, etc.)
int subAnimationStep = 0;   // General purpose sub-step counter (current color, inner loop index, etc.)

// Specific state for Theater Chase
int theaterChaseColorIndex = 0; // 0: White, 1: Red, 2: Blue for original theaterChase

// Specific state for Theater Chase Rainbow
long tcrFirstPixelHue = 0;

// --- Initialization functions for each state ---
void initColorWipeState() {
    animationStep = 0;      // Current pixel index
    subAnimationStep = 0;   // Current color: 0=Red, 1=Green, 2=Blue
    animationLastUpdateTime = millis(); // Start timing for the first step
    // strip.clear(); // Optional: clear strip if needed at start of this state sequence
    // strip.show();
}

void initTheaterChaseState() {
    animationStep = 0;      // Current 'b' value (chase offset)
    subAnimationStep = 0;   // Current 'a' value (outer repetition loop count)
    theaterChaseColorIndex = 0; // Current color (0=White, 1=Red, 2=Blue)
    animationLastUpdateTime = millis();
    strip.clear(); // Clear strip at the beginning of a full sequence
    strip.show();
}

void initRainbowState() {
    animationStep = 0;      // Represents firstPixelHue for strip.rainbow()
    animationLastUpdateTime = millis();
}

void initTheaterChaseRainbowState() {
    animationStep = 0;      // Current 'b' value (chase offset)
    subAnimationStep = 0;   // Current 'a' value (outer repetition loop count, 0-29)
    tcrFirstPixelHue = 0;   // Hue for the first pixel in the chase pattern
    animationLastUpdateTime = millis();
    strip.clear();
    strip.show();
}

// setup() function -- runs once at startup --------------------------------

void setup() {
  // These lines are specifically to support the Adafruit Trinket 5V 16 MHz.
  // Any other board, you can remove this part (but no harm leaving it):
#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  clock_prescale_set(clock_div_1);
#endif
  // END of Trinket-specific code.

  pinMode(BUTTON_NEXT_PIN, INPUT_PULLUP);
  pinMode(BUTTON_PREV_PIN, INPUT_PULLUP);

  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(10); // Set BRIGHTNESS to about 1/5 (max = 255)

  // Initialize the first state
  initColorWipeState(); 
}


// loop() function -- runs repeatedly as long as board is on ---------------

void loop() {
  // Read button states
  int buttonStateNext = digitalRead(BUTTON_NEXT_PIN);
  int buttonStatePrev = digitalRead(BUTTON_PREV_PIN);

  // Check for Next button press
  if (buttonStateNext == LOW && lastButtonStateNext == HIGH) {
    currentDemoState = (DemoState)((currentDemoState + 1) % NUM_DEMO_STATES);
    switch(currentDemoState) {
        case COLOR_WIPE_STATE: initColorWipeState(); break;
        case THEATER_CHASE_STATE: initTheaterChaseState(); break;
        case RAINBOW_STATE: initRainbowState(); break;
        case THEATER_CHASE_RAINBOW_STATE: initTheaterChaseRainbowState(); break;
        default: initColorWipeState(); break; // Default to a known state
    }
    delay(debounceDelay); // Simplified debounce
  }
  lastButtonStateNext = buttonStateNext; // Update last state for next iteration's edge detection

  // Check for Previous button press
  if (buttonStatePrev == LOW && lastButtonStatePrev == HIGH) {
    currentDemoState = (DemoState)((currentDemoState - 1 + NUM_DEMO_STATES) % NUM_DEMO_STATES);
    switch(currentDemoState) {
        case COLOR_WIPE_STATE: initColorWipeState(); break;
        case THEATER_CHASE_STATE: initTheaterChaseState(); break;
        case RAINBOW_STATE: initRainbowState(); break;
        case THEATER_CHASE_RAINBOW_STATE: initTheaterChaseRainbowState(); break;
        default: initColorWipeState(); break; // Default to a known state
    }
    delay(debounceDelay); // Simplified debounce
  }
  lastButtonStatePrev = buttonStatePrev; // Update last state for next iteration's edge detection

  // Run the current demo state's update function
  switch (currentDemoState) {
    case COLOR_WIPE_STATE:
      updateColorWipeState(50);
      break;
    case THEATER_CHASE_STATE:
      updateTheaterChaseState(50);
      break;
    case RAINBOW_STATE:
      updateRainbowState(10);
      break;
    case THEATER_CHASE_RAINBOW_STATE:
      updateTheaterChaseRainbowState(50);
      break;
    default:
      break;
  }
  // A small delay here can make button presses more responsive if animations are very short
  // but since these animations are blocking and long, it's less critical here.
  // delay(10); 
}


// Some functions of our own for creating animated effects -----------------

// Fill strip pixels one after another with a color. Strip is NOT cleared
// first; anything there will be covered pixel by pixel. Pass in color
// (as a single 'packed' 32-bit value, which you can get by calling
// strip.Color(red, green, blue) as shown in the loop() function above),
// and a delay time (in milliseconds) between pixels.
void updateColorWipeState(int wait) {
    if (millis() - animationLastUpdateTime < (unsigned long)wait) {
        return; // Not time yet for the next step
    }
    animationLastUpdateTime = millis();

    uint32_t color;
    if (subAnimationStep == 0) color = strip.Color(255, 0, 0);   // Red
    else if (subAnimationStep == 1) color = strip.Color(0, 255, 0); // Green
    else color = strip.Color(0, 0, 255);   // Blue

    if (animationStep < strip.numPixels()) {
        strip.setPixelColor(animationStep, color);
        strip.show();
        animationStep++;
    } else { // Current color wipe finished
        animationStep = 0; // Reset pixel index for the next color
        subAnimationStep++;
        if (subAnimationStep >= 3) { // All three colors done
            subAnimationStep = 0; // Reset to Red for the next cycle of this state
        }
    }
}

// Theater-marquee-style chasing lights. Pass in a color (32-bit value,
// a la strip.Color(r,g,b) as mentioned above), and a delay time (in ms)
// between frames.
void updateTheaterChaseState(int wait) {
    if (millis() - animationLastUpdateTime < (unsigned long)wait) {
        return;
    }
    animationLastUpdateTime = millis();

    uint32_t color;
    if (theaterChaseColorIndex == 0) color = strip.Color(127, 127, 127); // White
    else if (theaterChaseColorIndex == 1) color = strip.Color(127, 0, 0);   // Red
    else color = strip.Color(0, 0, 127);   // Blue

    strip.clear(); // Clear for each frame

    // animationStep is 'b' (0, 1, 2)
    for (int c = animationStep; c < strip.numPixels(); c += 3) {
        strip.setPixelColor(c, color);
    }
    strip.show();

    animationStep++; // Next 'b'
    if (animationStep >= 3) {
        animationStep = 0; // Reset 'b'
        subAnimationStep++; // Increment 'a' (outer loop)
        if (subAnimationStep >= 10) { // 10 repetitions for this color done
            subAnimationStep = 0; // Reset 'a'
            theaterChaseColorIndex++;
            if (theaterChaseColorIndex >= 3) { // All colors done
                theaterChaseColorIndex = 0; // Reset to White for next cycle of this state
            }
        }
    }
}

// Rainbow cycle along whole strip. Pass delay time (in ms) between frames.
void updateRainbowState(int wait) {
    if (millis() - animationLastUpdateTime < (unsigned long)wait) {
        return;
    }
    animationLastUpdateTime = millis();

    // animationStep is firstPixelHue
    strip.rainbow(animationStep);
    strip.show();

    animationStep += 256;
    if (animationStep >= 5 * 65536) { // Original condition for 5 full cycles
        animationStep = 0; // Reset for next cycle
    }
}

// Rainbow-enhanced theater marquee. Pass delay time (in ms) between frames.
void updateTheaterChaseRainbowState(int wait) {
    if (millis() - animationLastUpdateTime < (unsigned long)wait) {
        return;
    }
    animationLastUpdateTime = millis();

    strip.clear();
    // animationStep is 'b' (0,1,2)
    for (int c = animationStep; c < strip.numPixels(); c += 3) {
        int hue = tcrFirstPixelHue + c * 65536L / strip.numPixels();
        uint32_t colorVal = strip.gamma32(strip.ColorHSV(hue));
        strip.setPixelColor(c, colorVal);
    }
    strip.show();

    tcrFirstPixelHue += (65536 / 90); // Advance hue for the next frame
    // Simple wrap for tcrFirstPixelHue if it gets too large, though ColorHSV handles overflow.
    if (tcrFirstPixelHue >= 5 * 65536) { tcrFirstPixelHue = 0; } 

    animationStep++; // Next 'b'
    if (animationStep >= 3) {
        animationStep = 0; // Reset 'b'
        subAnimationStep++; // Increment 'a' (outer loop)
        if (subAnimationStep >= 30) { // 30 repetitions done
            subAnimationStep = 0; // Reset 'a' for the next cycle of this state
            // tcrFirstPixelHue continues to evolve or could be reset here if desired for a full visual loop
        }
    }
}
