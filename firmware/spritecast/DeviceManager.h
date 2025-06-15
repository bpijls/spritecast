#ifndef DEVICE_MANAGER_H
#define DEVICE_MANAGER_H

#include <WiFi.h>
#include "PixelGrid.h"

class DeviceManager {
public:
    DeviceManager(PixelGrid* grid);
    
    // Display the device ID on the pixel grid
    void displayDeviceId();
    
    // Get the device ID as a hex string
    String getDeviceId();
    
    // Get the device ID as raw bytes
    void getDeviceIdBytes(uint8_t* buffer);

private:
    PixelGrid* pixelGrid;
    void calculateChecksum(const uint8_t* mac, uint8_t* checksum);
};

#endif // DEVICE_MANAGER_H 