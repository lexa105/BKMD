/*
*
*Display class for debug
*
*/
#include <Arduino.h>

#pragma once

struct UiState {
  char big[32];       // current state string
  char text[64];      // small text
  char debug[96];     // debug line
  uint32_t counters[4];
  uint32_t lastRxMs;
  uint16_t lastLen;
  uint8_t  lastPreview[16];
};


class Display
{
private:
    void display_show_history(const char* text);
public:
    void display_init();
    void display_show_state(const char* state); // velký text
    void display_show_text(const char* text);   // menší text
    void display_show_debug(const char* text);
};




