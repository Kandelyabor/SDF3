/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sdf_retiming_neil.h
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


#ifndef SDFRETIMING_NEIL_H_
#define SDFRETIMING_NEIL_H_

#include "sdf_retiming.h"

/*
 * retiming according to O'Neil and Sha:
 *  T. O' Neil and E. H. M. Sha, "Retiming synchronous data-?ow graphs to reduce execution time,"
 * IEEE Trans. on Signal Processing, vol. 49, no. 10, pp. 2397--2407, 2001.
 */

namespace SDF {

class SDFretiming_Neil : public SDFretiming {
public:
    explicit SDFretiming_Neil(const TimedSDFgraph& sdfg);
    ~SDFretiming_Neil() override = default;

    // Prevent copying and moving
    SDFretiming_Neil(const SDFretiming_Neil&) = delete;
    SDFretiming_Neil& operator=(const SDFretiming_Neil&) = delete;
    SDFretiming_Neil(SDFretiming_Neil&&) = delete;
    SDFretiming_Neil& operator=(SDFretiming_Neil&&) = delete;


    // private:
    // compute Iteration Period of rg
    int computeIterationPeriod() override;
    // reVector is a feasible retiming if one exists
    bool feasibleRetiming(int dip) override;
    // compute retiming step for actor a
    int getRetimingStep(const TimedSDFactor *a) override;

    // compute max execution time of the longest zero
    // delay path to each actor in the HSDFG h
    //   MaxExTimeVector comHSDFIP(TimedSDFgraph *h);
    // Topologically sort actors in the hsdf graph
    // according to their data dependency
    //   SDFactors  hsdfTopSort(TimedSDFgraph *h);
};

std::unique_ptr<TimedSDFgraph> retime_Neil(const TimedSDFgraph& g, uint dip);

} // namespace SDF

#endif /* SDFRETIMING_NEIL_H_ */
