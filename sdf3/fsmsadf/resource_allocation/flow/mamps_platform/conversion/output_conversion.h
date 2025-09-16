/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   output_conversion.h
 *
 *  Author          :   Umar Waqas (u.waqas@student.tue.nl)
 *
 *  Date            :   November 16, 2011
 *
 *  Function        :   Convert the output files to a compatible format.
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

#ifndef FSMSADF_RESOURCE_ALLOCATION_FLOW_MAMPS_PLATFORM_OUTPUT_CONVERSION_CONVERSION_H_INCLUDED
#define FSMSADF_RESOURCE_ALLOCATION_FLOW_MAMPS_PLATFORM_OUTPUT_CONVERSION_CONVERSION_H_INCLUDED

#include "base/xml/xml.h"
#include "fsmsadf/resource_allocation/flow/base_platform/conversion/output_conversion.h"
#include "fsmsadf/resource_allocation/flow/compsoc_platform/conversion/helper.h" // IWYU pragma: keep
#include <iostream>
#include <ostream>

namespace FSMSADF {

/**
 * OutputConversionMampsPlatform provides implementation
 * of the output conversion layer to make MAMPS-SDF3 compatible
 * with each other.
 *
 */
class OutputConversionMampsPlatform : public OutputConversionBase {
private:
    std::unique_ptr<CDoc> output_doc;

public:
    explicit OutputConversionMampsPlatform(const CString &input_filename);
    ~OutputConversionMampsPlatform() override;

    OutputConversionMampsPlatform(const OutputConversionMampsPlatform &) = delete;
    OutputConversionMampsPlatform &operator=(const OutputConversionMampsPlatform &) = delete;
    OutputConversionMampsPlatform(OutputConversionMampsPlatform &&) = delete;
    OutputConversionMampsPlatform &operator=(OutputConversionMampsPlatform &&) = delete;

    void convert();
    // write the result
    void writeOutput(std::ostream &out);
    void writeOutput(const CString &outfile);

private:
    /* helper methods supporting the conversion */
    static void convertMapping(CNode *sdf3_src, CNode *sdf3_dst);
    static void convertRoot(CNode *sdf3_src, CNode *sdf3_dst);
    static void convertTile(
            CNode *arch_src, CNode *appGraph, CNode *sdf3_src, CNode *tile_src, CNode *tile_dst);
    static void convertProcessor(CNode *arch_src, CNode *tile_src, CNode *processor_dst);
    static void convertMemory(CNode *appGraph, CNode *sdf3_src, CNode *tile_src, CNode *memory_dst);
    static void convertActor(CNode *appGraph, CNode *actor_src, CNode *memory_dst);
    static void
    convertChannel(CNode *appGraph, CNode *sdf3_src, CNode *channel_src, CNode *memory_dst);
    static void convertNetworkInterface(CNode *appGraph,
                                        CNode *sdf3_src,
                                        CNode *ni_src,
                                        CNode *tile_dst,
                                        const CString &tileName);
    static void convertNetworkInterfaceChannel(CNode *resourceUsage,
                                               CNode *ni_channel_src,
                                               CNode *ni_dst,
                                               bool hasInAttribute,
                                               const CString &ni_name,
                                               const CString &tileName);
    static void convertUseCaseSchedule(CNode *sdf3_src, CNode *sdf3_dst);
    static void
    convertUseCaseScheduleTile(CNode *tileSrc, CNode *applicationSchedule, const CString &name);
    static void convertConnections(CNode *mapping_src, CNode *mapping_dst);
};
} // namespace FSMSADF
#endif /* CONVERSION_H */
