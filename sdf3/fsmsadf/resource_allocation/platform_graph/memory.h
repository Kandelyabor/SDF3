/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   memory.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   May 12, 2009
 *
 *  Function        :   Tile memory.
 *
 *  History         :
 *      04-05-11    :   Add support for multiple memory types (M.Koedam)
 *      12-05-09    :   Initial version.
 *
 * $Id: memory.h,v 1.1 2009-12-23 13:37:22 sander Exp $
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

#ifndef FSMSADF_RESOURCE_ALLOCATION_PLATFORM_GRAPH_MEMORY_H_INCLUDED
#define FSMSADF_RESOURCE_ALLOCATION_PLATFORM_GRAPH_MEMORY_H_INCLUDED

#include "fsmsadf/base/type.h"
#include "fsmsadf/base/memorytype.h"

namespace FSMSADF {

// Forward class definition
class Memory;
class Tile;

// List of memory pointers
using Memories = std::list<std::unique_ptr<Memory>>;
class MemoryRefs: public std::list<Memory*>{
public:
    explicit MemoryRefs() = default;

    explicit MemoryRefs(const Memories& m) {
        for (const auto& mem: m) {
            this->push_back(mem.get());
        }
    }
};

/**
 * Memory
 * Container for tile memory.
 */
class Memory : public GraphComponent {
public:
    // Constructor
    explicit Memory(const GraphComponent& c);

    // Destructor
    ~Memory() override;

    Memory(const Memory&) = delete;
    Memory& operator=(const Memory&) = delete;
    Memory(Memory&&) = delete;
    Memory& operator=(Memory&&) = delete;

    // Construct
    [[nodiscard]] static std::unique_ptr<Memory> create(const GraphComponent& c);
    [[nodiscard]] std::unique_ptr<Memory> createCopy(const GraphComponent& c) const;
    [[nodiscard]] std::unique_ptr<Memory> clone(const GraphComponent& c) const;

    // Construct
    void constructFromXML( CNodePtr memoryNode);

    // Convert
    void convertToXML( CNodePtr memoryNode);

    // Size
    [[nodiscard]] Size getSize() const { return size; };
    void setSize(const Size sz) { size = sz; };

    // Tile
    [[nodiscard]] Tile *getTile() const;

    // Type
    [[nodiscard]] MemoryType getType() const { return type; };

    void setType(MemoryType::Value t) { type = t; };
    void setType(const CString& t) { type = t; };

private:
    // Size
    Size size;
    MemoryType type;
};

} // End namespace FSMSADF

// Custom overload of "less" function on set of pointers
// to avoid undeterministic ordering in sets and maps
// NOLINTNEXTLINE(cert-dcl58-cpp)
template <> struct std::less<FSMSADF::Memory *> {
    bool operator()(const FSMSADF::Memory *const &lhs,
                    const FSMSADF::Memory *const &rhs) const {
        {
            return lhs->getId() < rhs->getId();
        }
    }
};


#endif
