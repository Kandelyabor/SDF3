/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   testsettings.cc
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

#include "testsettings.h"
#include "base/exception/exception.h"
#include "base/log/log.h"
#include "base/math/cmath.h"
#include "base/xml/xml.h"
#include <iomanip>

const char list_separator = ',';

/**
 * TestSettings()
 *
 * @param module the name of the module, e.g. fsmsadf
 * @param settings_type the name of the settings_type.
 *
 * The value off module and settings_type should match up with the
 * the type fields in the settings file.
 */
TestSettings::TestSettings(const CString &module, const CString &settings_type) :
    module(module), settings_type(settings_type) {}

/**
 * parseArguments()
 * @param argc
 * @param argv
 *
 * Parses the command line options.
 *
 * @throws CException when an error occurs.
 * @returns true when the program should stop. e.g. --help is called.
 */
bool TestSettings::parseArguments(int argc, char **argv) {
    int argc_iter = 1;
    while (argc_iter < argc) {
        CString key = argv[argc_iter];
        // Validate if it is a - option.
        if (key.length() <= 2 || key[0] != '-' || key[1] != '-') {
            printProgramHeader(std::cout);
            throw CException("Unexpected command line argument: " + key);
        }
        // skip the --
        key.erase(0, 2);
        if (key == "help") {
            printProgramHeader(std::cout);
            printHelp();
            return true;
        }
        if (key == "param") {
            if (argc_iter + 1 >= argc) {
                printProgramHeader(std::cout);
                throw CException("Expected parameter for command line argument: " + key);
            }
            CString param = argv[++argc_iter];
            CStrings set = param.split(':');
            if (set.size() < 2) {
                throw CException("The 'param' arguments needs to be in the form off key:value");
            }
            CString key = *(set.begin());
            set.pop_front();
            // hack if multiple :  where in the argument. reconstruct value.
            CString value = CString::join(set, ':');

            // Update, or if missing add.
            if (hasKey(key)) {
                setValue(key, value, CMDL);
            } else {
                addValue(key, value, "(passed by argument)", UNKNOWN);
            }
        } else if (hasKey(key)) {
            if (argc_iter + 1 >= argc) {
                auto i = settings.find(key);
                // If type is bool, we accept no param (defaults to true)
                if (i->second.type == BOOL) {
                    setValue(key, "true", CMDL);
                } else {
                    printProgramHeader(std::cout);
                    throw CException("Expected parameter for command line argument: " + key);
                }
            } else {
                CString value = argv[++argc_iter];
                setValue(key, value.trim(), CMDL);
            }
        } else {
            printProgramHeader(std::cout);
            throw CException("Unexpected command line argument: " + key);
        }

        argc_iter++;
    }
    return false;
}
/**
 * parseOptionFile()
 *
 * Parses the file specified by settings entry.
 * By default this is sdf3.opt.
 * @throws CException when it failed to parse the file.
 */
void TestSettings::parseOptionFile() {
    CString filename = getValue("settings");

    // If no file specified, do not load it.

    // Open settings file and get root node
    std::unique_ptr<CDoc> settingsDoc = CParseFile(filename);
    CNode *sdf3Node = settingsDoc->getRootNode();
    if (sdf3Node == nullptr) {
        throw CException("Failed opening '" + filename + "'.");
    }

    // Is the node of the correct type?
    if (!module.empty() && CGetAttribute(sdf3Node, "type") != module) {
        throw CException("Root element in file '" + filename
                         + "' is not "
                           "of type '"
                         + module + "'.");
    }
    CNode *settingsNode = CFindNodeWithAttribute(sdf3Node, "settings", "type", settings_type);

    // Is the node of the correct type?
    if (CGetAttribute(settingsNode, "type") != settings_type) {
        throw CException("Root element in file '" + filename
                         + "' is not "
                           "of type '"
                         + settings_type + "'.");
    }
    for (CNode *child = CGetChildNode(settingsNode); child != nullptr; child = CNextNode(child)) {
        CString key = CIsNode(child);
        if (hasKey(key)) {
            if (isSet(key)) {
                logMsg("Key: '" + key + "' is already set. ignoring");
            } else {
                setValue(key, CGetNodeContent(child), OPTIONF);
            }
        } else {
            addValue(key, CGetNodeContent(child), "", UNKNOWN);
        }
    }
}

/******************** Private *******************************/
/**
 * printProgramHeader()
 * Prints the program header.
 */
void TestSettings::printProgramHeader(std::ostream &out) {
    out << "SDF3 " << settings_type << "-" << module << std::endl;
}

/**
 * printHelp()
 *
 * Prints the command line arguments and tries todo a little bit
 * off formatting.
 */
void TestSettings::printHelp() {
    constexpr int MinLength = 6;
    int length = MinLength;
    for (auto &[key, e] : settings) {
        length = MAX(length, static_cast<uint>(key.length()) + 2);
    }
    std::cout << std::endl;
    std::cout << "Application Options:" << std::endl;
    for (auto& [key, e]: settings) {
        std::cout << std::setw(static_cast<std::streamsize>(length - key.length() + 2)) << "--" << key << ": ";
        std::cout << e.help;
        if (!e.value.empty()) {
            std::cout << " (default: \"" << e.value << "\")" << std::endl;
        } else {
            std::cout << std::endl;
        }
    }
    std::cout << std::setw(length + 3) << "--help:"
              << " This help message." << std::endl;
}
/**
 * addValue()
 * @param key The key to add.
 * @param default_value The default value of \a key.
 *
 * Adds \a key:value to the settings system.
 * @throws CException is \a key already exists.
 */
void TestSettings::addValue(const CString &key, const CString &default_value, Type type) {
    if (hasKey(key)) {
        throw CException("Key: " + key + " already exists.");
    }
    Entry entry;
    entry.set_by = DEFAULT;
    entry.type = type;
    entry.value = default_value;
    entry.help = "";
    settings.insert(std::pair<CString, Entry>(key, entry));
}
/**
 * addValue()
 * @param key The key to add.
 * @param default_value The default value of \a key.
 * @param help The help message for this key.
 * @param type The \a Type of key. (single or multiple entries)
 *
 * Adds \a key:value to the settings system with help message \a help.
 * @throws CException is \a key already exists.
 */
void TestSettings::addValue(const CString &key,
                            const CString &default_value,
                            const CString &help,
                            Type type) {
    if (hasKey(key)) {
        throw CException("Key: " + key + " already exists.");
    }
    Entry entry;
    entry.set_by = DEFAULT;
    entry.type = type;
    entry.value = default_value;
    entry.help = help;
    settings.insert(std::pair<CString, Entry>(key, entry));
}
/**
 * setValue()
 * @param key the \a key to set.
 * @param value the value to set \a key too.
 *
 * Associate \a key with \a value.
 */
void TestSettings::setValue(const CString &key, const CString &value, SetBy sb) {
    if (!hasKey(key)) {
        throw CException("Key: " + key + " does not exist.");
    }
    auto i = settings.find(key);
    // If key is already set by higher priority, ignore it.
    // So option file trumps default, and command line trumps option file.
    if (i->second.set_by > sb) {
        // std::cout << "Ignoring: " << key << " already set by higher priority" << std::endl;
        return;
    }
    if (i->second.type == SINGLE) {
        i->second.value = value;
    } else {
        // Clear and set when set by higher priority.
        if (i->second.set_by < sb) {
            i->second.value = value;
        } else {
            i->second.value += CString(list_separator) + value;
        }
    }
    i->second.set_by = sb;
}
/**
 * isSet()
 * @param key The key to lookup
 *
 * Check if \a key has been explicitly set.
 * Multiple always return false.
 * @returns true if value has been set.
 */
bool TestSettings::isSet(const CString& key) {
    if (!hasKey(key)) {
        throw CException("Key: " + key + " does not exist.");
    }
    auto i = settings.find(key);
    if (i->second.type == MULTIPLE) {
        return false;
    }
    return !(i->second.set_by == DEFAULT);
}
/**
 * hasKey()
 * @param key the key to lookup.
 * @returns if the \a key exists in the settings object.
 */
bool TestSettings::hasKey(const CString &key) {
    auto t = settings.find(key);
    return (t != settings.end());
}
/**
 * getValue()
 * @param key the key to lookup.
 *
 * Get the value associated with \a key
 * @returns: The value associated with \a key.
 * @throws:  CException if \a key was not found.
 */
CString TestSettings::getValue(const CString &key) {
    auto t = settings.find(key);
    if (t == settings.end()) {
        throw CException("Key: " + key + " does not exist.");
    }
    return t->second.value;
}

/**
 * getValues()
 * @param key the key to lookup.
 *
 * Get the values associated with \a key
 * @returns: The values associated with \a key.
 * @throws:  CException if \a key was not found.
 */
CStrings TestSettings::getValues(const CString &key) {
    auto t = settings.find(key);
    if (t == settings.end()) {
        throw CException("Key: " + key + " does not exist.");
    }
    return t->second.value.split(list_separator);
}

/**
 * getValueB()
 * @param key the key to get.
 * Gets the boolean form of the value associated with \a key.
 * @returns \a true if value == "true".
 */
bool TestSettings::getValueB(const CString &key) {
    auto t = settings.find(key);
    if (t == settings.end()) {
        throw CException("Key: " + key + " does not exist.");
    }
    CString val = t->second.value;
    return val.toLower() == "true";
}

/**
 * getValuesAsPairs()
 * @param key the key to lookup.
 *
 * Get the values associated with \a key.
 * The values are split into pairs. a(b) becomes
 * val->key = a, val->value = b.
 *
 * @returns: The values associated with \a key.
 * @throws:  CException if \a key was not found.
 */
TestSettings::CPairs TestSettings::getValuesAsPairs(const CString &key) {
    CString val = this->getValue(key);
    return parseSwitchArgument(val);
}

TestSettings::CPairs TestSettings::parseSwitchArgument(CString arguments) {
    TestSettings::CPairs pairs;
    TestSettings::CPair p;

    while (!arguments.empty()) {
        char c = 0;
        p.key = "";
        p.value = "";

        // Get key from argument string
        do {
            c = arguments[0];
            arguments = arguments.substr(1);
            if (c == list_separator || c == '(') {
                break;
            }
            p.key += c;
        } while (!arguments.empty());

        // Is next part of argument a value?
        if (c == '(') {
            CString::size_type ePos = 0;
            int level = 1;

            // Find the matching closing brace
            while (level != 0 && !arguments.empty()) {
                if (arguments.operator[](static_cast<int>(ePos)) == ')') {
                    level--;
                } else if (arguments.operator[](static_cast<int>(ePos)) == '(') {
                    level++;
                }

                // Next
                ePos++;
            }

            // Closing brace found?
            if (level != 0) {
                throw CException("Missing closing brace in value of argument.");
            }

            // Get value
            p.value = arguments.substr(0, ePos - 1);

            // More arguments left?
            if (arguments.size() > ePos) {
                arguments = arguments.substr(ePos + 1);
            } else {
                arguments = "";
            }
        }

        // Add pair to list of pairs
        pairs.push_back(p);
    }

    return pairs;
}
