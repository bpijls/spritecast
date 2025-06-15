#include "WiFiManager.h"
#include "config.h"

WiFiManager::WiFiManager(PixelGrid* grid) : pixelGrid(grid) {}

bool WiFiManager::begin() {
    String ssid, password;
    loadCredentials(ssid, password);

    if (ssid.length() == 0) {
        Serial.println("No WiFi credentials found.");
        enterConfigMode();
        return false;
    }

    wifiMulti.addAP(ssid.c_str(), password.c_str());
    WiFi.setTxPower(WIFI_TX_POWER);
    
    Serial.print("Waiting for WiFi... ");
    unsigned long startTime = millis();
    while (wifiMulti.run() != WL_CONNECTED) {
        Serial.print(".");
        delay(500);
        if (millis() - startTime > 30000) { // 30 second timeout
            Serial.println("\nFailed to connect to WiFi.");
            enterConfigMode();
            return false;
        }
    }

    Serial.println("\nWiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    return true;
}

void WiFiManager::enterConfigMode() {
    Serial.println("Entering configuration mode.");

    // Simple pixel animation to show config mode is active
    pixelGrid->fillGrid(0);
    for (int i=0; i < GRID_WIDTH; i++) {
        pixelGrid->setPixel(i, 0, 4); // White color from default palette
        pixelGrid->draw();
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

    saveCredentials(newSsid, newPassword);

    Serial.println("Credentials saved. Restarting in 3 seconds...");
    delay(3000);
    ESP.restart();
}

bool WiFiManager::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

String WiFiManager::getIPAddress() {
    return WiFi.localIP().toString();
}

void WiFiManager::loadCredentials(String& ssid, String& password) {
    preferences.begin(PREFERENCES_NAMESPACE, true); // read-only
    ssid = preferences.getString(SSID_KEY, "");
    password = preferences.getString(PASSWORD_KEY, "");
    preferences.end();
}

void WiFiManager::saveCredentials(const String& ssid, const String& password) {
    preferences.begin(PREFERENCES_NAMESPACE, false);
    preferences.putString(SSID_KEY, ssid);
    preferences.putString(PASSWORD_KEY, password);
    preferences.end();
} 