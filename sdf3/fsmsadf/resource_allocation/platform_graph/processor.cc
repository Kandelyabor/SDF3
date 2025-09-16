/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   processor.cc
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
 * $Id: processor.cc,v 1.1 2009-12-23 13:37:23 sander Exp $
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

#include "processor.h"
#include "tile.h"

namespace FSMSADF {

/**
 * Processor()
 * Constructor.
 */
Processor::Processor(const GraphComponent &c) :
    GraphComponent(c),
    wheelsize(0),
    contextSwitchOverhead(0),
    slotLength(0),
    supportsPreemption(true) {}

/**
 * ~Processor()
 * Destructor.
 */
Processor::~Processor() = default;

/**
 * create()
 * The function returns a new processor object.
 */
std::unique_ptr<Processor> Processor::create(const GraphComponent &c) { return std::make_unique<Processor>(c); }

/**
 * createCopy()
 * The function returns a new processor object and copies all properties.
 */
std::unique_ptr<Processor> Processor::createCopy(const GraphComponent &c) const {
    std::unique_ptr<Processor> p = create(c);

    // Properties
    p->setName(getName());
    p->setType(getType());
    p->setArbitrationType(getArbitrationType());
    p->setWheelsize(getWheelsize());
    p->setSlotLength(getSlotLength());
    p->setContextSwitchOverhead(getContextSwitchOverhead());
    p->setSupportForPreemption(hasSupportForPreemption());

    // SystemCosts
    for (const auto& sc : systemCosts) {
        p->addSystemCost(sc->createCopy(GraphComponent(p.get(), getId(), getName())));
    }

    return p;
}

/**
 * clone()
 * The function returns a new processor object and copies all properties, but
 * when the parent of the graph component c and this object are different, all
 * pointers are relocated.
 */
std::unique_ptr<Processor> Processor::clone(const GraphComponent &c) const {
    std::unique_ptr<Processor> p = createCopy(c);

    // SystemCosts
    for (const auto& sc : systemCosts) {
        p->addSystemCost(sc->clone(GraphComponent(p.get(), getId(), getName())));
    }
    return p;
}

/**
 * constructFromXML()
 * Load all properties of this processor from the supplied XML node.
 */
void Processor::constructFromXML(const CNodePtr processorNode) {
    // Name
    if (!CHasAttribute(processorNode, "name")) {
        throw CException("Invalid graph, missing processor name.");
    }
    setName(CGetAttribute(processorNode, "name"));

    // Type
    if (!CHasAttribute(processorNode, "type")) {
        throw CException("Invalid graph, missing processor type.");
    }
    setType(CGetAttribute(processorNode, "type"));

    // Arbitration
    if (CHasChildNode(processorNode, "arbitration")) {
        CNode *arbitrationNode = CGetChildNode(processorNode, "arbitration");

        // Type
        if (!CHasAttribute(arbitrationNode, "type")) {
            throw CException("Invalid graph, missing arbitration type.");
        }
        setArbitrationType(CGetAttribute(arbitrationNode, "type"));

        // Only supported type is TDMA
        if (getArbitrationType() != "TDMA") {
            throw CException("Invalid graph, arbitration type not supported.");
        }

        // Wheelsize
        if (!CHasAttribute(arbitrationNode, "wheelsize")) {
            throw CException("Invalid graph, missing TDMA wheelsize.");
        }
        double wheelsize = static_cast<CDouble>(CGetAttribute(arbitrationNode, "wheelsize"));
        setWheelsize(static_cast<Time>(wheelsize));

        // Context switch overhead
        if (!CHasAttribute(arbitrationNode, "contextSwitchOverhead")) {
            throw CException("Invalid graph, missing contextSwitchOverhead.");
        }
        double contextSwitchOverhead =
                static_cast<CDouble>(CGetAttribute(arbitrationNode, "contextSwitchOverhead"));
        setContextSwitchOverhead(static_cast<Time>(contextSwitchOverhead));

        // Slot length
        if (CHasAttribute(arbitrationNode, "slotLength")) {
            double slotLength = static_cast<CDouble>(CGetAttribute(arbitrationNode, "slotLength"));
            setSlotLength(static_cast<Time>(slotLength));
        }

        // Arbitration sanity checks:
        ASSERT(getWheelsize() >= (getContextSwitchOverhead() + getSlotLength()),
               "Error constructing architecture: "
               "Wheelsize must be larger than ContextSwitchOverhead + SlotLength.");
        ASSERT((getWheelsize() % (getContextSwitchOverhead() + getSlotLength())) == 0,
               "Error constructing architecture: "
               "Wheelsize is not an integer multiple of ContextSwitchOverhead + SlotLength");

        // TDMA support for preemption
        if (!CHasAttribute(arbitrationNode, "preemptive")) {
            throw CException("Invalid graph, missing preemptive.");
        }
        if (CGetAttribute(arbitrationNode, "preemptive") == "true") {
            setSupportForPreemption(true);
        } else {
            setSupportForPreemption(false);
        }
    }

    // system Cost
    if (CHasChildNode(processorNode, "systemCost")) {
        CNode *systemCostNode = CGetChildNode(processorNode, "systemCost");

        SystemCost *sc = createSystemCost("initial");
        sc->constructFromXML(systemCostNode);
    }
}

/**
 * convertToXML()
 * Add all properties of this processor to the supplied XML node.
 */
void Processor::convertToXML(const CNodePtr processorNode) {
    // Name
    CAddAttribute(processorNode, "name", getName());

    // Type
    CAddAttribute(processorNode, "type", getType());

    // Arbitration
    CNode *arbitrationNode = CAddNode(processorNode, "arbitration");

    // Arbitration type
    CAddAttribute(arbitrationNode, "type", getArbitrationType());

    // Wheelsize
    CAddAttribute(arbitrationNode, "wheelsize", CString(getWheelsize()));

    // slotLength
    CAddAttribute(arbitrationNode, "slotLength", CString(getSlotLength()));

    // Context switch overhead
    CAddAttribute(arbitrationNode, "contextSwitchOverhead", CString(getContextSwitchOverhead()));

    // TDMA support for preemption
    if (hasSupportForPreemption()) {
        CAddAttribute(arbitrationNode, "preemptive", "true");
    } else {
        CAddAttribute(arbitrationNode, "preemptive", "false");
    }

    // SystemCosts
    for (const auto& sc : systemCosts) {
        sc->convertToXML(CAddNode(processorNode, "systemCost"));
    }
}

/**
 * getSystemCost()
 * The function returns a pointer to a systemCost with the supplied name. When
 * no such systemCost exists, an exception is thrown.
 */
SystemCost *Processor::getSystemCost(const CString &name) const {
    for (const auto& sc : systemCosts) {
        if (name == sc->getName()) {
            return sc.get();
        }
    }

    throw CException("Processor does not contain a systemCost with name '" + name
                     + "' (perhaps systemCost is not defined in the XML?)).");
}

/**
 * createSystemCost()
 * The function creates a new systemCost and adds it to this processor.
 */
SystemCost *Processor::createSystemCost(const CString &name) {
    auto p = std::make_unique<SystemCost>(GraphComponent(this, static_cast<CId>(systemCosts.size()), name));
    auto* pr = p.get();
    addSystemCost(std::move(p));
    return pr;
}

/**
 * addSystemCost()
 * The function adds systemCost p to this processor.
 */
void Processor::addSystemCost(std::unique_ptr<SystemCost> sc) { systemCosts.push_back(std::move(sc)); }

/**
 * removeSystemCost()
 * The function removes the systemCost p from the processor.
 */
void Processor::removeSystemCost(SystemCost *sc) {
    for (auto i = systemCosts.begin(); i != systemCosts.end(); i++) {
        if (sc->getName() == (*i)->getName()) {
            systemCosts.erase(i);
            break;
        }
    }
}

// Tile
Tile *Processor::getTile() const {
    return dynamic_cast<Tile *>(getParent());
};


} // End namespace FSMSADF
