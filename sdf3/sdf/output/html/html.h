/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   html.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   March 9, 2008
 *
 *  Function        :   Output SDF graph, architecture, mapping, etcetera in
 *                      HTML format.
 *
 *  History         :
 *      09-03-08    :   Initial version.
 *
 * $Id: html.h,v 1.2 2008-03-20 16:16:18 sander Exp $
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

#ifndef SDF_OUTPUT_HTML_HTML_H_INCLUDED
#define SDF_OUTPUT_HTML_HTML_H_INCLUDED

#include "sdf/resource_allocation/mpsoc_arch/graph.h"
#include "sdf/resource_allocation/noc_allocation/problem/problem.h"
namespace SDF
{


    class SDFconvertToHTML
    {
        public:
            // Constructor
            SDFconvertToHTML()
            {
                sdfGraphFilename = "graph";
                platformGraphFilename = "platform";
                platformMappingFilename = "platform_mapping";
                platformUsageFilename = "platform_usage";
                interconnectGraphFilename = "interconnect";
                interconnectMappingFilename = "interconnect_mapping";
                interconnectUsageFilename = "interconnect_usage";
            };

            // Destructor
            ~SDFconvertToHTML() = default;

            // Delete copy and move constructors and assignment operators
            SDFconvertToHTML(const SDFconvertToHTML&) = delete;
            SDFconvertToHTML& operator=(const SDFconvertToHTML&) = delete;
            SDFconvertToHTML(SDFconvertToHTML&&) = delete;
            SDFconvertToHTML& operator=(SDFconvertToHTML&&) = delete;

            // SDF graph
            TimedSDFgraph *getSDFgraph()
            {
                return sdfGraph;
            };
            void setSDFgraph(TimedSDFgraph *g)
            {
                sdfGraph = g;
            };

            // Platform graph
            PlatformGraph *getPlatformGraph()
            {
                return platformGraph;
            };
            void setPlatformGraph(PlatformGraph *g)
            {
                platformGraph = g;
            };

            // NoC scheduling problems
            SetOfNoCScheduleProblems *getSetOfNoCScheduleProblems()
            {
                return setOfNoCScheduleProblems;
            };
            void setSetOfNoCScheduleProblems(SetOfNoCScheduleProblems *p)
            {
                setOfNoCScheduleProblems = p;
            };

            // Convert data structures to HTML files
            void convert( const CString& prefix = "");

        private:
            // Conversion functions
            void convertSDFgraph();
            void convertPlatformGraph();
            void convertPlatformMapping();
            void convertPlatformUsage();
            void convertInterconnectGraph();
            void convertInterconnectMapping();
            void convertInterconnectUsage();

            // Basic HTML functions
            static void printStylesheet(std::ostream &out);
            void printHeader(std::ostream &out, const CString& pageTitle);
            static void printFooter(std::ostream &out);

            // Conversion to PNG image
            static void outputSDFgraphAsDot(SDFgraph *g, std::ostream &out);
            static void outputPlatformGraphAsDot(PlatformGraph *g, std::ostream &out);
            static void outputPlatformMappingAsDot(TimedSDFgraph *sdfGraph,
                                            PlatformGraph *platformGraph, std::ostream &out);
            static void outputInterconnectGraphAsDot(InterconnectGraph *g, std::ostream &out);
            static CString convertSDFgraphToPNG(TimedSDFgraph *g, const CString&filename);
            static CString convertPlatformGraphToPNG(PlatformGraph *g, const CString&filename);
            static CString convertPlatformMappingToPNG(TimedSDFgraph *sdfGraph,
                                                PlatformGraph *platformGraph, const CString&filename);
            static CString convertInterconnectGraphToPNG(InterconnectGraph *g,
                                                  const CString&filename);

            // Filename
            CString getFullName(const CString& filename, const CString& extension)
            {
                return fileNamePrefix + filename + "." + extension;
            };


            // Filename prefix
            CString fileNamePrefix;

            // SDF graph
            TimedSDFgraph *sdfGraph{nullptr};

            // Platform graph
            PlatformGraph *platformGraph{nullptr};

            // NoC scheduling problems
            SetOfNoCScheduleProblems *setOfNoCScheduleProblems{nullptr};

            // Filenames
            CString sdfGraphFilename;
            CString platformGraphFilename;
            CString platformMappingFilename;
            CString platformUsageFilename;
            CString interconnectGraphFilename;
            CString interconnectMappingFilename;
            CString interconnectUsageFilename;
    };

}//namespace SDF
#endif

