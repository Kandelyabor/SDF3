/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   thrutils.h
 *
 *  Author          :   Marc Geilen (m.c.w.geilen@tue.nl)
 *
 *  Date            :   May 6, 2010
 *
 *  Function        :   definitions shared for different throughput
 *                      algorithms.
 *
 *  History         :
 *      06-05-10    :   Initial version.
 *
 * $Id: thrutils.h,v 1.1.2.4 2010-08-06 07:17:49 mgeilen Exp $
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

#ifndef FSMSADF_ANALYSIS_THROUGHPUT_THRUTILS_H_INCLUDED
#define FSMSADF_ANALYSIS_THROUGHPUT_THRUTILS_H_INCLUDED

#include "fsmsadf/base/fsm.h"
#include "fsmsadf/base/graph.h"
#include "maxplus/algebra/maxplus.h" // IWYU pragma: keep
#include "maxplus/base/fsm/fsm.h"
#include "maxplus/graph/mpstatespace.h" // IWYU pragma: keep
#include "sdf/analysis/maxplus/mpexplore.h"
#include <memory>
#include <utility>


namespace FSMSADF {

using FiringOrder = const std::list<CString>;

class ScenarioMatrices : public std::map<CString, std::unique_ptr<MaxPlus::Matrix>> {
public:
    ScenarioMatrices() = default;
    ~ScenarioMatrices() = default;

    // Delete copy/move constructors and assignment operators
    ScenarioMatrices(const ScenarioMatrices &) = delete;
    ScenarioMatrices &operator=(const ScenarioMatrices &) = delete;
    ScenarioMatrices(ScenarioMatrices &&) = delete;
    ScenarioMatrices &operator=(ScenarioMatrices &&) = delete;

    // get the vector size for the matrices in the map
    // assumes at least one matrix has been stored
    size_t vectorSize() { return this->begin()->second->getSize(); }
};

class ScenarioRewards : public std::map<CString, CDouble> {};

// Required information for generating traces
class Tracinginfo {
public:
    void create(std::unique_ptr<MaxPlus::Matrix> sm,
                std::unique_ptr<MaxPlus::Matrix> tm,
                std::unique_ptr<FiringOrder> firingOrder) {
        ScenarioMatrix = std::move(sm);
        TracingMatrix = std::move(tm);
        actorsFiringOrder = std::move(firingOrder);
    }
    MaxPlus::Matrix *getScenarioMatrix() { return ScenarioMatrix.get(); }
    MaxPlus::Matrix *getTracingMatrix() { return TracingMatrix.get(); }
    const FiringOrder* getActorsFiringOrder() { return actorsFiringOrder.get(); }

private:
    std::unique_ptr<MaxPlus::Matrix> ScenarioMatrix;
    std::unique_ptr<MaxPlus::Matrix> TracingMatrix;
    std::unique_ptr<FiringOrder> actorsFiringOrder;
};

class ScenarioTracinginfo : public std::map<CString, std::unique_ptr<Tracinginfo>> {
public:
    ScenarioTracinginfo() = default;
    ~ScenarioTracinginfo() = default;

    // Delete copy/move constructors and assignment operators
    ScenarioTracinginfo(const ScenarioTracinginfo &) = delete;
    ScenarioTracinginfo &operator=(const ScenarioTracinginfo &) = delete;
    ScenarioTracinginfo(ScenarioTracinginfo &&) = delete;
    ScenarioTracinginfo &operator=(ScenarioTracinginfo &&) = delete;


    // get the vector size for the matrices in the map
    // assumes at least one matrix has been stored
    size_t vectorSize() { return this->begin()->second->getScenarioMatrix()->getSize(); }
};

class GraphConversion {
public:
    // Get the maxplus matrices for each of the scenarios.
    static std::unique_ptr<ScenarioMatrices> computeMaxPlusMatricesFromScenarios(const Graph &g);
    static std::unique_ptr<ScenarioRewards> getRewardsFromScenarios(const Graph &g);
    static std::unique_ptr<ScenarioTracinginfo> computeTracinginfoFromScenarios(const Graph &g);
    static std::unique_ptr<ScenarioMatrices>
    computeMaxPlusMatricesFromScenariosForWeakFSMSADF(const Graph &g);
    static std::unique_ptr<ScenarioTracinginfo>
    computeTracinginfoFromScenariosWeakConsistency(const Graph &g);

    // ensure the FSM SADF is strongly bounded by introducing a dummy
    // actor with a self loop, so that sub collections of initial tokens
    // cannot develop independently on particular scenario sequences
    static std::unique_ptr<Graph> ensureStronglyBounded(const Graph &g);
};

// Matrix and reward Labelled Scenario Automaton
using MLSEdgeLabel = struct MLSEdgeLabel {
    std::shared_ptr<MaxPlus::Matrix> mat;
    double rew;
};
using MLSState = ::MaxPlus::FSM::Labeled::State<CId, MLSEdgeLabel>;
using MLSStateRef = ::MaxPlus::FSM::Labeled::StateRef<CId, MLSEdgeLabel>;
using MLSEdge = ::MaxPlus::FSM::Labeled::Edge<CId, MLSEdgeLabel>;
using MLSEdgeRef = ::MaxPlus::FSM::Labeled::EdgeRef<CId, MLSEdgeLabel>;
using MLSSetOfStates = ::MaxPlus::FSM::Labeled::SetOfStates<CId, MLSEdgeLabel>;
using MLSSetOfEdges = ::MaxPlus::FSM::Abstract::SetOfEdges;
class MatrixLabeledScenarioFSM : public ::MaxPlus::FSM::Labeled::FiniteStateMachine<CId, MLSEdgeLabel> {};

class ThroughputAnalysisInformation {
public:
    ThroughputAnalysisInformation() = default;
    ~ThroughputAnalysisInformation() = default;

    // Delete copy/move constructors and assignment operators
    ThroughputAnalysisInformation(const ThroughputAnalysisInformation &) = delete;
    ThroughputAnalysisInformation &operator=(const ThroughputAnalysisInformation &) = delete;
    ThroughputAnalysisInformation(ThroughputAnalysisInformation &&) = delete;
    ThroughputAnalysisInformation &operator=(ThroughputAnalysisInformation &&) = delete;


    void store(const CString &scenarioName,
               std::unique_ptr<MaxPlus::Matrix> m,
               CDouble r,
               const CString &se) {
        scenarioMatrices[scenarioName] = std::move(m);
        subExpressions[scenarioName] = se;
        scenarioRewards[scenarioName] = r;
        scenarios.insert(scenarioName);
    }

    ScenarioMatrices &getScenarioMatrices() { return scenarioMatrices; }

    ScenarioRewards &getScenarioRewards() { return scenarioRewards; }

    std::set<CString> getScenarios() { return scenarios; }

    std::map<CString, CString> getsubExpressions() { return subExpressions; }

private:
    std::set<CString> scenarios;
    std::map<CString, CString> subExpressions;
    ScenarioMatrices scenarioMatrices;
    ScenarioRewards scenarioRewards;
};

using namespace MaxPlus::FSM;

class FSMConverter {
public:
    static std::unique_ptr<::FSMSADF::EdgeLabeledScenarioFSM> convertFSMToEdgeLabeled(const FSM& fsm);
    static std::unique_ptr<::FSMSADF::EdgeLabeledScenarioFSM> minimalEdgeLabeledFSM(const FSM& fsm);
    static std::unique_ptr<MatrixLabeledScenarioFSM> convertToMatrixLabeledScenarioFSM(
            const ::FSMSADF::EdgeLabeledScenarioFSM &elsFsm,
            ScenarioMatrices &sm,
            ScenarioRewards &sr,
            bool combineEdges = true);
    static std::unique_ptr<MatrixLabeledScenarioFSM> convertToFullMatrixLabeledScenarioFSM(
            const ::FSMSADF::EdgeLabeledScenarioFSM &elsFsm,
            ScenarioMatrices &sm,
            ScenarioRewards &sr);
    static std::unique_ptr<::FSMSADF::EdgeLabeledScenarioFSM>
    convertNFAeToNFA(const ::FSMSADF::EdgeLabeledScenarioFSM &);
};

class CScenarioDeadlockException : public SDF::MaxPlusAnalysis::CDeadlockException {
public:
    Scenario *scenario;

    CScenarioDeadlockException(const CString &message, Scenario *sc) :
        CDeadlockException(message), scenario(sc) {}
};

} // namespace FSMSADF

#endif
