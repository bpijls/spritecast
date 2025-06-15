#ifndef PIXELGRID_H
#define PIXELGRID_H

#include <Adafruit_NeoPixel.h>
#include <stdint.h>
#include "config.h"

class PixelGrid {
public:
    uint8_t width;
    uint8_t height;
    uint8_t* grid_data; // 1D array for the grid
    uint32_t palette[PALETTE_SIZE];
    Adafruit_NeoPixel* strip_obj; // Renamed to avoid conflict with strip in .ino

    PixelGrid(uint8_t w, uint8_t h, Adafruit_NeoPixel* s);
    ~PixelGrid();

    void setPixel(uint8_t x, uint8_t y, uint8_t paletteIndex);
    uint8_t getPixel(uint8_t x, uint8_t y);
    void setPaletteColor(uint8_t index, uint32_t color);
    void fillGrid(uint8_t paletteIndex);
    void draw();
    void setBrightness(uint8_t brightness);
};

#endif // PIXELGRID_H 