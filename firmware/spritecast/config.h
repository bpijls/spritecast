#pragma once

// --- WiFi Credentials ---
// Replace with your network details and rename this file to config.h
// #define WIFI_SSID "YOUR_WIFI_SSID"
// #define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"

// --- Buttons ---
#define BUTTON_1_PIN 0
#define BUTTON_2_PIN 1

// --- NVS Storage ---
#define PREFERENCES_NAMESPACE "wifi_creds"
#define SSID_KEY "ssid"
#define PASSWORD_KEY "password"

// --- Display Settings ---
#define LED_PIN    4
#define LED_COUNT 64
#define BRIGHTNESS 10 // Set brightness (0-255)
#define GRID_WIDTH 8 
#define GRID_HEIGHT 8 
#define PALETTE_SIZE 16

// --- Network Settings ---
#define DATA_SERVER_URL "http://192.168.2.117:5001/sprite/random" 
#define REQUEST_DELAY 5000 // Delay between requests in ms
#define WIFI_TX_POWER WIFI_POWER_8_5dBm // Set WiFi TX power

// --- Firmware Settings ---
#define SERIAL_BAUD_RATE 115200
#define MAX_SPRITE_SIZE 256

// ------------------------- 