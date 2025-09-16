/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   tools.h
 *
 *  Author          :   Marc Geilen (m.c.w.geilen@tue.nl)
 *
 *  Date            :   6 April 2009
 *
 *  Function        :   Generic tool functionality
 *
 *  History         :
 *      06-04-09    :   Initial version.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * In other words, you are welcome to use, share and improve this program.
 * You are forbidden to forbid anyone else to use, share and improve
 * what you give them.   Happy coding!
 */

#ifndef TOOLS_BASIS_H_INCLUDED
#define TOOLS_BASIS_H_INCLUDED

#include <memory>
#include <utility>

#include "base/string/cstring.h"

using CPair = struct CPair {
    CString key;
    CString value;
};

using CPairs = std::list<CPair>;
using CPairsIter = CPairs::iterator;

class Settings {
public:
    Settings() = default;
    virtual ~Settings() = default;
    Settings(const Settings&) = delete;
    Settings& operator=(const Settings&) = delete;
    Settings(Settings&&) = delete;
    Settings& operator=(Settings&&) = delete;


    // Output file
    CString outputFile;
};

class Tool {
public:
    Tool() = default;
    virtual ~Tool() = default;

    // Delete copy and move constructors and assignment operators
    Tool(const Tool&) = delete;
    Tool& operator=(const Tool&) = delete;
    Tool(Tool&&) = delete;
    Tool& operator=(Tool&&) = delete;

    virtual void parseCommandLine(int argc, char **argv);
    virtual bool parseCommand(int argc, char **argv, int *arg) = 0;
    virtual CPairs parseSwitchArgument(CString arguments);
    virtual void initSettings(const CString &modulename, int argc, char **argv);
    virtual void setDefaults();
    virtual bool checkRequiredSettings() = 0;
    virtual void helpMessage(std::ostream &out) = 0;
    virtual void cleanUp();
    virtual void postInitSettings() {}
    virtual Settings& settings() { return *_settings; }
    virtual void setSettings(std::unique_ptr<Settings> settings) {
        _settings = std::move(settings);
    }

private:
    std::unique_ptr<Settings> _settings;
};

#endif
