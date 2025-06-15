#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <WiFi.h>
#include <WiFiMulti.h>
#include <Preferences.h>
#include "PixelGrid.h"

class WiFiManager {
public:
    WiFiManager(PixelGrid* grid);
    
    // Initialize WiFi connection
    bool begin();
    
    // Enter configuration mode
    void enterConfigMode();
    
    // Check if WiFi is connected
    bool isConnected();
    
    // Get current IP address
    String getIPAddress();

private:
    PixelGrid* pixelGrid;
    WiFiMulti wifiMulti;
    Preferences preferences;
    
    void loadCredentials(String& ssid, String& password);
    void saveCredentials(const String& ssid, const String& password);
};

#endif // WIFI_MANAGER_H 