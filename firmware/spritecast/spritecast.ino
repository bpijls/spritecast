#include <Adafruit_NeoPixel.h>
#include "PixelGrid.h"

#include <WiFi.h>
#include <WiFiMulti.h>

#include <HTTPClient.h>
// #include <WiFiManager.h> // WiFiManager is no longer used

// --- WiFi Credentials ---
// Replace with your network details!
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD" ;
// -------------------------

// Which pin on the Arduino is connected to the NeoPixels?
#define LED_PIN    4

// How many NeoPixels are attached to the Arduino?
#define LED_COUNT 64

// Define the size of the grid (n x n)
#define GRID_WIDTH 8 
#define GRID_HEIGHT 8 

// Server details
const char* dataServerUrl = "http://192.168.2.117:5000/sprite/random"; 
#define MAX_SPRITE_SIZE 256

// Declare our NeoPixel strip object:
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
// Declare our PixelGrid object
PixelGrid pixelGrid(GRID_WIDTH, GRID_HEIGHT, &strip);
WiFiMulti WiFiMulti;

void setup() {
  Serial.begin(115200);
  delay(10); 
  Serial.println("\nStarting up...");


   // We start by connecting to a WiFi network
  WiFiMulti.addAP(ssid, password);
WiFi.setTxPower(WIFI_POWER_8_5dBm);
  Serial.println();
  Serial.println();
  Serial.print("Waiting for WiFi... ");

  while (WiFiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  strip.begin();
  strip.show(); 
  strip.setBrightness(10); 
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
  HTTPClient http;

  Serial.print("[HTTP] begin...\n");
  http.begin(dataServerUrl); 

  Serial.print("[HTTP] GET...\n");
  int httpCode = http.GET();

  if (httpCode > 0) {
    Serial.printf("[HTTP] GET... code: %d\n", httpCode);

    if (httpCode == HTTP_CODE_OK) {
      int len = http.getSize();
      Serial.printf("[HTTP] Payload size: %d\n", len);

      if (len > 0 && len <= MAX_SPRITE_SIZE) {
        uint8_t buff[MAX_SPRITE_SIZE] = {0};
        WiFiClient * stream = http.getStreamPtr();
        stream->readBytes(buff, len);
        
        Serial.println("[HTTP] Decoding and drawing...");
        decodeAndDraw(buff, len);
      } else if (len > MAX_SPRITE_SIZE) {
        Serial.printf("[HTTP] Error: Payload size %d exceeds max size %d\n", len, MAX_SPRITE_SIZE);
      } else {
        Serial.println("[HTTP] Warning: Empty or invalid response from server.");
      }
    }
  } else {
    Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end();

  Serial.println("Waiting 5 seconds before next request...");
  delay(5000); 
}
