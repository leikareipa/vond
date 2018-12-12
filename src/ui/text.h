/*
 * Tarpeeksi Hyvae Soft 2018 /
 * Vond
 *
 */

#ifndef UI_TEXT_H
#define UI_TEXT_H

#include <string>
#include <vector>
#include "../../src/vector.h"

// A single entry of text at the given coordinates on the screen.
struct text_entry_s
{
    std::string text;
    vector2_s<uint> coords;
};

void ktext_clear_ui_text_entries(void);

void ktext_add_ui_text(const std::string text, const vector2_s<uint> screenCoords);

std::vector<text_entry_s> ktext_ui_text_entries(void);

#endif
