class ImageEncoder {
  encode(pixels) {
    const paletteSet = Array.from(new Set(pixels));
    const paletteSize = paletteSet.length;

    let colorMap = new Map();
    paletteSet.forEach((c, i) => colorMap.set(c, i));

    let output = [];
    output.push(paletteSize);

    for (let i = 0; i < paletteSize; i += 2) {
      let hi = paletteSet[i] & 0x0F;
      let lo = (paletteSet[i + 1] || 0) & 0x0F;
      output.push((hi << 4) | lo);
    }

    let i = 0;
    while (i < pixels.length) {
      let color = pixels[i];
      let run = 1;
      while (i + run < pixels.length && pixels[i + run] === color && run < 16) {
        run++;
      }
      output.push(((run & 0x0F) << 4) | (colorMap.get(color) & 0x0F));
      i += run;
    }

    return Uint8Array.from(output);
  }

  decode(bytes) {
    const paletteSize = bytes[0];
    const palette = [];

    for (let i = 0; i < Math.ceil(paletteSize / 2); i++) {
      const byte = bytes[1 + i];
      palette.push((byte >> 4) & 0x0F);
      if (palette.length < paletteSize) palette.push(byte & 0x0F);
    }

    let pixels = [];
    let i = 1 + Math.ceil(paletteSize / 2);
    while (pixels.length < 64 && i < bytes.length) {
      const byte = bytes[i++];
      const run = (byte >> 4) & 0x0F;
      const color = palette[byte & 0x0F];
      for (let j = 0; j < run; j++) {
        pixels.push(color);
      }
    }

    return pixels;
  }
} 