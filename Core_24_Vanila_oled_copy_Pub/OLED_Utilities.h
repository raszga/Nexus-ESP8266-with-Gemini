/* 
Developed with Gemini
           ---
           )|(
          <-o->
           )|(
           ---
*/
#ifndef OLED_UTILITIES_H
#define OLED_UTILITIES_H
#include <Arduino.h>
#include <Adafruit_GFX.h>      // Core graphics library
#include <Adafruit_SSD1306.h>  // LCD driver
#include <qrcode.h>
// Define the standard screen size (matching your initial setup)
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
// An I2C address is needed to initialize the display. Common addresses are 0x3C or 0x3D.
// Check your display module for the correct one. 0x3C is used as a default here.
#define OLED_I2C_ADDRESS 0x3C
// Reset pin is typically not used in 4-wire I2C setup, so we use -1
#define OLED_RESET -1

class OLED_Utilities {
public:
  /**
     * @brief Constructor: Initializes the display object.
     */
  OLED_Utilities();

  /**
     * @brief Starts the display hardware connection and clears the buffer.
     * @return true if initialization was successful, false otherwise.
     */
  bool begin();

  /**
     * @brief Clears the screen buffer and shows the content.
     */
  void clear();


  void invert(bool inv);

  void genQR(String msg, unsigned long int tt);

  /**
     * @brief Prints a line of text to the specified row.
     * @param text The string to display.
     * @param row The line number (0-indexed, 0=top, 7=bottom for 64-pixel display).
     * @param size The font size multiplier (e.g., 1 for 6x8 font, 2 for 12x16 font).
     * @param invert If true, display text in black on white background.
     */
  void printLine(const char* text, uint8_t row, uint8_t size = 1, bool invert = false);

  /**
     * @brief Draws a progress bar.
     * @param y The top-left y-coordinate for the bar.
     * @param width The total width of the bar.
     * @param height The height of the bar.
     * @param percentage The percentage filled (0-100).
     */
  void drawProgressBar(int16_t y,int x, int16_t width, int16_t height, uint8_t percentage);

  /**
     * @brief Provides direct access to the underlying Adafruit_SSD1306 object.
     * @return A reference to the display object.
     */
  Adafruit_SSD1306& getDisplay() {
    return _display;
  }

private:
  Adafruit_SSD1306 _display;
  uint8_t _textLineHeight;
};
extern OLED_Utilities oled;
extern QRCode qrcode;
extern const char* qrData;
extern bool inv;
#endif  // OLED_UTILITIES_H