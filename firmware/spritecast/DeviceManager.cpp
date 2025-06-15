#include "DeviceManager.h"

DeviceManager::DeviceManager(PixelGrid* grid) : pixelGrid(grid) {}

void DeviceManager::displayDeviceId() {
    uint8_t mac[6];
    WiFi.macAddress(mac);
    
    uint8_t checksum[2];
    calculateChecksum(mac, checksum);
    
    // Display each byte as a row of bits
    for(int row = 0; row < 6; row++) {
        for(int col = 0; col < 8; col++) {
            bool bit = (mac[row] >> (7-col)) & 0x01;
            pixelGrid->setPixel(col, row, bit ? 1 : 0);
        }
    }
    
    // Display checksum in last two rows
    for(int row = 6; row < 8; row++) {
        for(int col = 0; col < 8; col++) {
            bool bit = (checksum[row-6] >> (7-col)) & 0x01;
            pixelGrid->setPixel(col, row, bit ? 1 : 0);
        }
    }
    
    pixelGrid->draw();
}

String DeviceManager::getDeviceId() {
    uint8_t mac[6];
    WiFi.macAddress(mac);
    
    String id = "";
    for(int i = 0; i < 6; i++) {
        if(mac[i] < 0x10) id += "0";
        id += String(mac[i], HEX);
    }
    return id;
}

void DeviceManager::getDeviceIdBytes(uint8_t* buffer) {
    WiFi.macAddress(buffer);
}

void DeviceManager::calculateChecksum(const uint8_t* mac, uint8_t* checksum) {
    checksum[0] = 0;
    for(int i = 0; i < 6; i++) {
        checksum[0] ^= mac[i];
    }
    checksum[1] = ~checksum[0];
} 