/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   html.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   December 6, 2009
 *
 *  Function        :   Output FSM-based SADF in HTML format
 *
 *  History         :
 *      06-12-09    :   Initial version.
 *
 * $Id: html.h,v 1.2 2010-02-08 08:04:42 sander Exp $
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

#ifndef FSMSADF_OUTPUT_HTML_HTML_H_INCLUDED
#define FSMSADF_OUTPUT_HTML_HTML_H_INCLUDED

#include "fsmsadf/resource_allocation/flow/base_platform/base/flow.h"

namespace FSMSADF
{

    /**
     * OutputHTML
     * Output FSM-based SADF in HTML format.
     */
    class OutputHTML
    {
        public:
            // Constructor
            OutputHTML();

            // Destructor
            ~OutputHTML();

            OutputHTML(const OutputHTML&) = delete;
            OutputHTML& operator=(const OutputHTML&) = delete;
            OutputHTML(OutputHTML&&) = delete;
            OutputHTML& operator=(OutputHTML&&) = delete;

            // Settings
            void setDirname(const CString &name)
            {
                dirname = name;
            };
            [[nodiscard]] [[nodiscard]] CString getDirname() const
            {
                return dirname;
            };

            // Output
            void outputAsHTML(SDF3FlowBase *flow);

        private:
            // Application graph
            void printApplicationGraph(SDF3FlowBase *flow);

            // Binding aware graphs
            void printBindingAwareGraphs(SDF3FlowBase *flow);

            // Platform graph
            void printPlatformGraph(SDF3FlowBase *flow);

            // Platform bindings
            void printPlatformBindings(SDF3FlowBase *flow);
            void printPlatformBinding(PlatformBinding *pb, SDF3FlowBase *flow);

            // Create directory for HTML output
            void createOutputDirectory();

            // Create output file
            void createOutputFileApplicationGraph(Graph *applicationGraph,
                                                  std::ofstream &out);
            void createOutputFileBindingAwareGraph(PlatformBinding *platformBinding,
                                                  std::ofstream &out);
            void createOutputFilePlatformGraph(PlatformGraph *platformGraph,
                                               std::ofstream &out);
            void createOutputFilePlatformBinding(PlatformBinding *platformBinding,
                                                 std::ofstream &out);
            void createOutputFilePlatformBindings(std::ofstream &out);
            void createOutputFile(const CString &name, std::ofstream &out);

            // Header and footer
            static void printHeader(std::ostream &oFile, SDF3FlowBase *flow);
            static void printFooter(std::ostream &oFile, SDF3FlowBase *flow);
            static void printStylesheet(std::ostream &oFile);

            // Application graph
            void printScenario(const Scenario& s, const CString& name, std::ostream &oFile);

            // Binding aware application graph
            void printBAGScenario(const Scenario& s, std::ostream &oFile);

            // FSM
            void printFSM(FSM *fsm, std::ostream &oFile);

            // URLs
            static CString getApplicationGraphURL(Graph *applicationGraph);
            static CString getBindingAwareGraphURL(PlatformBinding *platformBinding);
            static CString getScenarioURL(const Scenario& s, bool onlyName = false);
            static CString getActorURL(const Scenario& s, ActorRef a, bool onlyName = false);
            static CString getChannelURL(const Scenario& s, Channel *c, bool onlyName = false);
            static CString getPlatformGraphURL(PlatformGraph *platformGraph);
            static CString getPlatformBindingURL(PlatformBinding *platformBinding);
            static CString getPlatformBindingsURL();
            static CString getScenarioGraphPicURL(const Scenario& s);
            static CString getScenarioBAGPicURL(const Scenario& s, PlatformBinding *platformBinding);
            static CString getFSMPicURL();
            static CString getTileURL(Tile *t, bool onlyName = false);
            static CString getConnectionURL(Connection *c, bool onlyName = false);
            static CString getPlatformGraphPicURL(PlatformGraph *pg);
            static CString getNetworkInterfaceURL(NetworkInterface *n, bool onlyName = false);
            static CString getProcessorURL(Processor *p, bool onlyName = false);
            static CString getMemoryURL(Memory *m, bool onlyName = false);
            static CString getScenarioPlatformBindingURL(const Scenario& s, PlatformBinding *pb,
                                                  bool onlyName = false);
            static CString getPlatformBindingPicURL(const Scenario& s, PlatformBinding *pb);

            // Pictures
            bool convertGraphToPNG(CString &graph, CString &filename, CString &map) const;
            void convertScenarioGraphToPNG(const Scenario& s, ScenarioGraph *sg,
                                           CString filename, CString &map);
            void convertFSMToPNG(FSM *fsm, CString filename, CString &map);
            void convertPlatformGraphToPNG(PlatformGraph *pg, CString filename,
                                           CString &map);
            void convertPlatformBindingToPNG(const Scenario& s, PlatformBinding *pb,
                                             CString filename, CString &map);


            CString dirname;
    };

} // End namespace FSMSADF

#endif

