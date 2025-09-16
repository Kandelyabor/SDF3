/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sadf_transition.h
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   10 October 2006
 *
 *  Function        :   SADF Transition
 *
 *  History         :
 *      10-10-06    :   Initial version.
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

#ifndef SADF_TRANSITION_H_INCLUDED
#define SADF_TRANSITION_H_INCLUDED

// Include type definitions

#include "sadf/base/sadf/sadf_graph.h" // IWYU pragma: keep

// Forward declaration

class SADF_Configuration;

// SADF_Transition Definition

class SADF_Transition {

public:
    // Constructor
    SADF_Transition(SADF_Configuration *D, CDouble P, CDouble T);

    // Destructor
    ~SADF_Transition() = default;

    SADF_Transition(const SADF_Transition&) = default;
    SADF_Transition& operator=(const SADF_Transition&) = delete;
    SADF_Transition(SADF_Transition&&) = delete;
    SADF_Transition& operator=(SADF_Transition&&) = delete;


    // Access to instance variables

    [[nodiscard]] SADF_Configuration *getDestination() const { return Destination; };
    [[nodiscard]] CDouble getProbability() const { return Probability; };
    [[nodiscard]] CDouble getTimeSample() const { return TimeSample; };

    void setProbability(const CDouble P) { Probability = P; };

private:
    // Instance Variables

    SADF_Configuration *Destination;
    CDouble Probability;
    CDouble TimeSample;
};

class SADF_ListOfTransitions: public std::list<std::unique_ptr<SADF_Transition>> {
public:
    SADF_ListOfTransitions() = default;
    ~SADF_ListOfTransitions() = default;

    // Delete copy/move constructors and assignment operators
    SADF_ListOfTransitions(const SADF_ListOfTransitions&);
    SADF_ListOfTransitions& operator=(const SADF_ListOfTransitions&) = delete;
    SADF_ListOfTransitions(SADF_ListOfTransitions&&) = delete;
    SADF_ListOfTransitions& operator=(SADF_ListOfTransitions&&) = delete;
};

#endif
