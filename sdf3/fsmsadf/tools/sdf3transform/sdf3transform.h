/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sdf3transform.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   July 21, 2009
 *
 *  Function        :   FSM-based SADF transformation functionality
 *
 *  History         :
 *      21-07-09    :   Initial version.
 *
 * $Id: sdf3transform.h,v 1.1.2.3 2010-05-11 03:27:08 mgeilen Exp $
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

#ifndef FSMSADF_TOOLS_SDF3TRANSFORM_SDF3TRANSFORM_H_INCLUDED
#define FSMSADF_TOOLS_SDF3TRANSFORM_SDF3TRANSFORM_H_INCLUDED

/**
 * Tool name and version
 */
#include "base/xml/xml.h"
#include "fsmsadf/resource_allocation/platform_binding/graph.h"
#include "fsmsadf/resource_allocation/platform_graph/graph.h"
#include <memory>
#define TOOL               "sdf3transform-fsmsadf"
#define DOTTED_VERSION     "1.0"
#define VERSION             10000

/**
 * Module (MoC) supported by the tool
 */
#define MODULE             "fsmsadf"

/**
 * Settings type used by the tool
 */
#define SETTINGS_TYPE      "transform"

#include "tools/base/tools.h"
#include <iostream>


namespace FSMSADF
{

    /**
     * Settings
     * class to store program settings.
     */
    class TransformSettings: public Settings
    {
        public:
            // settings file
            CString settingsFile;

            // Input file with graph
            CString graphFile;

            // Scenario
            CString scenario;

            // Switch argument(s) given to algorithm
            CPairs arguments;
    };

    class ToolTransform: public Tool
    {
        public:
            // inherited methods
             void postInitSettings() override;
             void setDefaults() override;
             bool parseCommand(int argc, char **argv, int *arg) override;
             void initSettings(const CString &modulename, int argc, char **argv) override;
             bool checkRequiredSettings() override;
             void helpMessage(std::ostream &out) override;
             void cleanUp() override;

            // transform specific methods
            void transformGraph(std::ostream &out);
            void loadGraphs();
            TransformSettings& transformSettings()
            {
                return dynamic_cast<TransformSettings&>(this->settings());
            }
        private:
            void parseSettingsFile(const CString& module, const CString& type);
            static void loadGraphsFromFile(const CString& file, const CString& module, CNode **appGraphNode, CNode **archGraphNode, std::list <CNode *> *mappingsNodes, std::unique_ptr<CDoc> *appGraphDoc);

            std::unique_ptr<Graph> transformGraph(Graph *ag, PlatformGraph *pg, PlatformBindings *pbs, CPairs &transforms, std::ostream &out);

            // Application graph
            CNode *xmlAppGraph;
            std::unique_ptr<CDoc> xmlAppGraphDoc;

            // Architecture graph
            CNode *xmlArchGraph;

            // Mappings
            std::list <CNode *> xmlMappings;
    };

} // namespace FSMSADF
#endif
