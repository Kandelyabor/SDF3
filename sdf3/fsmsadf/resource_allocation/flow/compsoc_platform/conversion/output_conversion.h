/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   conversion.h
 *
 *  Author          :   Martijn Koedam (m.l.p.j.koedam@tue.nl)
 *
 *  Date            :   Mei 16, 2011
 *
 *  Function        :   Output format conversion
 *
 *  History         :
 *      16-05-11    :   Initial version.
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

#ifndef FSMSADF_RESOURCE_ALLOCATION_FLOW_COMPSOC_PLATFORM_OUTPUT_CONVERSION_CONVERSION_H_INCLUDED
#define FSMSADF_RESOURCE_ALLOCATION_FLOW_COMPSOC_PLATFORM_OUTPUT_CONVERSION_CONVERSION_H_INCLUDED

#include "fsmsadf/resource_allocation/flow/base_platform/conversion/output_conversion.h"
#ifdef MSC_VER
#include <utility>
#endif

#include "base/xml/xml.h"
#include "fsmsadf/base/memorytype.h"
#include "fsmsadf/base/type.h"



namespace FSMSADF {

/**
 * Class that takes care to convert from the platform specific input file
 * to the generic format supported by sdf3.
 */
class OutputConversionCompSoCPlatform : public OutputConversionBase {
public:
    explicit OutputConversionCompSoCPlatform(const CString& filename);
    ~OutputConversionCompSoCPlatform() override = default;

    OutputConversionCompSoCPlatform(const OutputConversionCompSoCPlatform&) = delete;
    OutputConversionCompSoCPlatform& operator=(const OutputConversionCompSoCPlatform&) = delete;
    OutputConversionCompSoCPlatform(OutputConversionCompSoCPlatform&&) = delete;
    OutputConversionCompSoCPlatform& operator=(OutputConversionCompSoCPlatform&&) = delete;

    // See the __FILE__.c file to see help for each function.

    // write communication file.
    void writeCommunicationFile(const CString &file, int mapping = -1);
    // Write mapping file.
    void writeMappingFile(const CString& output_file, int mapping = -1);
    void setApplicationInputFile(const CString &input_file);
    void setArchitectureInputFile(const CString &input_file);

private:
    void architectureInputFileReadDefaultClockSpeed();
    double architectureInputFileReadTileClockSpeed(const CString& tile_name);
    CString
    applicationInputFileGetPortDirection(const CString& app_name, const CString& actor_name, const CString& port_name);

    int
    applicationInputFileGetFifoArgument(const CString& app_name, const CString& actor_name, const CString& port_name);
    CString
    applicationInputFileGetFiringRule(const CString& app_name, const CString& actor_name, const CString& port_name);
    // Read the input file and get the dataUnit and timeUnit.
    void applicationInputFileReadUnits();
    // <communication>/<application>/<connection*>
    static void writeCommunicationAppConnections(CNode *src_app_node, CNode *dst_app_node);
    // <communication>/<application id="monitoring">
    static void writeCommunicationMonitor(CNode *dst_root);
    // <communication>/<application id="fake">
    static void writeCommunicationFakeNetwork(CNode *src_map_node, CNode *dst_root);

    static CNode *getApplicationGraph(CNode *src_map_node);
    static CNode *getArchitectureGraph(CNode *src_map_node);

    void writeMappingApplications(CNode *src_root, CNode *src_map_node, CNode *dst_root);
    static void writeMappingApplicationsProcessor(const CString& procId,
                                           CNode *src_map_node,
                                           CNode *src_proc_node,
                                           CNode *dst_app);
    static void writeApplicationProcessorTDMSlots(const CString& procId,
                                           CNode *src_map_node,
                                           CNode *src_proc_node,
                                           CNode *dst_proc_node);
    static void writeTaskScheduler(const CString& procId,
                            CNode *src_map_node,
                            CNode *src_proc_node,
                            CNode *dst_proc_node);
    static void writeSlackManager(const CString& procId, CNode *src_proc_node, CNode *dst_proc_node);
    void writeMappingProcessorTiles(CNode *src_root, CNode *src_map_node, CNode *dst_root);

    static std::list<std::pair<CString, CStrings>> getProcSchedules(const CString& procId, CNode *src_map_node);
    static Size getMemorySizeActor(CNode *src_map_node, const CString& actor_name, MemoryType mem_type);
    static int getNumFifos(CNode *src_map_node, const CString& actor_name, const CString& type);
    static CString getChannelActorOrPort(CNode *src_map_node, const CString& chan_name, const CString& type);

    static Size getChannelSize(CNode *src_map_node, const CString& chan_name);
    static CString getChannelDMA(CNode *src_tile_out, const CString& src_fifo_name);

    static Size getChannelTransactionSize(CNode *src_map_node, const CString& chan_name);
    static CNode *getChannelMemory(CNode *src_map_node, const CString& chan_name, const CString& bloc);
    static Bandwidth getChannelBandwidth(CNode *src_map_node, const CString& chan_name);
    static CNode *findChannelTile(CNode *src_map_node, const CString& chan_name, const CString& type);

    static int getMaxChannelRate(CNode *src_map_node, const CString& chan_name, const CString& loc);

    static int getNumScenarios(CNode *map_node);


    /// Application document. This one is needed as input document.
    std::unique_ptr<CDoc> application_document = nullptr;
    std::unique_ptr<CDoc> architecture_document = nullptr;

    /// The time unit size (in seconds)
    double timeUnit;
    /// The data unit (in bytes)
    double dataUnit;
    /// The number of time-units in a cycle
    /// for the NoC. This is the default 'clock'
    double cycleTime;
};
} // namespace FSMSADF
#endif /* CONVERSION_H */
