/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   conversion.h
 *
 *  Author          :   Martijn Koedam (m.l.p.j.koedam@tue.nl)
 *
 *  Date            :   April 21, 2011
 *
 *  Function        :   Input/Output format conversion
 *
 *  History         :
 *      21-04-11    :   Initial version.
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

#ifndef FSMSADF_RESOURCE_ALLOCATION_FLOW_COMPSOC_PLATFORM_CONVERSION_CONVERSION_H_INCLUDED
#define FSMSADF_RESOURCE_ALLOCATION_FLOW_COMPSOC_PLATFORM_CONVERSION_CONVERSION_H_INCLUDED

#include "base/exception/exception.h" // IWYU pragma: keep
#include "base/xml/xml.h"
#include "fsmsadf/base/type.h" // IWYU pragma: keep
#include "fsmsadf/resource_allocation/flow/base_platform/conversion/input_conversion.h"

namespace FSMSADF {

/**
 * Class that takes care to convert from the platform specific input file
 * to the generic format supported by sdf3.
 */
class InputConversionCompSoCPlatform : public InputConversionBase {
public:
    InputConversionCompSoCPlatform();
    ~InputConversionCompSoCPlatform() override;

    InputConversionCompSoCPlatform(const InputConversionCompSoCPlatform&) = delete;
    InputConversionCompSoCPlatform& operator=(const InputConversionCompSoCPlatform&) = delete;
    InputConversionCompSoCPlatform(InputConversionCompSoCPlatform&&) = delete;
    InputConversionCompSoCPlatform& operator=(InputConversionCompSoCPlatform&&) = delete;


    // Platform specific loaders
    void readApplicationFile(std::string input_file) override;
    void readArchitectureFile(std::string input_file) override;

private:
    // Helper functions
    static int getNumScenarios(CNode *app_graph);

    // Application Graph <applicationGraph>
    void createApplicationGraph(CDoc *application_doc);

    // <applicationGraph><fsmsadf>
    static void addApplicationFsmsadf(CNode *app_src_graph, CNode *dest);

    // <<applicationGraph>fsmsadf><scenarioGraph>
    static void addAppFsmsadfScenarioGraph(CNode *app_src_graph, CNode *fsmsadf_dest);

    // <applicationGraph><fsmsadf><scenarioGraph><Actor>
    static void
    addAppFsmsadfScenarioGraphActor(int scenario_num, CNode *src_actor, CNode *scenario_dest_graph);

    // <applicationGraph><fsmsadf><scenarioGraph><Channel>
    static void addAppFsmsadfScenarioGraphChannel(int scenario_num,
                                           CNode *app_src_graph,
                                           CNode *src_channel,
                                           CNode *scenario_dest_graph);

    // <applicationGraph><fsmsadfproperties>
    void addAppFsmsadfProp(CNode *app_graph_src, CNode *dest);

    // <applicationGraph><fsmsadfproperties><Scenario><ActorProperties>
    void addAppFsmsadfPropScenarioActorProperties(int scenario_num,
                                                  CNode *app_graph_src,
                                                  CNode *scenario_node);

    // <applicationGraph><fsmsadfproperties><Scenario><ActorProperties><Processor>
    void addAppFsmsadfPropScenarioActorPropProcessor(int scenario_num,
                                                     const CString& actor_name,
                                                     CNode *app_graph_src,
                                                     CNode *actor_prop_src,
                                                     CNode *actor_prop_dest);
    // <applicationGraph><fsmsadfproperties><Scenario><ActorProperties><Processor><Memory>
    void addAppFsmsadfPropScenarioActorPropProcessorMemory(CNode *actor_prop_src,
                                                           CNode *actor_prop_dest) const;

    // <applicationGraph><fsmsadfproperties><Scenario><ChannelProperties>
    void addAppFsmsadfPropScenarioChannelProp(int i, CNode *app_graph_src, CNode *scenario_node) const;

    // <applicationGraph><fsm>
    static void addApplicationFsm(CNode *app_graph_src, CNode *app_graph_dest);

    // ArchitectureGraph
    // <architectureGraph>
    void createArchitectureGraph(CDoc *app_doc);
    // <architectureGraph><Tile>
    void addArchitectureTiles(CNode *src, CNode *dest);
    // <architectureGraph><Tile><Processor>
    void addArchitectureTileProcessor(CNode *src, CNode *dest);
    // <architectureGraph><Tile><Processor><arbitration>
    void addArchitectureTileProcessorArbitration(CNode *dest) const;
    // <architectureGraph><Tile><Memory>
    static void addArchitectureTileMemory(CNode *src, CNode *dest);
    // <architectureGraph><Tile><Connections>
    static void addArchitectureConnections(CNode *src, CNode *dest);

    //
    void appReadUnits(CDoc *app_doc);

public:
    /// These are in ns, in the output stage these will be converted to cycles.
    int wheelsize;
    int contextSwitchOverhead;
    int slotLength;
    bool preemptive;

private:
    // The size of one dataUnit in bytes.
    double dataUnit;
    // The time of one unit (in nano seconds)
    double timeUnit;
};
} // namespace FSMSADF
#endif /* CONVERSION_H */
