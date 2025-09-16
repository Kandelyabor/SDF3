/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   deadlock.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   November 13, 2007
 *
 *  Function        :   State-space based deadlock analysis
 *
 *  History         :
 *      13-11-07    :   Initial version.
 *
 * $Id: deadlock.h,v 1.1 2008-03-06 10:49:44 sander Exp $
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

#ifndef SDF_ANALYSIS_STATESPACE_DEADLOCK_H_INCLUDED
#define SDF_ANALYSIS_STATESPACE_DEADLOCK_H_INCLUDED

#include "sdf/base/untimed/graph.h"
#include "sdf/basic_types.h"

namespace SDF {
/**
 * Deadlock analysis
 * Analyze whether an SDFG is deadlock free or not.
 */
class SDFstateSpaceDeadlockAnalysis {
public:
    // Constructor
    SDFstateSpaceDeadlockAnalysis() = default;

    // Destructor
    ~SDFstateSpaceDeadlockAnalysis() = default;

    // Delete copy/move constructors and assignment operators
    SDFstateSpaceDeadlockAnalysis(const SDFstateSpaceDeadlockAnalysis&) = delete;
    SDFstateSpaceDeadlockAnalysis& operator=(const SDFstateSpaceDeadlockAnalysis&) = delete;
    SDFstateSpaceDeadlockAnalysis(SDFstateSpaceDeadlockAnalysis&&) = delete;
    SDFstateSpaceDeadlockAnalysis& operator=(SDFstateSpaceDeadlockAnalysis&&) = delete;

    // Analyze absence of deadlock of the graph
    static bool isDeadlockFree(SDFgraph *g);

private:
    /***************************************************************************
     * Transition system
     **************************************************************************/
    class TransitionSystem {
    public:
        /***********************************************************************
         * State
         **********************************************************************/
        class State {
        public:
            // Constructor
            explicit State(const uint nrActors = 0, const uint nrChannels = 0) {
                init(nrActors, nrChannels);
            };

            // Destructor
            ~State() = default;

            // Default copy/move constructors and assignment operators
            State(const State&) = default;
            State& operator=(const State&) = default;
            State(State&&) = default;
            State& operator=(State&&) = default;

            // Initialize the state
            void init(const uint /*nrActors*/, const uint nrChannels) { ch.resize(nrChannels); };

            // Compare states
            bool operator==(const State &s);

            // Clear state
            void clear();

            // Output state
            void print(std::ostream &out);

            // State information
            std::vector<TBufSize> ch;
        };

        // Sequence of states in transition system
        using States = std::list<State>;
        using StatesIter = States::iterator;

        // Constructor
        explicit TransitionSystem(SDFgraph *gr) : g(gr){};

        // Destructor
        ~TransitionSystem() = default;

        // Delete copy/move constructors and assignment operators
        TransitionSystem(const TransitionSystem&) = delete;
        TransitionSystem& operator=(const TransitionSystem&) = delete;
        TransitionSystem(TransitionSystem&&) = delete;
        TransitionSystem& operator=(TransitionSystem&&) = delete;

        // Execute the SDFG
        bool execSDFgraph();

    private:
        // State transitions
        bool actorReadyToFire(const SDFactor& a);
        void fireActor(const SDFactor& a);

        // SDF graph and output actor
        SDFgraph *g;

        // Current state
        State initialState;
        State currentState;
    };
};
} // namespace SDF
#endif
