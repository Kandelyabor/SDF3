/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   component.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   July 13, 2005
 *
 *  Function        :   SDF base component object
 *
 *  History         :
 *      13-07-05    :   Initial version.
 *
 * $Id: component.h,v 1.2.2.1 2009-04-06 12:15:32 mgeilen Exp $
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

#ifndef SDF_BASE_UNTIMED_COMPONENT_H_INCLUDED
#define SDF_BASE_UNTIMED_COMPONENT_H_INCLUDED

#include "base/basic_types.h"
#include "base/string/cstring.h"

namespace SDF {

/**
 * SDFcomponent
 * SDF component object serves as basis for all components in an SDF graph.
 */
class SDFcomponent {
public:
    // Constructor
    explicit SDFcomponent(SDFcomponent *parent = nullptr, CId id = 0);
    SDFcomponent(SDFcomponent *parent, CId id, const CString &name);

    // Delete copy and move constructors and assignment operators
    SDFcomponent(const SDFcomponent&) = default;
    SDFcomponent& operator=(const SDFcomponent&) = default;
    SDFcomponent(SDFcomponent&&) = default;
    SDFcomponent& operator=(SDFcomponent&&) = default;

    // Destructor
    virtual ~SDFcomponent()= default;

    // Information
    [[nodiscard]] CId getId() const { return id; };
    void setId(CId i) { id = i; };
    [[nodiscard]] SDFcomponent *getParent() const { return parent; };
    [[nodiscard]] CString getName() const { return name; };
    void setName(const CString &n) { name = n; };

private:
    // Information
    SDFcomponent *parent;
    CId id;
    CString name;
};

} // namespace SDF
#endif
