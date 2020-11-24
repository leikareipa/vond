/*
 * Tarpeeksi Hyvae Soft 2018 /
 * Vond
 *
 * Sketching out a config file reader.
 *
 * Essentially, config files provide text-based data for various aspects, including
 * 3d meshes (but excluding textures). The typical format of a config file is n number
 * of lines of text, where a line might be like the following:
 *
 *      v:0;-50;0;0;0.999
 *
 * Here, v is the line's command, and the rest a ;-separated set of parameters to that
 * command. Which command v stands for depends on the file type, but for e.g. mesh files,
 * this would be the 'vertex' command, i.e. a command that defines the coordinates of a
 * single vertex.
 *
 * Note that in config files, line indentation is functional and separates different
 * blocks of data. In the above example, the v command is subject to the p command
 * ('polygon'), and always needs to be one level of indentation higher relative to
 * it.
 *
 */

#include <regex>
#include "auxiliary/config_file_read.h"
#include "vond/assert.h"

config_file_read_c::config_file_read_c(const char * const filename) :
    fileName(filename)
{
    curLineNum = 0;

    fileStream.open(fileName);
    vond_assert(fileStream.is_open(), "Failed to open the data file.");

    return;
}

config_file_read_c::~config_file_read_c()
{
    fileStream.close();

    return;
}

void config_file_read_c::error_if_not(const bool condition, const char *const errorString)
{
    if (!condition)
    {
        fprintf(stderr, "While parsing line #%u of mesh file '%s': %s", current_line_num(), file_name().c_str(), errorString);
        assert(0);
    }

    return;
}

config_file_line_s config_file_read_c::next_line()
{
    config_file_line_s line;

    // Extract the next line from the file.
    std::string lineStr;
    {
        std::string tmp;
        std::getline(fileStream, tmp, '\n');
        if (!fileStream)
        {
            goto return_empty;
        }
        curLineNum++;

        if (tmp.empty())
        {
            goto return_empty;
        }

        // The indent level is the number of prefixed tabs.
        vond_assert((strspn(tmp.c_str(), " ") == 0), "Spaces aren't allowed as indent markers.");
        line.indentLevel = strspn(tmp.c_str(), "\t");

        // Now that we know the indent level, we can remove all whitespace to make
        // parsing the line simpler.
        lineStr = std::regex_replace(tmp, std::regex("\\s+"), "");

        line.command = lineStr.at(0);

        // Validate the line.
        if (lineStr.at(0) == '?') // Comment lines, ones beginning with ?, don't need to be validated.
        {
            goto return_empty;
        }
        else
        {
            if (lineStr.empty())
            {
                goto return_empty;
            }

            this->error_if_not((lineStr.size() >= 2), "Malformed file string.");
            this->error_if_not((lineStr.at(1) == (char)syntax::paramsStartChar), "Malformed file string.");
        }
    }

    line.params = params_in_line(lineStr);

    return line;

    return_empty:
    return config_file_line_s();    // Return a default line to signal there was no actual line to return.
}

std::vector<std::string> config_file_read_c::params_in_line(const std::string &line)
{
    this->error_if_not(!line.empty(), "The line is empty.");
    this->error_if_not((line.at(1) == (char)syntax::paramsStartChar), "Bad parameter list lead character.");

    const char *curChar = (line.c_str() + 2);    // Skip the command.

    // Extract the parameters.
    std::vector<std::string> params;
    std::string param = "";
    while (1)
    {
        const char ch = *curChar++;

        if (ch == (char)syntax::paramsSeparator ||
            ch == '\0')
        {
            params.push_back(param);
            param = "";

            if (ch == '\0')
            {
                break;
            }
        }
        else
        {
            param += ch;
        }
    }

    return params;
}
