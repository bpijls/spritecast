let gridSize = 8;
let paletteSize = 16;
let cellSize = 32;
let encoder;
let uiDiv, rawDiv, compressedDiv, colorPicker;
let grid, palette;

function preload() {
  loadStyle("style.css");
}

function setup() {
  encoder = new ImageEncoder();
  createCanvas(cellSize * gridSize, cellSize * (gridSize + 2));

  palette = new Palette(paletteSize, cellSize);
  grid = new Grid(gridSize, palette);

  uiDiv = createDiv().id("UI");
  rawDiv = createDiv().parent(uiDiv);
  compressedDiv = createDiv().parent(uiDiv);
  colorPicker = createColorPicker(palette.colors[0]).parent(uiDiv);
  colorPicker.input(() => {
    palette.setColor(palette.currentColorIndex, color(colorPicker.value()));
    redraw();
  });

  noLoop();
  updateDisplay();
}

function draw() {
  background(240);
  grid.draw();
  palette.draw();
}

function mousePressed() {
  handleDraw();
}

function mouseDragged() {
  handleDraw();
}

function handleDraw() {
  if (grid.handleInput(mouseX, mouseY)) {
    updateDisplay();
  } else if (palette.handleInput(mouseX, mouseY)) {
    colorPicker.value(palette.colors[palette.currentColorIndex].toString('#rrggbb'));
    redraw();
  }
}

function updateDisplay() {
  rawDiv.html(
    '<b>Raw:</b> ' + grid.pixels.map(n => n.toString(16).padStart(2, '0')).join(' ')
  );

  const encoded = encoder.encode(grid.pixels);
  const paletteHex = Array.from(encoded.slice(1, 1 + Math.ceil(encoded[0] / 2)))
    .map(b => b.toString(16).padStart(2, '0')).join(' ');
  const dataHex = Array.from(encoded.slice(1 + Math.ceil(encoded[0] / 2)))
    .map(b => b.toString(16).padStart(2, '0')).join(' ');

  compressedDiv.html(
    `<b>Compressed:</b><br>Palette: ${paletteHex}<br>Data: ${dataHex}` +
    `<br><b>Compression Ratio:</b> ${(encoded.length / 64 * 100).toFixed(1)}%`
  );
  redraw();
}

class Grid {
  constructor(size, palette) {
    this.size = size;
    this.palette = palette;
    this.cellSize = cellSize;
    this.pixels = new Array(size * size).fill(0);
  }

  draw() {
    for (let y = 0; y < this.size; y++) {
      for (let x = 0; x < this.size; x++) {
        let idx = y * this.size + x;
        fill(this.palette.colors[this.pixels[idx]]);
        stroke(0);
        rect(x * this.cellSize, y * this.cellSize, this.cellSize, this.cellSize);
      }
    }
  }

  handleInput(mx, my) {
    let x = floor(mx / this.cellSize);
    let y = floor(my / this.cellSize);
    if (x >= 0 && x < this.size && y >= 0 && y < this.size) {
      let idx = y * this.size + x;
      if (this.pixels[idx] !== this.palette.currentColorIndex) {
        this.pixels[idx] = this.palette.currentColorIndex;
        return true;
      }
    }
    return false;
  }
}

class Palette {
  constructor(size, cellSize) {
    this.size = size;
    this.cellSize = cellSize;
    this.colors = [];
    for (let i = 0; i < size; i++) {
      this.colors.push(color(`hsl(${(i * 360) / size}, 100%, 50%)`));
    }
    this.currentColorIndex = 0;
  }

  draw() {
    for (let i = 0; i < this.size; i++) {
      let px = (i % 8) * this.cellSize;
      let py = floor(i / 8) * this.cellSize + gridSize * this.cellSize;
      fill(this.colors[i]);
      stroke(this.currentColorIndex === i ? 255 : 0);
      rect(px, py, this.cellSize, this.cellSize);
    }
  }

  handleInput(mx, my) {
    let y = floor(my / this.cellSize) - gridSize;
    let x = floor(mx / this.cellSize);
    if (y >= 0 && y < 2 && x >= 0 && x < 8) {
      let idx = y * 8 + x;
      if (idx < this.size && this.currentColorIndex !== idx) {
        this.currentColorIndex = idx;
        return true;
      }
    }
    return false;
  }

  setColor(index, newColor) {
    this.colors[index] = newColor;
  }
}

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
