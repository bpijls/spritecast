#include <Adafruit_NeoPixel.h>
#include "PixelGrid.h"

#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager


// Which pin on the Arduino is connected to the NeoPixels?
#define LED_PIN    4

// How many NeoPixels are attached to the Arduino?
#define LED_COUNT 64

// Define the size of the grid (n x n)
#define GRID_WIDTH 8 
#define GRID_HEIGHT 8 

// Server details (replace with your actual server URL)
const char* dataServerUrl = "http://your-server.com/getPixelData"; // Placeholder

// Declare our NeoPixel strip object:
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
// Declare our PixelGrid object
PixelGrid pixelGrid(GRID_WIDTH, GRID_HEIGHT, &strip);

uint8_t doggo[] = {0x01,0x25,0x40,0x41,0x40,0x41,0x40,0x12,0x11,0x12,0x11,0x32,0x51,0x32,0x51,0x32,0x51,0x52,0x31,0x13,0x21,0x13,41};


void setup() {
  Serial.begin(115200);
  delay(10); 
  Serial.println("\nStarting up...");

  WiFiManager wifiManager;
  // wifiManager.resetSettings(); // Uncomment to erase stored credentials
  if (!wifiManager.autoConnect("SpriteCastAP_Config")) {
    Serial.println("Failed to connect to WiFi and hit timeout");
    delay(3000);
    ESP.restart();
    delay(5000);
  }

  Serial.println("Connected to WiFi!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  strip.begin();
  strip.show(); 
  strip.setBrightness(50); 

  // Initial palette (can be overwritten by server)
  pixelGrid.setPaletteColor(0, strip.Color(255, 0, 0));    // Red
  pixelGrid.setPaletteColor(1, strip.Color(255, 127, 0));  // Orange

  // ... (rest of default palette setup from previous step)
  for (int i = 2; i < PALETTE_SIZE; ++i) { // Fill rest with black initially
    pixelGrid.setPaletteColor(i, strip.Color(0,0,0)); 
  }
  
    decodeAndDraw(doggo, sizeof(doggo));
}

void decodeAndDraw(const uint8_t* data, int length) {
  if (length < 2) {
    Serial.println("Invalid data: too short");
    return;
  }

  uint8_t paletteSize = data[0];
  if (paletteSize > PALETTE_SIZE) {
      Serial.println("Invalid data: palette size too large");
      return;
  }
  uint8_t imagePalette[PALETTE_SIZE];
  
  int paletteByteCount = (paletteSize + 1) / 2;
  int paletteEndIndex = 1 + paletteByteCount;

  if (length < paletteEndIndex) {
    Serial.println("Invalid data: not enough data for palette");
    return;
  }

  // Unpack the custom palette
  for (int i = 0; i < paletteByteCount; ++i) {
    uint8_t byte = data[1 + i];
    int paletteIdx1 = i * 2;
    int paletteIdx2 = i * 2 + 1;

    if (paletteIdx1 < paletteSize) {
      imagePalette[paletteIdx1] = (byte >> 4) & 0x0F;
    }
    if (paletteIdx2 < paletteSize) {
      imagePalette[paletteIdx2] = byte & 0x0F;
    }
  }

  // Decode the RLE data
  int pixelIndex = 0;
  for (int i = paletteEndIndex; i < length && pixelIndex < LED_COUNT; ++i) {
    uint8_t byte = data[i];
    uint8_t run = (byte >> 4) & 0x0F;
    uint8_t paletteIndexInImage = byte & 0x0F;

    if (run == 0) {
      run = 16; // The JS encoder bug encodes a run of 16 as 0
    }
    
    if (paletteIndexInImage >= paletteSize) {
      Serial.println("Invalid data: palette index out of bounds");
      continue;
    }

    uint8_t finalPaletteIndex = imagePalette[paletteIndexInImage];

    for (int j = 0; j < run && pixelIndex < LED_COUNT; ++j) {
      uint8_t y = pixelIndex / GRID_WIDTH;
      uint8_t x = pixelIndex % GRID_WIDTH;
      pixelGrid.setPixel(x, y, finalPaletteIndex);
      pixelIndex++;
    }
  }
  
  pixelGrid.draw(); // Draw the decoded image
}

void loop() {
  pixelGrid.draw(); // Keep redrawing the current grid state
  delay(1000); 

}
