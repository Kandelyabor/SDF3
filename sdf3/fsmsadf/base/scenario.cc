/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   scenario.cc
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
 * $Id: scenario.cc,v 1.1.2.2 2010-04-22 07:07:10 mgeilen Exp $
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

#include "scenario.h"
#include "fsmsadf/base/actor.h"
#include "graph.h"
#include <algorithm>
#include <memory>

namespace FSMSADF {

/**
 * Scenario ()
 * Constructor.
 */
Scenario::Scenario(const GraphComponent &c) :
    GraphComponent(c), scenarioGraph(nullptr), reward(1.0) {}

/**
 * ~Scenario ()
 * Destructor.
 */
Scenario::~Scenario() = default;

/**
 * create ()
 * The function returns a pointer to a newly allocated scenario object.
 */
std::unique_ptr<Scenario> Scenario::create(const GraphComponent &c) { return std::make_unique<Scenario>(c); }

/**
 * createCopy ()
 * The function returns a pointer to a newly allocated scenario object.
 * The name of the new scenario is equal to the this scenario. However, the
 * scenario graph is not copied. To also initialize the scenario graph, you
 * should use the clone function.
 */
std::unique_ptr<Scenario> Scenario::createCopy(const GraphComponent &c) const {
    auto s = std::make_unique<Scenario>(c);

    // Properties
    s->setName(this->getName());
    s->setReward(this->getReward());

    return s;
}

/**
 * clone ()
 * The function returns a pointer to a newly allocated scenario object.
 * All properties of the scenario are cloned.
 */
std::unique_ptr<Scenario> Scenario::clone(const GraphComponent &c) const {
    std::unique_ptr<Scenario> s = createCopy(c);

    // Scenario graph
    s->setScenarioGraph(s->getGraph()->getScenarioGraph(getScenarioGraph()->getName()));
    s->setReward(this->getReward());

    return s;
}

/**
 * constructFromXML ()
 * The function initializes all actor properties based on the XML data.
 */
void Scenario::constructFromXML(const CNodePtr scenarioNode) {
    Scenario *s = this;
    ScenarioGraph *sg = nullptr;

    // Name
    if (!CHasAttribute(scenarioNode, "name")) {
        throw CException("Invalid scenario, missing actor name.");
    }
    s->setName(CGetAttribute(scenarioNode, "name"));

    // Graph
    if (!CHasAttribute(scenarioNode, "graph")) {
        throw CException("Invalid scenario, missing graph name.");
    }
    sg = getGraph()->getScenarioGraph(CGetAttribute(scenarioNode, "graph"));
    s->setScenarioGraph(sg);
    sg->constructPropertiesFromXML(*s, scenarioNode);
}

/**
 * convertToXML ()
 * The function converts the scenario to XML data.
 */
void Scenario::convertToXML(const CNodePtr scenarioNode) {
    ScenarioGraph *sg = nullptr;

    // Name
    CAddAttribute(scenarioNode, "name", getName());

    // Graph
    sg = getScenarioGraph();
    CAddAttribute(scenarioNode, "graph", sg->getName());
    sg->convertPropertiesToXML(*this, scenarioNode);
}

bool Scenario::allActorsHaveRepetitions() {
    const Actors &actors = this->scenarioGraph->getActors();
    return std::all_of(actors.begin(), actors.end(), [this](const std::unique_ptr<Actor>& a) {
        return a->hasRepetitionsOfScenario(*this);
    });
}

bool Scenario::noActorsHaveRepetitions() {
    const Actors &actors = this->scenarioGraph->getActors();
    return std::all_of(actors.begin(), actors.end(), [this](const std::unique_ptr<Actor>& a) {
        return !a->hasRepetitionsOfScenario(*this);
    });
}

void Scenario::setRepetitionVector(const RepetitionVector &q) {
    Actors &actors = this->scenarioGraph->getActors();
    auto j = q.begin();
    for (const auto& a : actors) {
        a->setRepetitionsOfScenario(*this, (*j));
        j++;
    }
}

Graph *Scenario::getGraph() const {
    return dynamic_cast<Graph *>(getParent());
}


namespace {

/**
 * calcFractionsConnectedActors ()
 * The function calculates and firing ration (as fractions) of
 * all actors connected to actor 'a' based on its firing rate. In case
 * of an inconsistent graph, all fractions are set to 0.
 * @TODO largely copies similar function in sdf/base/algo/repetition_vector.cc
 * preferably factor out
 */
void calcFractionsConnectedActors(CFractions &fractions, ActorRef a, const Scenario& s) {
    CFraction fractionA = fractions[a->getId()];

    // Inconsistent graph?
    if (fractionA == CFraction(0, 0)) {
        return;
    }

    // Calculate the rate for each actor 'b' connected to actor 'a'
    for (const auto& pA : a->getPorts()) {
        // Get actor 'b' on other side of channel and its port 'pB'
        Channel *c = pA->getChannel();
        ASSERT(c != nullptr,
               "Port '" + pA->getName() + "' on actor: '" + a->getName() + "' in scenario: '"
                       + s.getName() + "' has no channel.");
        Port *pB = c->getSrcPort();
        ActorRef b = &(pB->getActor());
        if (a->getId() == b->getId()) {
            pB = c->getDstPort();
            b = &(pB->getActor());
        }

        // Calculate firing rate 'b'
        CFraction ratioAB;
        FSMSADF::Rate pBr = pB->getRateOfScenario(s);
        FSMSADF::Rate pAr = pA->getRateOfScenario(s);
        if (pBr == 0) {
            if (pAr == 0) {
                // both rates are zero. The edge can be ignored
            } else {
                // rate A is not zero, but B is. Graph cannot be consistent, unless repetition of A
                // is zero
                if (fractionA != CFraction(0, 1)) {
                    // inconsistent
                    for (auto &fraction : fractions) {
                        fraction = CFraction(0, 0);
                    }
                }
                return;
            }
        } else {
            ratioAB = CFraction(static_cast<int>(pAr), static_cast<int>(pBr));

            CFraction fractionB = fractionA * ratioAB;

            // Known firing rate for 'b'
            CFraction knownFractionB = fractions[b->getId()];

            // Compare known and calculated firing rate of 'b'
            if (knownFractionB != CFraction(-1, 1) && fractionB != knownFractionB) {
                // Inconsistent graph, set all fractions to 0
                for (auto &fraction : fractions) {
                    fraction = CFraction(0, 0);
                }

                return;
            }
            if (knownFractionB == CFraction(-1, 1)) {
                // Set the firing rate of actor 'b'
                fractions[b->getId()] = fractionB;

                // Calculate firing rate for all actors connected to 'b'
                calcFractionsConnectedActors(fractions, b, s);

                // Is graph inconsistent?
                if (fractions[b->getId()] == CFraction(0, 0)) {
                    return;
                }
            }
        }
    }
}


/**
 * calcRepetitionVector ()
 * Convert the fractions to the smallest integers.
 */
RepetitionVector calcRepetitionVector(CFractions &fractions) {
    RepetitionVector repetitionVector(fractions.size(), 0);
    int64_t l = 1;

    // Find lowest common multiple (lcm) of all denominators
    for (auto& f: fractions) {
        l = lcm(l, f.denominator());
    }

    // Zero vector?
    if (l == 0) {
        return repetitionVector;
}

    // Calculate non-zero repetition vector
    for (uint i = 0; i < fractions.size(); i++) {
        repetitionVector[i] = static_cast<int>((fractions[i].numerator() * l) / fractions[i].denominator());
    }

    // Find greatest common divisor (gcd)
    int g = repetitionVector[0];
    for (uint i = 1; i < repetitionVector.size(); i++) {
        g = gcd(g, repetitionVector[i]);
    }

    // Minimize the repetition vector using the gcd
    for (int & i : repetitionVector) {
        i = i / g;
    }

    return repetitionVector;
}

} // namespace

/**
 * computeRepetitionVector ()
 * The function computes the repetition vector of a scenario graph for a given
 * scenario.
 */
RepetitionVector Scenario::computeRepetitionVector() const {
    CFractions fractions(this->scenarioGraph->getActors().size(), CFraction(-1, 1));
    ScenarioGraph *g = this->scenarioGraph;
    // Calculate firing ratio (as fraction) for each actor
    for (const auto& a: g->getActors()) {
        CFraction &f = fractions[a->getId()];

        if (f == CFraction(-1, 1)) {
            f = CFraction(1, 1);
            calcFractionsConnectedActors(fractions, a.get(), *this);
        }
    }

    // Calculate repetition vector based on firing ratios
    return calcRepetitionVector(fractions);
}

/**
 * computeRepetitionVectorOfScenarioGraph ()
 * The function computes the repetition vector of a specified scenario graph
 * that is not necessarily the standard graph of this scenario.
 */
RepetitionVector Scenario::computeRepetitionVectorOfScenarioGraph(ScenarioGraph *sg) const {
    CFractions fractions(sg->getActors().size(), CFraction(0, 1));

    // Calculate firing ratio (as fraction) for each actor
    for (const auto& a: sg->getActors()) {
        CFraction &f = fractions[a->getId()];

        if (f == CFraction(0, 1)) {
            f = CFraction(1, 1);
            calcFractionsConnectedActors(fractions, a.get(), *this);
        }
    }

    // Calculate repetition vector based on firing ratios
    return calcRepetitionVector(fractions);
}

/**
 * isScenarioGraphConsistent ()
 * The function checks the consistency of the scenario graph and returns true
 * when the graph is consistent, else it returns false.
 */
bool Scenario::isScenarioGraphConsistent() const {
    RepetitionVector repVec = computeRepetitionVector();
    return repVec[0] != 0;
}

/**
 * print ()
 * Print the scenario to the supplied output stream.
 */
std::ostream &Scenario::print(std::ostream &out) {
    out << "Scenario (" << getName() << ")" << std::endl;
    out << "id:        " << getId() << std::endl;
    out << "graph:     " << getScenarioGraph()->getName() << std::endl;
    out << std::endl;

    return out;
}

} // End namespace FSMSADF
