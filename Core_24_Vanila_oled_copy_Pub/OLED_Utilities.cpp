/* 
Developed with Gemini
           ---
           )|(
          <-o->
           )|(
           ---
*/
#include <Wire.h>  // I2C communication library
#include "OLED_Utilities.h"
#include "SensorHandler.h"
// The constructor initializes the Adafruit_SSD1306 object
OLED_Utilities oled;
QRCode qrcode;
bool inv = false;

OLED_Utilities::OLED_Utilities()
  : _display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET),
    _textLineHeight(8)  // Default height for font size 1
{
  ;  // Nothing else needed here, initialization happens in begin()
}

bool OLED_Utilities::begin() {
  // SSD1306_SWITCHCAPVCC generates the high voltage internally
  if (!_display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDRESS)) {
    // Serial is helpful for debugging if the library user includes it
    // Serial.println(F("SSD1306 allocation failed"));
    return false;
  }
  _display.setRotation(0);
  _display.setTextWrap(false);  // Disable text wrapping -----------------------------------------
  _display.setTextColor(SSD1306_WHITE);
  _display.clearDisplay();
  _display.display();

  return true;
}

void OLED_Utilities::clear() {
  _display.clearDisplay();
  _display.display();
}

void OLED_Utilities::invert(bool inv) {
  _display.invertDisplay(inv);
}

void OLED_Utilities::genQR(String msg, unsigned long int tt) {

  msg += ESPid;
  char qrData[128];
  msg.toCharArray(qrData, sizeof(qrData));
  uint8_t qrcodeData[qrcode_getBufferSize(3)];
  invert(1);
  clear();
  qrcode_initText(&qrcode, qrcodeData, 2, 0, qrData);
  int scale = min(SCREEN_WIDTH / qrcode.size, SCREEN_HEIGHT / qrcode.size) - 1;  // 2 colors screen
  int offsetX = (SCREEN_WIDTH - (qrcode.size * scale)) / 2;
  int offsetY = (SCREEN_HEIGHT - (qrcode.size * scale)) / 2 + 12;

  for (uint8_t y = 0; y < qrcode.size; y++) {
    for (uint8_t x = 0; x < qrcode.size; x++) {
      if (qrcode_getModule(&qrcode, x, y)) {
        // Draw each module as a block for easier scanning
        _display.fillRect(offsetX + (x * scale), offsetY + (y * scale), scale, scale, SSD1306_WHITE);
      }
    }
  }
  _display.display();
  delay(tt);
  invert(0);
  clear();
  oled.printLine(ESPid.c_str(), 0, 1);
  _display.display();
}

void OLED_Utilities::printLine(const char* text, uint8_t row, uint8_t size, bool invert) {
  // Calculate the top Y position for the given row
  int16_t yPos = row * (_textLineHeight * size);

  // Check if the calculated position is within screen bounds
  if (yPos >= SCREEN_HEIGHT) return;

  // Clear the area where the text will be displayed (optional, but ensures a clean background)
  _display.fillRect(0, yPos, SCREEN_WIDTH, _textLineHeight * size, SSD1306_BLACK);

  // Set cursor position (X=0 for left-justified)
  _display.setCursor(0, yPos);

  // Set text size and color
  _display.setTextSize(size);

  if (invert) {
    _display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);  // Black text on White background
  } else {
    _display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);  // White text on Black background
  }

  _display.print(text);

  // Reset color to standard white on black for subsequent drawing
  _display.setTextColor(SSD1306_WHITE);

  _display.display();  // Push the changes to the screen
}

void OLED_Utilities::drawProgressBar(int16_t y,int x, int16_t width, int16_t height, uint8_t percentage) {
  if (percentage > 100) percentage = 100;

  // 1. Draw the empty frame (outline)
  _display.drawRect(x, y, width, height, SSD1306_WHITE);

  // 2. Calculate the filled width
  // The fill area should be slightly smaller than the frame for better aesthetics
  int16_t fill_x = x+1;                 // Start one pixel in from the left edge
  int16_t fill_y = y + 1;             // Start one pixel down from the top edge
  int16_t inner_width = width - 2;    // Total inner width
  int16_t inner_height = height - 2;  // Total inner height

  // Calculate the width of the filled portion
  int16_t filled_width = map(percentage, 0, 100, 0, inner_width);

  // 3. Draw the filled portion
  _display.fillRect(fill_x, fill_y, filled_width, inner_height, SSD1306_WHITE);

  // 4. Clear the remaining empty portion (optional, but good practice)
  _display.fillRect(fill_x + filled_width, fill_y, inner_width - filled_width, inner_height, SSD1306_BLACK);

  // 5. Display the percentage text over the bar (optional)
  _display.setTextSize(1);
  _display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);  // Set text color

  // Create a string for the percentage
  char percent_str[5];  // Max 4 characters: "100%"
  sprintf(percent_str, "%d%%", percentage);

  // Center the text within the bar area
  int16_t x1, y1;
  uint16_t w, h;
  _display.getTextBounds(percent_str, 0, 0, &x1, &y1, &w, &h);

  _display.setCursor(x+(width - w) / 2, y + (height - h) / 2);
  _display.print(percent_str);

  _display.display();  // Push the changes to the screen
}