#include "latency.h"
#include "fsmsadf/analysis/latency/regexparser_lat.h"
#include <memory>

// #define _PRINT_REGEX

namespace FSMSADF {

using namespace SDF::MaxPlusAnalysis;
using namespace MaxPlus;
using namespace SDF;

namespace {

/*
computes the system matrices of all scenarios in a weakly-consistent FSMSADF
*/
std::unique_ptr<SystemsMatrices> computeSystemMatricesFromScenariosForWeakFSMSADF(const Graph &g) {
    auto sysMats = std::make_unique<SystemsMatrices>();
    const Scenarios& scenarios = g.getConstScenarios();

    for (const auto& sc : scenarios) {
        CStrings persistentTokens;
        CStrings persistentTokensFinal;
        PersistentTokenIndexMap mp = sc->getScenarioGraph()->getPersistentTokenIndexMap();
        PersistentTokenIndexMap mpf = sc->getScenarioGraph()->getPersistentTokenIndexMapFinal();

        // collect names of the initial anf final persistent tokens
        for (auto &[ptMi, idx] : mp) {
            persistentTokens.push_back(ptMi);
        }
        for (auto &[ptMi, idx] : mpf) {
            persistentTokensFinal.push_back(ptMi);
        }

        // sort them to get a canonical order
        persistentTokens.sort();
        persistentTokensFinal.sort();

        std::list<uint> tokenIndicesInitial;
        for (const auto &ti : persistentTokens) {
            tokenIndicesInitial.push_back(mp[ti]);
        }
        std::list<uint> tokenIndicesFinal;
        for (const auto &ti : persistentTokensFinal) {
            tokenIndicesFinal.push_back(mpf[ti]);
        }

        auto sg = std::unique_ptr<TimedSDFgraph>(FSMSADF::Graph::extractSDFgraph(*sc));
        auto partialRepVec =
                std::unique_ptr<RepetitionVector>(FSMSADF::Graph::getPartialRepetitionVector(*sc));
        auto G = std::make_unique<GraphDecoration::Graph>(*sg, true, partialRepVec.get());

        MaxPlusAnalysis::Exploration mpe;
        mpe.OG = std::make_unique<GraphDecoration::Graph>(*G);
        mpe.G = std::move(G);
        mpe.g = sg.get();
        std::pair<std::unique_ptr<Matrix>, std::unique_ptr<Matrix>> stateAndOutputMatrices =
                mpe.computeStateAndOutputMatrices();
        // create the sub matrix of only the persistent tokens.
        auto oMatrix = std::make_unique<MaxPlus::Matrix>(
                stateAndOutputMatrices.second->getSubMatrixNonSquare(tokenIndicesInitial));
        auto sMatrix = std::make_unique<MaxPlus::Matrix>(
                stateAndOutputMatrices.first->getSubMatrix(tokenIndicesFinal, tokenIndicesInitial));
        auto sm = std::make_unique<SystemMatrices>(std::move(sMatrix), std::move(oMatrix));
        sysMats->systemsMatrices[sc->getName()] = std::move(sm);
    }

    return sysMats;
}

} // namespace

/**
 * latencyAnalysisGraphUsingRegex()
 * Compute the latency of a weakly consistent Regex-based SADF graph.
 */
void latencyAnalysisGraphUsingRegex(Graph *g, MPTime mu, std::ostream &out) {
    std::unique_ptr<Graph> gt = nullptr;

    // check if there is a regex in g
    if (g->getRegEx().empty()) {
        throw ::CException("Graph does not contain a regular expression.");
    }

    // create a copy of the graph
    gt = g->clone(GraphComponent(g->getParent(), g->getId(), g->getName()));

    // check for invalid scenario names.
    gt->checkScenarioNames();

    // check consistency
    gt->checkWeaklyConsistency();

    // compute the system matrices of all scenarios
    std::unique_ptr<SystemsMatrices> systemsMatrices =
            computeSystemMatricesFromScenariosForWeakFSMSADF(*gt);

    // a parser to be constructed each time a regular expression needs to be parsed.
    RegExParserLat parser(gt->getRegEx(), *systemsMatrices);

    // parse the regular expression
    std::unique_ptr<RegEx> regEx = parser.parse();

#ifdef _PRINT_REGEX
    std::cout << "the entered regular expression interpreted in the following way:" << std::endl;
    regEx->print();
    std::cout << std::endl;
#endif

    // cast to a regular expression type designed for throughput analysis
    auto *regExLat = dynamic_cast<RegExLat *>(regEx.get());

    // set the period
    RegExLat::period = mu;

    // compute the latency
    auto lat = regExLat->computeLatencyVector();

    // get the name of the starting scenario
    CString firstScenarioName = regEx->moveToNextLeaf()->getName();

    // get starting scenario
    CStrings persistentTokensOfFirstScenario =
            g->getScenarioRef(firstScenarioName)->getScenarioGraph()->getPersistentTokens();

    // push the initial tokens of the first scenario to a string
    CString initialTokenNamesString;

    for (auto ti = persistentTokensOfFirstScenario.begin(),
              tiEnd = persistentTokensOfFirstScenario.end();
         ti != tiEnd;
         ti++) {
        initialTokenNamesString = initialTokenNamesString + *ti + "  ";
    }

    // convert the latency vector into a string
    MPString latencyVectorString;
    lat->toString(latencyVectorString, 1);

    // print the result
    out << "The latency vector is : " << latencyVectorString << "with respect to initial tokens "
        << initialTokenNamesString << std::endl;
}
} // namespace FSMSADF