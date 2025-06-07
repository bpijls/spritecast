class ImageEncoder {
  encode(pixels, p5Colors) {
    const paletteIndices = Array.from(new Set(pixels));
    const paletteSize = paletteIndices.length;

    if (paletteSize > 15) {
      alert("Sprite can't have more than 15 unique colors.");
      return new Uint8Array();
    }

    const imageColors = paletteIndices.map(index => p5Colors[index]);
    const colorMap = new Map();
    paletteIndices.forEach((originalIndex, newIndex) => colorMap.set(originalIndex, newIndex));

    let nibbles = [];

    // 1. Palette Size (1 nibble)
    nibbles.push(paletteSize);

    // 2. Palette Colors (n * 3 nibbles: 4-bit R, G, B)
    for (const p5Color of imageColors) {
      const r = p5Color.levels[0] >> 4;
      const g = p5Color.levels[1] >> 4;
      const b = p5Color.levels[2] >> 4;
      nibbles.push(r, g, b);
    }

    // 3. Pixel Runs (run, colorIndex)
    let i = 0;
    while (i < pixels.length) {
      const color = pixels[i];
      let run = 1;
      while (i + run < pixels.length && pixels[i + run] === color && run < 15) {
        run++;
      }
      
      const mappedColorIndex = colorMap.get(color);
      nibbles.push(run, mappedColorIndex);
      i += run;
    }
    
    // Pack nibbles into bytes
    const output = [];
    for (let j = 0; j < nibbles.length; j += 2) {
      const hi = nibbles[j];
      const lo = (j + 1 < nibbles.length) ? nibbles[j + 1] : 0;
      output.push((hi << 4) | lo);
    }

    return Uint8Array.from(output);
  }

  decode(bytes) {
    if (!bytes || bytes.length === 0) {
      return { pixels: [], palette: [] };
    }
    
    const nibbles = [];
    for (const byte of bytes) {
      nibbles.push(byte >> 4);
      nibbles.push(byte & 0x0F);
    }

    const paletteSize = nibbles.shift();
    
    const palette = [];
    if (paletteSize > 0) {
        const paletteNibbleCount = paletteSize * 3;
        if (nibbles.length < paletteNibbleCount) {
          console.error("Invalid data: not enough data for palette");
          return { pixels: [], palette: [] };
        }
        const paletteNibbles = nibbles.splice(0, paletteNibbleCount);

        for (let i = 0; i < paletteNibbleCount; i += 3) {
            // Scale 4-bit color to 8-bit (e.g., 0xF -> 0xFF)
            const r = (paletteNibbles[i] << 4) | paletteNibbles[i];
            const g = (paletteNibbles[i + 1] << 4) | paletteNibbles[i + 1];
            const b = (paletteNibbles[i + 2] << 4) | paletteNibbles[i + 2];
            palette.push(color(r, g, b));
        }
    }
    
    const pixels = [];
    while (pixels.length < 64 && nibbles.length >= 2) {
      const run = nibbles.shift();
      const colorIndex = nibbles.shift();

      if (colorIndex >= paletteSize) {
        console.error("Invalid data: color index out of bounds");
        continue;
      }

      for (let j = 0; j < run; j++) {
        if (pixels.length < 64) {
          pixels.push(colorIndex);
        }
      }
    }

    return { pixels, palette };
  }
} 