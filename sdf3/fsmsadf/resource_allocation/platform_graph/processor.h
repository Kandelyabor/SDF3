/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   processor.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   May 12, 2009
 *
 *  Function        :   Tile processor.
 *
 *  History         :
 *      12-05-09    :   Initial version.
 *
 * $Id: processor.h,v 1.1 2009-12-23 13:37:23 sander Exp $
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

#ifndef FSMSADF_RESOURCE_ALLOCATION_PLATFORM_GRAPH_PROCESSOR_H_INCLUDED
#define FSMSADF_RESOURCE_ALLOCATION_PLATFORM_GRAPH_PROCESSOR_H_INCLUDED

#include "fsmsadf/base/type.h"
#include "systemcost.h"
#include <memory>

namespace FSMSADF {

// Forward class definition
class Processor;
class Tile;

// List of processor pointers
using Processors = std::list<std::unique_ptr<Processor>>;
using ProcessorRefs = std::list<Processor *>;

/**
 * Processor
 * Container for processor.
 */
class Processor : public GraphComponent {
public:
    // Constructor
    explicit Processor(const GraphComponent &c);

    // Destructor
    ~Processor() override;

    Processor(const Processor &) = delete;
    Processor &operator=(const Processor &) = delete;
    Processor(Processor &&) = delete;
    Processor &operator=(Processor &&) = delete;

    // Construct
    [[nodiscard]] static std::unique_ptr<Processor> create(const GraphComponent &c);
    [[nodiscard]] std::unique_ptr<Processor> createCopy(const GraphComponent &c) const;
    [[nodiscard]] std::unique_ptr<Processor> clone(const GraphComponent &c) const;

    // Construct
    void constructFromXML(CNodePtr processorNode);

    // Convert
    void convertToXML(CNodePtr processorNode);

    // Type
    [[nodiscard]] CString getType() const { return type; };
    void setType(const CString &t) { type = t; };

    // Arbitration type
    [[nodiscard]] CString getArbitrationType() const { return arbitrationType; };
    void setArbitrationType(const CString &t) { arbitrationType = t; };

    // TDMA timewheel
    [[nodiscard]] Time getWheelsize() const { return wheelsize; };
    void setWheelsize(const Time sz) { wheelsize = sz; };

    // TDMA context switch overhead
    [[nodiscard]] Time getContextSwitchOverhead() const { return contextSwitchOverhead; };
    void setContextSwitchOverhead(const Time sz) { contextSwitchOverhead = sz; };

    // TDMA slotLength
    [[nodiscard]] Time getSlotLength() const { return slotLength; };
    void setSlotLength(const Time sz) { slotLength = sz; };
    // TDMA support for preemption
    [[nodiscard]] bool hasSupportForPreemption() const { return supportsPreemption; };
    void setSupportForPreemption(const bool f) { supportsPreemption = f; };

    // Tile
    [[nodiscard]] Tile *getTile() const;

    // SystemCost
    SystemCosts &getSystemCosts() { return systemCosts; };
    [[nodiscard]] SystemCost *getSystemCost(const CString &name) const;
    SystemCost *createSystemCost(const CString &name);
    void addSystemCost(std::unique_ptr<SystemCost> sc);
    void removeSystemCost(SystemCost *sc);

private:
    // Type
    CString type;

    // Arbitration type
    CString arbitrationType;

    // TDMA timewheel
    Time wheelsize;

    // TDMA context switch overhead
    Time contextSwitchOverhead;

    // TDMA time slot
    Time slotLength;

    // TDMA support for preemption
    bool supportsPreemption;

    // SystemCost
    SystemCosts systemCosts;
};

} // End namespace FSMSADF

// Custom overload of "less" function on set of pointers
// to avoid undeterministic ordering in sets and maps
// NOLINTNEXTLINE(cert-dcl58-cpp)
template <> struct std::less<FSMSADF::Processor *> {
    bool operator()(const FSMSADF::Processor *const &lhs,
                    const FSMSADF::Processor *const &rhs) const {
        {
            return lhs->getId() < rhs->getId();
        }
    }
};

#endif
