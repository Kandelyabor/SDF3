/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sadf_component.h
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   29 August 2006
 *
 *  Function        :   SADF Component
 *
 *  History         :
 *      29-08-06    :   Initial version.
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

#ifndef SADF_COMPONENT_H_INCLUDED
#define SADF_COMPONENT_H_INCLUDED

// Include defines

#include "sadf_defines.h" // IWYU pragma: keep

// SADF_Component Definition

class SADF_Component {

public:
    // Constructors

    SADF_Component();

    // Destructor

    ~SADF_Component() = default;

    SADF_Component(const SADF_Component&) = delete;
    SADF_Component& operator=(const SADF_Component&) = delete;
    SADF_Component(SADF_Component&&) = delete;
    SADF_Component& operator=(SADF_Component&&) = delete;


    // Access to instance variables

    void setIdentity(const CId ID) { Identity = ID; };
    void setName(const CString &N) { Name = N; };
    void setType(const CId T) { Type = T; };

    [[nodiscard]] CId getIdentity() const { return Identity; };
    [[nodiscard]] CString getName() const { return Name; };
    [[nodiscard]] CId getType() const { return Type; };

private:
    // Instance variables

    CId Identity;
    CString Name;
    CId Type;
};

#endif
