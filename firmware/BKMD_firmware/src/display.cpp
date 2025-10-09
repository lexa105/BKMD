/*
* BKMD Project Display Library
*
* Custom function for TFT_eSPI telemetry. 
* TODO:
* rewrite functions to work with Sprite = offscreen buffer, for offscreen compositions, flicker free transition
*/

#include "display.h"



static TFT_eSPI tft = TFT_eSPI();

#define TERM_GREEN 0x07E0  // čistě zelená (RGB565)
#define TERM_BG    TFT_BLACK

//KEYBOARD MODE
static String keyboard_history[3] = {"", "", ""};

//MOUSE MODE
static int cross_x = -1;
static int cross_y = -1;
static const int CROSS_SIZE = 8;     // half-length of crosshair lines in px
static const int CROSS_THICK = 1;    // line thickness (1 = single-pixel)

void display_init() {
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TERM_BG);
    tft.setTextFont(1); // GLCD 8x8
    tft.setTextColor(TERM_GREEN, TERM_BG);
    tft.setTextDatum(TL_DATUM); // Zarovnání vlevo nahoře
}

//  stavový text 
void display_show_state(const char* state) {
    tft.fillScreen(TERM_BG);
    tft.setTextFont(1); // GLCD 8x8
    tft.setTextColor(TERM_GREEN, TERM_BG);
    tft.setTextDatum(TL_DATUM); // Zarovnání vlevo nahoře
    String line = ">";
    line += state;
    tft.drawString(line, 0, 8); // 8 px odshora pro lepší vzhled
}

//  běžný text (pod stavem)
void display_show_text(const char* text) {
    tft.setTextFont(1); // GLCD 8x8
    tft.setTextColor(TERM_GREEN, TERM_BG);
    tft.setTextDatum(TL_DATUM); // Zarovnání vlevo nahoře
    String line = ">";
    line += text;
    tft.setTextPadding(tft.width());   // <--- ensures old text is blanked
    tft.drawString(line, 0, 24); // 24 px odshora (pod stavem)
}

// Menší běžný text (pod stavem)
void display_show_keyboard_text(const char* text) {
    tft.setTextFont(1); // GLCD 8x8
    tft.setTextColor(TERM_GREEN, TERM_BG);
    tft.setTextDatum(TL_DATUM); // Zarovnání vlevo nahoře
    String line = ">";
    line += text;
    tft.setTextPadding(tft.width());   // <--- ensures old text is blanked
    tft.drawString(line, 0, 40); // 40 px odshora (pod textem)
    
    display_show_keyboard_history(line);
}

void display_show_keyboard_history(String text){
    // Posun historie
    tft.fillRect(0, 56, tft.width(), 8, TERM_BG);
    tft.fillRect(0, 72, tft.width(), 8, TERM_BG);

    keyboard_history[2] = keyboard_history[1];
    keyboard_history[1] = keyboard_history[0];
    keyboard_history[0] = text;

    tft.setTextFont(1); // GLCD 8x8
    tft.setTextColor(TERM_GREEN, TERM_BG);
    tft.setTextDatum(TL_DATUM); // Zarovnání vlevo nahoře
    tft.drawString(keyboard_history[1], 0, 56);
    tft.drawString(keyboard_history[2], 0, 72);
}

//mouse functions

// call once after tft.init() to set initial position (optional)
void display_init_crosshair(int x = -1, int y = -1) {
    tft.fillScreen(TERM_BG);
    if (x < 0) x = tft.width() / 2;
    if (y < 0) y = tft.height() / 2;
    cross_x = x;
    cross_y = y;

    // draw initial crosshair
    // draw with TERM_GREEN, leave previous nothing to erase
    // horizontal
    for (int t = 0; t < CROSS_THICK; ++t) {
        tft.drawFastHLine(cross_x - CROSS_SIZE, cross_y + t, CROSS_SIZE * 2 + 1, TERM_GREEN);
    }
    // vertical
    for (int t = 0; t < CROSS_THICK; ++t) {
        tft.drawFastVLine(cross_x + t, cross_y - CROSS_SIZE, CROSS_SIZE * 2 + 1, TERM_GREEN);
    }
}

// Move crosshair by dx, dy. Erases old crosshair (no ghosts).
void display_move_crosshair(int dx, int dy) {
  if (cross_x < 0 || cross_y < 0) {
    // not initialized -> set to center
    cross_x = tft.width() / 2;
    cross_y = tft.height() / 2;
  }

  // erase previous crosshair by drawing same lines in background color
  for (int t = 0; t < CROSS_THICK; ++t) {
    tft.drawFastHLine(cross_x - CROSS_SIZE, cross_y + t, CROSS_SIZE * 2 + 1, TERM_BG);
    tft.drawFastVLine(cross_x + t, cross_y - CROSS_SIZE, CROSS_SIZE * 2 + 1, TERM_BG);
  }

  // compute new position and clamp to screen bounds so we never draw off-screen
  int nx = cross_x + dx;
  int ny = cross_y + dy;

  // clamp so entire crosshair fits on-screen
  if (nx - CROSS_SIZE < 0) nx = CROSS_SIZE;
  if (nx + CROSS_SIZE >= tft.width()) nx = tft.width() - 1 - CROSS_SIZE;
  if (ny - CROSS_SIZE < 0) ny = CROSS_SIZE;
  if (ny + CROSS_SIZE >= tft.height()) ny = tft.height() - 1 - CROSS_SIZE;

  cross_x = nx;
  cross_y = ny;

  // draw crosshair in foreground color
  for (int t = 0; t < CROSS_THICK; ++t) {
    tft.drawFastHLine(cross_x - CROSS_SIZE, cross_y + t, CROSS_SIZE * 2 + 1, TERM_GREEN);
    tft.drawFastVLine(cross_x + t, cross_y - CROSS_SIZE, CROSS_SIZE * 2 + 1, TERM_GREEN);
  }
}