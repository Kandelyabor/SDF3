/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   memory.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   March 30, 2006
 *
 *  Function        :   Tile memory.
 *
 *  History         :
 *      30-03-06    :   Initial version.
 *
 * $Id: memory.cc,v 1.1.1.1.2.1 2010-04-25 00:31:44 mgeilen Exp $
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

#include "memory.h"
#include <cmath>

namespace SDF {
/**
 * getSize ()
 * Get the size of the memory.
 */
CSize Memory::getSize() const {
    double sz = NAN;

    if (memActor == nullptr) {
        return 0;
    }

    sz = memActor->getMaxVal(0);

    return static_cast<CSize>(sz);
}

/**
 * setSize ()
 * Set the size of the memory. The function removes all existing component
 * bindings to the memory.
 */
void Memory::setSize(const CSize sz) {
    std::vector<double> maxVals(1);
    maxVals[0] = static_cast<double>(sz);

    // Create new component bindings component for memory
    memActor = std::make_unique<CompBindings>(maxVals, CompBindings::Type::TypeMax);
    memChannel = std::make_unique<CompBindings>(maxVals, CompBindings::Type::TypeSum);
}

/**
 * availableMemorySize ()
 * The function returns the amount of memory which is free.
 */
CSize Memory::availableMemorySize() const {
    double sz = memActor->getMaxVal(0) - memActor->getCurVal(0) - memChannel->getCurVal(0);

    return static_cast<CSize>(sz);
}

/**
 * occupiedMemorySizeByActors ()
 * The function returns the amount of memory used by the bound actors.
 */
CSize Memory::occupiedMemorySizeByActors() const {
    return static_cast<CSize>(memActor->getCurVal(0));
}

/**
 * reserveMemory ()
 * The function tries to reserve memory space with size sz for the SDF actor a.
 * On success, it returns true and it creates a binding between the actor
 * and the memory. Else, it returns false and no binding is created.
 */
bool Memory::reserveMemory(const SDFactor *a, CSize sz) {
    // Memory requirement of actor more than current usage?
    if (occupiedMemorySizeByActors() < sz) {
        // Not enough space available?
        if (availableMemorySize() < sz - occupiedMemorySizeByActors()) {
            return false;
        }
    }

    // Create component binding
    auto b = std::make_unique<ComponentBinding>(a, 1);
    b->setValue(0, static_cast<double>(sz));

    return memActor->bind(std::move(b));
}

/**
 * releaseMemory ()
 * The function releases the memory reserved by SDF actor a and removes its
 * binding to the memory.
 */
bool Memory::releaseMemory(const SDFactor *a) { return memActor->unbind(a); }

/**
 * reserveMemory ()
 * The function tries to reserve memory space with size sz for the SDF channel c.
 * On success, it returns true and it creates a binding between the channel
 * and the memory. Else, it returns false and no binding is created.
 */
bool Memory::reserveMemory(const SDFchannel *c, CSize sz) {
    // Enough space available?
    if (availableMemorySize() < sz) {
        return false;
    }

    // Create component binding
    auto b = std::make_unique<ComponentBinding>(c, 1);
    b->setValue(0, static_cast<double>(sz));

    return memChannel->bind(std::move(b));
}

/**
 * releaseMemory ()
 * The function releases the memory reserved by SDF channel c and removes its
 * binding to the memory.
 */
bool Memory::releaseMemory(const SDFchannel *c) { return memChannel->unbind(c); }

/**
 * reserveMemory ()
 * The function tries to reserve memory space with size sz. On success, it
 * returns true. Else, it returns false and no binding is created.
 */
bool Memory::reserveMemory(CSize sz) {
    // Enough space available?
    if (availableMemorySize() < sz) {
        return false;
    }

    // Create component binding
    auto b = std::make_unique<ComponentBinding>(nullptr, 1);
    b->setValue(0, static_cast<double>(sz));

    return memChannel->bind(std::move(b));
}

/**
 * releaseMemory ()
 * The function releases the memory.
 */
bool Memory::releaseMemory() { return memChannel->unbind(nullptr); }
} // namespace SDF