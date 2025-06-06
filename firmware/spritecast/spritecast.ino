#include <Adafruit_NeoPixel.h>
#include "PixelGrid.h"

#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager

// For Base64 decoding - A simple implementation or include a library
// For this example, we'll outline where it's used.
// You might need to add a library like "Base64.h" from Arduino's library manager
// or use mbedtls for ESP32 if available and preferred.
// For simplicity, I'll sketch a basic decoding logic conceptual step.
// A robust library is recommended for production.

// --- Simple Base64 Decode Function (Illustrative) ---
// This is a very basic decoder and might not cover all edge cases.
// Consider using a well-tested library.
String base64Decode(String base64Str) {
    String decodedStr = "";
    // This is a placeholder. A real implementation is more complex.
    // For ESP-IDF, you can use mbedtls_base64_decode.
    // For Arduino, a library like "arduino-base64" by Adam Rudd can be used.
    // Example:
    // int inputLen = base64Str.length();
    // char decodedChars[inputLen]; // Approximation of output size
    // int decodedLen = decode_base64((unsigned char*)base64Str.c_str(), inputLen, (unsigned char*)decodedChars);
    // if (decodedLen > 0) decodedStr = String(decodedChars).substring(0, decodedLen);
    
    // Simulate decoding for now, replace with actual decoder
    if (base64Str.startsWith("SIMULATED_")) { // Placeholder check
        // This is NOT real Base64 decoding
        String temp = base64Str.substring(10);
        for(unsigned int i=0; i < temp.length(); i+=2) {
            char hex[3] = {temp.charAt(i), temp.charAt(i+1), '\0'};
            decodedStr += (char)strtol(hex, NULL, 16);
        }
    } else {
        Serial.println("ERROR: Base64 decoding not fully implemented. Please add a Base64 library.");
    }
    return decodedStr;
}
// --- End Simple Base64 Decode Function ---


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

// Helper function to convert two hex chars to a byte
uint8_t hexToByte(char c1, char c2) {
    uint8_t value = 0;
    if (c1 >= '0' && c1 <= '9') value = (c1 - '0') << 4;
    else if (c1 >= 'A' && c1 <= 'F') value = (c1 - 'A' + 10) << 4;
    else if (c1 >= 'a' && c1 <= 'f') value = (c1 - 'a' + 10) << 4;

    if (c2 >= '0' && c2 <= '9') value |= (c2 - '0');
    else if (c2 >= 'A' && c2 <= 'F') value |= (c2 - 'A' + 10);
    else if (c2 >= 'a' && c2 <= 'f') value |= (c2 - 'a' + 10);
    return value;
}

void parseAndApplyPalette(String palettePayload) {
    int currentIndex = 0;
    int paletteIndex = 0;
    while (currentIndex < palettePayload.length() && paletteIndex < PALETTE_SIZE) {
        String colorHex = palettePayload.substring(currentIndex, currentIndex + 6);
        if (colorHex.length() == 6) {
            uint8_t r = hexToByte(colorHex.charAt(0), colorHex.charAt(1));
            uint8_t g = hexToByte(colorHex.charAt(2), colorHex.charAt(3));
            uint8_t b = hexToByte(colorHex.charAt(4), colorHex.charAt(5));
            pixelGrid.setPaletteColor(paletteIndex, strip.Color(r, g, b));
            paletteIndex++;
        }
        currentIndex += 6;
        if (currentIndex < palettePayload.length() && palettePayload.charAt(currentIndex) == ',') {
            currentIndex++; // Skip comma
        }
    }
    Serial.printf("Applied %d colors to palette.\n", paletteIndex);
}

void parseAndApplyGrid(String base64GridData) {
    String decodedData = base64Decode(base64GridData); // Use a proper Base64 decoder here
    
    if (decodedData.length() == 0 && base64GridData.length() > 0) {
        Serial.println("Base64 decoding failed or returned empty. Grid not updated.");
        return;
    }
    
    int expectedBytes = (GRID_WIDTH * GRID_HEIGHT) / 2; // 2 pixels per byte (4-bits per pixel)
    if ((GRID_WIDTH * GRID_HEIGHT) % 2 != 0) {
        expectedBytes++; // If odd number of pixels, one pixel will take a full byte (or half of last byte is unused)
                         // For 4-bit scheme, better to ensure total pixels is even or handle padding.
                         // Assuming GRID_WIDTH * GRID_HEIGHT is even.
    }

    if (decodedData.length() < expectedBytes) {
        Serial.printf("Decoded grid data too short. Expected %d bytes, got %d. Grid not updated.\n", expectedBytes, decodedData.length());
        return;
    }

    int pixelDataIndex = 0;
    for (int i = 0; i < expectedBytes && pixelDataIndex < (GRID_WIDTH * GRID_HEIGHT); ++i) {
        uint8_t byteVal = decodedData.charAt(i);
        
        // First pixel from the high nibble
        uint8_t paletteIdx1 = (byteVal >> 4) & 0x0F;
        uint8_t y1 = pixelDataIndex / GRID_WIDTH;
        uint8_t x1 = pixelDataIndex % GRID_WIDTH;
        pixelGrid.setPixel(x1, y1, paletteIdx1);
        pixelDataIndex++;

        if (pixelDataIndex < (GRID_WIDTH * GRID_HEIGHT)) {
            // Second pixel from the low nibble
            uint8_t paletteIdx2 = byteVal & 0x0F;
            uint8_t y2 = pixelDataIndex / GRID_WIDTH;
            uint8_t x2 = pixelDataIndex % GRID_WIDTH;
            pixelGrid.setPixel(x2, y2, paletteIdx2);
            pixelDataIndex++;
        }
    }
    Serial.printf("Applied grid data. Processed %d pixels from %d decoded bytes.\n", pixelDataIndex, expectedBytes);
}

void fetchAndUpdatePixelData() {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        Serial.print("[HTTP] begin...\n");
#if defined(ESP32)
        http.begin(dataServerUrl); 
#elif defined(ESP8266)
        WiFiClient client; // ESP8266 requires WiFiClient
        http.begin(client, dataServerUrl);
#endif
        Serial.print("[HTTP] GET...\n");
        int httpCode = http.GET();

        if (httpCode > 0) {
            Serial.printf("[HTTP] GET... code: %d\n", httpCode);
            if (httpCode == HTTP_CODE_OK) {
                String payload = http.getString();
                Serial.println("Received payload:");
                // Serial.println(payload); // Can be very long, print with caution

                int paletteStart = payload.indexOf("PALETTE:");
                int gridStart = payload.indexOf("GRID:");

                if (paletteStart != -1 && gridStart != -1) {
                    paletteStart += 8; // Length of "PALETTE:"
                    String paletteData = payload.substring(paletteStart, gridStart -1); // -1 to remove trailing newline before GRID:
                    paletteData.trim();
                    Serial.println("Palette Data Extracted: " + paletteData);
                    parseAndApplyPalette(paletteData);

                    gridStart += 5; // Length of "GRID:"
                    String gridData = payload.substring(gridStart);
                    gridData.trim();
                    Serial.println("Grid Data Extracted (Base64): " + gridData.substring(0, min(30, (int)gridData.length())) + "..."); // Print first 30 chars
                    parseAndApplyGrid(gridData);
                    
                    pixelGrid.draw(); // Draw the newly updated grid
                } else {
                    Serial.println("Payload format error. Could not find PALETTE: or GRID: tags.");
                }
            } else {
                Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
            }
        } else {
            Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }
        http.end();
    } else {
        Serial.println("WiFi Disconnected. Cannot fetch data.");
    }
}


void setup() {
  Serial.begin(115200);
  delay(10); 
  Serial.println("\nStarting up...");

  WiFiManager wifiManager;
  // wifiManager.resetSettings(); // Uncomment to erase stored credentials
  if (!wifiManager.autoConnect("PixelBrosAP_Config")) {
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
  
  // Fetch data from server
  fetchAndUpdatePixelData(); 
  // The loop will continue to draw whatever is in pixelGrid
}

void loop() {
  // The main display logic is now in fetchAndUpdatePixelData (called once in setup)
  // and subsequently pixelGrid.draw() updates the display.
  // If you want to periodically update from server, call fetchAndUpdatePixelData() here.
  // For now, it just displays the last fetched/set data.
  
  // To demonstrate it's alive, we can keep the row-filling logic
  // or simply redraw. For server-driven content, redrawing the existing buffer is enough.
  pixelGrid.draw(); // Keep redrawing the current grid state
  delay(1000); 

  // Example: Fetch new data every 60 seconds
  /*
  static unsigned long lastUpdateTime = 0;
  unsigned long currentTime = millis();
  if (currentTime - lastUpdateTime > 60000) { // 60 seconds
      lastUpdateTime = currentTime;
      fetchAndUpdatePixelData();
  }
  */
}
