#pragma once
#include <TFT_eSPI.h>

void display_init();
void display_show_state(const char* state); // velký text
void display_show_text(const char* text);   // menší text
void display_show_keyboard_text(const char* text);
void display_show_keyboard_history(String text);

void display_init_crosshair(int x, int y);
void display_move_crosshair(int dx, int dy);
