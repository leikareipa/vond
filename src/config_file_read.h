/*
 * Tarpeeksi Hyvae Soft 2018 /
 * Vond
 *
 * Sketching out a config file reader.
 *
 */

#ifndef CONFIG_FILE_READ_H
#define CONFIG_FILE_READ_H

#include <fstream>

// A single line (up to \n) in a given txt file, split up into its constituents,
// i.e. the command and any preceding parameters.
struct config_file_line_s
{
    char command = 0;
    uint indentLevel = 0;
    std::vector<std::string> params;
};

class config_file_read_c
{
public:
    config_file_read_c(const char *const filename);
    ~config_file_read_c();

    const std::string& file_name(void) const
    {
        return fileName;
    }

    uint current_line_num(void) const
    {
        return curLineNum;
    }

    bool file_is_at_end(void) const
    {
        return !bool(fileStream);
    }

    // Call this like you would assert; i.e. if the given condition is false,
    // the user will be notified of a read error at the current line.
    //
    void error_if_not(const bool condition, const char *const errorString);

    config_file_line_s next_line(void);

private:
    // Named constants for various significant characters.
    enum class syntax : char
    {
        paramsStartChar = ':',  // Follows the command character to mark the beginning of the parameter block.
        paramsSeparator = ';'   // Separates parameters in a parameter block.
    };

    const std::string fileName;
    std::ifstream fileStream;
    uint curLineNum = 0;

    // Returns the parameters in the given line of config text. For instance, the
    // line "v:-50;108;1" would be returned as the strings "-50", "108", and "1".
    //
    std::vector<std::string> params_in_line(const std::string &line);
};

#include "../../src/data_access/mesh_file.h"
#include "../../src/data_access/world_file.h"

#endif
