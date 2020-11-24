/*
 * Tarpeeksi Hyvae Soft 2018 /
 * Vond
 *
 * A temporary text renderer. Used to display strings to the user over the main window.
 *
 * This will be replaced with a more permanent solution, later.
 *
 */

#include "auxiliary/ui/text.h"

static std::vector<text_entry_s> TEXT_ENTRIES;

void ktext_clear_ui_text_entries(void)
{
    TEXT_ENTRIES.clear();

    return;
}

void ktext_add_ui_text(const std::string text, const vector2_s<uint> screenCoords)
{
    TEXT_ENTRIES.push_back({text, screenCoords});

    return;
}

// Returns a copy of all the current text entries.
//
std::vector<text_entry_s> ktext_ui_text_entries(void)
{
    return TEXT_ENTRIES;
}
