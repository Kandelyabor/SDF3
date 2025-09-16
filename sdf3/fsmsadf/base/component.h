/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   component.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   April 22, 1009
 *
 *  Function        :   Base component object
 *
 *  History         :
 *      22-04-09    :   Initial version.
 *
 * $Id: component.h,v 1.1.2.2 2010-04-22 07:07:10 mgeilen Exp $
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

#ifndef FSMSADF_BASE_COMPONENT_H_INCLUDED
#define FSMSADF_BASE_COMPONENT_H_INCLUDED

#include "base/base.h" // IWYU pragma: keep

namespace FSMSADF
{

    // Forward class definition
    class GraphComponent;

    /**
     * GraphComponent
     * GraphComponent object serves as basis for all components in a graph.
     */
    class GraphComponent
    {
        public:

            // Constructor
            GraphComponent(GraphComponent *parent, CId id);
            GraphComponent(GraphComponent *parent, CId id, const CString &name);

            // Destructor
            virtual ~GraphComponent() = default;

            GraphComponent(const GraphComponent&) = default;
            GraphComponent& operator=(const GraphComponent&) = default;
            GraphComponent(GraphComponent&&) = default;
            GraphComponent& operator=(GraphComponent&&) = default;

            // Id
            [[nodiscard]] CId getId() const
            {
                return id;
            };
            void setId(CId i)
            {
                id = i;
            };

            // Parent
            [[nodiscard]] GraphComponent *getParent() const
            {
                return parent;
            };
            void setParent(GraphComponent *p)
            {
                parent = p;
            };

            // Name
            [[nodiscard]] CString getName() const
            {
                return name;
            };
            void setName(const CString &n)
            {
                name = n;
            };

            static CId uniqueId() { return _uniqueId++; };

        private:
            // Parent
            GraphComponent *parent;

            // Id
            CId id;

            // Name
            CString name;

            // static ID counter
            static CId _uniqueId;
    };

} // End namespace FSMSADF

#endif
