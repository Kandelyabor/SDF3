/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   memorytype.h
 *
 *  Author          :   Martijn Koedam (m.l.p.j.koedam@tue.nl
 *
 *  Date            :   May 09, 2011
 *
 *  Function        :   Memory Type.
 *
 *  History         :
 *      09-05-11    :   Initial version. (move from memory.h in arch)
 *
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

#ifndef SDF3_FSMSADF_BASE_MEMORY_TYPE_H_INCLUDED
#define SDF3_FSMSADF_BASE_MEMORY_TYPE_H_INCLUDED
#include "base/string/cstring.h"
namespace FSMSADF {
/**
 * Memory Type.
 * Enumeration class denoting the type of memory.
 * Instruction, Data, Communication (in/out) and shared.
 */
class MemoryType {
public:
    enum class Value {
        Instr = 0, // Instruction memory
        Data,      // Data memory
        CommIn,    // Communication  (in)
        CommOut,   // Communication (out)
        Shared     // Unspecified memory
    };

    explicit MemoryType(const CString &l) : _v(asType(l)) {}

    explicit MemoryType(const Value &l) : _v(l) {}

    [[nodiscard]] CString asString() const {
        if (_v == Value::Instr) {
            return "instr";
        }
        if (_v == Value::Data) {
            return "data";
        }
        if (_v == Value::CommIn) {
            return "commin";
        }
        if (_v == Value::CommOut) {
            return "commout";
        }
        return "shared";
    }

    [[nodiscard]] static Value asType(const CString &l) {
        if (l == "instr") {
            return Value::Instr;
        }
        if (l == "data") {
            return Value::Data;
        }
        if (l == "commin") {
            return Value::CommIn;
        }
        if (l == "commout") {
            return Value::CommOut;
        }
        return Value::Shared;
    }

    explicit operator CString() const { return asString(); }

    MemoryType& operator=(Value const &a) { 
        _v = a; 
        return *this;
    }

    MemoryType& operator=(CString const &a) {
        _v = asType(a);
        return *this;
    }

    bool operator==(Value const &b) { return this->_v == b; }
    bool operator==(MemoryType const &b) { return _v == b._v; }
    bool operator==(CString const &b) { return _v == asType(b); }

    bool operator!=(Value const &b) { return this->_v != b; }
    bool operator!=(MemoryType const &b) { return _v != b._v; }

    bool operator!=(CString const &b) { return _v != asType(b); }

private:
    Value _v;
};

} // namespace FSMSADF
#endif // SDF3_FSMSADF_BASE_MEMORY_TYPE_H_INCLUDED