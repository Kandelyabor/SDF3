/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sadf2poosl_process.cc
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   13 September 2006
 *
 *  Function        :   Output SADF graph in POOSL format (process class(es))
 *
 *  History         :
 *      13-09-06    :   Initial version.
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

#include "sadf2poosl_process.h"

#include <algorithm>

namespace {

void SADF2POOSL_Kernel(const SADF_Process& Kernel, std::ostream &out, bool PERT, bool p4r) {

    bool Timed = false;

    for (CId i = 0; !Timed && i != Kernel.getNumberOfScenarios(); i++) {
        for (CId j = 0; !Timed && j != Kernel.getNumberOfProfiles(i); j++) {
            if (Kernel.getProfile(i, j)->getExecutionTime() != 0) {
                Timed = true;
            }
        }
    }

    out << "process class Kernel" << Kernel.getIdentity()
        << "(Monitor: Boolean, MonitorID: Integer, Trace: Boolean, Name: String)" << std::endl;
    out << "instance variables" << std::endl;

    if (Timed) {
        out << "ExecutionTimes: Array, ";
    }

    out << "Status: ProcessMonitor" << std::endl << std::endl;

    if (!p4r) {

        out << "port interface" << std::endl;

        if (Kernel.hasControlInputChannels()) {
            out << "CI" << Kernel.getControlInputChannels()[0]->getIdentity() << ", ";
        }

        for (const auto &j : Kernel.getDataInputChannels()) {
            out << "DI" << j->getIdentity() << ", ";
        }

        for (const auto &j : Kernel.getDataOutputChannels()) {
            out << "DO" << j->getIdentity() << ", ";
        }

        out << "Monitor" << std::endl << std::endl;
        out << "message interface" << std::endl;

        if (Kernel.hasControlInputChannels()) {
            out << "CI" << Kernel.getControlInputChannels()[0]->getIdentity() << "!Inspect,"
                << std::endl;
            out << "CI" << Kernel.getControlInputChannels()[0]->getIdentity()
                << "?Scenario(Integer)," << std::endl;
            out << "CI" << Kernel.getControlInputChannels()[0]->getIdentity() << "!Read,"
                << std::endl;
        }

        for (const auto &j : Kernel.getDataInputChannels()) {
            out << "DI" << j->getIdentity() << "!Inspect(Integer)," << std::endl;
            out << "DI" << j->getIdentity() << "!Read," << std::endl;
        }

        for (const auto &j : Kernel.getDataOutputChannels()) {
            out << "DO" << j->getIdentity() << "!Room(Integer)," << std::endl;
            out << "DO" << j->getIdentity() << "!Reserve," << std::endl;
            out << "DO" << j->getIdentity() << "!Write," << std::endl;
        }

        out << "Monitor?StopSimulation," << std::endl;
        out << "Monitor!Results(String)," << std::endl;
        out << "Monitor?Status(Integer)," << std::endl;
        out << "Monitor!AccuracyStatus(Integer, Boolean)" << std::endl;
    }

    out << std::endl;
    out << "initial method call" << std::endl;
    out << "Initialise()()" << std::endl << std::endl;
    out << "instance methods" << std::endl;

    out << "Initialise()()";

    if (Timed) {

        if (PERT) {
            out << " |Distribution: PERT|" << std::endl << std::endl;
        } else {
            out << " |Distribution: GeneralDiscrete|" << std::endl << std::endl;
        }

        if (p4r) {
            out << "ExecutionTimes := new(Array) size(" << Kernel.getNumberOfScenarios() << ");"
                << std::endl;
        } else {
            out << "ExecutionTimes := new(Array) resize(" << Kernel.getNumberOfScenarios() << ");"
                << std::endl;
        }

        for (CId i = 0; i != Kernel.getNumberOfScenarios(); i++) {

            if (PERT) {
                out << "Distribution := new(PERT)" << std::endl;

                SADF_Profile *Minimum = Kernel.getProfile(i, 0);
                SADF_Profile *Maximum = Kernel.getProfile(i, 0);
                SADF_Profile *Mode = Kernel.getProfile(i, 0);

                for (CId j = 0; j != Kernel.getNumberOfProfiles(i); j++) {
                    if (Kernel.getProfile(i, j)->getExecutionTime()
                        < Minimum->getExecutionTime()) {
                        Minimum = Kernel.getProfile(i, j);
                    }
                    if (Kernel.getProfile(i, j)->getExecutionTime()
                        > Maximum->getExecutionTime()) {
                        Maximum = Kernel.getProfile(i, j);
                    }
                }
                for (CId j = 0; j != Kernel.getNumberOfProfiles(i); j++) {
                    if (((Kernel.getProfile(i, j) != Minimum)
                         && (Kernel.getProfile(i, j) != Maximum))) {
                        Mode = Kernel.getProfile(i, j);
                    }
                }

                out << " withParameters(";
                if (Minimum->getExecutionTime()
                    != static_cast<CDouble>(static_cast<uint64_t>(Minimum->getExecutionTime()))) {
                    out << Minimum->getExecutionTime();
                } else {
                    out << (static_cast<uint64_t>(Minimum->getExecutionTime()));
                }
                out << ", ";
                if (Mode->getExecutionTime()
                    != static_cast<CDouble>(static_cast<uint64_t>(Mode->getExecutionTime()))) {
                    out << Mode->getExecutionTime();
                } else {
                    out << (static_cast<uint64_t>(Mode->getExecutionTime()));
                }
                out << ", ";
                if (Maximum->getExecutionTime()
                    != static_cast<CDouble>(static_cast<uint64_t>(Maximum->getExecutionTime()))) {
                    out << Maximum->getExecutionTime();
                } else {
                    out << (static_cast<uint64_t>(Maximum->getExecutionTime()));
                }
                out << ");" << std::endl;

            } else {

                out << "Distribution := new(GeneralDiscrete)" << std::endl;

                for (CId j = 0; j != Kernel.getNumberOfProfiles(i); j++) {

                    out << "   withParameters(";

                    if (Kernel.getProfile(i, j)->getExecutionTime()
                        != static_cast<CDouble>(static_cast<uint64_t>(
                                Kernel.getProfile(i, j)->getExecutionTime()))) {
                        out << Kernel.getProfile(i, j)->getExecutionTime();
                    } else {
                        out << (static_cast<uint64_t>(
                                Kernel.getProfile(i, j)->getExecutionTime()));
                    }

                    if (j < Kernel.getNumberOfProfiles(i) - 1) {
                        out << ", " << Kernel.getProfile(i, j)->getWeight() << ")" << std::endl;
                    } else {
                        out << ", " << Kernel.getProfile(i, j)->getWeight() << ");" << std::endl;
                    }
                }
            }

            out << "ExecutionTimes put(" << i + 1 << ", Distribution);" << std::endl;
        }

    } else {
        out << std::endl << std::endl;
    }

    out << "Status := new(ProcessMonitor";
    if (PERT) {
        out << "_PERT";
    }
    out << ") init(Monitor, MonitorID, Trace, Name);" << std::endl;

    out << "abort" << std::endl;
    out << "   par Fire()() and if Monitor then CheckAccuracyStatus()() fi rap" << std::endl;
    out << "with Monitor?StopSimulation; if Monitor then Monitor!Results(Status getResults) fi."
        << std::endl
        << std::endl;

    out << "CheckAccuracyStatus()() |ID: Integer|" << std::endl << std::endl;
    out << "Monitor?Status(ID | ID = MonitorID);" << std::endl;
    out << "Monitor!AccuracyStatus(MonitorID, Status accurate);" << std::endl;
    out << "CheckAccuracyStatus()()." << std::endl << std::endl;

    out << "Fire()() |Scenario: Integer|" << std::endl << std::endl;

    if (Kernel.hasControlInputChannels()) {
        out << "CI" << Kernel.getControlInputChannels()[0]->getIdentity() << "!Inspect;"
            << std::endl;
        out << "CI" << Kernel.getControlInputChannels()[0]->getIdentity() << "?Scenario(Scenario);"
            << std::endl;
    } else {
        out << "Scenario := 0;" << std::endl;
    }

    if (Kernel.getNumberOfScenarios() == 1) {

        for (const auto &j : Kernel.getDataInputChannels()) {
            out << "DI" << j->getIdentity() << "!Inspect(" << j->getConsumptionRate(0) << ");"
                << std::endl;
        }

        for (const auto &j : Kernel.getDataOutputChannels()) {
            out << "DO" << j->getIdentity() << "!Room(" << j->getProductionRate(0) << ");"
                << std::endl;
        }

        for (const auto &j : Kernel.getDataOutputChannels()) {
            out << "DO" << j->getIdentity() << "!Reserve;" << std::endl;
        }

        out << "Status start(currentTime);" << std::endl;

        if (Timed) {
            if (p4r) {
                out << "delay(ExecutionTimes get(Scenario + 1) sample);" << std::endl;
            } else {
                out << "delay(ExecutionTimes at(Scenario + 1) sample);" << std::endl;
            }
        }

        out << "Status end(currentTime);" << std::endl;

        for (const auto &j : Kernel.getDataInputChannels()) {
            out << "DI" << j->getIdentity() << "!Read;" << std::endl;
        }

        for (const auto &j : Kernel.getDataOutputChannels()) {
            out << "DO" << j->getIdentity() << "!Write;" << std::endl;
        }

    } else {

        for (CId i = 0; i != Kernel.getNumberOfScenarios(); i++) {

            bool TimedScenario = false;

            if (Timed) {
                for (CId j = 0; !TimedScenario && j != Kernel.getNumberOfProfiles(i); j++) {
                    if (Kernel.getProfile(i, j)->getExecutionTime() != 0) {
                        TimedScenario = true;
                    }
                }
            }

            out << "if Scenario = " << i << " then" << std::endl;

            for (const auto &j : Kernel.getDataInputChannels()) {
                if (j->getConsumptionRate(i) > 0) {
                    out << "   DI" << j->getIdentity() << "!Inspect(" << j->getConsumptionRate(i)
                        << ");" << std::endl;
                }
            }

            for (const auto &j : Kernel.getDataOutputChannels()) {
                if (j->getProductionRate(i) > 0) {
                    out << "   DO" << j->getIdentity() << "!Room(" << j->getProductionRate(i)
                        << ");" << std::endl;
                }
            }

            for (const auto &j : Kernel.getDataOutputChannels()) {
                if (j->getProductionRate(i) > 0) {
                    out << "   DO" << j->getIdentity() << "!Reserve;" << std::endl;
                }
            }

            out << "   Status start(currentTime);" << std::endl;

            if (TimedScenario) {
                if (p4r) {
                    out << "   delay(ExecutionTimes get(Scenario + 1) sample);" << std::endl;
                } else {
                    out << "   delay(ExecutionTimes at(Scenario + 1) sample);" << std::endl;
                }
            }

            out << "   Status end(currentTime)";

            for (const auto &j : Kernel.getDataInputChannels()) {
                if (j->getConsumptionRate(i) > 0) {
                    out << ";" << std::endl << "   DI" << j->getIdentity() << "!Read";
                }
            }

            for (const auto &j : Kernel.getDataOutputChannels()) {
                if (j->getProductionRate(i) > 0) {
                    out << ";" << std::endl << "   DO" << j->getIdentity() << "!Write";
                }
            }

            out << std::endl << "fi;" << std::endl;
        }
    }

    if (Kernel.hasControlInputChannels()) {
        out << "CI" << Kernel.getControlInputChannels()[0]->getIdentity() << "!Read;" << std::endl;
    }

    out << "Fire()()." << std::endl << std::endl;
}

void SADF2POOSL_Detector(const SADF_Process &Detector, std::ostream &out, bool PERT, bool p4r) {

    bool Timed = false;

    for (CId i = 0; !Timed && i != Detector.getNumberOfSubScenarios(); i++) {
        for (CId j = 0; !Timed && j != Detector.getNumberOfProfiles(i); j++) {
            if (Detector.getProfile(i, j)->getExecutionTime() != 0) {
                Timed = true;
            }
        }
    }

    out << "process class Detector" << Detector.getIdentity()
        << "(Monitor: Boolean, MonitorID: Integer, Trace: Boolean, Name: String)" << std::endl;
    out << "instance variables" << std::endl;

    if (Timed) {
        out << "ExecutionTimes: Array, ";
    }

    out << "Status: ProcessMonitor, MarkovChains: Array" << std::endl << std::endl;

    if (!p4r) {
        out << "port interface" << std::endl;

        if (Detector.hasControlInputChannels()) {
            out << "CI" << Detector.getControlInputChannels()[0]->getIdentity() << ", ";
        }

        for (const auto &j : Detector.getDataInputChannels()) {
            out << "DI" << j->getIdentity() << ", ";
        }

        for (const auto &j : Detector.getDataOutputChannels()) {
            out << "DO" << j->getIdentity() << ", ";
        }

        for (const auto &j : Detector.getControlOutputChannels()) {
            out << "CO" << j->getIdentity() << ", ";
        }

        out << "Monitor" << std::endl << std::endl;
        out << "message interface" << std::endl;

        if (Detector.hasControlInputChannels()) {
            out << "CI" << Detector.getControlInputChannels()[0]->getIdentity() << "!Inspect,"
                << std::endl;
            out << "CI" << Detector.getControlInputChannels()[0]->getIdentity()
                << "?Scenario(Integer)," << std::endl;
            out << "CI" << Detector.getControlInputChannels()[0]->getIdentity() << "!Read,"
                << std::endl;
        }

        for (const auto &j : Detector.getDataInputChannels()) {
            out << "DI" << j->getIdentity() << "!Inspect(Integer)," << std::endl;
            out << "DI" << j->getIdentity() << "!Read," << std::endl;
        }

        for (const auto &j : Detector.getDataOutputChannels()) {
            out << "DO" << j->getIdentity() << "!Room(Integer)," << std::endl;
            out << "DO" << j->getIdentity() << "!Reserve," << std::endl;
            out << "DO" << j->getIdentity() << "!Write," << std::endl;
        }

        for (const auto &j : Detector.getControlOutputChannels()) {
            out << "CO" << j->getIdentity() << "!Room(Integer)," << std::endl;
            out << "CO" << j->getIdentity() << "!Reserve," << std::endl;
            out << "CO" << j->getIdentity() << "!Write(Integer)," << std::endl;
        }

        out << "Monitor?StopSimulation," << std::endl;
        out << "Monitor!Results(String)," << std::endl;
        out << "Monitor?Status(Integer)," << std::endl;
        out << "Monitor!AccuracyStatus(Integer, Boolean)" << std::endl;
    }

    out << "initial method call" << std::endl;
    out << "Initialise()()" << std::endl << std::endl;
    out << "instance methods" << std::endl;

    out << "Initialise()() |MarkovChain: MarkovChain";

    if (Timed) {

        if (PERT) {
            out << ", Distribution: PERT|" << std::endl << std::endl;
        } else {
            out << ", Distribution: GeneralDiscrete|" << std::endl << std::endl;
        }

        if (p4r) {
            out << "ExecutionTimes := new(Array) size(" << Detector.getNumberOfSubScenarios()
                << ");" << std::endl;
        } else {
            out << "ExecutionTimes := new(Array) resize(" << Detector.getNumberOfSubScenarios()
                << ");" << std::endl;
        }

        for (CId i = 0; i != Detector.getNumberOfSubScenarios(); i++) {

            if (PERT) {

                out << "Distribution := new(PERT)" << std::endl;

                SADF_Profile *Minimum = Detector.getProfile(i, 0);
                SADF_Profile *Maximum = Detector.getProfile(i, 0);
                SADF_Profile *Mode = Detector.getProfile(i, 0);

                for (CId j = 0; j != Detector.getNumberOfProfiles(i); j++) {
                    if (Detector.getProfile(i, j)->getExecutionTime()
                        < Minimum->getExecutionTime()) {
                        Minimum = Detector.getProfile(i, j);
                    }
                    if (Detector.getProfile(i, j)->getExecutionTime()
                        > Maximum->getExecutionTime()) {
                        Maximum = Detector.getProfile(i, j);
                    }
                }
                for (CId j = 0; j != Detector.getNumberOfProfiles(i); j++) {
                    if (((Detector.getProfile(i, j) != Minimum)
                         && (Detector.getProfile(i, j) != Maximum))) {
                        Mode = Detector.getProfile(i, j);
                    }
                }

                out << " withParameters(";
                if (Minimum->getExecutionTime()
                    != static_cast<CDouble>(static_cast<uint64_t>(Minimum->getExecutionTime()))) {
                    out << Minimum->getExecutionTime();
                } else {
                    out << (static_cast<uint64_t>(Minimum->getExecutionTime()));
                }
                out << ", ";
                if (Mode->getExecutionTime()
                    != static_cast<CDouble>(static_cast<uint64_t>(Mode->getExecutionTime()))) {
                    out << Mode->getExecutionTime();
                } else {
                    out << (static_cast<uint64_t>(Mode->getExecutionTime()));
                }
                out << ", ";
                if (Maximum->getExecutionTime()
                    != static_cast<CDouble>(static_cast<uint64_t>(Maximum->getExecutionTime()))) {
                    out << Maximum->getExecutionTime();
                } else {
                    out << (static_cast<uint64_t>(Maximum->getExecutionTime()));
                }
                out << ");" << std::endl;

            } else {

                out << "Distribution := new(GeneralDiscrete)" << std::endl;

                for (CId j = 0; j != Detector.getNumberOfProfiles(i); j++) {

                    out << "   withParameters(";

                    if (Detector.getProfile(i, j)->getExecutionTime()
                        != static_cast<CDouble>(static_cast<uint64_t>(
                                Detector.getProfile(i, j)->getExecutionTime()))) {
                        out << Detector.getProfile(i, j)->getExecutionTime();
                    } else {
                        out << (static_cast<uint64_t>(
                                Detector.getProfile(i, j)->getExecutionTime()));
                    }

                    if (j < Detector.getNumberOfProfiles(i) - 1) {
                        out << ", " << Detector.getProfile(i, j)->getWeight() << ")" << std::endl;
                    } else {
                        out << ", " << Detector.getProfile(i, j)->getWeight() << ");" << std::endl;
                    }
                }
            }

            out << "ExecutionTimes put(" << i + 1 << ", Distribution);" << std::endl;
        }

    } else {
        out << "|" << std::endl << std::endl;
    }

    if (p4r) {
        out << "MarkovChains := new(Array) size(" << Detector.getNumberOfScenarios() << ");"
            << std::endl;
    } else {
        out << "MarkovChains := new(Array) resize(" << Detector.getNumberOfScenarios() << ");"
            << std::endl;
    }

    for (CId s = 0; s != Detector.getNumberOfScenarios(); s++) {

        out << "MarkovChain := new(MarkovChain) init" << std::endl;

        for (CId i = 0; i != Detector.getMarkovChain(s)->getNumberOfStates(); i++) {

            out << "   addState(\"" << Detector.getMarkovChain(s)->getStateName(i) << "\", "
                << Detector.getMarkovChain(s)->getSubScenario(i) << ")" << std::endl;

            CDouble CumulativeWeight = 0;

            for (CId j = 0; j != Detector.getMarkovChain(s)->getNumberOfStates(); j++) {
                if (Detector.getMarkovChain(s)->getTransitionProbability(i, j) != 0) {

                    out << "      addTransition(\"" << Detector.getMarkovChain(s)->getStateName(i)
                        << "\", \"" << Detector.getMarkovChain(s)->getStateName(j) << "\", "
                        << CumulativeWeight << ", "
                        << CumulativeWeight
                                    + Detector.getMarkovChain(s)->getTransitionProbability(i, j)
                        << ")" << std::endl;

                    CumulativeWeight += Detector.getMarkovChain(s)->getTransitionProbability(i, j);
                }
            }
        }

        out << "   setInitialState(\""
            << Detector.getMarkovChain(s)->getStateName(
                       Detector.getMarkovChain(s)->getInitialState())
            << "\");" << std::endl;
        out << "MarkovChains put(" << s + 1 << ", MarkovChain);" << std::endl;
    }

    out << "Status := new(ProcessMonitor";
    if (PERT) {
        out << "_PERT";
    }
    out << ") init(Monitor, MonitorID, Trace, Name);" << std::endl;
    out << "abort" << std::endl;
    out << "   par Fire()() and if Monitor then CheckAccuracyStatus()() fi rap" << std::endl;
    out << "with Monitor?StopSimulation; if Monitor then Monitor!Results(Status getResults) fi."
        << std::endl
        << std::endl;

    out << "CheckAccuracyStatus()() |ID: Integer|" << std::endl << std::endl;
    out << "Monitor?Status(ID | ID = MonitorID);" << std::endl;
    out << "Monitor!AccuracyStatus(MonitorID, Status accurate);" << std::endl;
    out << "CheckAccuracyStatus()()." << std::endl << std::endl;

    out << "Fire()() |Scenario, SubScenario: Integer|" << std::endl << std::endl;

    if (Detector.hasControlInputChannels()) {
        out << "CI" << Detector.getControlInputChannels()[0]->getIdentity() << "!Inspect;"
            << std::endl;
        out << "CI" << Detector.getControlInputChannels()[0]->getIdentity()
            << "?Scenario(Scenario);" << std::endl;
    } else {
        out << "Scenario := 0;" << std::endl;
    }

    if (p4r) {
        out << "SubScenario := MarkovChains get(Scenario + 1) getNextSubScenario;" << std::endl;
    } else {
        out << "SubScenario := MarkovChains at(Scenario + 1) getNextSubScenario;" << std::endl;
    }

    if (Detector.getNumberOfSubScenarios() == 1) {

        for (const auto &j : Detector.getDataInputChannels()) {
            out << "DI" << j->getIdentity() << "!Inspect(" << j->getConsumptionRate(0) << ");"
                << std::endl;
        }

        for (const auto &j : Detector.getDataOutputChannels()) {
            out << "DO" << j->getIdentity() << "!Room(" << j->getProductionRate(0) << ");"
                << std::endl;
        }

        for (const auto &j : Detector.getControlOutputChannels()) {
            out << "CO" << j->getIdentity() << "!Room(" << j->getProductionRate(0) << ");"
                << std::endl;
        }

        for (const auto &j : Detector.getDataOutputChannels()) {
            out << "DO" << j->getIdentity() << "!Reserve;" << std::endl;
        }

        for (const auto &j : Detector.getControlOutputChannels()) {
            out << "CO" << j->getIdentity() << "!Reserve;" << std::endl;
        }

        out << "Status start(currentTime);" << std::endl;

        if (Timed) {
            if (p4r) {
                out << "delay(ExecutionTimes get(SubScenario + 1) sample);" << std::endl;
            } else {
                out << "delay(ExecutionTimes at(SubScenario + 1) sample);" << std::endl;
            }
        }

        out << "Status end(currentTime);" << std::endl;

        for (const auto &j : Detector.getDataInputChannels()) {
            out << "DI" << j->getIdentity() << "!Read;" << std::endl;
        }

        for (const auto &j : Detector.getDataOutputChannels()) {
            out << "DO" << j->getIdentity() << "!Write;" << std::endl;
        }

        for (const auto &j : Detector.getControlOutputChannels()) {
            out << "CO" << j->getIdentity() << "!Write(" << j->getProductionScenarioID(0) << ");"
                << std::endl;
        }
    } else {

        for (CId i = 0; i != Detector.getNumberOfSubScenarios(); i++) {

            bool TimedScenario = false;

            if (Timed) {
                for (CId j = 0; !TimedScenario && j != Detector.getNumberOfProfiles(i); j++) {
                    if (Detector.getProfile(i, j)->getExecutionTime() != 0) {
                        TimedScenario = true;
                    }
                }
            }

            out << "if SubScenario = " << i << " then" << std::endl;

            for (const auto &j : Detector.getDataInputChannels()) {
                if (j->getConsumptionRate(i) > 0) {
                    out << "   DI" << j->getIdentity() << "!Inspect(" << j->getConsumptionRate(i)
                        << ");" << std::endl;
                }
            }

            for (const auto &j : Detector.getDataOutputChannels()) {
                if (j->getProductionRate(i) > 0) {
                    out << "   DO" << j->getIdentity() << "!Room(" << j->getProductionRate(i)
                        << ");" << std::endl;
                }
            }

            for (const auto &j : Detector.getControlOutputChannels()) {
                out << "   CO" << j->getIdentity() << "!Room(" << j->getProductionRate(i) << ");"
                    << std::endl;
            }

            for (const auto &j : Detector.getDataOutputChannels()) {
                if (j->getProductionRate(i) > 0) {
                    out << "   DO" << j->getIdentity() << "!Reserve;" << std::endl;
                }
            }

            for (const auto &j : Detector.getControlOutputChannels()) {
                out << "   CO" << j->getIdentity() << "!Reserve;" << std::endl;
            }

            out << "   Status start(currentTime);" << std::endl;

            if (TimedScenario) {
                if (p4r) {
                    out << "   delay(ExecutionTimes get(SubScenario + 1) sample);" << std::endl;
                } else {
                    out << "   delay(ExecutionTimes at(SubScenario + 1) sample);" << std::endl;
                }
            }

            out << "   Status end(currentTime)";

            for (const auto &j : Detector.getDataInputChannels()) {
                if (j->getConsumptionRate(i) > 0) {
                    out << ";" << std::endl << "   DI" << j->getIdentity() << "!Read";
                }
            }

            for (const auto &j : Detector.getDataOutputChannels()) {
                if (j->getProductionRate(i) > 0) {
                    out << ";" << std::endl << "   DO" << j->getIdentity() << "!Write";
                }
            }

            for (const auto &j : Detector.getControlOutputChannels()) {
                out << ";" << std::endl
                    << "   CO" << j->getIdentity() << "!Write(" << j->getProductionScenarioID(i)
                    << ")";
            }

            out << std::endl << "fi;" << std::endl;
        }
    }

    if (Detector.hasControlInputChannels()) {
        out << "CI" << Detector.getControlInputChannels()[0]->getIdentity() << "!Read;"
            << std::endl;
    }

    out << "Fire()()." << std::endl << std::endl;
}

} // namespace

void SADF2POOSL_Process(const SADF_Graph &Graph,
                        const SADF_SimulationSettings &Settings,
                        std::ostream &out,
                        bool PERT,
                        bool p4r) {

    for (CId i = 0; i != Graph.getNumberOfKernels(); i++) {
        SADF2POOSL_Kernel(*(Graph.getKernel(i)), out, PERT, p4r);
    }

    for (CId i = 0; i != Graph.getNumberOfDetectors(); i++) {
        SADF2POOSL_Detector(*(Graph.getDetector(i)), out, PERT, p4r);
    }

    // Generate Process Class SimulationController

    out << "process class SimulationController(NumberOfMonitors: Integer, LogFile: String)"
        << std::endl;
    out << "instance variables" << std::endl;
    out << "Status: MonitorStatus" << std::endl << std::endl;

    if (!p4r) {
        out << "port interface" << std::endl;
        out << "Monitor" << std::endl << std::endl;
        out << "message interface" << std::endl;
        out << "Monitor?Results(String)," << std::endl;
        out << "Monitor!Status(Integer)," << std::endl;
        out << "Monitor?AccuracyStatus(Integer, Boolean)," << std::endl;
        out << "Monitor!StopSimulation" << std::endl;
    }

    out << "initial method call" << std::endl;
    out << "Initialise()()" << std::endl << std::endl;
    out << "instance methods" << std::endl;

    out << "Initialise()() |MonitorID: Integer, Results: String|" << std::endl << std::endl;
    out << "Status := new(MonitorStatus) init(NumberOfMonitors, LogFile);" << std::endl;

    if (Settings.getMaximumModelTime() != SADF_UNDEFINED) {
        if (Settings.getMaximumModelTime()
            != static_cast<CDouble>(static_cast<uint64_t>(Settings.getMaximumModelTime()))) {
            out << "abort CheckAccuracyStatus()() with delay(" << Settings.getMaximumModelTime()
                << ");" << std::endl;
        } else {
            out << "abort CheckAccuracyStatus()() with delay("
                << (static_cast<uint64_t>(Settings.getMaximumModelTime())) << ");" << std::endl;
        }
    } else {
        out << "CheckAccuracyStatus()();" << std::endl;
    }

    out << "abort while true do Monitor!StopSimulation od with delay(1.0e-10);" << std::endl;

    out << "MonitorID := 1;" << std::endl;
    out << "while MonitorID <= NumberOfMonitors do" << std::endl;
    out << "   Monitor?Results(Results){Status append(Results)};" << std::endl;
    out << "   MonitorID := MonitorID + 1" << std::endl;
    out << "od;" << std::endl;
    out << "Status log." << std::endl << std::endl;

    out << "CheckAccuracyStatus()() |MonitorID: Integer, Accurate: Boolean|" << std::endl
        << std::endl;

    CDouble MaximumTimeStep = 0;
    CDouble MinimumTimeStep = -1;

    for (CId i = 0; i != Graph.getNumberOfKernels(); i++) {
        for (CId j = 0; j != Graph.getKernel(i)->getNumberOfScenarios(); j++) {
            for (CId k = 0; k != Graph.getKernel(i)->getNumberOfProfiles(j); k++) {

                MaximumTimeStep =
                        (std::max)(Graph.getKernel(i)->getProfile(j, k)->getExecutionTime(),
                                   MaximumTimeStep);

                if ((MinimumTimeStep == -1)
                    || ((MinimumTimeStep != -1)
                        && (Graph.getKernel(i)->getProfile(j, k)->getExecutionTime()
                            < MinimumTimeStep))) {
                    MinimumTimeStep = Graph.getKernel(i)->getProfile(j, k)->getExecutionTime();
                }
            }
        }
    }

    for (CId i = 0; i != Graph.getNumberOfDetectors(); i++) {
        for (CId j = 0; j != Graph.getDetector(i)->getNumberOfSubScenarios(); j++) {
            for (CId k = 0; k != Graph.getDetector(i)->getNumberOfProfiles(j); k++) {

                MaximumTimeStep =
                        (std::max)(Graph.getDetector(i)->getProfile(j, k)->getExecutionTime(),
                                   MaximumTimeStep);

                if ((MinimumTimeStep == -1)
                    || ((MinimumTimeStep != -1)
                        && (Graph.getDetector(i)->getProfile(j, k)->getExecutionTime()
                            < MinimumTimeStep))) {
                    MinimumTimeStep = Graph.getDetector(i)->getProfile(j, k)->getExecutionTime();
                }
            }
        }
    }

    CDouble TimeStep = MaximumTimeStep - MinimumTimeStep;

    constexpr CDouble ScaleFactor = 50;
    if (TimeStep != 0) {
        if (TimeStep != static_cast<CDouble>(static_cast<CId>(TimeStep))) {
            out << "delay(" << (ScaleFactor * TimeStep) << ");" << std::endl;
        } else {
            out << "delay(" << (ScaleFactor * (static_cast<CId>(TimeStep))) << ");" << std::endl;
        }
    } else {
        if (MinimumTimeStep != static_cast<CDouble>(static_cast<CId>(MinimumTimeStep))) {
            out << "delay(" << (ScaleFactor * MinimumTimeStep) << ");" << std::endl;
        } else {
            out << "delay(" << (ScaleFactor * (static_cast<CId>(MinimumTimeStep))) << ");" << std::endl;
        }
    }

    out << "MonitorID := 1;" << std::endl;
    out << "while MonitorID <= NumberOfMonitors do" << std::endl;
    out << "   Monitor!Status(MonitorID);" << std::endl;
    out << "   Monitor?AccuracyStatus(MonitorID, Accurate){Status register(MonitorID, Accurate)};"
        << std::endl;
    out << "   MonitorID := MonitorID + 1" << std::endl;
    out << "od;" << std::endl;
    out << "if (Status accurate not) then CheckAccuracyStatus()() fi." << std::endl << std::endl;

    // Generate Process Class DataBuffer

    out << "process class DataBuffer(BufferSize: Integer, InitialTokens: Integer, TokenSize: "
           "Integer, Monitor: Boolean, MonitorID: Integer, Trace: Boolean, Name: String)"
        << std::endl;
    out << "instance variables" << std::endl;
    out << "Status: DataBufferMonitor" << std::endl << std::endl;

    if (!p4r) {
        out << "port interface" << std::endl;
        out << "Monitor, In, Out" << std::endl << std::endl;
        out << "message interface" << std::endl;
        out << "In?Room(Integer)," << std::endl;
        out << "In?Reserve," << std::endl;
        out << "In?Write," << std::endl;
        out << "Out?Inspect(Integer)," << std::endl;
        out << "Out?Read," << std::endl;
        out << "Monitor!Results(String)," << std::endl;
        out << "Monitor?Status(Integer)," << std::endl;
        out << "Monitor!AccuracyStatus(Integer, Boolean)," << std::endl;
        out << "Monitor?StopSimulation" << std::endl;
    }

    out << "initial method call" << std::endl;
    out << "Initialise()()" << std::endl << std::endl;
    out << "instance methods" << std::endl;

    out << "Initialise()()" << std::endl << std::endl;
    out << "Status := new(DataBufferMonitor) init(BufferSize, InitialTokens, TokenSize, Monitor, "
           "MonitorID, Trace, Name);"
        << std::endl;
    out << "abort" << std::endl;
    out << "   par HandleInput()() and HandleOutput()() and if Monitor then "
           "CheckAccuracyStatus()() fi rap"
        << std::endl;
    out << "with Monitor?StopSimulation; if Monitor then Monitor!Results(Status getResults) fi."
        << std::endl
        << std::endl;

    out << "CheckAccuracyStatus()() |ID: Integer|" << std::endl << std::endl;
    out << "Monitor?Status(ID | ID = MonitorID);" << std::endl;
    out << "Monitor!AccuracyStatus(MonitorID, Status accurate);" << std::endl;
    out << "CheckAccuracyStatus()()." << std::endl << std::endl;

    out << "HandleInput()() |NumberOfTokens: Integer|" << std::endl << std::endl;
    out << "In?Room(NumberOfTokens | Status room(NumberOfTokens));" << std::endl;
    out << "In?Reserve{Status reserve(NumberOfTokens, currentTime)};" << std::endl;
    out << "In?Write{Status write(NumberOfTokens)};" << std::endl;
    out << "HandleInput()()." << std::endl << std::endl;

    out << "HandleOutput()() |NumberOfTokens: Integer|" << std::endl << std::endl;
    out << "Out?Inspect(NumberOfTokens | Status available(NumberOfTokens));" << std::endl;
    out << "Out?Read{Status remove(NumberOfTokens, currentTime)};" << std::endl;
    out << "HandleOutput()()." << std::endl << std::endl;

    // Generate Process Class ControlBuffer

    out << "process class ControlBuffer(BufferSize: Integer, NumbersInitialTokens: Queue, "
           "ContentInitialTokens: Queue, TokenSize: Integer, Monitor: Boolean, MonitorID: Integer, "
           "Trace: Boolean, Name: String)"
        << std::endl;
    out << "instance variables" << std::endl;
    out << "Status: ControlBufferMonitor" << std::endl << std::endl;

    if (!p4r) {
        out << "port interface" << std::endl;
        out << "Monitor, In, Out" << std::endl << std::endl;
        out << "message interface" << std::endl;
        out << "In?Room(Integer)," << std::endl;
        out << "In?Reserve," << std::endl;
        out << "In?Write(Integer)," << std::endl;
        out << "Out?Inspect," << std::endl;
        out << "Out!Scenario(Integer)," << std::endl;
        out << "Out?Read," << std::endl;
        out << "Monitor!Results(String)," << std::endl;
        out << "Monitor?Status(Integer)," << std::endl;
        out << "Monitor!AccuracyStatus(Integer, Boolean)," << std::endl;
        out << "Monitor?StopSimulation" << std::endl;
    }

    out << "initial method call" << std::endl;
    out << "Initialise()()" << std::endl << std::endl;
    out << "instance methods" << std::endl;

    out << "Initialise()()" << std::endl << std::endl;
    out << "Status := new(ControlBufferMonitor) init(BufferSize, NumbersInitialTokens, "
           "ContentInitialTokens, TokenSize, Monitor, MonitorID, Trace, Name);"
        << std::endl;
    out << "abort" << std::endl;
    out << "   par HandleInput()() and HandleOutput()() and if Monitor then "
           "CheckAccuracyStatus()() fi rap"
        << std::endl;
    out << "with Monitor?StopSimulation; if Monitor then Monitor!Results(Status getResults) fi."
        << std::endl
        << std::endl;

    out << "CheckAccuracyStatus()() |ID: Integer|" << std::endl << std::endl;
    out << "Monitor?Status(ID | ID = MonitorID);" << std::endl;
    out << "Monitor!AccuracyStatus(MonitorID, Status accurate);" << std::endl;
    out << "CheckAccuracyStatus()()." << std::endl << std::endl;

    out << "HandleInput()() |NumberOfTokens: Integer, Scenario: Integer|" << std::endl << std::endl;
    out << "In?Room(NumberOfTokens | Status room(NumberOfTokens));" << std::endl;
    out << "In?Reserve{Status reserve(NumberOfTokens, currentTime)};" << std::endl;
    out << "In?Write(Scenario){Status write(NumberOfTokens, Scenario)};" << std::endl;
    out << "HandleInput()()." << std::endl << std::endl;

    out << "HandleOutput()()" << std::endl << std::endl;
    out << "[Status available] Out?Inspect;" << std::endl;
    out << "Out!Scenario(Status inspect);" << std::endl;
    out << "Out?Read{Status remove(currentTime)};" << std::endl;
    out << "HandleOutput()()." << std::endl << std::endl;
}
