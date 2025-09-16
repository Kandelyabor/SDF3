/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   input_conversion.h
 *
 *  Author          :   Umar Waqas (u.waqas@student.tue.nl)
 *
 *  Date            :   November 16, 2011
 *
 *  Function        :   Convert the input files to a compatible format.
 *
 *  History         :
 *      16-11-11    :   Initial version.
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

#ifndef FSMSADF_RESOURCE_ALLOCATION_FLOW_MAMPS_PLATFORM_CONVERSION_CONVERSION_H_INCLUDED
#define FSMSADF_RESOURCE_ALLOCATION_FLOW_MAMPS_PLATFORM_CONVERSION_CONVERSION_H_INCLUDED

#include "base/xml/xml.h"
#include "fsmsadf/resource_allocation/flow/base_platform/conversion/input_conversion.h"

#define SUPPORTED_MODEL "ca_with_fsl"
#define DEFAULT_SCENARIO "scenario-s1"
namespace FSMSADF {

/**
 * InputConversionMampsPlatform provides implementation
 * of the input conversion layer to make MAMPS-SDF3 compatible
 * with each other.
 *
 */
class InputConversionMampsPlatform : public InputConversionBase {
private:
    std::unique_ptr<CDoc> internal_doc;

public:
    InputConversionMampsPlatform();
    ~InputConversionMampsPlatform() override;

    InputConversionMampsPlatform(const InputConversionMampsPlatform&) = delete;
    InputConversionMampsPlatform& operator=(const InputConversionMampsPlatform&) = delete;
    InputConversionMampsPlatform(InputConversionMampsPlatform&&) = delete;
    InputConversionMampsPlatform& operator=(InputConversionMampsPlatform&&) = delete;


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
    // <applicationGraph><fsmsadfproperties>
    static void addAppFsmsadfProp(CNode *app_graph_src, CNode *dest);
    // <applicationGraph><fsm>
    static void addApplicationFsm(CNode *app_graph_src, CNode *app_graph_dest);
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
    // <applicationGraph><fsmsadfproperties><defaultProperties><ActorProperties>
    static void addAppFsmsadfPropScenarioActorProperties(CNode *app_graph_src, CNode *scenario_node);

    // <applicationGraph><fsmsadfproperties><defaultProperties><ActorProperties><Processor>
    static void addAppFsmsadfPropScenarioActorPropProcessor(const CString& actor_name,
                                                     CNode *app_graph_src,
                                                     CNode *actor_prop_src,
                                                     CNode *actor_prop_dest);
    // <applicationGraph><fsmsadfproperties><defaultProperties><ActorProperties><Processor><Memory>
    static void addAppFsmsadfPropScenarioActorPropProcessorMemory(CNode *actor_prop_src,
                                                           CNode *actor_prop_dest);

    // <applicationGraph><fsmsadfproperties><defaultProperties><ChannelProperties>
    static void addAppFsmsadfPropScenarioChannelProp(CNode *app_graph_src, CNode *scenario_node);

    // <applicationGraph><fsmsadfproperties><Scenarios>
    static void addAppFsmsadfPropScenarios(CNode *app_graph_src, CNode *fsmsadfprop_dest_node);

    // Following methods support the parsing of architectureGraph

    //<architectureGraph>
    static void addArchGraph(CNode *arch_graph_src, CNode *arch_graph_dest);

    //<architectureGraph><tile>
    static void addArchGraphTiles(CNode *arch_graph_src, CNode *arch_graph_dest);

    //<architectureGraph><connection>
    static void addArchGraphConnections(CNode *arch_graph_src, CNode *arch_graph_dest);

    //<architectureGraph><tile><processor>
    static void addArchGraphTileProcessors(CNode *arch_tile_src, CNode *arch_tile_dest);

    //<architectureGraph><tile><memory>
    static void addArchGraphTileMemories(CNode *arch_tile_mem_src, CNode *arch_tile_mem_dest);

    //<architectureGraph><tile><networkInterface>
    static void addArchGraphTileNetworkInterfaces(CNode *arch_tile_mem_src, CNode *arch_tile_mem_dest);

    //<architectureGraph><tile><processor><scheduler>
    static void addArchGraphTileProcessorsSchedulers(CNode *arch_tile_proc_src,
                                              CNode *arch_tile_proc_dest);

    //<architectureGraph><tile><processor><scheduler>
    static void addArchGraphTileProcessorsSchedulersArbitration(CNode *arch_proc_sched_src,
                                                         CNode *arch_proc_proc_dest);
};
} // namespace FSMSADF
#endif /* CONVERSION_H */
