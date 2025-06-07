#include <Adafruit_NeoPixel.h>
#include "PixelGrid.h"
#include "config.h"

#include <WiFi.h>
#include <WiFiMulti.h>

#include <HTTPClient.h>
#include <Preferences.h>
#include <WiFiClientSecure.h>

const char* root_ca = R"string_literal(
-----BEGIN CERTIFICATE-----
MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4
WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu
ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY
MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc
h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+
0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U
A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW
T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH
B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC
B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv
KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn
OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn
jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw
qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI
rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV
HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq
hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL
ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ
3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK
NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5
ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur
TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC
jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc
oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq
4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA
mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d
emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=
-----END CERTIFICATE-----
)string_literal";


// --- WiFi Credentials ---
// Credentials are loaded from NVS. Hold both buttons on startup to enter config mode.
String ssid;
String password;
// -------------------------

Preferences preferences;

// Declare our NeoPixel strip object:
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
// Declare our PixelGrid object
PixelGrid pixelGrid(GRID_WIDTH, GRID_HEIGHT, &strip);
WiFiMulti WiFiMulti;

void enterConfigMode() {
  Serial.println("Entering configuration mode.");

  // Simple pixel animation to show config mode is active.
  pixelGrid.fillGrid(0);
  for (int i=0; i < GRID_WIDTH; i++) {
    pixelGrid.setPixel(i, 0, 4); // White color from default palette
    pixelGrid.draw();
    delay(50);
  }

  Serial.println("Please enter WiFi SSID:");
  while (!Serial.available()) {
    delay(100);
  }
  String newSsid = Serial.readStringUntil('\n');
  newSsid.trim();
  Serial.print("SSID set to: ");
  Serial.println(newSsid);

  Serial.println("Please enter WiFi Password:");
  while (!Serial.available()) {
    delay(100);
  }
  String newPassword = Serial.readStringUntil('\n');
  newPassword.trim();
  Serial.println("Password set."); // Don't print password to serial

  preferences.begin(PREFERENCES_NAMESPACE, false);
  preferences.putString(SSID_KEY, newSsid);
  preferences.putString(PASSWORD_KEY, newPassword);
  preferences.end();

  Serial.println("Credentials saved. Restarting in 3 seconds...");
  delay(3000);
  ESP.restart();
}

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  delay(10); 
  Serial.println("\nStarting up...");

  strip.begin();
  strip.show(); 
  strip.setBrightness(BRIGHTNESS); 

  pinMode(BUTTON_1_PIN, INPUT_PULLUP);
  pinMode(BUTTON_2_PIN, INPUT_PULLUP);

  delay(100); // Small delay for buttons to settle

  if (digitalRead(BUTTON_1_PIN) == LOW && digitalRead(BUTTON_2_PIN) == LOW) {
    enterConfigMode();
  }
  
  preferences.begin(PREFERENCES_NAMESPACE, true); // read-only
  ssid = preferences.getString(SSID_KEY, "");
  password = preferences.getString(PASSWORD_KEY, "");
  preferences.end();

  if (ssid.length() == 0) {
    Serial.println("No WiFi credentials found.");
    enterConfigMode();
  }


   // We start by connecting to a WiFi network
  WiFiMulti.addAP(ssid.c_str(), password.c_str());
  WiFi.setTxPower(WIFI_TX_POWER);
  Serial.println();
  Serial.println();
  Serial.print("Waiting for WiFi... ");

  unsigned long startTime = millis();
  while (WiFiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
    if (millis() - startTime > 30000) { // 30 second timeout
        Serial.println("\nFailed to connect to WiFi.");
        enterConfigMode();
    }
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
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
  WiFiClientSecure client;
  client.setCACert(root_ca);

  HTTPClient http;

  Serial.print("[HTTPS] begin...\n");
  http.begin(client, DATA_SERVER_URL); 

  Serial.print("[HTTPS] GET...\n");
  int httpCode = http.GET();

  if (httpCode > 0) {
    Serial.printf("[HTTPS] GET... code: %d\n", httpCode);

    if (httpCode == HTTP_CODE_OK) {
      int len = http.getSize();
      Serial.printf("[HTTPS] Payload size: %d\n", len);

      if (len > 0 && len <= MAX_SPRITE_SIZE) {
        uint8_t buff[MAX_SPRITE_SIZE] = {0};
        WiFiClient * stream = http.getStreamPtr();
        stream->readBytes(buff, len);
        
        Serial.println("[HTTPS] Decoding and drawing...");
        decodeAndDraw(buff, len);
      } else if (len > MAX_SPRITE_SIZE) {
        Serial.printf("[HTTPS] Error: Payload size %d exceeds max size %d\n", len, MAX_SPRITE_SIZE);
      } else {
        Serial.println("[HTTPS] Warning: Empty or invalid response from server.");
      }
    }
  } else {
    Serial.printf("[HTTPS] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end();

  Serial.println("Waiting 5 seconds before next request...");
  delay(REQUEST_DELAY); 
}
