/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   scenario.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   April 22, 1009
 *
 *  Function        :   FSM-based SADF scenario
 *
 *  History         :
 *      22-04-09    :   Initial version.
 *
 * $Id: scenario.h,v 1.1.2.2 2010-04-22 07:07:10 mgeilen Exp $
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

#ifndef FSMSADF_BASE_SCENARIO_H_INCLUDED
#define FSMSADF_BASE_SCENARIO_H_INCLUDED

#include "component.h"

namespace FSMSADF {

// Forward class definition
class Scenario;
class ScenarioGraph;
class Graph;

/**
 * Repetition vector
 */
using RepetitionVector = std::vector<int>;

/**
 * Scenario
 * Scenario in FSM-based SADF graph
 */
class Scenario : public FSMSADF::GraphComponent {
public:
    // Constructor
    explicit Scenario(const FSMSADF::GraphComponent &c);

    // Destructor
    ~Scenario() override;

    // copy/move constructors and assignment operators
    Scenario(const Scenario&) = default;
    Scenario& operator=(const Scenario&) = default;
    Scenario(Scenario&&) = delete;
    Scenario& operator=(Scenario&&) = delete;

    // Construct
    [[nodiscard]] static std::unique_ptr<Scenario> create(const GraphComponent& c);
    [[nodiscard]] std::unique_ptr<Scenario> createCopy(const GraphComponent& c) const;
    [[nodiscard]] std::unique_ptr<Scenario> clone(const GraphComponent& c) const;

    // Construct
    void constructFromXML(CNodePtr scenarioNode);

    // Convert
    void convertToXML( CNodePtr scenarioNode);

    // Scenario graph
    [[nodiscard]] ScenarioGraph *getScenarioGraph() const { return scenarioGraph; };
    void setScenarioGraph(ScenarioGraph *g) { scenarioGraph = g; };

    // Graph
    [[nodiscard]] Graph *getGraph() const;

    // Print
    std::ostream &print(std::ostream &out);
    friend std::ostream &operator<<(std::ostream &out, Scenario &s) { return s.print(out); };

    // reward
    [[nodiscard]] CDouble getReward() const { return reward; };
    void setReward(CDouble r) { reward = r; };

    // properties
    bool allActorsHaveRepetitions();
    bool noActorsHaveRepetitions();
    void setRepetitionVector(const RepetitionVector &q);

    // compute repetition vector
    [[nodiscard]] RepetitionVector computeRepetitionVector() const;
    // compute repetition vector
    [[nodiscard]] RepetitionVector computeRepetitionVectorOfScenarioGraph(ScenarioGraph *sg) const ;

    // check consistency
    bool isScenarioGraphConsistent() const;

private:
    // Scenario graph
    ScenarioGraph *scenarioGraph;

    // reward (amount of progress represented by this scenario
    CDouble reward;
};
using ScenarioRef = Scenario *;
using ScenarioConstRef = const Scenario *;

// Set of scenarios
struct ScenarioLess {
    bool operator()(std::unique_ptr<Scenario> const &lhs, std::unique_ptr<Scenario> const &rhs) const {
        return lhs->getId() < rhs->getId();
    };
};
using Scenarios = std::set<std::unique_ptr<Scenario> , ScenarioLess>;
struct ScenarioRefLess {
    bool operator()(const ScenarioRef &lhs, const ScenarioRef &rhs) const {
        return lhs->getId() < rhs->getId();
    };
};
using ScenarioRefs = std::set<ScenarioRef, ScenarioRefLess>;
// List of scenarios
using ScenarioList = std::list<std::unique_ptr<Scenario> >;
using ScenarioRefList = std::list<ScenarioRef>;
using ScenarioConstRefList = std::list<ScenarioConstRef>;

} // End namespace FSMSADF


// Custom overload of "less" function on set of pointers
// to avoid undeterministic ordering in sets and maps
template <> struct std::less<std::unique_ptr<FSMSADF::Scenario> > {
    bool operator()(const std::unique_ptr<FSMSADF::Scenario> &lhs, const std::unique_ptr<FSMSADF::Scenario> &rhs) const {
        { return lhs->getId() < rhs->getId(); }
    }
};

// Custom overload of "less" function on set of pointers
// to avoid undeterministic ordering in sets and maps
template <> struct std::less<FSMSADF::ScenarioRef> { // NOLINT(cert-dcl58-cpp)
    bool operator()(const FSMSADF::ScenarioRef &lhs,
                    const FSMSADF::ScenarioRef &rhs) const {
        {
            return lhs->getId() < rhs->getId();
        }
    }
};

// Custom overload of "less" function on set of pointers
// to avoid undeterministic ordering in sets and maps
template <> struct std::less<const FSMSADF::Scenario*> { // NOLINT(cert-dcl58-cpp)
    bool operator()(const FSMSADF::Scenario* lhs, const FSMSADF::Scenario* rhs) const {
        {
            return lhs->getId() < rhs->getId();
        }
    }
};


#endif
