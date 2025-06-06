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
