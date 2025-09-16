/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sadf2poosl_data.cc
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   13 September 2006
 *
 *  Function        :   Output SADF graph in POOSL format (data classes)
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

#include "sadf2poosl_data.h"

void SADF2POOSL_Data(std::ostream &out, bool PERT, bool p4r) { // NOLINT(*function-size)

    // Generate Data Classes for PERT Variant

    if (PERT) {
        out << "data class Gamma" << std::endl;
        out << "extends Distribution" << std::endl;
        out << "instance variables" << std::endl;
        out << "Alpha: Real, Beta: Real" << std::endl;
        out << "instance methods" << std::endl;
        out << "withParameters(a,b: Real) : Gamma" << std::endl;
        out << "self initialise;" << std::endl;
        out << "if a <= 0.0 then self error(\"Shape parameter of Gamma distribution should be "
               "larger than 0.0\") fi;"
            << std::endl;
        out << "if b <= 0.0 then self error(\"Scale parameter of Gamma distribution should be "
               "larger than 0.0\") fi;"
            << std::endl;
        out << "Alpha := a;" << std::endl;
        out << "Beta := b;" << std::endl;
        out << "return self." << std::endl;
        out << "sample : Real |T, log4, e, random, ainv, b, p, bbb, ccc, u, u1, u2, v, x, z, r, "
               "MAGICCONST: Real|"
            << std::endl;
        out << "MAGICCONST := 1.0 + 4.5 ln();" << std::endl;
        out << "log4 := 4.0 ln();" << std::endl;
        out << "e := 1.0 exp();" << std::endl;
        out << "random = Random random();" << std::endl;
        out << "T := -1.0;" << std::endl;
        out << "if Alpha > 1.0 then" << std::endl;
        out << "   ainv := (2.0 * Alpha - 1.0) sqrt();" << std::endl;
        out << "   bbb := Alpha - log4;" << std::endl;
        out << "   ccc := Alpha + ainv;" << std::endl;
        out << "   while T < 0.0 do" << std::endl;
        out << "      u1 := Random random();" << std::endl;
        out << "      if (1.0e-7 < u1) & (u1 < 0.9999999) then" << std::endl;
        out << "         u2 := 1.0 - Random random();" << std::endl;
        out << "         v := ((u1/(1.0-u1)) ln()) / ainv;" << std::endl;
        out << "         x := Alpha * (v exp());" << std::endl;
        out << "         z := u1 * u1 * u2;" << std::endl;
        out << "         r := bbb + ccc * v - x;" << std::endl;
        out << "         if ((r + MAGICCONST - 4.5 * z) >= 0.0) | (r >= (z ln())) then"
            << std::endl;
        out << "            T := x * Beta" << std::endl;
        out << "         fi" << std::endl;
        out << "      fi" << std::endl;
        out << "   od" << std::endl;
        out << "fi;" << std::endl;
        out << "if Alpha = 1.0 then" << std::endl;
        out << "   u := Random random();" << std::endl;
        out << "   while u <= 1.0e-7 do" << std::endl;
        out << "      u := Random random()" << std::endl;
        out << "   od;" << std::endl;
        out << "   T := -(u ln()) * Beta" << std::endl;
        out << "fi;" << std::endl;
        out << "if Alpha < 1.0 then" << std::endl;
        out << "   while T < 0.0 do" << std::endl;
        out << "      u := Random random();" << std::endl;
        out << "      b := (e + Alpha) / e;" << std::endl;
        out << "      p := b * u;" << std::endl;
        out << "      if p <= 1.0 then" << std::endl;
        out << "         x := p power(1.0 / Alpha)" << std::endl;
        out << "      else" << std::endl;
        out << "         x := -(((b-p) / Alpha) ln())" << std::endl;
        out << "      fi;" << std::endl;
        out << "      u1 := Random random();" << std::endl;
        out << "      if p > 1.0 then" << std::endl;
        out << "         if u1 <= (x power(Alpha - 1.0)) then" << std::endl;
        out << "            T := x * Beta" << std::endl;
        out << "         fi" << std::endl;
        out << "      else" << std::endl;
        out << "         if u1 <= ((-x) exp()) then" << std::endl;
        out << "            T := x * Beta" << std::endl;
        out << "         fi" << std::endl;
        out << "      fi" << std::endl;
        out << "   od" << std::endl;
        out << "fi;" << std::endl;
        out << "return T." << std::endl;

        out << "data class PERT" << std::endl;
        out << "extends Object" << std::endl;
        out << "instance variables" << std::endl;
        out << "Mode: Real, UpperBound: Real, Fixed: Boolean, GammaB: Gamma, LowerBound: Real, "
               "GammaA: Gamma"
            << std::endl;
        out << "instance methods" << std::endl;
        out << "withParameters(L, M, U: Real) : PERT |Mean, Std, Alpha, Beta: Real|" << std::endl;
        out << "Fixed := false; LowerBound := L; Mode := M; UpperBound := U;" << std::endl;
        out << "if LowerBound = UpperBound then Fixed := true fi;" << std::endl;
        out << "if Fixed not then" << std::endl;
        out << "   if (LowerBound >= Mode) | (Mode >= UpperBound) then self error(\"Parameters of "
               "PERT distribution do not satisfy LowerBound < Mode < UpperBound or LowerBound = "
               "UpperBound\") fi;"
            << std::endl;
        out << "   Mean := (LowerBound + 4 * Mode + UpperBound) / 6;" << std::endl;
        out << "   Std := (UpperBound - LowerBound) / 6;" << std::endl;
        out << "   Alpha := ((Mean - LowerBound)/(UpperBound - LowerBound)) * ((((Mean - "
               "LowerBound) * (UpperBound - Mean)) / (Std sqr)) - 1);"
            << std::endl;
        out << "   Beta := ((UpperBound - Mean) / (Mean - LowerBound)) * Alpha;" << std::endl;
        out << "   GammaA := new(Gamma) withParameters(Alpha, 1);" << std::endl;
        out << "   GammaB := new(Gamma) withParameters(Beta, 1)" << std::endl;
        out << "fi;" << std::endl;
        out << "return self." << std::endl;
        out << "sample : Real |Sample: Real|" << std::endl;
        out << "if Fixed then" << std::endl;
        out << "   return LowerBound" << std::endl;
        out << "else" << std::endl;
        out << "   Sample := GammaA sample;" << std::endl;
        out << "   if Sample = 0 then return LowerBound else return LowerBound + (UpperBound - "
               "LowerBound) * (Sample / (Sample + GammaB sample)) fi"
            << std::endl;
        out << "fi." << std::endl;

        out << "data class ProcessMonitor_PERT" << std::endl;
        out << "extends ProcessMonitor" << std::endl;
        out << "instance variables" << std::endl;
        out << "Samples: Queue" << std::endl << std::endl;
        out << "instance methods" << std::endl;
        out << "init(M: Boolean, ID: Integer, T: Boolean, N: String) : ProcessMonitor_PERT"
            << std::endl
            << std::endl;
        out << "Utilisation := 0;" << std::endl;
        out << "PreviousUpdateTime := nil; PreviousFireTime:= nil;" << std::endl;
        out << "Name := N;" << std::endl;
        out << "Monitor := M;" << std::endl;
        out << "if Monitor then" << std::endl;
        out << "   ResponseDelay := nil;" << std::endl;
        out << "   MaximumDelay := 0.0; MinimumDelay = nil;" << std::endl;
        out << "   Throughput := new(LongRunRateAverage) withParameters(0.95, 0.95);" << std::endl;
        out << "   AverageDelay := new(LongRunSampleAverage) withParameters(0.95, 0.95);"
            << std::endl;
        out << "   VarianceDelay := new(LongRunSampleVariance) withParameters(0.95, 0.95);"
            << std::endl;
        out << "   Samples := new(Queue) init" << std::endl;
        out << "fi;" << std::endl;
        out << "Trace := T;" << std::endl;
        out << "if Trace then" << std::endl;
        out << R"(   TraceFile := new(FileOut) destination("Process_" + Name + ".trace") open;)"
            << std::endl;
        out << "   TraceFile writeString(\"Traced Behaviour of Process: \" + Name cr cr)"
            << std::endl;
        out << "fi;" << std::endl;
        out << "return(self)." << std::endl << std::endl;
        out << "end(CurrentTime: Real) : ProcessMonitor_PERT" << std::endl << std::endl;
        out << "if Trace & (CurrentTime > PreviousUpdateTime) then" << std::endl;
        out << R"(   TraceFile writeString("At time: " + PreviousUpdateTime printString + " Utilisation: " + Utilisation printString cr))"
            << std::endl;
        out << "fi;" << std::endl;
        out << "Utilisation := Utilisation - 1;" << std::endl;
        out << "if Monitor & (ResponseDelay = nil) then ResponseDelay := CurrentTime fi;"
            << std::endl;
        out << "if Monitor & (PreviousFireTime != nil) then" << std::endl;
        out << "   if (CurrentTime - PreviousFireTime) > MaximumDelay then MaximumDelay := "
               "CurrentTime - PreviousFireTime fi;"
            << std::endl;
        out << "   if MinimumDelay != nil then" << std::endl;
        out << "      if (CurrentTime - PreviousFireTime) < MinimumDelay then MinimumDelay := "
               "CurrentTime - PreviousFireTime fi"
            << std::endl;
        out << "   else" << std::endl;
        out << "      MinimumDelay := CurrentTime - PreviousFireTime" << std::endl;
        out << "   fi;" << std::endl;
        out << "   Throughput rewardBM(1, CurrentTime);" << std::endl;
        out << "   AverageDelay rewardBM(CurrentTime - PreviousFireTime);" << std::endl;
        out << "   VarianceDelay rewardBM(CurrentTime - PreviousFireTime);" << std::endl;
        out << "   Samples put(CurrentTime - PreviousFireTime)" << std::endl;
        out << "fi;" << std::endl;
        out << "PreviousUpdateTime := CurrentTime; PreviousFireTime := CurrentTime;" << std::endl;
        out << "return(self)." << std::endl << std::endl;
        out << "getResults : String |Result: String, H: Histogram|" << std::endl << std::endl;
        out << R"(Result := "-------------------------------" cr + "Statistics for Process: " + Name cr cr;)"
            << std::endl;
        out << "Result := Result + \"Observed Response Delay: \" + ResponseDelay printString cr;"
            << std::endl;
        out << "Result := Result + \"Observed Minimum Inter-Firing Latency: \" + MinimumDelay "
               "printString cr;"
            << std::endl;
        out << "Result := Result + \"Observed Maximum Inter-Firing Latency: \" + MaximumDelay "
               "printString cr cr;"
            << std::endl;
        out << "Result := Result + \"Average Inter-Firing Latency:\" cr + AverageDelay "
               "logStatistics cr cr;"
            << std::endl;
        out << "Result := Result + \"Variance in Inter-Firing Latency:\" cr + VarianceDelay "
               "logStatistics cr cr;"
            << std::endl;
        out << "Result := Result + \"Throughput:\" cr + Throughput logStatistics cr cr;"
            << std::endl;
        out << "Result := Result + \"Histogram Inter-Firing Time:\" cr;" << std::endl;
        out << "H := new(Histogram) withParameters(MinimumDelay, MaximumDelay, 100);" << std::endl;
        out << "while Samples isNotEmpty do H sample(Samples remove) od;" << std::endl;
        out << "Result := Result + H printString cr cr;" << std::endl;
        out << "return(Result)." << std::endl << std::endl;

        out << "data class Histogram" << std::endl;
        out << "extends Object" << std::endl;
        out << "instance variables" << std::endl;
        out << "NumberOfSlots: Integer, Maximum: Real, SizeOfInterval: Real, Minimum: Real, "
               "NumberOfSamples: Integer, Histogram: Array"
            << std::endl;
        out << "instance methods" << std::endl;
        out << "withParameters(L, U, N: Integer) : Histogram" << std::endl;
        out << "Minimum := L; Maximum := U; NumberOfSlots := N;" << std::endl;
        out << "if Minimum >= Maximum then self error(\"Minimum parameter of Histogram must be "
               "smaller than Maximum parameter\") fi;"
            << std::endl;
        if (p4r) {
            out << "Histogram := new(Array) size(NumberOfSlots) putAll(0);" << std::endl;
        } else {
            out << "Histogram := new(Array) resize(NumberOfSlots) putAll(0);" << std::endl;
        }
        out << "NumberOfSamples := 0; SizeOfInterval := (Maximum - Minimum) / NumberOfSlots;"
            << std::endl;
        out << "return self." << std::endl;
        out << "printString : String |i: Integer, Result: String|" << std::endl;
        out << "Result := \"\";" << std::endl;
        out << "if NumberOfSamples != 0 then" << std::endl;
        out << "   i := 1;" << std::endl;
        out << "   while i <= NumberOfSlots do" << std::endl;
        if (p4r) {
            out << "      Result := Result + (Minimum + (i - 1) * SizeOfInterval) printString tab "
                   "+ Histogram get(i) printString cr;"
                << std::endl;
        } else {
            out << "      Result := Result + (Minimum + (i - 1) * SizeOfInterval) printString tab "
                   "+ Histogram at(i) printString cr;"
                << std::endl;
        }
        out << "      i := i + 1" << std::endl;
        out << "   od" << std::endl;
        out << "fi;" << std::endl;
        out << "return Result." << std::endl;
        out << "sample(Value: Real) : Histogram |i: Integer, b: Boolean|" << std::endl;
        out << "NumberOfSamples := NumberOfSamples + 1;" << std::endl;
        out << "if (Value >= Minimum) & (Value <= Maximum) then" << std::endl;
        out << "   b := true; i := 1;" << std::endl;
        out << "   while b & (i <= NumberOfSlots) do" << std::endl;
        if (p4r) {
            out << "      if (Value > Minimum + (i - 1) * SizeOfInterval) & (Value <= Minimum + i "
                   "* SizeOfInterval) then Histogram put(i, Histogram get(i) + 1); b := false fi;"
                << std::endl;
        } else {
            out << "      if (Value > Minimum + (i - 1) * SizeOfInterval) & (Value <= Minimum + i "
                   "* SizeOfInterval) then Histogram put(i, Histogram at(i) + 1); b := false fi;"
                << std::endl;
        }
        out << "      i := i + 1" << std::endl;
        out << "   od" << std::endl;
        out << "fi;" << std::endl;
        out << "return self." << std::endl << std::endl;
    }

    // Generate Non-Primitive Data Class MarkovChain

    out << "data class MarkovChain" << std::endl;
    out << "extends Object" << std::endl;
    out << "instance variables" << std::endl;
    out << "NumberOfStates: Integer, StateSpace: Array, SubScenarios: Array, Transitions: Array, "
           "CurrentState: String, Random: Uniform"
        << std::endl;
    out << "instance methods" << std::endl;

    out << "init : MarkovChain" << std::endl << std::endl;
    out << "NumberOfStates := 0;" << std::endl;
    out << "StateSpace := new(Array);" << std::endl;
    out << "SubScenarios := new(Array);" << std::endl;
    out << "Transitions := new(Array);" << std::endl;
    out << "Random := new(Uniform) withParameters(0.0, 1.0);" << std::endl;
    out << "return(self)." << std::endl << std::endl;

    out << "addState(State: String, SubScenario: Integer) : MarkovChain" << std::endl << std::endl;
    out << "NumberOfStates := NumberOfStates + 1;" << std::endl;
    if (p4r) {
        out << "StateSpace size(NumberOfStates); StateSpace put(NumberOfStates, State);"
            << std::endl;
        out << "SubScenarios size(NumberOfStates); SubScenarios put(NumberOfStates, SubScenario);"
            << std::endl;
        out << "Transitions size(NumberOfStates); Transitions put(NumberOfStates, new(Array));"
            << std::endl;
    } else {
        out << "StateSpace resize(NumberOfStates); StateSpace put(NumberOfStates, State);"
            << std::endl;
        out << "SubScenarios resize(NumberOfStates); SubScenarios put(NumberOfStates, SubScenario);"
            << std::endl;
        out << "Transitions resize(NumberOfStates); Transitions put(NumberOfStates, new(Array));"
            << std::endl;
    }
    out << "return(self)." << std::endl << std::endl;

    out << "addTransition(State: String, NextState: String, LowerBound: Real, UpperBound: Real) : "
           "MarkovChain |i, StateNumber: Integer, T, A: Array|"
        << std::endl
        << std::endl;
    out << "i := 1;" << std::endl;
    if (p4r) {
        out << "while i <= StateSpace getSize do" << std::endl;
        out << "   if StateSpace get(i) = State then StateNumber := i fi;" << std::endl;
    } else {
        out << "while i <= StateSpace size do" << std::endl;
        out << "   if StateSpace at(i) = State then StateNumber := i fi;" << std::endl;
    }
    out << "   i := i + 1" << std::endl;
    out << "od;" << std::endl;
    if (p4r) {
        out << "T := Transitions get(StateNumber);" << std::endl;
        out << "A := new(Array) size(3); A put(1, NextState) put(2, LowerBound) put(3, UpperBound);"
            << std::endl;
        out << "T size(T getSize + 1); T put(T getSize, A);" << std::endl;
    } else {
        out << "T := Transitions at(StateNumber);" << std::endl;
        out << "A := new(Array) resize(3); A put(1, NextState) put(2, LowerBound) put(3, "
               "UpperBound);"
            << std::endl;
        out << "T resize(T size + 1); T put(T size, A);" << std::endl;
    }
    out << "return(self)." << std::endl << std::endl;

    out << "setInitialState(State: String) : MarkovChain" << std::endl << std::endl;
    out << "CurrentState := State;" << std::endl;
    out << "return(self)." << std::endl << std::endl;

    out << "getNextSubScenario : Integer |NextState: String, SubScenario, i, StateNumber: Integer, "
           "T: Array, Sample: Real|"
        << std::endl
        << std::endl;
    out << "i := 1;" << std::endl;
    if (p4r) {
        out << "while i <= StateSpace getSize do" << std::endl;
        out << "   if StateSpace get(i) = CurrentState then StateNumber := i fi;" << std::endl;
    } else {
        out << "while i <= StateSpace size do" << std::endl;
        out << "   if StateSpace at(i) = CurrentState then StateNumber := i fi;" << std::endl;
    }
    out << "   i := i + 1" << std::endl;
    out << "od;" << std::endl;
    if (p4r) {
        out << "T := Transitions get(StateNumber);" << std::endl;
    } else {
        out << "T := Transitions at(StateNumber);" << std::endl;
    }
    out << "Sample := Random sample;" << std::endl;
    out << "i := 1;" << std::endl;
    if (p4r) {
        out << "while i <= T getSize do" << std::endl;
        out << "   if ((Sample > T get(i) get(2)) & (Sample <= T get(i) get(3))) then NextState := "
               "T get(i) get(1) fi;"
            << std::endl;
    } else {
        out << "while i <= T size do" << std::endl;
        out << "   if ((Sample > T at(i) at(2)) & (Sample <= T at(i) at(3))) then NextState := T "
               "at(i) at(1) fi;"
            << std::endl;
    }
    out << "   i := i + 1" << std::endl;
    out << "od;" << std::endl;
    out << "i := 1;" << std::endl;
    if (p4r) {
        out << "while i <= StateSpace getSize do" << std::endl;
        out << "   if StateSpace get(i) = NextState then SubScenario := SubScenarios get(i) fi;"
            << std::endl;
    } else {
        out << "while i <= StateSpace size do" << std::endl;
        out << "   if StateSpace at(i) = NextState then SubScenario := SubScenarios at(i) fi;"
            << std::endl;
    }
    out << "   i := i + 1" << std::endl;
    out << "od;" << std::endl;
    out << "CurrentState := NextState;" << std::endl;
    out << "return(SubScenario)." << std::endl << std::endl;

    // Generate Non-Primitive Data Class ProcessMonitor

    out << "data class ProcessMonitor" << std::endl;
    out << "extends Object" << std::endl;
    out << "instance variables" << std::endl;
    out << "Name: String, Memory: Integer, Utilisation: Integer, Throughput: LongRunRateAverage, "
           "PreviousUpdateTime: Real, PreviousFireTime: Real, ResponseDelay: Real, ";
    out << "Monitor: Boolean, Trace: Boolean, TraceFile: FileOut, AverageDelay: "
           "LongRunSampleAverage, VarianceDelay: LongRunSampleVariance, MaximumDelay: Real, "
           "MinimumDelay: Real"
        << std::endl
        << std::endl;
    out << "instance methods" << std::endl;

    out << "init(M: Boolean, ID: Integer, T: Boolean, N: String) : ProcessMonitor" << std::endl
        << std::endl;
    out << "Utilisation := 0;" << std::endl;
    out << "PreviousUpdateTime := nil; PreviousFireTime:= nil;" << std::endl;
    out << "Name := N;" << std::endl;
    out << "Monitor := M;" << std::endl;
    out << "if Monitor then" << std::endl;
    out << "   ResponseDelay := nil;" << std::endl;
    out << "   MaximumDelay := 0.0; MinimumDelay = nil;" << std::endl;
    out << "   Throughput := new(LongRunRateAverage) withParameters(0.95, 0.95);" << std::endl;
    out << "   AverageDelay := new(LongRunSampleAverage) withParameters(0.95, 0.95);" << std::endl;
    out << "   VarianceDelay := new(LongRunSampleVariance) withParameters(0.95, 0.95)" << std::endl;
    out << "fi;" << std::endl;
    out << "Trace := T;" << std::endl;
    out << "if Trace then" << std::endl;
    out << R"(   TraceFile := new(FileOut) destination("Process_" + Name + ".trace") open;)"
        << std::endl;
    out << "   TraceFile writeString(\"Traced Behaviour of Process: \" + Name cr cr)" << std::endl;
    out << "fi;" << std::endl;
    out << "return(self)." << std::endl << std::endl;

    out << "start(CurrentTime: Real) : ProcessMonitor" << std::endl << std::endl;
    out << "if PreviousUpdateTime != nil then" << std::endl;
    out << R"(   if Trace & (CurrentTime > PreviousUpdateTime) then TraceFile writeString("At time: " + PreviousUpdateTime printString + " Utilisation: " + Utilisation printString cr) fi)"
        << std::endl;
    out << "fi;" << std::endl;
    out << "Utilisation := Utilisation + 1;" << std::endl;
    out << "PreviousUpdateTime := CurrentTime;" << std::endl;
    out << "return(self)." << std::endl << std::endl;

    out << "end(CurrentTime: Real) : ProcessMonitor" << std::endl << std::endl;
    out << "if Trace & (CurrentTime > PreviousUpdateTime) then" << std::endl;
    out << R"(   TraceFile writeString("At time: " + PreviousUpdateTime printString + " Utilisation: " + Utilisation printString cr))"
        << std::endl;
    out << "fi;" << std::endl;
    out << "Utilisation := Utilisation - 1;" << std::endl;
    out << "if Monitor & (ResponseDelay = nil) then ResponseDelay := CurrentTime fi;" << std::endl;
    out << "if Monitor & (PreviousFireTime != nil) then" << std::endl;
    out << "   if (CurrentTime - PreviousFireTime) > MaximumDelay then MaximumDelay := CurrentTime "
           "- PreviousFireTime fi;"
        << std::endl;
    out << "   if MinimumDelay != nil then" << std::endl;
    out << "      if (CurrentTime - PreviousFireTime) < MinimumDelay then MinimumDelay := "
           "CurrentTime - PreviousFireTime fi"
        << std::endl;
    out << "   else" << std::endl;
    out << "      MinimumDelay := CurrentTime - PreviousFireTime" << std::endl;
    out << "   fi;" << std::endl;
    out << "   Throughput rewardBM(1.0, CurrentTime);" << std::endl;
    out << "   AverageDelay rewardBM(CurrentTime - PreviousFireTime);" << std::endl;
    out << "   VarianceDelay rewardBM(CurrentTime - PreviousFireTime)" << std::endl;
    out << "fi;" << std::endl;
    out << "PreviousUpdateTime := CurrentTime; PreviousFireTime := CurrentTime;" << std::endl;
    out << "return(self)." << std::endl << std::endl;

    out << "accurate : Boolean" << std::endl << std::endl;
    out << "if Monitor then" << std::endl;
    out << "   return((Throughput accurate) & (AverageDelay accurate))" << std::endl;
    out << "else" << std::endl;
    out << "   return(true)" << std::endl;
    out << "fi." << std::endl << std::endl;

    out << "getResults : String |Result: String|" << std::endl << std::endl;
    out << R"(Result := "-------------------------------" cr + "Statistics for Process: " + Name cr cr;)"
        << std::endl;
    out << "Result := Result + \"Observed Response Delay: \" + ResponseDelay printString cr;"
        << std::endl;
    out << "Result := Result + \"Observed Minimum Inter-Firing Latency: \" + MinimumDelay "
           "printString cr;"
        << std::endl;
    out << "Result := Result + \"Observed Maximum Inter-Firing Latency: \" + MaximumDelay "
           "printString cr cr;"
        << std::endl;
    out << "Result := Result + \"Average Inter-Firing Latency:\" cr + AverageDelay logStatistics "
           "cr cr;"
        << std::endl;
    out << "Result := Result + \"Variance in Inter-Firing Latency:\" cr + VarianceDelay "
           "logStatistics cr cr;"
        << std::endl;
    out << "Result := Result + \"Throughput:\" cr + Throughput logStatistics cr cr;" << std::endl;
    out << "return(Result)." << std::endl << std::endl;

    // Generate Non-Primitive Data Class DataBufferMonitor

    out << "data class DataBufferMonitor" << std::endl;
    out << "extends Object" << std::endl;
    out << "instance variables" << std::endl;
    out << "Name: String, AvailableTokens: Integer, AverageOccupation: LongRunTimeAverage, "
           "VarianceOccupation: LongRunTimeVariance, BufferSize: Integer, MaximumOccupation: "
           "Integer, Monitor: Boolean, PreviousTime: Real, ReservedLocations: Integer, TokenSize: "
           "Integer, Trace: Boolean, TraceFile: FileOut"
        << std::endl
        << std::endl;
    out << "instance methods" << std::endl;

    out << "init(BS, IT, TS: Integer, M: Boolean, ID: Integer, T: Boolean, N: String) : "
           "DataBufferMonitor"
        << std::endl
        << std::endl;
    out << "Name := N;" << std::endl;
    out << "BufferSize := BS; TokenSize := TS; AvailableTokens := 0; ReservedLocations := 0;"
        << std::endl;
    out << "PreviousTime := 0.0;" << std::endl;
    out << "Monitor := M;" << std::endl;
    out << "if Monitor then" << std::endl;
    out << "   MaximumOccupation := 0;" << std::endl;
    out << "   AverageOccupation := new(LongRunTimeAverage) withParameters(0.95, 0.95);"
        << std::endl;
    out << "   VarianceOccupation := new(LongRunTimeVariance) withParameters(0.95, 0.95)"
        << std::endl;
    out << "fi;" << std::endl;
    out << "Trace := T;" << std::endl;
    out << "if Trace then" << std::endl;
    out << R"(   TraceFile := new(FileOut) destination("Channel_" + Name + ".trace") open;)"
        << std::endl;
    out << "   TraceFile writeString(\"Traced Buffer Occupancy for Channel: \" + Name cr cr)"
        << std::endl;
    out << "fi;" << std::endl;
    out << "self reserve(IT, 0.0); self write(IT);" << std::endl;
    out << "return(self)." << std::endl << std::endl;

    out << "room(NumberOfTokens: Integer) : Boolean" << std::endl << std::endl;
    out << "if BufferSize > 0 then" << std::endl;
    out << "   return((ReservedLocations + AvailableTokens + NumberOfTokens) <= BufferSize)"
        << std::endl;
    out << "else" << std::endl;
    out << "   return(true)" << std::endl;
    out << "fi." << std::endl << std::endl;

    out << "available(NumberOfTokens: Integer) : Boolean" << std::endl << std::endl;
    out << "return(AvailableTokens >= NumberOfTokens)." << std::endl << std::endl;

    out << "reserve(NumberOfTokens: Integer, CurrentTime: Real) : DataBufferMonitor |Occupation: "
           "Integer|"
        << std::endl
        << std::endl;
    out << "Occupation := ReservedLocations + AvailableTokens;" << std::endl;
    out << "if Trace & (CurrentTime > PreviousTime) then" << std::endl;
    out << R"(   TraceFile writeString("At time: " + PreviousTime printString + " Occupation: " + (Occupation * TokenSize) printString cr))"
        << std::endl;
    out << "fi;" << std::endl;
    out << "ReservedLocations := ReservedLocations + NumberOfTokens; Occupation := Occupation + "
           "NumberOfTokens;"
        << std::endl;
    out << "if Monitor then" << std::endl;
    out << "   if (Occupation * TokenSize) > MaximumOccupation then MaximumOccupation := "
           "Occupation * TokenSize fi;"
        << std::endl;
    out << "   AverageOccupation rewardBM(Occupation * TokenSize, CurrentTime); VarianceOccupation "
           "rewardBM(Occupation * TokenSize, CurrentTime)"
        << std::endl;
    out << "fi;" << std::endl;
    out << "PreviousTime := CurrentTime;" << std::endl;
    out << "return(self)." << std::endl << std::endl;

    out << "write(NumberOfTokens: Integer) : DataBufferMonitor" << std::endl << std::endl;
    out << "ReservedLocations := ReservedLocations - NumberOfTokens;" << std::endl;
    out << "AvailableTokens := AvailableTokens + NumberOfTokens;" << std::endl;
    out << "return(self)." << std::endl << std::endl;

    out << "remove(NumberOfTokens: Integer, CurrentTime: Real) : DataBufferMonitor |Occupation: "
           "Integer|"
        << std::endl
        << std::endl;
    out << "Occupation := ReservedLocations + AvailableTokens;" << std::endl;
    out << "if Trace & (CurrentTime > PreviousTime) then" << std::endl;
    out << R"(   TraceFile writeString("At time: " + PreviousTime printString + " Occupation: " + (Occupation * TokenSize) printString cr))"
        << std::endl;
    out << "fi;" << std::endl;
    out << "AvailableTokens := AvailableTokens - NumberOfTokens; Occupation := Occupation - "
           "NumberOfTokens;"
        << std::endl;
    out << "if Monitor then" << std::endl;
    out << "   AverageOccupation rewardBM(Occupation * TokenSize, CurrentTime); VarianceOccupation "
           "rewardBM(Occupation * TokenSize, CurrentTime)"
        << std::endl;
    out << "fi;" << std::endl;
    out << "PreviousTime := CurrentTime;" << std::endl;
    out << "return(self)." << std::endl << std::endl;

    out << "accurate : Boolean" << std::endl << std::endl;
    out << "if Monitor then" << std::endl;
    out << "   return(AverageOccupation accurate)" << std::endl;
    out << "else" << std::endl;
    out << "   return(true)" << std::endl;
    out << "fi." << std::endl << std::endl;

    out << "getResults : String |Result: String|" << std::endl << std::endl;
    out << R"(Result := "-------------------------------" cr + "Statistics for Channel: " + Name cr cr;)"
        << std::endl;
    out << "Result := Result + \"Observed Maximum Buffer Occupancy: \" + MaximumOccupation "
           "printString cr cr;"
        << std::endl;
    out << "Result := Result + \"Average Buffer Occupancy:\" cr + AverageOccupation logStatistics "
           "cr cr;"
        << std::endl;
    out << "Result := Result + \"Variance in Buffer Occupancy:\" cr + VarianceOccupation "
           "logStatistics cr cr;"
        << std::endl;
    out << "return(Result)." << std::endl << std::endl;

    // Generate Non-Primitive Data Class ControlBufferMonitor

    out << "data class ControlBufferMonitor" << std::endl;
    out << "extends Object" << std::endl;
    out << "instance variables" << std::endl;
    out << "Name: String, AvailableTokens: Integer, AverageOccupation: LongRunTimeAverage, "
           "VarianceOccupation: LongRunTimeVariance, BufferSize: Integer, MaximumOccupation: "
           "Integer, Monitor: Boolean, PreviousTime: Real, ReservedLocations: Integer, TokenSize: "
           "Integer, Trace: Boolean, TraceFile: FileOut, Buffer: Queue"
        << std::endl
        << std::endl;
    out << "instance methods" << std::endl;

    out << "init(BS: Integer, NIT, CIT: Queue, TS: Integer, M: Boolean, ID: Integer, T: Boolean, "
           "N: String) : ControlBufferMonitor"
        << std::endl
        << std::endl;
    out << "Name := N;" << std::endl;
    out << "BufferSize := BS; TokenSize := TS; AvailableTokens := 0; ReservedLocations := 0;"
        << std::endl;
    out << "PreviousTime := 0.0;" << std::endl;
    out << "Monitor := M;" << std::endl;
    out << "if Monitor then" << std::endl;
    out << "   MaximumOccupation := 0;" << std::endl;
    out << "   AverageOccupation := new(LongRunTimeAverage) withParameters(0.95, 0.95);"
        << std::endl;
    out << "   VarianceOccupation := new(LongRunTimeVariance) withParameters(0.95, 0.95)"
        << std::endl;
    out << "fi;" << std::endl;
    out << "Trace := T;" << std::endl;
    out << "if Trace then" << std::endl;
    out << R"(   TraceFile := new(FileOut) destination("Channel_" + Name + ".trace") open;)"
        << std::endl;
    out << "   TraceFile writeString(\"Traced Buffer Occupancy for Channel: \" + Name cr cr)"
        << std::endl;
    out << "fi;" << std::endl;
    out << "Buffer := new(Queue) init;" << std::endl;
    out << "while NIT isNotEmpty do" << std::endl;
    out << "   self reserve(NIT inspect, 0.0); self write(NIT inspect, CIT inspect);" << std::endl;
    out << "   NIT remove; CIT remove" << std::endl;
    out << "od;" << std::endl;
    out << "return(self)." << std::endl << std::endl;

    out << "room(NumberOfTokens: Integer) : Boolean" << std::endl << std::endl;
    out << "if BufferSize > 0 then" << std::endl;
    out << "   return((ReservedLocations + AvailableTokens + NumberOfTokens) <= BufferSize)"
        << std::endl;
    out << "else" << std::endl;
    out << "   return(true)" << std::endl;
    out << "fi." << std::endl << std::endl;

    out << "inspect() : Integer" << std::endl << std::endl;
    out << "return(Buffer inspect)." << std::endl << std::endl;

    out << "reserve(NumberOfTokens: Integer, CurrentTime: Real) : ControlBufferMonitor "
           "|Occupation: Integer|"
        << std::endl
        << std::endl;
    out << "Occupation := ReservedLocations + AvailableTokens;" << std::endl;
    out << "if Trace & (CurrentTime > PreviousTime) then" << std::endl;
    out << R"(   TraceFile writeString("At time: " + PreviousTime printString + " Occupation: " + (Occupation * TokenSize) printString cr))"
        << std::endl;
    out << "fi;" << std::endl;
    out << "ReservedLocations := ReservedLocations + NumberOfTokens; Occupation := Occupation + "
           "NumberOfTokens;"
        << std::endl;
    out << "if Monitor then" << std::endl;
    out << "   if (Occupation * TokenSize) > MaximumOccupation then MaximumOccupation := "
           "Occupation * TokenSize fi;"
        << std::endl;
    out << "   AverageOccupation rewardBM(Occupation * TokenSize, CurrentTime); VarianceOccupation "
           "rewardBM(Occupation * TokenSize, CurrentTime)"
        << std::endl;
    out << "fi;" << std::endl;
    out << "PreviousTime := CurrentTime;" << std::endl;
    out << "return(self)." << std::endl << std::endl;

    out << "write(NumberOfTokens: Integer, Scenario: Integer) : ControlBufferMonitor |Counter: "
           "Integer|"
        << std::endl
        << std::endl;
    out << "ReservedLocations := ReservedLocations - NumberOfTokens;" << std::endl;
    out << "AvailableTokens := AvailableTokens + NumberOfTokens;" << std::endl;
    out << "Counter := NumberOfTokens;" << std::endl;
    out << "while (Counter > 0) do" << std::endl;
    out << "   Buffer put(Scenario);" << std::endl;
    out << "   Counter := Counter - 1" << std::endl;
    out << "od;" << std::endl;
    out << "return(self)." << std::endl << std::endl;

    out << "available : Boolean" << std::endl << std::endl;
    out << "return(AvailableTokens >= 1)." << std::endl << std::endl;

    out << "remove(CurrentTime: Real) : Integer |Occupation: Integer|" << std::endl << std::endl;
    out << "Occupation := ReservedLocations + AvailableTokens;" << std::endl;
    out << "if Trace & (CurrentTime > PreviousTime) then" << std::endl;
    out << R"(   TraceFile writeString("At time: " + PreviousTime printString + " Occupation: " + (Occupation * TokenSize) printString cr))"
        << std::endl;
    out << "fi;" << std::endl;
    out << "AvailableTokens := AvailableTokens - 1; Occupation := Occupation - 1;" << std::endl;
    out << "if Monitor then" << std::endl;
    out << "   AverageOccupation rewardBM(Occupation * TokenSize, CurrentTime); VarianceOccupation "
           "rewardBM(Occupation * TokenSize, CurrentTime)"
        << std::endl;
    out << "fi;" << std::endl;
    out << "PreviousTime := CurrentTime;" << std::endl;
    out << "return(Buffer remove)." << std::endl << std::endl;

    out << "accurate : Boolean" << std::endl << std::endl;
    out << "if Monitor then" << std::endl;
    out << "   return(AverageOccupation accurate)" << std::endl;
    out << "else" << std::endl;
    out << "   return(true)" << std::endl;
    out << "fi." << std::endl << std::endl;

    out << "getResults : String |Result: String|" << std::endl << std::endl;
    out << R"(Result := "-------------------------------" cr + "Statistics for Channel: " + Name cr cr;)"
        << std::endl;
    out << "Result := Result + \"Observed Maximum Buffer Occupancy: \" + MaximumOccupation "
           "printString cr cr;"
        << std::endl;
    out << "Result := Result + \"Average Buffer Occupancy:\" cr + AverageOccupation logStatistics "
           "cr cr;"
        << std::endl;
    out << "Result := Result + \"Variance in Buffer Occupancy:\" cr + VarianceOccupation "
           "logStatistics cr cr;"
        << std::endl;
    out << "return(Result)." << std::endl << std::endl;

    // Generate Non-Primitive Data Class MonitorStatus

    out << "data class MonitorStatus" << std::endl;
    out << "extends Object" << std::endl;
    out << "instance variables" << std::endl;
    out << "Status: Array, NumberOfMonitors: Integer, LogFileName: String, Result: String"
        << std::endl;
    out << "instance methods" << std::endl;

    out << "init(N: Integer, LFN: String) : MonitorStatus" << std::endl << std::endl;
    out << "NumberOfMonitors := N;" << std::endl;
    if (p4r) {
        out << "Status := new(Array) size(NumberOfMonitors) putAll(false);" << std::endl;
    } else {
        out << "Status := new(Array) resize(NumberOfMonitors) putAll(false);" << std::endl;
    }
    out << "LogFileName := LFN;" << std::endl;
    out << "Result := \"Summary of Simulation Results\" cr cr;" << std::endl;
    out << "return(self)." << std::endl << std::endl;

    out << "accurate : Boolean |Accurate: Boolean, i: Integer|" << std::endl;
    out << "Accurate := true; i := 1;";
    if (p4r) {
        out << "while i <= NumberOfMonitors do Accurate := Accurate & Status get(i); i := i + 1 od;"
            << std::endl;
    } else {
        out << "while i <= NumberOfMonitors do Accurate := Accurate & Status at(i); i := i + 1 od;"
            << std::endl;
    }
    out << "return(Accurate)." << std::endl << std::endl;

    out << "register(ID: Integer, S: Boolean) : MonitorStatus" << std::endl;
    out << "Status put(ID, S);" << std::endl;
    out << "return(self)." << std::endl << std::endl;

    out << "append(R: String) : MonitorStatus" << std::endl << std::endl;
    out << "Result := Result + R;" << std::endl;
    out << "return(self)." << std::endl << std::endl;

    out << "log : MonitorStatus |LogFile: FileOut|" << std::endl << std::endl;
    out << "LogFile := new(FileOut) destination(LogFileName) open;" << std::endl;
    out << "LogFile writeString(Result);" << std::endl;
    out << "LogFile close;" << std::endl;
    out << "return(self)." << std::endl << std::endl;

    // Generate Performance Monitor Classes

    out << "data class ConfidenceInterval" << std::endl;
    out << "extends Object" << std::endl;
    out << "instance variables" << std::endl;
    out << "ConfidenceLevel: Real, UpperBound: Real, LowerBound: Real" << std::endl << std::endl;
    out << "instance methods" << std::endl;
    out << "extendedTimes(x, y: Real) : Real" << std::endl << std::endl;
    out << "   if (x = nil) | (y = nil) then" << std::endl;
    out << "      return(nil)" << std::endl;
    out << "   else" << std::endl;
    out << "      return(x * y)" << std::endl;
    out << "   fi." << std::endl << std::endl;
    out << "extendedPlus(x, y: Real) : Real" << std::endl;
    out << "   if (x = nil) | (y = nil) then" << std::endl;
    out << "      return(nil)" << std::endl;
    out << "   else" << std::endl;
    out << "      return(x + y)" << std::endl;
    out << "   fi." << std::endl << std::endl;
    out << "*(CI: ConfidenceInterval) : ConfidenceInterval |Lower, Upper, Level: Real|" << std::endl
        << std::endl;
    out << "   if self extendedLowerGreaterEqualZero then" << std::endl;
    out << "      if CI extendedLowerGreaterEqualZero then" << std::endl;
    out << "         Lower := self extendedTimes(LowerBound, CI getLowerBound);" << std::endl;
    out << "         Upper := self extendedTimes(UpperBound, CI getUpperBound)" << std::endl;
    out << "      else" << std::endl;
    out << "         if CI extendedUpperLessZero then" << std::endl;
    out << "            Lower := self extendedTimes(UpperBound, CI getLowerBound);" << std::endl;
    out << "            Upper := self extendedTimes(LowerBound, CI getUpperBound)" << std::endl;
    out << "         else" << std::endl;
    out << "            Lower := self extendedTimes(UpperBound, CI getLowerBound);" << std::endl;
    out << "            Upper := self extendedTimes(UpperBound, CI getUpperBound)" << std::endl;
    out << "         fi" << std::endl;
    out << "      fi" << std::endl;
    out << "   else" << std::endl;
    out << "      if self extendedUpperLessZero then" << std::endl;
    out << "         if CI extendedLowerGreaterEqualZero then" << std::endl;
    out << "            Lower := self extendedTimes(LowerBound, CI getUpperBound);" << std::endl;
    out << "            Upper := self extendedTimes(UpperBound, CI getLowerBound)" << std::endl;
    out << "         else" << std::endl;
    out << "            if CI extendedUpperLessZero then" << std::endl;
    out << "               Lower := self extendedTimes(UpperBound, CI getUpperBound);" << std::endl;
    out << "               Upper := self extendedTimes(LowerBound, CI getLowerBound)" << std::endl;
    out << "            else" << std::endl;
    out << "               Lower := self extendedTimes(LowerBound, CI getUpperBound);" << std::endl;
    out << "               Upper := self extendedTimes(LowerBound, CI getLowerBound)" << std::endl;
    out << "            fi" << std::endl;
    out << "         fi" << std::endl;
    out << "      else" << std::endl;
    out << "         if CI extendedLowerGreaterEqualZero then" << std::endl;
    out << "            Lower := self extendedTimes(LowerBound, CI getUpperBound);" << std::endl;
    out << "            Upper := self extendedTimes(UpperBound, CI getUpperBound)" << std::endl;
    out << "         else" << std::endl;
    out << "            if CI extendedUpperLessZero then" << std::endl;
    out << "               Lower := self extendedTimes(UpperBound, CI getLowerBound);" << std::endl;
    out << "               Upper := self extendedTimes(LowerBound, CI getLowerBound)" << std::endl;
    out << "            else" << std::endl;
    out << "               Lower := self extendedMin(self extendedTimes(LowerBound, CI "
           "getUpperBound), self extendedTimes(UpperBound, CI getLowerBound));"
        << std::endl;
    out << "               Upper := self extendedMax(self extendedTimes(LowerBound, CI "
           "getLowerBound), self extendedTimes(UpperBound, CI getUpperBound))"
        << std::endl;
    out << "            fi" << std::endl;
    out << "         fi" << std::endl;
    out << "      fi" << std::endl;
    out << "   fi;" << std::endl;
    out << "   Level := ConfidenceLevel + CI getConfidenceLevel - 1;" << std::endl;
    out << "   return(new(ConfidenceInterval) withParameters(Lower, Upper, Level))." << std::endl
        << std::endl;
    out << "/(CI: ConfidenceInterval) : ConfidenceInterval" << std::endl << std::endl;
    out << "   return(self * CI reciprocal)." << std::endl << std::endl;
    out << "getUpperBound() : Real" << std::endl << std::endl;
    out << "   return(UpperBound)." << std::endl << std::endl;
    out << "extendedMin(x, y: Real) : Real" << std::endl << std::endl;
    out << "   if (x = nil) | (y = nil) then" << std::endl;
    out << "      return(nil)" << std::endl;
    out << "   else" << std::endl;
    out << "      if x < y then" << std::endl;
    out << "         return(x)" << std::endl;
    out << "      else" << std::endl;
    out << "         return(y)" << std::endl;
    out << "      fi" << std::endl;
    out << "   fi." << std::endl << std::endl;
    out << "sqr() : ConfidenceInterval |Lower, Upper: Real|" << std::endl << std::endl;
    out << "   if self extendedLowerGreaterEqualZero then" << std::endl;
    out << "      Lower := self extendedTimes(LowerBound, LowerBound);" << std::endl;
    out << "      Upper := self extendedTimes(UpperBound, UpperBound)" << std::endl;
    out << "   else" << std::endl;
    out << "      if self extendedUpperLessZero then" << std::endl;
    out << "         Lower := self extendedTimes(UpperBound, UpperBound);" << std::endl;
    out << "         Upper := self extendedTimes(LowerBound, LowerBound)" << std::endl;
    out << "      else" << std::endl;
    out << "         Lower := 0.0;" << std::endl;
    out << "         Upper := self extendedMax(self extendedTimes(LowerBound, LowerBound), self "
           "extendedTimes(UpperBound, UpperBound))"
        << std::endl;
    out << "      fi" << std::endl;
    out << "   fi;" << std::endl;
    out << "   return(new(ConfidenceInterval) withParameters(Lower, Upper, ConfidenceLevel))."
        << std::endl
        << std::endl;
    out << "+(CI: ConfidenceInterval) : ConfidenceInterval |Lower, Upper, Level: Real|" << std::endl
        << std::endl;
    out << "   Lower := self extendedPlus(LowerBound, CI getLowerBound);" << std::endl;
    out << "   Upper := self extendedPlus(UpperBound, CI getUpperBound);" << std::endl;
    out << "   Level := ConfidenceLevel + CI getConfidenceLevel - 1;" << std::endl;
    out << "   return(new(ConfidenceInterval) withParameters(Lower, Upper, Level))." << std::endl
        << std::endl;
    out << "extendedUpperGreaterZero() : Boolean" << std::endl << std::endl;
    out << "   if UpperBound = nil then" << std::endl;
    out << "      return(true)" << std::endl;
    out << "   else" << std::endl;
    out << "      return(UpperBound > 0.0)" << std::endl;
    out << "   fi." << std::endl << std::endl;
    out << "extendedLowerGreaterEqualZero() : Boolean" << std::endl << std::endl;
    out << "   if LowerBound = nil then" << std::endl;
    out << "      return(false)" << std::endl;
    out << "   else" << std::endl;
    out << "      return(LowerBound >= 0.0)" << std::endl;
    out << "   fi." << std::endl << std::endl;
    out << "extendedLowerLessZero() : Boolean" << std::endl << std::endl;
    out << "   if LowerBound = nil then" << std::endl;
    out << "      return(true)" << std::endl;
    out << "   else" << std::endl;
    out << "      return(LowerBound < 0.0)" << std::endl;
    out << "   fi." << std::endl << std::endl;
    out << "getRelativeError() : Real |Lower, Upper: Real|" << std::endl << std::endl;
    out << "   if (LowerBound = nil) | (UpperBound = nil) then" << std::endl;
    out << "      return(nil)" << std::endl;
    out << "   else" << std::endl;
    out << "      if LowerBound > 0.0 then" << std::endl;
    out << "         return((UpperBound - LowerBound) / (2 * LowerBound))" << std::endl;
    out << "      else" << std::endl;
    out << "         if UpperBound < 0.0 then" << std::endl;
    out << "            return((LowerBound - UpperBound) / (2 * UpperBound))" << std::endl;
    out << "         else" << std::endl;
    out << "            return(nil)" << std::endl;
    out << "         fi" << std::endl;
    out << "      fi" << std::endl;
    out << "   fi." << std::endl << std::endl;
    out << "extendedUpperLessZero() : Boolean" << std::endl << std::endl;
    out << "   if UpperBound = nil then" << std::endl;
    out << "      return(false)" << std::endl;
    out << "   else" << std::endl;
    out << "      return(UpperBound < 0.0)" << std::endl;
    out << "   fi." << std::endl << std::endl;
    out << "getLowerBound() : Real" << std::endl << std::endl;
    out << "   return(LowerBound)." << std::endl << std::endl;
    out << "reciprocal() : ConfidenceInterval |Lower, Upper: Real|" << std::endl << std::endl;
    out << "   if (self extendedLowerLessZero) & (self extendedUpperGreaterZero) then" << std::endl;
    out << "      Lower := nil;" << std::endl;
    out << "      Upper := nil" << std::endl;
    out << "   else" << std::endl;
    out << "      Lower := self extendedReciprocal(UpperBound);" << std::endl;
    out << "      Upper := self extendedReciprocal(LowerBound)" << std::endl;
    out << "   fi;" << std::endl;
    out << "   return(new(ConfidenceInterval) withParameters(Lower, Upper, ConfidenceLevel))."
        << std::endl
        << std::endl;
    out << "-(CI : ConfidenceInterval) : ConfidenceInterval" << std::endl << std::endl;
    out << "   return(self + CI negate)." << std::endl << std::endl;
    out << "extendedNegate(x: Real) : Real" << std::endl << std::endl;
    out << "   if x = nil then" << std::endl;
    out << "      return(nil)" << std::endl;
    out << "   else" << std::endl;
    out << "      return(-x)" << std::endl;
    out << "   fi." << std::endl << std::endl;
    out << "extendedReciprocal(x: Real) : Real" << std::endl << std::endl;
    out << "   if x = nil then" << std::endl;
    out << "      return(0.0)" << std::endl;
    out << "   else" << std::endl;
    out << "      if x = 0.0 then" << std::endl;
    out << "         return(nil)" << std::endl;
    out << "      else" << std::endl;
    out << "         return(1 / x)" << std::endl;
    out << "      fi" << std::endl;
    out << "   fi." << std::endl << std::endl;
    out << "accurate(Accuracy: Real) : Boolean |RelativeError: Real|" << std::endl << std::endl;
    out << "   RelativeError := self getRelativeError;" << std::endl;
    out << "   if RelativeError = nil then" << std::endl;
    out << "      return(false)" << std::endl;
    out << "   else" << std::endl;
    out << "      return(RelativeError <= 1 - Accuracy)" << std::endl;
    out << "   fi." << std::endl << std::endl;
    out << "getConfidenceLevel() : Real" << std::endl << std::endl;
    out << "   return(ConfidenceLevel)." << std::endl << std::endl;
    out << "negate() : ConfidenceInterval |Lower,Upper: Real|" << std::endl << std::endl;
    out << "   Lower := self extendedNegate(UpperBound);" << std::endl;
    out << "   Upper := self extendedNegate(LowerBound);" << std::endl;
    out << "   return(new(ConfidenceInterval) withParameters(Lower, Upper, ConfidenceLevel))."
        << std::endl
        << std::endl;
    out << "extendedMax(x, y: Real) : Real" << std::endl << std::endl;
    out << "   if (x = nil) | (y = nil) then" << std::endl;
    out << "      return(nil)" << std::endl;
    out << "   else" << std::endl;
    out << "      if x > y then" << std::endl;
    out << "         return(x)" << std::endl;
    out << "      else" << std::endl;
    out << "         return(y)" << std::endl;
    out << "      fi" << std::endl;
    out << "   fi." << std::endl << std::endl;
    out << "withParameters(Lower, Upper, Level: Real) : ConfidenceInterval" << std::endl
        << std::endl;
    out << "   LowerBound := Lower;" << std::endl;
    out << "   UpperBound := Upper;" << std::endl;
    out << "   ConfidenceLevel := Level;" << std::endl;
    out << "   return(self)." << std::endl << std::endl;
    out << "logHeading : String" << std::endl << std::endl;
    out << R"(   return("Point Estimation:" tab + "Confidence Interval:" tab tab + "Confidence Level:" tab + "Relative Error:").)"
        << std::endl
        << std::endl;
    out << "logStatistics : String |RelativeError: Real, LogOut: String|" << std::endl << std::endl;
    out << "   if (LowerBound = nil) | (UpperBound = nil) then" << std::endl;
    out << R"(      LogOut := "Not Specified" tab + "[")" << std::endl;
    out << "   else" << std::endl;
    out << "      LogOut := (0.5 * (LowerBound + UpperBound)) printString tab tab + \"[\""
        << std::endl;
    out << "   fi;" << std::endl;
    out << "   if LowerBound = nil then" << std::endl;
    out << "      LogOut := LogOut + \"-inf, \"" << std::endl;
    out << "   else" << std::endl;
    out << "      LogOut := LogOut + LowerBound printString + \", \"" << std::endl;
    out << "   fi;" << std::endl;
    out << "   if UpperBound = nil then" << std::endl;
    out << "      LogOut := LogOut + \"inf]\" tab" << std::endl;
    out << "   else" << std::endl;
    out << "      LogOut := LogOut + UpperBound printString + \"]\" tab" << std::endl;
    out << "   fi;" << std::endl;
    out << "   if (LowerBound = nil) & (UpperBound = nil) then" << std::endl;
    out << "      LogOut tab tab tab" << std::endl;
    out << "   fi;" << std::endl;
    out << "   LogOut := LogOut + ConfidenceLevel printString tab tab tab;" << std::endl;
    out << "   RelativeError := self getRelativeError;" << std::endl;
    out << "   if RelativeError == nil then" << std::endl;
    out << "      LogOut := LogOut + \"inf\" tab tab" << std::endl;
    out << "   else" << std::endl;
    out << "      LogOut := LogOut + RelativeError printString" << std::endl;
    out << "   fi;" << std::endl;
    out << "   return(LogOut)." << std::endl << std::endl;

    out << "data class PerformanceMonitor" << std::endl;
    out << "extends Object" << std::endl;
    out << "instance variables" << std::endl;
    out << "ConfidenceLevel: Real, Accuracy: Real, BatchSize: Integer, IntervalEstimation: "
           "ConfidenceInterval, LogFile: FileOut"
        << std::endl
        << std::endl;
    out << "instance methods" << std::endl;
    out << "setDefaultBatchSize() : PerformanceMonitor" << std::endl << std::endl;
    out << "   BatchSize := 1000;" << std::endl;
    out << "   return(self)." << std::endl << std::endl;
    out << "accurate() : Boolean" << std::endl << std::endl;
    out << "   return(IntervalEstimation accurate(Accuracy))." << std::endl << std::endl;
    out << "logStatistics() : String" << std::endl << std::endl;
    out << "   return(IntervalEstimation logHeading tab + \"Accurate:\" cr + IntervalEstimation "
           "logStatistics tab + IntervalEstimation accurate(Accuracy) printString)."
        << std::endl
        << std::endl;

    out << "data class LongRunSampleAverage" << std::endl;
    out << "extends PerformanceMonitor" << std::endl << std::endl;
    out << "instance variables" << std::endl;
    out << "AverageSumLengthProduct: Real, CurrentSum: Real, CurrentLength: Integer, "
           "NumberOfCycles: Integer, AverageLength: Real, AverageSquaredSum: Real, AverageSum: "
           "Real, TransientMode: Boolean, AverageSquaredLength: Real, Constant: Real"
        << std::endl
        << std::endl;
    out << "instance methods" << std::endl;
    out << "setBatchSize(m: Integer) : LongRunSampleAverage" << std::endl << std::endl;
    out << "   if m > 0 then" << std::endl;
    out << "      BatchSize := m" << std::endl;
    out << "   else" << std::endl;
    out << "      self error(\"BatchSize parameter for Long Run Sample Average must be larger than "
           "0\")"
        << std::endl;
    out << "   fi;" << std::endl;
    out << "   return(self)." << std::endl << std::endl;
    out << "rewardBM(Reward: Real) : LongRunSampleAverage" << std::endl << std::endl;
    out << "   self rewardRC(Reward, (CurrentLength = 0) | (CurrentLength = BatchSize));"
        << std::endl;
    out << "   return(self)." << std::endl << std::endl;
    out << "withConfidenceLevel(CL: Real) : LongRunSampleAverage" << std::endl << std::endl;
    out << "   if (CL < 0.0) | (CL >= 1.0) then" << std::endl;
    out << "      self error(\"ConfidenceLevel parameter for Long Run Sample Average must be "
           "within the interval [0.0, 1.0)\")"
        << std::endl;
    out << "   fi;" << std::endl;
    out << "   ConfidenceLevel := CL;" << std::endl;
    out << "   IntervalEstimation := new(ConfidenceInterval) withParameters(nil, nil, "
           "ConfidenceLevel);"
        << std::endl;
    out << "   Constant := (2.0 sqrt) * (self calculateInverseErfC(1.0 - ConfidenceLevel));"
        << std::endl;
    out << "   TransientMode := true;" << std::endl;
    out << "   NumberOfCycles := 0;" << std::endl;
    out << "   CurrentLength := 0;" << std::endl;
    out << "   AverageSum := 0.0;" << std::endl;
    out << "   AverageLength := 0.0;" << std::endl;
    out << "   AverageSquaredSum := 0.0;" << std::endl;
    out << "   AverageSquaredLength := 0.0;" << std::endl;
    out << "   AverageSumLengthProduct := 0.0;" << std::endl;
    out << "   self setDefaultBatchSize;" << std::endl;
    out << "   return(self)." << std::endl << std::endl;
    out << "getCurrentLength() : Integer" << std::endl << std::endl;
    out << "   return(CurrentLength)." << std::endl << std::endl;
    out << "getIntervalEstimation() : ConfidenceInterval" << std::endl << std::endl;
    out << "   return(IntervalEstimation)." << std::endl << std::endl;
    out << "rewardRC(Reward: Real, RecurrenceCondition: Boolean) : LongRunSampleAverage "
           "|PointEstimation, StandardDeviation, Variance, Factor, HalfWidth: Real|"
        << std::endl
        << std::endl;
    out << "   if RecurrenceCondition then" << std::endl;
    out << "      if TransientMode then" << std::endl;
    out << "         TransientMode := false" << std::endl;
    out << "      else" << std::endl;
    out << "         NumberOfCycles := NumberOfCycles + 1;" << std::endl;
    out << "         Factor := (NumberOfCycles - 1) / NumberOfCycles;" << std::endl;
    out << "         AverageSum := (Factor * AverageSum) + (CurrentSum / NumberOfCycles);"
        << std::endl;
    out << "         AverageLength := (Factor * AverageLength) + (CurrentLength / NumberOfCycles);"
        << std::endl;
    out << "         AverageSquaredSum := (Factor * AverageSquaredSum) + (CurrentSum sqr / "
           "NumberOfCycles);"
        << std::endl;
    out << "         AverageSquaredLength := (Factor * AverageSquaredLength) + (CurrentLength sqr "
           "/ NumberOfCycles);"
        << std::endl;
    out << "         AverageSumLengthProduct := (Factor * AverageSumLengthProduct) + ((CurrentSum "
           "* CurrentLength) / NumberOfCycles);"
        << std::endl;
    out << "         if NumberOfCycles > 1 then" << std::endl;
    out << "            PointEstimation := AverageSum / AverageLength;" << std::endl;
    out << "            Variance := ((1 / Factor) * (AverageSquaredSum - (2 * PointEstimation * "
           "AverageSumLengthProduct) + (PointEstimation sqr * AverageSquaredLength)));"
        << std::endl;
    out << "            if Variance > 0 then StandardDeviation := Variance sqrt else "
           "StandardDeviation := 0.0 fi;"
        << std::endl;
    out << "            HalfWidth := (Constant * StandardDeviation) / (AverageLength * "
           "(NumberOfCycles asReal sqrt));"
        << std::endl;
    out << "            IntervalEstimation := new(ConfidenceInterval) "
           "withParameters(PointEstimation - HalfWidth, PointEstimation + HalfWidth, "
           "ConfidenceLevel)"
        << std::endl;
    out << "         fi" << std::endl;
    out << "      fi;" << std::endl;
    out << "      CurrentSum := Reward;" << std::endl;
    out << "      CurrentLength := 1" << std::endl;
    out << "   else" << std::endl;
    out << "      if TransientMode not then" << std::endl;
    out << "         CurrentSum := CurrentSum + Reward;" << std::endl;
    out << "         CurrentLength := CurrentLength + 1" << std::endl;
    out << "      fi" << std::endl;
    out << "   fi;" << std::endl;
    out << "   return(self)." << std::endl << std::endl;
    out << "withParameters(A, CL: Real) : LongRunSampleAverage" << std::endl << std::endl;
    out << "   if (A <= 0.0) | (A >= 1.0) then" << std::endl;
    out << "      self error(\"Accuracy parameter for Long Run Sample Average must be within the "
           "interval (0.0, 1.0)\")"
        << std::endl;
    out << "   fi;" << std::endl;
    out << "   Accuracy := A;" << std::endl;
    out << "   self withConfidenceLevel(CL);" << std::endl;
    out << "   return(self)." << std::endl << std::endl;
    out << "calculateInverseErfC(y: Real) : Real |s, t, u, w, x, z: Real|" << std::endl
        << std::endl;
    out << "    z := y;" << std::endl;
    out << "    w := 0.916461398268964 - z ln;" << std::endl;
    out << "    u := w sqrt;" << std::endl;
    out << "    s := (u ln + 0.488826640273108) / w;" << std::endl;
    out << "    t := 1 / (u + 0.231729200323405);" << std::endl;
    out << "    x := u * (1 - s * (s * 0.124610454613712 + 0.5)) - " << std::endl;
    out << "        ((((-0.0728846765585675 * t + 0.269999308670029) * t + " << std::endl;
    out << "        0.150689047360223) * t + 0.116065025341614) * t + " << std::endl;
    out << "        0.499999303439796) * t;" << std::endl;
    out << "    t := 3.97886080735226 / (x + 3.97886080735226);" << std::endl;
    out << "    u := t - 0.5;" << std::endl;
    out << "    s := (((((((((0.00112648096188977922 * u + " << std::endl;
    out << "        0.000105739299623423047) * u - 0.00351287146129100025) * u - " << std::endl;
    out << "        0.000771708358954120939) * u + 0.00685649426074558612) * u + " << std::endl;
    out << "        0.00339721910367775861) * u - 0.011274916933250487) * u - " << std::endl;
    out << "        0.0118598117047771104) * u + 0.0142961988697898018) * u + " << std::endl;
    out << "        0.0346494207789099922) * u + 0.00220995927012179067;" << std::endl;
    out << "    s := ((((((((((((s * u - 0.0743424357241784861) * u - " << std::endl;
    out << "        0.105872177941595488) * u + 0.0147297938331485121) * u + " << std::endl;
    out << "        0.316847638520135944) * u + 0.713657635868730364) * u + " << std::endl;
    out << "        1.05375024970847138) * u + 1.21448730779995237) * u + " << std::endl;
    out << "        1.16374581931560831) * u + 0.956464974744799006) * u + " << std::endl;
    out << "        0.686265948274097816) * u + 0.434397492331430115) * u + " << std::endl;
    out << "        0.244044510593190935) * t - " << std::endl;
    out << "        z * (2.718281828459045 power(x * x - 0.120782237635245222));" << std::endl;
    out << "    x := x + s * (x * s + 1);" << std::endl;
    out << "    return(x)." << std::endl << std::endl;

    out << "data class LongRunSampleVariance" << std::endl;
    out << "extends PerformanceMonitor" << std::endl;
    out << "instance variables" << std::endl;
    out << "AverageReward: LongRunSampleAverage, AverageSquaredReward: LongRunSampleAverage"
        << std::endl
        << std::endl;
    out << "instance methods" << std::endl;
    out << "setBatchSize(m: Integer) : LongRunSampleVariance" << std::endl << std::endl;
    out << "   if m > 0 then" << std::endl;
    out << "      BatchSize := m;" << std::endl;
    out << "      AverageReward setBatchSize(BatchSize);" << std::endl;
    out << "      AverageSquaredReward setBatchSize(BatchSize)" << std::endl;
    out << "   else" << std::endl;
    out << "      self error(\"BatchSize parameter for Long Run Sample Variance must be larger "
           "than 0\")"
        << std::endl;
    out << "   fi;" << std::endl;
    out << "   return(self)." << std::endl << std::endl;
    out << "rewardBM(Reward: Real) : LongRunSampleVariance" << std::endl << std::endl;
    out << "   self rewardRC(Reward, (AverageReward getCurrentLength = 0) | (AverageReward "
           "getCurrentLength = BatchSize));"
        << std::endl;
    out << "   return(self)." << std::endl << std::endl;
    out << "rewardRC(Reward: Real, RecurrenceCondition: Boolean) : LongRunSampleVariance"
        << std::endl
        << std::endl;
    out << "   AverageReward rewardRC(Reward, RecurrenceCondition);" << std::endl;
    out << "   AverageSquaredReward rewardRC(Reward * Reward, RecurrenceCondition);" << std::endl;
    out << "   if RecurrenceCondition then" << std::endl;
    out << "      IntervalEstimation := (AverageSquaredReward getIntervalEstimation) - "
           "((AverageReward getIntervalEstimation) sqr)"
        << std::endl;
    out << "   fi;" << std::endl;
    out << "   return(self)." << std::endl << std::endl;
    out << "withParameters(A, CL: Real) : LongRunSampleVariance" << std::endl << std::endl;
    out << "   if (A <= 0.0) | (A >= 1.0) then" << std::endl;
    out << "      self error(\"Accuracy parameter for Long Run Sample Variance must be within the "
           "interval (0.0, 1.0)\")"
        << std::endl;
    out << "   fi;" << std::endl;
    out << "   Accuracy := A;" << std::endl;
    out << "   if (CL < 0.0) | (CL >= 1.0) then" << std::endl;
    out << "      self error(\"ConfidenceLevel parameter for Long Run Sample Variance must be "
           "within the interval [0.0, 1.0)\")"
        << std::endl;
    out << "   fi;" << std::endl;
    out << "   ConfidenceLevel := CL;" << std::endl;
    out << "   AverageReward := new(LongRunSampleAverage) withConfidenceLevel((ConfidenceLevel + "
           "1) / 2);"
        << std::endl;
    out << "   AverageSquaredReward := new(LongRunSampleAverage) "
           "withConfidenceLevel((ConfidenceLevel + 1) / 2);"
        << std::endl;
    out << "   IntervalEstimation := (AverageSquaredReward getIntervalEstimation) - "
           "((AverageReward getIntervalEstimation) sqr);"
        << std::endl;
    out << "   self setDefaultBatchSize();" << std::endl;
    out << "   return(self)." << std::endl << std::endl;

    out << "data class LongRunTimeAverage" << std::endl;
    out << "extends PerformanceMonitor" << std::endl;
    out << "instance variables" << std::endl;
    out << "PreviousReward: Real, AverageTime: LongRunSampleAverage, AverageRewardTimeProduct: "
           "LongRunSampleAverage, LastTime: Real"
        << std::endl
        << std::endl;
    out << "instance methods" << std::endl;
    out << "setBatchSize(m: Integer) : LongRunTimeAverage" << std::endl << std::endl;
    out << "   if m > 0 then" << std::endl;
    out << "      BatchSize := m;" << std::endl;
    out << "      AverageRewardTimeProduct setBatchSize(BatchSize);" << std::endl;
    out << "      AverageTime setBatchSize(BatchSize)" << std::endl;
    out << "   else" << std::endl;
    out << "      self error(\"BatchSize parameter for Long Run Time Average must be larger than "
           "0\")"
        << std::endl;
    out << "   fi;" << std::endl;
    out << "   return(self)." << std::endl << std::endl;
    out << "rewardBM(Reward, CurrentTime: Real) : LongRunTimeAverage" << std::endl << std::endl;
    out << "   self rewardRC(Reward, CurrentTime, (AverageRewardTimeProduct getCurrentLength = 0) "
           "| (AverageRewardTimeProduct getCurrentLength = BatchSize));"
        << std::endl;
    out << "   return(self)." << std::endl << std::endl;
    out << "withConfidenceLevel(CL: Real) : LongRunTimeAverage" << std::endl << std::endl;
    out << "   if (CL < 0.0) | (CL >= 1.0) then" << std::endl;
    out << "      self error(\"ConfidenceLevel parameter for Long Run Time Average must be within "
           "the interval [0.0, 1.0)\")"
        << std::endl;
    out << "   fi;" << std::endl;
    out << "   ConfidenceLevel := CL;" << std::endl;
    out << "   AverageRewardTimeProduct := new(LongRunSampleAverage) "
           "withConfidenceLevel((ConfidenceLevel + 1) / 2);"
        << std::endl;
    out << "   AverageTime := new(LongRunSampleAverage) withConfidenceLevel((ConfidenceLevel + 1) "
           "/ 2);"
        << std::endl;
    out << "   IntervalEstimation := (AverageRewardTimeProduct getIntervalEstimation) / "
           "(AverageTime getIntervalEstimation);"
        << std::endl;
    out << "   self setDefaultBatchSize();" << std::endl;
    out << "   return(self)." << std::endl << std::endl;
    out << "getCurrentLength() : Integer" << std::endl << std::endl;
    out << "   return(AverageRewardTimeProduct getCurrentLength)." << std::endl << std::endl;
    out << "getIntervalEstimation() : ConfidenceInterval" << std::endl << std::endl;
    out << "   return(IntervalEstimation)." << std::endl << std::endl;
    out << "rewardRC(Reward, CurrentTime: Real, RecurrenceCondition: Boolean) : LongRunTimeAverage"
        << std::endl
        << std::endl;
    out << "   if LastTime != nil then" << std::endl;
    out << "      AverageRewardTimeProduct rewardRC(PreviousReward * (CurrentTime - LastTime), "
           "RecurrenceCondition);"
        << std::endl;
    out << "      AverageTime rewardRC(CurrentTime - LastTime, RecurrenceCondition)" << std::endl;
    out << "   fi;" << std::endl;
    out << "   PreviousReward := Reward;" << std::endl;
    out << "   LastTime := CurrentTime;" << std::endl;
    out << "   if RecurrenceCondition then" << std::endl;
    out << "      IntervalEstimation := (AverageRewardTimeProduct getIntervalEstimation) / "
           "(AverageTime getIntervalEstimation)"
        << std::endl;
    out << "   fi;" << std::endl;
    out << "   return(self)." << std::endl << std::endl;
    out << "withParameters(A, CL: Real) : LongRunTimeAverage" << std::endl << std::endl;
    out << "   if (A <= 0.0) | (A >= 1.0) then" << std::endl;
    out << "      self error(\"Accuracy parameter for Long Run Time Average must be within the "
           "interval (0.0, 1.0)\")"
        << std::endl;
    out << "   fi;" << std::endl;
    out << "   Accuracy := A;" << std::endl;
    out << "   self withConfidenceLevel(CL);" << std::endl;
    out << "   return(self)." << std::endl << std::endl;

    out << "data class LongRunRateAverage" << std::endl;
    out << "extends PerformanceMonitor" << std::endl;
    out << "instance variables" << std::endl;
    out << "AverageReward: LongRunSampleAverage, AverageTime: LongRunSampleAverage, LastTime: Real"
        << std::endl
        << std::endl;
    out << "instance methods" << std::endl;
    out << "setBatchSize(m: Integer) : LongRunRateAverage" << std::endl << std::endl;
    out << "   if m > 0 then" << std::endl;
    out << "      BatchSize := m;" << std::endl;
    out << "      AverageReward setBatchSize(BatchSize);" << std::endl;
    out << "      AverageTime setBatchSize(BatchSize)" << std::endl;
    out << "   else" << std::endl;
    out << "      self error(\"BatchSize parameter for Long Run Rate Average must be larger than "
           "0\")"
        << std::endl;
    out << "   fi;" << std::endl;
    out << "   return(self)." << std::endl << std::endl;
    out << "rewardBM(Reward, CurrentTime: Real) : LongRunRateAverage" << std::endl << std::endl;
    out << "   self rewardRC(Reward, CurrentTime, (AverageReward getCurrentLength = 0) | "
           "(AverageReward getCurrentLength = BatchSize));"
        << std::endl;
    out << "   return(self)." << std::endl << std::endl;
    out << "withConfidenceLevel(CL: Real) : LongRunRateAverage" << std::endl << std::endl;
    out << "   if (CL < 0.0) | (CL >= 1.0) then" << std::endl;
    out << "      self error(\"ConfidenceLevel parameter for Long Run Rate Average must be within "
           "the interval [0.0, 1.0)\")"
        << std::endl;
    out << "   fi;" << std::endl;
    out << "   ConfidenceLevel := CL;" << std::endl;
    out << "   AverageReward := new(LongRunSampleAverage) withConfidenceLevel((ConfidenceLevel + "
           "1) / 2);"
        << std::endl;
    out << "   AverageTime := new(LongRunSampleAverage) withConfidenceLevel((ConfidenceLevel + 1) "
           "/ 2);"
        << std::endl;
    out << "   IntervalEstimation := (AverageReward getIntervalEstimation) / (AverageTime "
           "getIntervalEstimation);"
        << std::endl;
    out << "   self setDefaultBatchSize();" << std::endl;
    out << "   return(self)." << std::endl << std::endl;
    out << "getCurrentLength() : Integer" << std::endl << std::endl;
    out << "   return(AverageReward getCurrentLength)." << std::endl << std::endl;
    out << "getIntervalEstimation() : ConfidenceInterval" << std::endl << std::endl;
    out << "   return(IntervalEstimation)." << std::endl << std::endl;
    out << "rewardRC(Reward, CurrentTime: Real, RecurrenceCondition: Boolean) : LongRunRateAverage"
        << std::endl
        << std::endl;
    out << "   if LastTime != nil then" << std::endl;
    out << "      AverageReward rewardRC(Reward, RecurrenceCondition);" << std::endl;
    out << "      AverageTime rewardRC(CurrentTime - LastTime, RecurrenceCondition)" << std::endl;
    out << "   fi;" << std::endl;
    out << "   LastTime := CurrentTime;" << std::endl;
    out << "   if RecurrenceCondition then" << std::endl;
    out << "      IntervalEstimation := (AverageReward getIntervalEstimation) / (AverageTime "
           "getIntervalEstimation)"
        << std::endl;
    out << "   fi;" << std::endl;
    out << "   return(self)." << std::endl << std::endl;
    out << "withParameters(A, CL: Real) : LongRunRateAverage" << std::endl << std::endl;
    out << "   if (A <= 0.0) | (A >= 1.0) then" << std::endl;
    out << "      self error(\"Accuracy parameter for Long Run Rate Average must be within the "
           "interval (0.0, 1.0)\")"
        << std::endl;
    out << "   fi;" << std::endl;
    out << "   Accuracy := A;" << std::endl;
    out << "   self withConfidenceLevel(CL);" << std::endl;
    out << "   return(self)." << std::endl << std::endl;

    out << "data class LongRunTimeVariance" << std::endl;
    out << "extends PerformanceMonitor" << std::endl;
    out << "instance variables" << std::endl;
    out << "AverageReward: LongRunTimeAverage, AverageSquaredReward: LongRunTimeAverage"
        << std::endl
        << std::endl;
    out << "instance methods" << std::endl;
    out << "setBatchSize(m: Integer) : LongRunTimeVariance" << std::endl << std::endl;
    out << "   if m > 0 then" << std::endl;
    out << "      BatchSize := m;" << std::endl;
    out << "      AverageReward setBatchSize(BatchSize);" << std::endl;
    out << "      AverageSquaredReward setBatchSize(BatchSize)" << std::endl;
    out << "   else" << std::endl;
    out << "      self error(\"BatchSize parameter for Long Run Time Variance must be larger than "
           "0\")"
        << std::endl;
    out << "   fi;" << std::endl;
    out << "   return(self)." << std::endl << std::endl;
    out << "rewardBM(Reward, CurrentTime: Real) : LongRunTimeVariance" << std::endl << std::endl;
    out << "   self rewardRC(Reward, CurrentTime, (AverageReward getCurrentLength = 0) | "
           "(AverageReward getCurrentLength = BatchSize));"
        << std::endl;
    out << "   return(self)." << std::endl << std::endl;
    out << "rewardRC(Reward, CurrentTime: Real, RecurrenceCondition: Boolean) : LongRunTimeVariance"
        << std::endl
        << std::endl;
    out << "   AverageReward rewardRC(Reward, CurrentTime, RecurrenceCondition);" << std::endl;
    out << "   AverageSquaredReward rewardRC(Reward * Reward, CurrentTime, RecurrenceCondition);"
        << std::endl;
    out << "   if RecurrenceCondition then" << std::endl;
    out << "      IntervalEstimation := (AverageSquaredReward getIntervalEstimation) - "
           "((AverageReward getIntervalEstimation) sqr)"
        << std::endl;
    out << "   fi;" << std::endl;
    out << "   return(self)." << std::endl << std::endl;
    out << "withParameters(A, CL: Real) : LongRunTimeVariance" << std::endl << std::endl;
    out << "   if (A <= 0.0) | (A >= 1.0) then" << std::endl;
    out << "      self error(\"Accuracy parameter for Long Run Sample Variance must within the "
           "interval (0.0, 1.0)\")"
        << std::endl;
    out << "   fi;" << std::endl;
    out << "   Accuracy := A;" << std::endl;
    out << "   if (CL < 0.0) | (CL >= 1.0) then" << std::endl;
    out << "      self error(\"ConfidenceLevel parameter for Long Run Sample Variance must be "
           "within the interval [0.0, 1.0)\")"
        << std::endl;
    out << "   fi;" << std::endl;
    out << "   ConfidenceLevel := CL;" << std::endl;
    out << "   AverageReward := new(LongRunTimeAverage) withConfidenceLevel((ConfidenceLevel + 1) "
           "/ 2);"
        << std::endl;
    out << "   AverageSquaredReward := new(LongRunTimeAverage) "
           "withConfidenceLevel((ConfidenceLevel + 1) / 2);"
        << std::endl;
    out << "   IntervalEstimation := (AverageSquaredReward getIntervalEstimation) - "
           "((AverageReward getIntervalEstimation) sqr);"
        << std::endl;
    out << "   self setDefaultBatchSize();" << std::endl;
    out << "   return(self)." << std::endl << std::endl;

    // Generate Data Class Distribution, Uniform and GeneralDiscrete

    out << "data class Distribution" << std::endl;
    out << "extends Object" << std::endl;
    out << "instance variables" << std::endl;
    out << "Random: RandomGenerator" << std::endl << std::endl;
    out << "instance methods" << std::endl;
    out << "initialise : Distribution" << std::endl << std::endl;
    out << "  Random := new(RandomGenerator) randomiseSeed;" << std::endl;
    out << "  return(self)." << std::endl << std::endl;

    out << "data class Uniform" << std::endl;
    out << "extends Distribution" << std::endl;
    out << "instance variables" << std::endl;
    out << "IntervalLength: Real, LowerBound: Real" << std::endl << std::endl;
    out << "instance methods" << std::endl;
    out << "withParameters(LB, UB: Real) : Uniform" << std::endl << std::endl;
    out << "   self initialise;" << std::endl;
    out << "   LowerBound := LB; IntervalLength := UB - LB;" << std::endl;
    out << "   return(self)." << std::endl << std::endl;
    out << "sample : Real" << std::endl << std::endl;
    out << "   return(LowerBound + (Random random * IntervalLength))." << std::endl << std::endl;

    out << "data class GeneralDiscrete" << std::endl;
    out << "extends Distribution" << std::endl;
    out << "instance variables" << std::endl;
    out << "UpperBounds: Array, LowerBounds: Array, Weights: Array, SampleValues: Array"
        << std::endl
        << std::endl;
    out << "instance methods" << std::endl;
    out << "withParameters(Value: Object, Weight: Real) : GeneralDiscrete |Index, Size: Integer, "
           "CumulativeWeight, Total: Real|"
        << std::endl
        << std::endl;
    out << "if Random = nil then" << std::endl;
    out << "    self initialise;" << std::endl;
    out << "    SampleValues := new(Array);" << std::endl;
    out << "    Weights := new(Array);" << std::endl;
    out << "    LowerBounds := new(Array);" << std::endl;
    out << "    UpperBounds := new(Array)" << std::endl;
    out << "fi;" << std::endl;
    if (p4r) {
        out << "Size := SampleValues getSize + 1;" << std::endl;
        out << "SampleValues size(Size);" << std::endl;
        out << "SampleValues put(Size, Value);" << std::endl;
        out << "Weights size(Size);" << std::endl;
        out << "Weights put(Size, Weight);" << std::endl;
        out << "LowerBounds size(Size);" << std::endl;
        out << "UpperBounds size(Size);" << std::endl;
    } else {
        out << "Size := SampleValues size + 1;" << std::endl;
        out << "SampleValues resize(Size);" << std::endl;
        out << "SampleValues put(Size, Value);" << std::endl;
        out << "Weights resize(Size);" << std::endl;
        out << "Weights put(Size, Weight);" << std::endl;
        out << "LowerBounds resize(Size);" << std::endl;
        out << "UpperBounds resize(Size);" << std::endl;
    }
    out << "Index := 1; Total := 0.0;" << std::endl;
    out << "while Index <= Size do" << std::endl;
    if (p4r) {
        out << "   Total := Total + Weights get(Index);" << std::endl;
    } else {
        out << "   Total := Total + Weights at(Index);" << std::endl;
    }
    out << "   Index := Index + 1" << std::endl;
    out << "od;" << std::endl;
    out << "Index := 1; CumulativeWeight := 0.0;" << std::endl;
    out << "while Index <= Size do" << std::endl;
    out << "    LowerBounds put(Index, CumulativeWeight/Total);" << std::endl;
    if (p4r) {
        out << "    CumulativeWeight := CumulativeWeight + Weights get(Index);" << std::endl;
    } else {
        out << "    CumulativeWeight := CumulativeWeight + Weights at(Index);" << std::endl;
    }
    out << "    UpperBounds put(Index, CumulativeWeight/Total);" << std::endl;
    out << "    Index := Index + 1" << std::endl;
    out << "od;" << std::endl;
    out << "return self." << std::endl << std::endl;
    out << "sample() : Object |Sample: Real, Result : Object, Index: Integer|" << std::endl
        << std::endl;
    out << "Sample := Random random;" << std::endl;
    out << "Index := 1;" << std::endl;
    out << "while Result = nil do" << std::endl;
    if (p4r) {
        out << "   if (LowerBounds get(Index) < Sample) & (Sample <= UpperBounds get(Index)) then "
               "Result := SampleValues get(Index) else Index := Index + 1 fi"
            << std::endl;
    } else {
        out << "   if (LowerBounds at(Index) < Sample) & (Sample <= UpperBounds at(Index)) then "
               "Result := SampleValues at(Index) else Index := Index + 1 fi"
            << std::endl;
    }
    out << "od;" << std::endl;
    out << "return Result." << std::endl << std::endl;

    // Generate Data Classes for Queues

    out << "data class Queue" << std::endl;
    out << "extends Object" << std::endl;
    out << "instance variables" << std::endl;
    out << "PrimairElement: QueueElement, Occupation: Integer" << std::endl << std::endl;
    out << "instance methods" << std::endl;
    out << "init : Queue" << std::endl << std::endl;
    out << "   PrimairElement := new(QueueElement);" << std::endl;
    out << "   PrimairElement setNext(PrimairElement) setPrevious(PrimairElement);" << std::endl;
    out << "   Occupation := 0;" << std::endl;
    out << "   return(self)." << std::endl << std::endl;
    out << "put(NewElement: Object) : Object |NQE: QueueElement|" << std::endl << std::endl;
    out << "   NQE := new(QueueElement) setElement(NewElement);" << std::endl;
    out << "   NQE setNext(PrimairElement getNext) setPrevious(PrimairElement);" << std::endl;
    out << "   PrimairElement setNext(NQE);" << std::endl;
    out << "   NQE getNext setPrevious(NQE);" << std::endl;
    out << "   Occupation := Occupation + 1;" << std::endl;
    out << "   return(self)." << std::endl << std::endl;
    out << "inspect : Object |QE: QueueElement|" << std::endl << std::endl;
    out << "   QE := PrimairElement getPrevious;" << std::endl;
    out << "   return(QE getElement)." << std::endl << std::endl;
    out << "remove : Object |QE: QueueElement|" << std::endl << std::endl;
    out << "   QE := PrimairElement getPrevious;" << std::endl;
    out << "   QE getNext setPrevious(QE getPrevious);" << std::endl;
    out << "   QE getPrevious setNext(QE getNext);" << std::endl;
    out << "   Occupation := Occupation - 1;" << std::endl;
    out << "   return(QE getElement)." << std::endl << std::endl;
    out << "isNotEmpty : Boolean" << std::endl << std::endl;
    out << "   return(Occupation != 0)." << std::endl << std::endl;

    out << "data class QueueElement" << std::endl;
    out << "extends Object" << std::endl;
    out << "instance variables" << std::endl;
    out << "Next: QueueElement, Element: Object, Previous: QueueElement" << std::endl << std::endl;
    out << "instance methods" << std::endl;
    out << "setNext(QE: QueueElement): QueueElement" << std::endl << std::endl;
    out << "   Next := QE;" << std::endl;
    out << "   return(self)." << std::endl << std::endl;
    out << "getPrevious: QueueElement" << std::endl << std::endl;
    out << "   return(Previous)." << std::endl << std::endl;
    out << "setElement(E: Object): QueueElement" << std::endl << std::endl;
    out << "   Element := E;" << std::endl;
    out << "   return(self)." << std::endl << std::endl;
    out << "getElement: Object" << std::endl << std::endl;
    out << "   return(Element)." << std::endl << std::endl;
    out << "getNext: QueueElement" << std::endl << std::endl;
    out << "   return(Next)." << std::endl << std::endl;
    out << "setPrevious(QE: QueueElement): QueueElement" << std::endl << std::endl;
    out << "   Previous := QE;" << std::endl;
    out << "   return(self)." << std::endl << std::endl;

    // Generate Primitive Data Class Definitions

    if (p4r) {
        out << "data class Object" << std::endl;
        out << "instance variables" << std::endl;
        out << "instance methods" << std::endl;
        out << "error(ErrorMessage: String): Object primitive." << std::endl;
        out << "deepCopy: Object primitive." << std::endl;
        out << "=  (Operand2: Object): Boolean primitive." << std::endl;
        out << "!= (Operand2: Object): Boolean primitive." << std::endl;
        out << "== (Operand2: Object): Boolean primitive." << std::endl;
        out << "!==(Operand2: Object): Boolean primitive." << std::endl;
        out << "printString: String primitive." << std::endl;
        out << "shallowCopy: Object primitive." << std::endl << std::endl;
    } else {
        out << "native data class Object" << std::endl;
        out << "instance variables" << std::endl;
        out << "instance methods" << std::endl;
        out << "native error(s: String): Object." << std::endl;
        out << "native deepCopy: Object." << std::endl;
        out << "native =  (o: Object): Boolean." << std::endl;
        out << "native != (o: Object): Boolean." << std::endl;
        out << "native == (o: Object): Boolean." << std::endl;
        out << "native !==(o: Object): Boolean." << std::endl;
        out << "native marshal: String." << std::endl;
        out << "native printString: String." << std::endl;
        out << "native shallowCopy: Object." << std::endl << std::endl;
    }

    if (p4r) {
        out << "data class Number" << std::endl;
        out << "extends Object" << std::endl;
        out << "instance variables" << std::endl;
        out << "instance methods" << std::endl << std::endl;
    } else {
        out << "native data class Nil" << std::endl;
        out << "extends Object" << std::endl;
        out << "instance variables" << std::endl;
        out << "instance methods" << std::endl << std::endl;
    }

    if (p4r) {
        out << "data class Integer" << std::endl;
        out << "extends Number" << std::endl;
        out << "instance variables" << std::endl;
        out << "instance methods" << std::endl;
        out << "+  (Operand2: Number): Number primitive." << std::endl;
        out << "/  (Operand2: Number): Number primitive." << std::endl;
        out << "*  (Operand2: Number): Number primitive." << std::endl;
        out << "-  (Operand2: Number): Number primitive." << std::endl;
        out << "<  (Operand2: Number): Boolean primitive." << std::endl;
        out << ">  (Operand2: Number): Boolean primitive." << std::endl;
        out << "<= (Operand2: Number): Boolean primitive." << std::endl;
        out << ">= (Operand2: Number): Boolean primitive." << std::endl;
        out << "unaryMinus: Integer primitive." << std::endl;
        out << "abs: Integer primitive." << std::endl;
        out << "&  (Operand2: Integer): Integer primitive." << std::endl;
        out << "|  (Operand2: Integer): Integer primitive." << std::endl;
        out << "not: Integer primitive." << std::endl;
        out << "sqrt: Number primitive." << std::endl;
        out << "power(Operand2: Number): Number primitive." << std::endl;
        out << "sqr: Integer primitive." << std::endl;
        out << "div(Operand2: Integer): Integer primitive." << std::endl;
        out << "modulo(Operand2: Number): Number primitive." << std::endl;
        out << "asInteger: Integer primitive." << std::endl;
        out << "asReal: Real primitive." << std::endl;
        out << "asString: String primitive." << std::endl;
        out << "printString: String primitive." << std::endl;
        out << "fac: Integer primitive." << std::endl;
        out << "monus(n: Integer): Integer if n > self then 0 else self - n fi." << std::endl
            << std::endl;
    } else {
        out << "native data class Integer" << std::endl;
        out << "extends Object" << std::endl;
        out << "instance variables" << std::endl;
        out << "instance methods" << std::endl;
        out << "native -: Integer." << std::endl;
        out << "native -(i: Integer): Integer." << std::endl;
        out << "native *(i: Integer): Integer." << std::endl;
        out << "native &(i: Integer): Integer." << std::endl;
        out << "native +(i: Integer): Integer." << std::endl;
        out << "native <(i: Integer): Boolean." << std::endl;
        out << "native <=(i: Integer): Boolean." << std::endl;
        out << "native >(i: Integer): Boolean." << std::endl;
        out << "native >=(i: Integer): Boolean." << std::endl;
        out << "native |(i: Integer): Integer." << std::endl;
        out << "native abs: Integer." << std::endl;
        out << "native asAsciiChar: Char." << std::endl;
        out << "native asReal: Real." << std::endl;
        out << "native div(i: Integer): Integer." << std::endl;
        out << "native fac: Integer." << std::endl;
        out << "native modulo(i: Integer): Integer." << std::endl;
        out << "native power(i: Integer): Integer." << std::endl;
        out << "native sqr: Integer." << std::endl;
        out << "native max(i: Integer): Integer." << std::endl;
        out << "native min(i: Integer): Integer." << std::endl;
        out << "native monus(i: Integer): Integer." << std::endl << std::endl;
    }

    if (p4r) {
        out << "data class Real" << std::endl;
        out << "extends Number" << std::endl;
        out << "instance variables" << std::endl;
        out << "instance methods" << std::endl;
        out << "+  (Operand2: Number): Real primitive." << std::endl;
        out << "/  (Operand2: Number): Real primitive." << std::endl;
        out << "*  (Operand2: Number): Real primitive." << std::endl;
        out << "-  (Operand2: Number): Real primitive." << std::endl;
        out << "<  (Operand2: Number): Boolean primitive." << std::endl;
        out << ">  (Operand2: Number): Boolean primitive." << std::endl;
        out << "<= (Operand2: Number): Boolean primitive." << std::endl;
        out << ">= (Operand2: Number): Boolean primitive." << std::endl;
        out << "unaryMinus: Real primitive." << std::endl;
        out << "acos: Real primitive." << std::endl;
        out << "asin: Real primitive." << std::endl;
        out << "atan: Real primitive." << std::endl;
        out << "atan2(Denom: Real): Real primitive." << std::endl;
        out << "cos: Real primitive." << std::endl;
        out << "sin: Real primitive." << std::endl;
        out << "tan: Real primitive." << std::endl;
        out << "abs: Real primitive." << std::endl;
        out << "log: Real primitive." << std::endl;
        out << "ln: Real primitive." << std::endl;
        out << "sqrt: Real primitive." << std::endl;
        out << "exp: Real primitive." << std::endl;
        out << "power(Operand2: Number): Real primitive." << std::endl;
        out << "sqr: Real primitive." << std::endl;
        out << "ceiling: Integer primitive." << std::endl;
        out << "floor: Integer primitive." << std::endl;
        out << "round: Integer primitive." << std::endl;
        out << "div(Operand2: Number): Integer primitive." << std::endl;
        out << "modulo(Operand2: Number): Real primitive." << std::endl;
        out << "asInteger: Integer primitive." << std::endl;
        out << "asReal: Real primitive." << std::endl;
        out << "asString: String primitive." << std::endl;
        out << "printString: String primitive." << std::endl << std::endl;
    } else {
        out << "native data class Real" << std::endl;
        out << "extends Object" << std::endl;
        out << "instance variables" << std::endl;
        out << "instance methods" << std::endl;
        out << "native -: Real." << std::endl;
        out << "native -(r: Real): Real." << std::endl;
        out << "native *(r: Real): Real." << std::endl;
        out << "native /(r: Real): Real." << std::endl;
        out << "native +(r: Real): Real." << std::endl;
        out << "native <(r:Real): Boolean." << std::endl;
        out << "native <=(r: Real): Boolean." << std::endl;
        out << "native >(r: Real): Boolean." << std::endl;
        out << "native >=(r: Real): Boolean." << std::endl;
        out << "native abs: Real." << std::endl;
        out << "native acos: Real." << std::endl;
        out << "native asin: Real." << std::endl;
        out << "native asInteger: Integer." << std::endl;
        out << "native atan: Real." << std::endl;
        out << "native atan2(r: Real): Real." << std::endl;
        out << "native ceiling: Real." << std::endl;
        out << "native cos: Real." << std::endl;
        out << "native exp: Real." << std::endl;
        out << "native floor: Real." << std::endl;
        out << "native ln: Real." << std::endl;
        out << "native log: Real." << std::endl;
        out << "native power(r: Real): Real." << std::endl;
        out << "native round: Real." << std::endl;
        out << "native sin: Real." << std::endl;
        out << "native sqr: Real." << std::endl;
        out << "native sqrt: Real." << std::endl;
        out << "native tan: Real." << std::endl;
        out << "native monus(r: Real): Real." << std::endl;
        out << "native max(r: Real): Real." << std::endl;
        out << "native min(r: Real): Real." << std::endl << std::endl;
    }

    if (p4r) {
        out << "primitive data class Boolean" << std::endl;
        out << "extends Object" << std::endl;
        out << "instance variables" << std::endl;
        out << "instance methods" << std::endl;
        out << "&  (Operand2: Boolean): Boolean primitive." << std::endl;
        out << "|  (Operand2: Boolean): Boolean primitive." << std::endl;
        out << "not: Boolean primitive." << std::endl;
        out << "printString: String primitive." << std::endl << std::endl;
    } else {
        out << "native data class Boolean" << std::endl;
        out << "extends Object" << std::endl;
        out << "instance variables" << std::endl;
        out << "instance methods" << std::endl;
        out << "native &(b: Boolean): Boolean." << std::endl;
        out << "native |(b: Boolean): Boolean." << std::endl;
        out << "native not : Boolean." << std::endl;
        out << "native xor(b: Boolean): Boolean." << std::endl << std::endl;
    }

    if (p4r) {
        out << "primitive data class Char" << std::endl;
        out << "extends Object" << std::endl;
        out << "instance variables" << std::endl;
        out << "instance methods" << std::endl;
        out << "printString: String primitive." << std::endl << std::endl;
    } else {
        out << "native data class Char" << std::endl;
        out << "extends Object" << std::endl;
        out << "instance variables" << std::endl;
        out << "instance methods" << std::endl;
        out << "native asciiIndex: Integer." << std::endl;
        out << "native asString: String." << std::endl << std::endl;
    }

    if (p4r) {
        out << "primitive data class Array" << std::endl;
        out << "extends Object" << std::endl;
        out << "instance variables" << std::endl;
        out << "instance methods" << std::endl;
        out << "get(FromPosition: Integer): Object primitive." << std::endl;
        out << "getSize: Integer primitive." << std::endl;
        out << "size(newSize: Integer): Array primitive." << std::endl;
        out << "put(AtPosition: Integer, Element: Object): Array primitive." << std::endl;
        out << "putAll(Element: Object): Array primitive." << std::endl;
        out << "printString: String | i, s: Integer, Str: String |" << std::endl;
        out << "   Str := \"Array (\"; i := 0; s := self getSize;" << std::endl;
        out << "   while i < s do Str append(self get(i := i + 1) printString) append(if i < s "
               "then \",\" else \")\" fi) od;"
            << std::endl;
        out << "   Str." << std::endl << std::endl;
    } else {
        out << "native data class Array" << std::endl;
        out << "extends Object" << std::endl;
        out << "instance variables" << std::endl;
        out << "instance methods" << std::endl;
        out << "native putAll(o: Object): Array." << std::endl;
        out << "/*native =(o: Object): Boolean.*/" << std::endl;
        out << "/*native deepCopy: Object.*/ " << std::endl;
        out << "/*native printString: String.*/" << std::endl;
        out << "/*native shallowCopy: Object.*/" << std::endl;
        out << "native at(i: Integer): Object." << std::endl;
        out << "native size: Integer." << std::endl;
        out << "native putAt(i: Integer, o: Object): Array." << std::endl;
        out << "native resize(i: Integer): Array." << std::endl;
        out << "native +(a: Array): Array." << std::endl;
        out << "native concat(a: Array): Array." << std::endl;
        out << "native find(i: Integer, o: Object): Integer." << std::endl;
        out << "native subArray(i, l: Integer): Array." << std::endl << std::endl;
    }

    if (p4r) {
        out << "primitive data class String" << std::endl;
        out << "extends Object" << std::endl;
        out << "instance variables" << std::endl;
        out << "instance methods" << std::endl;
        out << "+  (aString: String): String primitive." << std::endl;
        out << "<  (Operand2: String): Boolean primitive." << std::endl;
        out << ">  (Operand2: String): Boolean primitive." << std::endl;
        out << "<= (Operand2: String): Boolean primitive." << std::endl;
        out << ">= (Operand2: String): Boolean primitive." << std::endl;
        out << "append(aString: String): String primitive." << std::endl;
        out << "copy(From, To: Integer): String primitive." << std::endl;
        out << "concat(aString: String): String primitive." << std::endl;
        out << "cr: String primitive." << std::endl;
        out << "get(FromPosition: Integer): Char primitive." << std::endl;
        out << "length: Integer primitive." << std::endl;
        out << "printString: String primitive." << std::endl;
        out << "put(AtPosition: Integer, aChar: Char): String primitive." << std::endl;
        out << "tab: String primitive." << std::endl << std::endl;
    } else {
        out << "native data class String" << std::endl;
        out << "extends Object" << std::endl;
        out << "instance variables" << std::endl;
        out << "instance methods" << std::endl;
        out << "native +(s: String): String." << std::endl;
        out << "native concat(s: String): String." << std::endl;
        out << "native cr: String." << std::endl;
        out << "native tab: String." << std::endl;
        out << "/*native =(o: Object): Boolean.*/" << std::endl;
        out << "/*native deepCopy: Object.*/" << std::endl;
        out << "/*native shallowCopy: Object.*/" << std::endl;
        out << "native lf: String." << std::endl;
        out << "native find(i: Integer, s: String): Integer." << std::endl;
        out << "native at(i: Integer): Char." << std::endl;
        out << "native size: Integer." << std::endl;
        out << "native putAt(i: Integer, c: Char): String." << std::endl;
        out << "native subString(i, l: Integer): String." << std::endl;
        out << "native unmarshal: Object." << std::endl;
        out << "native splitOn(c: Char): Array." << std::endl;
        out << "native splitOnWhiteSpace: Array." << std::endl << std::endl;
    }

    if (p4r) {
        out << "primitive data class RandomGenerator" << std::endl;
        out << "extends Object" << std::endl;
        out << "instance variables" << std::endl;
        out << "instance methods" << std::endl;
        out << "random: Real primitive." << std::endl;
        out << "randomInt(n: Integer): Integer primitive." << std::endl;
        out << "randomiseSeed: RandomGenerator primitive." << std::endl;
        out << "setSeed(aSeed: Integer): RandomGenerator primitive." << std::endl << std::endl;
    } else {
        out << "native data class RandomGenerator" << std::endl;
        out << "extends Object" << std::endl;
        out << "instance variables" << std::endl;
        out << "instance methods" << std::endl;
        out << "native random: Real." << std::endl;
        out << "native randomInt(i: Integer): Integer." << std::endl;
        out << "native randomiseSeed: RandomGenerator." << std::endl;
        out << "native seed(i: Integer): RandomGenerator." << std::endl;
        out << "native setSeed(aSeed: Integer): RandomGenerator." << std::endl << std::endl;
    }

    if (p4r) {
        out << "primitive data class FileOut" << std::endl;
        out << "extends Object" << std::endl;
        out << "instance variables" << std::endl;
        out << "instance methods" << std::endl;
        out << "append: FileOut primitive." << std::endl;
        out << "destination(FileName: String): FileOut primitive." << std::endl;
        out << "close: Object primitive." << std::endl;
        out << "open: FileOut primitive." << std::endl;
        out << "writeString(aString: String): FileOut primitive." << std::endl;
        out << "write(anObject: Object): FileOut" << std::endl;
        out << "   self writeString(anObject printString append(\" \"))." << std::endl;
        out << "writeln(anObject: Object): FileOut" << std::endl;
        out << "   self writeString(anObject printString cr)." << std::endl << std::endl;
    } else {
        out << "native data class FileOut" << std::endl;
        out << "extends Object" << std::endl;
        out << "instance variables" << std::endl;
        out << "instance methods" << std::endl;
        out << "native destination(s: String): FileOut." << std::endl;
        out << "native open: FileOut." << std::endl;
        out << "native append: FileOut." << std::endl;
        out << "native close: FileOut." << std::endl;
        out << "native writeString(s: String): FileOut." << std::endl;
        out << "native write(s: String): FileOut." << std::endl;
        out << "native writeLine(s: String): FileOut." << std::endl;
        out << "native flush: FileOut." << std::endl << std::endl;
    }

    if (p4r) {
        out << "primitive data class FileIn" << std::endl;
        out << "extends Object" << std::endl;
        out << "instance variables" << std::endl;
        out << "instance methods" << std::endl;
        out << "open: FileIn primitive." << std::endl;
        out << "close: Object primitive." << std::endl;
        out << "readString: String primitive." << std::endl;
        out << "readInteger: Integer primitive." << std::endl;
        out << "readNumber: Object primitive." << std::endl;
        out << "readReal: Real primitive." << std::endl;
        out << "readBoolean: Boolean primitive." << std::endl;
        out << "readChar: Char primitive." << std::endl;
        out << "source(FileName: String): FileIn primitive." << std::endl << std::endl;
    }
}
