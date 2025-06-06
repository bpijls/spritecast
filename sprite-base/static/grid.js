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