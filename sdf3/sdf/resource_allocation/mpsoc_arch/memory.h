/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   memory.h
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
 * $Id: memory.h,v 1.1.1.1 2007-10-02 10:59:47 sander Exp $
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

#ifndef SDF_RESOURCE_ALLOCATION_MPSOC_MEMORY_H_INCLUDED
#define SDF_RESOURCE_ALLOCATION_MPSOC_MEMORY_H_INCLUDED

#include "component.h"
#include "binding.h"
namespace SDF
{
    /**
     * Memory
     * Container for tile memory.
     */
    class Memory : public ArchComponent
    {
        public:

            // Constructor
            explicit Memory(ArchComponent &c) : ArchComponent(c)
            {
            };

            // Delete copy/move constructors and assignment operators
            Memory(const Memory&) = delete;
            Memory& operator=(const Memory&) = delete;
            Memory(Memory&&) = delete;
            Memory& operator=(Memory&&) = delete;

            // Destructor
             ~Memory() override = default;

            // Size
            [[nodiscard]] CSize getSize() const;
            void setSize( CSize sz);
            [[nodiscard]] CSize availableMemorySize() const;
            [[nodiscard]] CSize occupiedMemorySizeByActors() const;
            bool reserveMemory(const SDFactor *a, CSize sz);
            bool releaseMemory(const SDFactor *a);
            bool reserveMemory(const SDFchannel *c, CSize sz);
            bool releaseMemory(const SDFchannel *c);
            bool reserveMemory(CSize sz);
            bool releaseMemory();

            // Bindings
            [[nodiscard]] CompBindings *getActorBindings() const
            {
                return memActor.get();
            };
            [[nodiscard]] CompBindings *getChannelBindings() const
            {
                return memChannel.get();
            };

        private:
            // Size
            std::unique_ptr<CompBindings> memActor;
            std::unique_ptr<CompBindings> memChannel;
    };

    using MemoryRefs = std::list<Memory *>;

}//namespace SDF
#endif
