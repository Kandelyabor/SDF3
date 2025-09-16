/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sdf_retiming_sa_new.h
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


#ifndef SDFRETIMING_SANEW_H_
#define SDFRETIMING_SANEW_H_

#include "sdf_retiming.h"
#include <memory>

/*
 * Retiming according to the method described in the paper
 * X. Y. Zhu, T. Basten, M. Geilen and S. Stuijk, "Efficient Retiming of Multirate DSP Algorithms,"
 * in IEEE Transactions on Computer-Aided Design of Integrated Circuits and Systems, vol. 31, no. 6, pp. 831-844, June 2012.
 * doi: 10.1109/TCAD.2011.2182352
*/

namespace SDF
{

class SDFretiming_SAnew: public SDFretiming
{
    public:
        explicit SDFretiming_SAnew(const TimedSDFgraph& sdfg);
         ~SDFretiming_SAnew() override = default;

        // Prevent copying and moving
        SDFretiming_SAnew(const SDFretiming_SAnew&) = delete;
        SDFretiming_SAnew& operator=(const SDFretiming_SAnew&) = delete;
        SDFretiming_SAnew(SDFretiming_SAnew&&) = delete;
        SDFretiming_SAnew& operator=(SDFretiming_SAnew&&) = delete;

        void init() override;

        //private:
        //reVector is a feasible retiming if one exists
        bool feasibleRetiming(int dip) override;

        bool isRetimableSS(int dip);
        bool feasibleRetimingSS(int dip);

        int optimalRetiming_INS();
        int optimalRetiming_1();
        int optimalRetiming_BS();
        void printOptReVector();

        //compute Iteration Period of rg, dfs
        int computeIterationPeriod() override;
        void getNextT(const TimedSDFactor *actor, uint l);
        //compute Iteration Period of rg, bfs
        int computeIterationPeriodBFS();
        void findNextCriticalActor(const TimedSDFactor *actor, uint l);
        //a wrong implementation of bfs
        int computeIterationPeriodBFSerr();


        [[nodiscard]] DelayVector getCurrentDelayVector() const
        {
            return curDelayVector;
        };
        //compute new delay distribution of rg according to current retiming vector
        bool legalRetimed();

        //after legalRetimed(),curDelayVector is the delay distribution of the retimed graph
        DelayVector curDelayVector;


};

// find an optimal retiming for the graph (i.e., with minimal iteration period)
std::unique_ptr<TimedSDFgraph> retime_SAnew(const TimedSDFgraph& g);
// find a feasible retiming for the desired iteration period dip, if it exists
std::unique_ptr<TimedSDFgraph> retime_SAnew(const TimedSDFgraph& g, uint dip);

} // namespace SDF

#endif /* SDFRETIMING_SANEW_H_ */
