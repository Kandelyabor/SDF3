/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   to_binding_aware.h
 *
 *  Author          :   Reinier van Kampenhout (j.r.v.kampenhout@tue.nl)
 *
 *  Date            :   December 17, 2015
 *
 *  Function        :   Transform FSM-based SADF to binding-aware graph
 *
 *  History         :
 *      17-12-15    :   Initial version.
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

#ifndef FSMSADF_TRANSFORM_TO_BINDING_AWARE_H_INCLUDED
#define FSMSADF_TRANSFORM_TO_BINDING_AWARE_H_INCLUDED

#include "fsmsadf/base/graph.h"

#include "fsmsadf/resource_allocation/platform_binding/graph.h"
#include <memory>

namespace FSMSADF
{

    /**
     * toBindingAware ()
     * The function converts the graph to a binding-aware graph.
     */
    std::unique_ptr<Graph> toBindingAware(Graph *ag, PlatformGraph *pg, PlatformBindings *pbs);

} // End namespace FSMSADF

#endif

