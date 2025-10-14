/*
*
*Display class for debug
*
*/

#pragma once


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




