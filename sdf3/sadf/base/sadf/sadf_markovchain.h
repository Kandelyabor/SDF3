/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sadf_markovchain.h
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   29 August 2006
 *
 *  Function        :   SADF Markov Chain
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

#ifndef SADF_MARKOVCHAIN_H_INCLUDED
#define SADF_MARKOVCHAIN_H_INCLUDED

// Include type definitions

#include "sadf_component.h" // IWYU pragma: keep

// SADF_MarkovChain Definition

class SADF_MarkovChain {

public:
    // Constructors

    SADF_MarkovChain() = default;

    // Destructor

    virtual ~SADF_MarkovChain() = default;

    SADF_MarkovChain(const SADF_MarkovChain&) = delete;
    SADF_MarkovChain& operator=(const SADF_MarkovChain&) = delete;
    SADF_MarkovChain(SADF_MarkovChain&&) = delete;
    SADF_MarkovChain& operator=(SADF_MarkovChain&&) = delete;

    // Access to instance variables

    void setInitialState(const CId StateID) { InitialState = StateID; };
    void addState(const CString &Name, CId SubScenarioID);
    void
    setTransitionProbability(const CId SourceID, const CId DestinationID, const CDouble Weight) {
        TransitionMatrix[SourceID][DestinationID] = Weight;
    };

    [[nodiscard]] CId getInitialState() const { return InitialState; };
    CId getSubScenario(const CId StateID) { return SubScenarios[StateID]; };
    CString &getStateName(const CId StateID) { return StateSpace[StateID]; };
    CId getStateID(const CString &StateName);
    [[nodiscard]] CId getNumberOfStates() const { return static_cast<CId>(StateSpace.size()); };
    [[nodiscard]] CDouble getTransitionProbability(const CId SourceID,
                                                   const CId DestinationID) const {
        return TransitionMatrix[SourceID][DestinationID];
    };

    // Functions to determine properties

    bool isSingleComponent();
    bool isSingleStronglyConnectedComponent();
    bool isDeterministicCycle();
    std::vector<CDouble> computeEquilibriumDistribution();

private:
    // Instance variables

    CId InitialState{};
    std::vector<std::vector<CDouble>> TransitionMatrix;
    std::vector<CId> SubScenarios;
    std::vector<CString> StateSpace;

    // Functions

    void colorConnectedStates(CId StateID, std::vector<CId> &Color);
    void colorReverselyConnectedStates(CId StateID, std::vector<CId> &Color);
};

#endif
