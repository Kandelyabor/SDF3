/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sdf_retiming.h
 *
 *  Author          :   Xue-Yang Zhu
 *
 *  Date            :   2012
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


#ifndef SDF_OPTIMIZE_RETIMING_SDFRETIMING_H_INCLUDE
#define SDF_OPTIMIZE_RETIMING_SDFRETIMING_H_INCLUDE

#include "sdf/base/algo/repetition_vector.h"
#include "sdf/base/timed/graph.h"
#include <memory>

namespace SDF
{
/**
 * current Retiming vector
 */
using RetimingVector = std::vector<int>;
/**
 * current Delay distribution vector
 */
using DelayVector = std::vector<int>;
// Sequence of states in transition system
using DelayVectors = std::list<DelayVector>;
using DelayVectorsIter = DelayVectors::iterator;
/**
 * Max Execution time  vector
 */
using MaxExTimeVector = std::vector<int>;
/**
 *  end of zero-delay reachable path vector
 */
using EofZDVector = std::vector<bool>;


namespace RETIMING_GLOBALS {

// NOLINTBEGIN(*global-variables)

//-------------------------
extern std::vector<std::vector<int>> exTemp;
//-------------------------
extern RetimingVector optRe;

// NOLINTEND(*global-variables)

} // namespace RETIMING_GLOBALS


class SDFretiming
{
    public:
        explicit SDFretiming(const TimedSDFgraph& sdfg);
        virtual ~SDFretiming();

        // Prevent copying and moving
        SDFretiming(const SDFretiming&) = delete;
        SDFretiming& operator=(const SDFretiming&) = delete;
        SDFretiming(SDFretiming&&) = delete;
        SDFretiming& operator=(SDFretiming&&) = delete;

        [[nodiscard]] RetimingVector getRetimingVector() const
        {
            return reVector;
        };
        [[nodiscard]] MaxExTimeVector getMaxExTimeVector() const
        {
            return exTVector;
        };
        //  DelayVector getCurrentDelayVector(){return curDelayVector;};
        [[nodiscard]] uint getCurrIP() const
        {
            return newIP;
        };


        //retime the graph g ,return true if it is retimable with dip
        virtual bool isRetimable(int dip);

        virtual TimedSDFgraph* getRetimedSDFG();
        virtual std::unique_ptr<TimedSDFgraph> takeRetimedSDFG();

        virtual void printFeasibleRetimingVector();

        //protected:
        //initialize variables
        virtual void init();
        //compute Iteration Period of rg
        virtual int computeIterationPeriod();

        void getNextT(const TimedSDFactor *actor, int l);
        //reVector is a feasible retiming if one exists
        virtual bool feasibleRetiming(int dip);
        virtual int getRetimingStep(const TimedSDFactor * /*actor*/)
        {
            return 1;
        };
        void rePrev(const TimedSDFactor *a, int dip);

        //compute new delay distribution of rg according to current retiming vector
        bool legalRetimed();


        void printRetimingVector();
        void printMaxExTimeVector();
        void printDelayVector() const;


        // SDF graph. g is the original graph, rg used for transformation in the middle steps
        const TimedSDFgraph& g;
        std::unique_ptr<TimedSDFgraph> rg;

        //retiming vector
        RetimingVector reVector;
        //exTVector is the longest zero delay path execution time; tempT is a temporal variable
        MaxExTimeVector exTVector;
        //the max value of execution times of all actors in g
        int maxExTimeOfActor;
        //repetition vector of g
        RepetitionVector q;
        //after legalRetimed(),curDelayVector is the delay distribution of the retimed graph
        //  DelayVector curDelayVector;

        //sum of the factors of repetition vector
        uint sumQ;
        int newIP;
};

TimedSDFgraph *retime(TimedSDFgraph *g, uint dip);

RetimingVector retimingVector(TimedSDFgraph *g);

} // namespace SDF


#endif /* SDFRETIMING_H_ */
