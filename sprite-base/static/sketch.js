let gridSize = 8;
let paletteSize = 16;
let cellSize = 32;
let encoder;
let uiDiv, rawDiv, compressedDiv, colorPicker, nameInput, saveButton;
let grid, palette;

function preload() {
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

  nameInput = createInput('').parent(uiDiv);
  nameInput.attribute('placeholder', 'Sprite Name');

  saveButton = createButton('Save Sprite').parent(uiDiv);
  saveButton.mousePressed(saveSprite);

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

  const encoded = encoder.encode(grid.pixels, palette.colors);
  const dataHex = Array.from(encoded)
    .map(b => b.toString(16).padStart(2, '0')).join(' ');

  compressedDiv.html(
    `<b>Compressed:</b><br>Data: ${dataHex}` +
    `<br><b>Compression Ratio:</b> ${(encoded.length / 64 * 100).toFixed(1)}%`
  );
  redraw();
}

function saveSprite() {
  const spriteName = nameInput.value();
  if (!spriteName) {
    alert('Please enter a name for the sprite.');
    return;
  }

  const encodedData = encoder.encode(grid.pixels, palette.colors);

  fetch(`http://localhost:5000/sprite/${spriteName}`, {
    method: 'POST',
    headers: {
      'Content-Type': 'application/octet-stream',
    },
    body: encodedData,
  })
  .then(response => response.json())
  .then(data => {
    console.log(data);
    alert(data.message || data.error);
  })
  .catch(error => {
    console.error('Error:', error);
    alert('Failed to save sprite.');
  });
}
