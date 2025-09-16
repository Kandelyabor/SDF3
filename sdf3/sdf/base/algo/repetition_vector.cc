/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   repetition_vector.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   July 26, 2007
 *
 *  Function        :   Repetition vector
 *
 *  History         :
 *      26-07-07    :   Initial version.
 *
 * $Id: repetition_vector.cc,v 1.2.2.1 2010-04-25 00:10:19 mgeilen Exp $
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

#include "repetition_vector.h"
namespace SDF {

namespace {


/**
 * calcFractionsConnectedActors ()
 * The function calculates and firing ration (as fractions) of
 * all actors connected to actor 'a' based on its firing rate. In case
 * of an inconsistent graph, all fractions are set to 0.
 */
void calcFractionsConnectedActors(CFractions &fractions, SDFactor *a) {
    CFraction fractionA = fractions[a->getId()];

    // Inconsistent graph?
    if (fractionA == CFraction(0, 0)) {
        return;
    }

    // Calculate the rate for each actor 'b' connected to actor 'a'
    for (const auto& pA : a->getPorts()) {
        // Get actor 'b' on other side of channel and its port 'pB'
        SDFchannel *c = pA->getChannel();
        //  SDFinputchannel* ic = pA->getInputChannel();
        //  SDFoutputchannel* oc = pA->getOutputChannel();

        if (c != nullptr) {
            SDFport *pB = c->getSrcPort();
            SDFactor *b = pB->getActor();

            if (a->getId() == b->getId()) {
                pB = c->getDstPort();
                b = pB->getActor();
            }

            // Calculate firing rate 'b'
            CFraction ratioAB;
            SDF::SDFrate pBr = pB->getRate();
            SDF::SDFrate pAr = pA->getRate();
            if (pBr == 0) {
                if (pAr == 0) {
                    // both rates are zero. The edge can be ignored
                } else {
                    // rate A is not zero, but B is. Graph cannot be consistent unless repetitions
                    // of A are zero
                    if (fractionA == CFraction(0, 1)) {
                        for (auto &fraction : fractions) {
                            fraction = CFraction(0, 0);
                        }
                    }
                    return;
                }
            } else {
                ratioAB = CFraction(pAr, pBr); // NOLINT(*-narrowing-conversions)

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
                    calcFractionsConnectedActors(fractions, b);

                    // Is graph inconsistent?
                    if (fractions[b->getId()] == CFraction(0, 0)) {
                        return;
                    }
                }
            }
        }
    }
}

} // namespace

/**
 * calcRepetitionVector ()
 * Convert the fractions to the smallest integers.
 */
RepetitionVector calcRepetitionVector(CFractions &fractions) {
    RepetitionVector repetitionVector(fractions.size(), 0);
    int64_t l = 1;

    // Find lowest common multiple (lcm) of all denominators
    for (auto &f : fractions) {
        l = lcm(l, f.denominator());
    }

    // Zero vector?
    if (l == 0) {
        return repetitionVector;
    }

    // Calculate non-zero repetition vector
    for (uint i = 0; i < fractions.size(); i++) {
        repetitionVector[i] =
                static_cast<int>((fractions[i].numerator() * l) / fractions[i].denominator());
    }

    // Find greatest common divisor (gcd)
    int g = repetitionVector[0];
    for (uint i = 1; i < repetitionVector.size(); i++) {
        g = gcd(g, repetitionVector[i]);
    }

    // Minimize the repetition vector using the gcd
    for (int &i : repetitionVector) {
        i = i / g;
    }

    return repetitionVector;
}

/**
 * computeRepetitionVector ()
 * The function computes the repetition vector of an SDFG.
 */
RepetitionVector computeRepetitionVector(const SDFgraph& g) {
    CFractions fractions(g.nrActors(), CFraction(-1, 1));

    // Calculate firing ratio (as fraction) for each actor

    for (const auto& a : g.getActors()) {
        CFraction &f = fractions[a->getId()];

        if (f == CFraction(-1, 1)) {

            f = CFraction(1, 1);
            calcFractionsConnectedActors(fractions, a.get());
        }
    }

    // Calculate repetition vector based on firing ratios
    return calcRepetitionVector(fractions);
}

/**
 * isSDFgraphConsistent ()
 * The function checks the consistency of the SDFG and returns true when
 * the graph is consistent, else it returns false.
 */
bool isSDFgraphConsistent(const SDFgraph& g) {
    RepetitionVector repVec = computeRepetitionVector(g);

    return repVec[0] != 0;
}
} // namespace SDF
