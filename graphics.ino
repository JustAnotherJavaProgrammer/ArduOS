const int screen_width = 640;
const int screen_height = 480;
int vertical_top_boundary = 0;
int vertical_bottom_boundary = screen_height;
// A larger cache results in better performance of your display connection but less memory for applications.
const byte cacheSize = 255;

byte operationsColorCache[cacheSize];
byte operationsPositionCache[cacheSize][2];
byte cachedOperations = 0;

void clear(unsigned long color) {
  Serial.print(F("C"));
  Serial.println(color);
}

void clear(byte r, byte g, byte b, byte a) {
  clear(rgbaToLong(r, g, b, a));
}

void clear(byte r, byte g, byte b) {
  clear(rgbToLong(r, g, b));
}

void clear() {
  clear(255ul);
}

void clearSlow(unsigned long color) {
  fillRect(0, 0, screen_width, screen_height, color);
}

void clearSlow(byte r, byte g, byte b, byte a) {
  fillRect(0, 0, screen_width, screen_height, rgbaToLong(r, g, b, a));
}

void clearSlow(byte r, byte g, byte b) {
  fillRect(0, 0, screen_width, screen_height, rgbToLong(r, g, b));
}

void clearSlow() {
  clearSlow(255ul);
}

void commitOperations() {
  if (cachedOperations == 0)
    return;
  String command = String(F("P"));
  for (byte i = 0; i < cachedOperations; i++) {
    command += F(";");
    command += operationsPositionCache[i][0];
    command += F(",");
    command += operationsPositionCache[i][1];
    command += F(",");
    command += operationsColorCache[i];
  }
  Serial.println(command);

  // Blink every time, pixels are committed to the screen
  static bool high = false;
  digitalWrite(LED_BUILTIN, high ? HIGH : LOW);
  high = !high;
}

// byte rgbaToByte(byte r, byte g, byte b, byte a) {
//  return (r << 6) | (g << 4) | (b << 2) | (a);
//}

//byte rgbToByte(byte r, byte g, byte b) {
//  return rgbaToByte(r, g, b, B11);
//}

unsigned long rgbaToLong(byte r, byte g, byte b, byte a) {
  return (r << 24) | (g << 16) | (b << 8) | (a);
}

unsigned long rgbToLong(byte r, byte g, byte b) {
  return rgbaToLong(r, g, b, 255);
}

void setPixel(int x, int y, unsigned long color) {
  x += vertical_top_boundary;
  if (x < 0 || x >= screen_width || y < vertical_top_boundary || y >= vertical_bottom_boundary)
    return;
  operationsColorCache[cachedOperations] = color;
  operationsPositionCache[cachedOperations][0] = x;
  operationsPositionCache[cachedOperations][1] = y;
  cachedOperations++;
  if (cachedOperations >= cacheSize)
    commitOperations();
}

void setPixel(int x, int y, byte r, byte g, byte b, byte a) {
  return setPixel(x, y, rgbaToLong(r, g, b, a));
}

void setPixel(int x, int y, byte r, byte g, byte b) {
  setPixel(x, y, rgbToLong(r, g, b));
}

// Bresenham's line drawing algorithm, code partially copied from and based on https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
void drawLine(int startx, int starty, int endx, int endy, unsigned long color, int err, bool defaultErr) {
  int x0 = min(startx, endx);
  int x1 = max(startx, endx);
  int y0 = min(starty, endy);
  int y1 = max(starty, endy);
  int dx = x1 - x0;
  int dy = y0 - y1;
  short sx = x0 < x1 ? 1 : -1;
  short sy = y0 < y1 ? 1 : -1;
  if (defaultErr)
    err = dx + dy;
  while (true) {
    setPixel(x0, y0, color);
    if (x0 == x1 && y0 == y1)
      break;
    int e2 = err * 2;
    if (e2 >= dy) {
      err += dy;
      x0 += sx;
    }
    if (e2 <= dx) {
      err += dx;
      y0 += sy;
    }
  }
}

void drawLine(int startx, int starty, int endx, int endy, unsigned long color) {
  drawLine(startx, starty, endx, endy, color, 0, true);
}

void drawLine(int startx, int starty, int endx, int endy, unsigned long color, int thickness) {
  // HELP
}

void drawRect(int x, int y, int width, int height, unsigned long color) {
  drawLine(x, y, x + width, y, color);
  drawLine(x, y, x, y + height, color);
  drawLine(x + width, y, x + width, y + height, color);
  drawLine(x, y + height, x + width, y + height, color);
}

void fillRect(int x, int y, int height, int width, unsigned long color) {
  for (int cx = x; cx < x + width; cx++) {
    for (int cy = y; cy < y + height; cy++) {
      setPixel(cx, cy, color);
    }
  }
}
