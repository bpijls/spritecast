#include "PixelGrid.h"

PixelGrid::PixelGrid(uint8_t w, uint8_t h, Adafruit_NeoPixel* s) 
    : width(w), height(h), strip_obj(s) {
    grid_data = new uint8_t[w * h];
    // Initialize grid with a default value (e.g., 0)
    for (int i = 0; i < w * h; ++i) {
        grid_data[i] = 0;
    }

    // Initialize a basic palette
    for (int i = 0; i < PALETTE_SIZE; ++i) {
        palette[i] = 0; // Default to black
    }
    if (PALETTE_SIZE > 1) {
        palette[0] = strip_obj->Color(0,0,0); // Black
        palette[1] = strip_obj->Color(255, 0, 0); // Red
        palette[2] = strip_obj->Color(0, 255, 0); // Green
        palette[3] = strip_obj->Color(0, 0, 255); // Blue
        palette[4] = strip_obj->Color(255,255,255); // White
    }
}

PixelGrid::~PixelGrid() {
    delete[] grid_data;
}

void PixelGrid::setPixel(uint8_t x, uint8_t y, uint8_t paletteIndex) {
    if (x < width && y < height && paletteIndex < PALETTE_SIZE) {
        grid_data[y * width + x] = paletteIndex;
    }
}

uint8_t PixelGrid::getPixel(uint8_t x, uint8_t y) {
    if (x < width && y < height) {
        return grid_data[y * width + x];
    }
    return 0; // Or some other default/error indicator
}

void PixelGrid::setPaletteColor(uint8_t index, uint32_t color) {
    if (index < PALETTE_SIZE) {
        palette[index] = color;
    }
}

void PixelGrid::fillGrid(uint8_t paletteIndex) {
    if (paletteIndex < PALETTE_SIZE) {
        for (int i = 0; i < width * height; ++i) {
            grid_data[i] = paletteIndex;
        }
    }
}

void PixelGrid::draw() {
    for (uint8_t y = 0; y < height; ++y) {
        for (uint8_t x = 0; x < width; ++x) {
            int pixelIndex;
            // Assuming serpentine layout, adjust if your wiring is different.
            if (y % 2 == 0) { // Even rows, left to right
                pixelIndex = y * width + x;
            } else { // Odd rows, right to left (serpentine)
                pixelIndex = y * width + (width - 1 - x);
            }
            
            if (pixelIndex < strip_obj->numPixels()) {
                uint8_t paletteIndex = grid_data[y * width + x];
                strip_obj->setPixelColor(pixelIndex, palette[paletteIndex]);
            }
        }
    }
    strip_obj->show();
} 