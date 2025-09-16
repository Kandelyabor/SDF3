/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sdf3analyze.h
 *
 *  Author          :   Marc Geilen (m.c.w.geilen@tue.nl)
 *
 *  Date            :   6 April 2009
 *
 *  Function        :   FSM-based SADF Graph Analysis Functionality
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

#ifndef FSMSADF_SDF3ANALYZE_H_INCLUDED
#define FSMSADF_SDF3ANALYZE_H_INCLUDED

/**
 * Tool name and version
 */
#include "base/xml/xml.h"
#define TOOL "sdf3analyze-fsmsadf"
#define DOTTED_VERSION "1.0"
#define VERSION 10000

/**
 * Module (MoC) supported by the tool
 */
#define MODULE "fsmsadf"

/**
 * Settings type used by the tool
 */
#define SETTINGS_TYPE "analyze"

#include "tools/base/tools.h"
#include <iostream>

namespace FSMSADF {

/**
 * Settings
 * class to store program settings.
 */
class AnalyzeSettings : public Settings {
public:
    // settings file
    CString settingsFile;

    // Input file with graph
    CString graphFile;

    // Switch argument(s) given to analysis algorithm
    CPairs arguments;

    // Comma separated scenario sequence provided to the FSM-SADF trace generation
    CString seq;

    // desired trace format for the FSM-SADF trace generation
    CString traceFormat;

    // Provided filenames for TRACE file and TRACE configuration file
    CString traceFiles;

    // Provided period for latency (latency is computed relative to this period)
    CString period;
};

class ToolAnalyze : public Tool {
public:
    // inherited methods
    bool parseCommand(int argc, char **argv, int *arg) override;
    void setDefaults() override;
    bool checkRequiredSettings() override;
    void postInitSettings() override;
    void initSettings(const CString &modulename, int argc, char **argv) override;
    void helpMessage(std::ostream &out) override;
    void cleanUp() override;

    // analyse specific methods
    void analyzeGraph(std::ostream &out);
    void loadGraphs();
    AnalyzeSettings& analyzeSettings() {
        return dynamic_cast<AnalyzeSettings&>(this->settings());
    }
private:
    void parseSettingsFile(const CString& module, const CString& type);
    static void loadGraphsFromFile(const CString &file,
                            const CString& module,
                            CNode **appGraphNode,
                            CNode **archGraphNode,
                            CNode **mappingGraphNode,
                            std::unique_ptr<CDoc> *appGraphDoc);

    // Application graph
    CNode *xmlAppGraph;
    std::unique_ptr<CDoc> xmlAppGraphDoc;

    // Architecture graph
    CNode *xmlArchGraph;

    // Mapping
    CNode *xmlMappingGraph;
};

} // namespace FSMSADF

#endif
