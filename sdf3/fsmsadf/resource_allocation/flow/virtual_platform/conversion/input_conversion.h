/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   conversion.h
 *
 *  Author          :   Martijn Koedam (m.l.p.j.koedam@tue.nl)
 *
 *  Date            :   April 20, 2011
 *
 *  Function        :   Input/Output format conversion
 *
 *  History         :
 *      20-04-11    :   Initial version.
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

#ifndef FSMSADF_RESOURCE_ALLOCATION_FLOW_VIRTUAL_PLATFORM_CONVERSION_CONVERSION_H_INCLUDED
#define FSMSADF_RESOURCE_ALLOCATION_FLOW_VIRTUAL_PLATFORM_CONVERSION_CONVERSION_H_INCLUDED

#include "base/xml/xml.h"
#include "fsmsadf/resource_allocation/flow/base_platform/conversion/input_conversion.h"

namespace FSMSADF {

/**
 * Class that takes care to convert from the platform specific input file
 * to the generic format supported by sdf3.
 */
class InputConversionVirtualPlatform : public InputConversionBase {
private:
    std::unique_ptr<CDoc> internal_doc;

public:
    InputConversionVirtualPlatform();
    ~InputConversionVirtualPlatform() override;

    InputConversionVirtualPlatform(const InputConversionVirtualPlatform&) = delete;
    InputConversionVirtualPlatform& operator=(const InputConversionVirtualPlatform&) = delete;
    InputConversionVirtualPlatform(InputConversionVirtualPlatform&&) = delete;
    InputConversionVirtualPlatform& operator=(InputConversionVirtualPlatform&&) = delete;

    // Platform specific loaders
    void readApplicationFile(std::string input_file) override;
    void readArchitectureFile(std::string input_file) override;

private:
};
} // namespace FSMSADF
#endif /* CONVERSION_H */
