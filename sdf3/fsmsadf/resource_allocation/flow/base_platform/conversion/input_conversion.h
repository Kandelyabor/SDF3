/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   conversion.h
 *
 *  Author          :   Martijn Koedam (m.l.p.j.koedam@tue.nl)
 *
 *  Date            :   April 19, 2011
 *
 *  Function        :   Input/Output format conversion
 *
 *  History         :
 *      19-04-11    :   Initial version.
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

#ifndef FSMSADF_RESOURCE_ALLOCATION_FLOW_CONVERSION_CONVERSION_H_INCLUDED
#define FSMSADF_RESOURCE_ALLOCATION_FLOW_CONVERSION_CONVERSION_H_INCLUDED
#include <iostream>
#include <ostream>

#include "base/xml/xml.h"
namespace FSMSADF {

/**
 * Class that takes care to convert from the platform specific input file
 * to the generic format supported by sdf3.
 */
class InputConversionBase {
private:
    std::unique_ptr<CDoc> output_doc;

public:
    InputConversionBase();
    virtual ~InputConversionBase() = default;

    InputConversionBase(const InputConversionBase&) = delete;
    InputConversionBase& operator=(const InputConversionBase&) = delete;
    InputConversionBase(InputConversionBase&&) = delete;
    InputConversionBase& operator=(InputConversionBase&&) = delete;

    // Platform specific loaders
    virtual void readApplicationFile(std::string input_file) = 0;
    virtual void readArchitectureFile(std::string input_file) = 0;

    // Platform based implementations want to make custom readInput()
    // functions.
    // write the result
    void writeOutput(std::ostream &out);
    void writeOutput(const std::string& outfile);

protected:
    [[nodiscard]] CDoc *get_output_doc() const { return this->output_doc.get();};
    void set_output_doc(std::unique_ptr<CDoc> new_output_doc) {this->output_doc = std::move(new_output_doc);};

};
} // namespace FSMSADF
#endif /* CONVERSION_H */
