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