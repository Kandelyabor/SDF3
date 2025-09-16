/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   testsettings.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *                      Martijn Koedam (m.l.p.j.koedam@tue.nl)
 *
 *  Date            :   May 14, 2009
 *
 *  Function        :   Settings for sdf3 test
 *
 *  History         :
 *      14-05-09    :   Initial version.
 *      29-10-15    :   Integrated test-run from GlobalSettings branch
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

#ifndef TOOLS_TEST_SETTINGS_H_INCLUDED
#define TOOLS_TEST_SETTINGS_H_INCLUDED

#include "base/string/cstring.h"
#include <ostream>

class TestSettings {
public:
    enum Type { SINGLE, BOOL, MULTIPLE, UNKNOWN };
    enum SetBy { DEFAULT = 0, OPTIONF = 1, CMDL = 2 };
    using CPair = struct CPair {
        CString key;
        CString value;
    };

    using CPairs = std::list<CPair>;
    using CPairsIter = CPairs::iterator;

private:
    struct Entry {
        Type type;
        SetBy set_by;
        CString value;
        CString help;
    };
    using EntryList = std::map<CString, Entry>;
    EntryList settings;

public:
    CString module;
    CString settings_type;

    // functions

    // Parses the command line arguments.
    bool parseArguments(int argc, char **argv);

    // Parse the option file
    void parseOptionFile();

    // print a custom 'header'
    virtual void printProgramHeader(std::ostream &out);

    // accessors

    // Check if key exists in TestSettings object.
    bool hasKey(const CString &key);

    // Get the value of key in TestSettings.
    CString getValue(const CString &key);

    // Get the values of key in TestSettings.
    CStrings getValues(const CString &key);

    // Get the value of key in TestSettings.
    bool getValueB(const CString &key);
    CPairs getValuesAsPairs(const CString &key);

    // Destructor
    virtual ~TestSettings() = default;
    // Prevent copying and moving
    TestSettings(const TestSettings&) = delete;
    TestSettings& operator=(const TestSettings&) = delete;
    TestSettings(TestSettings&&) = delete;
    TestSettings& operator=(TestSettings&&) = delete;


    /************************************************
     * Private                                      *
     ************************************************/
private:
    void printHelp();
    void setValue(const CString& key, const CString& value, SetBy sb);
    bool isSet(const CString& key);
    static CPairs parseSwitchArgument(CString arguments);

protected:
    // Constructor
    TestSettings(const CString &module, const CString &settings_type);

public:
    // Add entries
    void
    addValue(const CString &key, const CString &default_value, Type type = TestSettings::SINGLE);
    void addValue(const CString &key,
                  const CString &default_value,
                  const CString &help,
                  Type type = TestSettings::SINGLE);
};

#endif
