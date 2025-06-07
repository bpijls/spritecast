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

uint8_t image[] = {0x5a, 0x52, 0xfa, 0x60, 0x00, 0xff, 0xf4, 0x67, 0x80, 0x71, 0x10, 0x11, 0x12, 0x13, 0x11, 0x12, 0x13, 0x11, 0x10, 0x11, 0x23, 0x11, 0x23, 0x11, 0x10, 0xa1, 0x22, 0x61, 0x22, 0x31, 0x10, 0x14, 0x61, 0x14}
;

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

  decodeAndDraw(image, sizeof(image));
}

void decodeAndDraw(const uint8_t* data, int length) {
  if (length == 0) {
    Serial.println("Decoder Error: No data.");
    return;
  }

  // Unpack all bytes into a stream of nibbles
  const int numNibbles = length * 2;
  uint8_t nibbles[numNibbles];
  for (int i = 0; i < length; ++i) {
    nibbles[i * 2]     = (data[i] >> 4) & 0x0F;
    nibbles[i * 2 + 1] = data[i] & 0x0F;
  }

  int nibbleIndex = 0;

  // First nibble is the number of colors in the palette
  uint8_t paletteSize = nibbles[nibbleIndex++];
  if (paletteSize > PALETTE_SIZE) {
    Serial.println("Decoder Error: Palette size exceeds max.");
    return;
  }
  
  // Decode the palette colors (3 nibbles per color: R, G, B)
  for (int i = 0; i < paletteSize; ++i) {
    if (nibbleIndex + 2 >= numNibbles) {
      Serial.println("Decoder Error: Incomplete palette data.");
      return;
    }
    uint8_t r4 = nibbles[nibbleIndex++];
    uint8_t g4 = nibbles[nibbleIndex++];
    uint8_t b4 = nibbles[nibbleIndex++];
    
    // Scale 4-bit color to 8-bit for NeoPixel
    uint8_t r8 = (r4 << 4) | r4;
    uint8_t g8 = (g4 << 4) | g4;
    uint8_t b8 = (b4 << 4) | b4;
    
    pixelGrid.setPaletteColor(i, strip.Color(r8, g8, b8));
  }

  // Decode the RLE pixel data (2 nibbles per run: run_length, color_index)
  int pixelIndex = 0;
  while (nibbleIndex + 1 < numNibbles && pixelIndex < LED_COUNT) {
    uint8_t run = nibbles[nibbleIndex++];
    uint8_t colorIndex = nibbles[nibbleIndex++];

    if (colorIndex >= paletteSize) {
      Serial.println("Decoder Error: Invalid color index.");
      continue;
    }

    for (int i = 0; i < run && pixelIndex < LED_COUNT; ++i) {
      uint8_t y = pixelIndex / GRID_WIDTH;
      uint8_t x = pixelIndex % GRID_WIDTH;
      pixelGrid.setPixel(x, y, colorIndex);
      pixelIndex++;
    }
  }
  
  pixelGrid.draw();
}

void loop() {
  Example of how you might fetch and draw data in the future
  http.begin(client, server_url);
  int httpCode = http.GET();
  if (httpCode == HTTP_CODE_OK) {
    int len = http.getSize();
    uint8_t buff[len];
    http.getStream().readBytes(buff, len);
    decodeAndDraw(buff, len);
  }
  http.end();
  
  delay(5000); // Check for a new image every 5 seconds
}
