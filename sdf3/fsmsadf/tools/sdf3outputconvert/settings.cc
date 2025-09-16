/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   settings.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   May 14, 2009
 *
 *  Function        :   Settings for sdf3flow
 *
 *  History         :
 *      28-04-11    :   Copy for SDF3Convert (M.L.P.J. Koedam)
 *      14-05-09    :   Initial version.
 *
 * $Id: settings.cc,v 1.1.2.3 2010-08-01 17:26:44 mgeilen Exp $
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

#include "settings.h"

namespace FSMSADF {

/**
 * Settings ()
 * Constructor. Create settings object with default values.
 */
ConvertSettings::ConvertSettings(const CString &module, const CString &type) :
    module(module),
    type(type),
    // No help requested
    helpFlag(false),
    stepFlag(false),
    // PlatformType
    mapping(-1),
    platformType(PlatformTypeUnknown),
    outputHTML(false) {
    // Settings file
    settingsFile = "sdf3.opt";

    // Output (stdout)
    outputFile = "";

    // file containing the application graph
    inputFile = "input.xml";

    // output files for CompSoC
    /// @todo this is not 'generic'
    communicationFile = "communication.xml";
    mappingFile = "mapping.xml";
    applicationFile = "application.xml";
    architectureFile = "architecture.xml";
}

/**
 * ~FlowSettings ()
 * Destructor.
 */
ConvertSettings::~ConvertSettings() = default;

/**
 * init ()
 * Initialize the settings using the supplied arguments. Note that the first
 * argument (argv[0]) is skipped.
 */
void ConvertSettings::init(int argc, char **argv) {
    CStrings args;

    for (int i = 1; i < argc; i++) {
        args.push_back(argv[i]);
    }

    init(args);
}

/**
 * init ()
 * Initialize the settings using the supplied arguments. The function
 * loads also external files (e.g. settings, graph, etc) when required.
 */
void ConvertSettings::init(const CStrings &args) {
    // Parse list of arguments
    parseArguments(args);

    // Only continue when no help is needed
    if (helpFlag) {
        return;
    }

    // Parse settings file
    parseSettingsFile();
}

/**
 * parseArguments ()
 * Parse the supplied arguments.
 */
void ConvertSettings::parseArguments(CStrings args) {
    auto argIter = args.begin();

    // Parse list of arguments
    while (argIter != args.end()) {
        CString arg;
        CString argNext;

        // Current argument
        arg = *argIter;

        // Argument left in list of arguments?
        if (argIter != args.end()) {
            argIter++;
            if (argIter != args.end()) {
                argNext = *argIter;
            }
        }

        // Parse argument
        if (arg == "--settings") {
            settingsFile = argNext;
            argIter++;
        } else if (arg == "--output") {
            outputFile = argNext;
            argIter++;
        } else if (arg == "--mapping") {
            mapping = int(argNext);
            argIter++;
        } else {
            helpFlag = true;
        }
    }
}

/**
 * parseSettingsFile ()
 * Load all settings from the settings file.
 */
void ConvertSettings::parseSettingsFile() {
    CNode *settingsNode = nullptr;
    CNode *sdf3Node = nullptr;
    CNode *inputFileNode = nullptr;
    CNode *platformTypeNode = nullptr;
    CString name;
    CString file;
    CString platformTypeString;
    std::unique_ptr<CDoc> settingsDoc = nullptr;

    // Open settings file and get root node
    settingsDoc = CParseFile(settingsFile);
    sdf3Node = settingsDoc->getRootNode();
    if (sdf3Node == nullptr) {
        throw CException("Failed opening '" + settingsFile + "'.");
    }

    // Is the node of the correct type?
    if (CGetAttribute(sdf3Node, "type") != module) {
        throw CException("Root element in file '" + settingsFile
                         + "' is not "
                           "of type '"
                         + module + "'.");
    }

    // Get the settings element of the tool
    for (settingsNode = CGetChildNode(sdf3Node, "settings"); settingsNode != nullptr;
         settingsNode = CNextNode(settingsNode, "settings")) {
        if (CGetAttribute(settingsNode, "type") == type) {
            break;
        }
    }

    // Found the correct settings element?
    if (settingsNode == nullptr) {
        throw CException("File '" + settingsFile + "' contains no settings " + "of type '" + type
                         + "'.");
    }

    // Platform type
    platformTypeNode = CGetChildNode(settingsNode, "platformType");
    if (platformTypeNode != nullptr) {
        if (!CHasAttribute(platformTypeNode, "type")) {
            throw CException("Missing type attribute on platformType.");
        }
        platformTypeString = CGetAttribute(platformTypeNode, "type");
        if (platformTypeString == "Virtual") {
            platformType = PlatformTypeVirtual;
        } else if (platformTypeString == "CompSoC") {
            platformType = PlatformTypeCompSoC;
        } else if (platformTypeString == "MAMPS") {
            platformType = PlatformTypeMAMPS;
        } else {
            throw CException("Platform type '" + platformTypeString + "'is not supported.");
        }
    }

    // inputFile
    if (!CHasChildNode(settingsNode, "input")) {
        throw CException("File '" + settingsFile + "' does not contains inputFile.");
    }
    inputFileNode = CGetChildNode(settingsNode, "input");
    inputFile = CGetAttribute(inputFileNode, "file");

    if (CHasChildNode(settingsNode, "output")) {
        CNode *outputNode = CGetChildNode(settingsNode, "output");
        outputFile = CGetAttribute(outputNode, "file");
    }

    if (CHasChildNode(settingsNode, "communicationGraph")) {
        CNode *outputNode = CGetChildNode(settingsNode, "communicationGraph");
        communicationFile = CGetAttribute(outputNode, "file");
    }
    if (CHasChildNode(settingsNode, "mappingGraph")) {
        CNode *outputNode = CGetChildNode(settingsNode, "mappingGraph");
        mappingFile = CGetAttribute(outputNode, "file");
    }
    if (CHasChildNode(settingsNode, "mapping")) {
        CNode *outputNode = CGetChildNode(settingsNode, "mapping");
        mapping = int(CGetAttribute(outputNode, "number"));
    }

    if (CHasChildNode(settingsNode, "applicationGraph")) {
        CNode *outputNode = CGetChildNode(settingsNode, "applicationGraph");
        applicationFile = CGetAttribute(outputNode, "file");
    }

    if (CHasChildNode(settingsNode, "architectureGraph")) {
        CNode *outputNode = CGetChildNode(settingsNode, "architectureGraph");
        architectureFile = CGetAttribute(outputNode, "file");
    }
}

/**
 * initOutputStream ()
 * Initialize the output stream.
 */
void ConvertSettings::initOutputStream() {
    // Set output stream
    if (!outputFile.empty()) {
        outputStream.open(outputFile.c_str());
    } else {
        (static_cast<std::ostream &>(outputStream)).rdbuf(std::cout.rdbuf());
    }
}

} // End namespace FSMSADF
