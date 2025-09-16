/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   settings.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   July 24, 2007
 *
 *  Function        :   Settings for sdf3flow
 *
 *  History         :
 *      24-07-07    :   Initial version.
 *
 * $Id: settings.h,v 1.3 2008-03-20 16:16:21 sander Exp $
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

#ifndef SDF_TOOLS_SDF3FLOW_SETTINGS_H_INCLUDED
#define SDF_TOOLS_SDF3FLOW_SETTINGS_H_INCLUDED

#include "sdf/sdf.h" // IWYU pragma: export - indirectly includes

namespace SDF
{


    /**
     * Settings
     * Object containing all settings for the tool.
     */
    class Settings
    {
        public:
            // Constructor
            Settings(CString module, CString type);

            // Destructor
            ~Settings();

            // Delete copy and move constructors and assignment operators
            Settings(const Settings&) = delete;
            Settings& operator=(const Settings&) = delete;
            Settings(Settings&&) = delete;
            Settings& operator=(Settings&&) = delete;

            // Initialize settings
            void init(int argc, char **argv);
            void init(CStrings args);

            // Output stream
            void initOutputStream();

        private:
            // Parsing of settings
            void parseArguments(CStrings args);
            void parseSettingsFile();

            // Loading of applications, architecture and usage data
            CNode *loadApplicationGraphFromFile(CString &file, std::unique_ptr<CDoc>& appGraphDoc) const;
            CNode *loadArchitectureGraphFromFile(CString &file, std::unique_ptr<CDoc>& archGraphDoc) const;
            CNode *loadSystemUsageFromFile(CString &file, std::unique_ptr<CDoc>& systemUsageDoc) const;

        public:
            // MoC supported by the tool
            CString module;

            // Settings type used by the tool
            CString type;

            // Usage information of the tool requested
            bool helpFlag;

            // Settings file
            CString settingsFile;

            // Output stream
            CString outputFile;
            std::ofstream outputStream;

            // Run flow step-by-step
            bool stepFlag;

            // Output results as HTML
            bool outputAsHTML;

            // Flow type
            SDFflowType flowType;

            // Tile mapping algorithm
            CString tileMappingAlgo;

            // NoC mapping algorithm
            CString nocMappingAlgo;

            // Application graphs
            std::list<CNode *> xmlAppGraphs;
            std::list<std::unique_ptr<CDoc>> xmlAppGraphDocs;

            // Architecture graph
            CNode *xmlArchGraph;
            std::unique_ptr<CDoc> xmlArchGraphDoc;

            // Architecture usage information
            CNode *xmlSystemUsage;
            std::unique_ptr<CDoc> systemUsageDoc;

            // Constants used in tile cost function
            double cnst_a, cnst_b, cnst_c, cnst_d, cnst_e, cnst_f, cnst_g;
            double cnst_k, cnst_l, cnst_m, cnst_n, cnst_o, cnst_p, cnst_q;

            // Constrains used in NoC scheduling problem
            uint maxDetour;
            uint maxNrRipups;
            uint maxNrTries;
    };

}//namespace SDF
#endif
