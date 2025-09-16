/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sadf_control_status.h
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   29 September 2006
 *
 *  Function        :   SADF Control Status
 *
 *  History         :
 *      29-09-06    :   Initial version.
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

#ifndef SADF_CONTROL_STATUS_H_INCLUDED
#define SADF_CONTROL_STATUS_H_INCLUDED

// Include type definitions

#include "sadf/base/sadf/sadf_graph.h" // IWYU pragma: keep

// SADF_ControlStatus Definition

class SADF_ControlStatus {

public:
    // Constructor

    explicit SADF_ControlStatus(SADF_Channel *C);
    explicit SADF_ControlStatus(SADF_ControlStatus *S);

    // Destructor

    ~SADF_ControlStatus() = default;

    SADF_ControlStatus(const SADF_ControlStatus&) = delete;
    SADF_ControlStatus& operator=(const SADF_ControlStatus&) = delete;
    SADF_ControlStatus(SADF_ControlStatus&&) = delete;
    SADF_ControlStatus& operator=(SADF_ControlStatus&&) = delete;

    // Access to current status

    [[nodiscard]] CId getAvailableTokens() const { return AvailableTokens; };
    [[nodiscard]] CId getReservedLocations() const { return ReservedLocations; };
    [[nodiscard]] CId getOccupation() const { return AvailableTokens + ReservedLocations; };

    CQueue &getContentQueue() { return ContentQueue; };
    CQueue &getNumbersQueue() { return NumbersQueue; };

    // Functions to change status

    void reserve(CId NumberOfTokens);
    void write(CId NumberOfTokens, CId ScenarioID);
    CId inspect();
    void remove();

    // Equality operator

    bool equal(SADF_ControlStatus *S);

private:
    // instance variables

    CId AvailableTokens;
    CId ReservedLocations;

    CQueue ContentQueue;
    CQueue NumbersQueue;
};

#endif
