/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sdf_retiming_llw.h
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

#ifndef SDFRETIMING_LLW_H_
#define SDFRETIMING_LLW_H_

#include "sdf_retiming.h"
#include <memory>

/*
* retiming according to Liveris, Lin and Wang et al.:
* N. Liveris, C. Lin, J. Wang, H. Zhou, and P. Banerjee,
* "Retiming for synchronous data flow graphs," in Proc. of the 2007 Asia and South Pacific Design Automation Conf. IEEE, 2007, pp. 480--485
*/

namespace SDF
{

class SDFretiming_LLW: public SDFretiming
{
    public:
        explicit SDFretiming_LLW(const TimedSDFgraph& sdfg);
        ~SDFretiming_LLW() override = default;

        // Prevent copying and moving
        SDFretiming_LLW(const SDFretiming_LLW&) = delete;
        SDFretiming_LLW& operator=(const SDFretiming_LLW&) = delete;
        SDFretiming_LLW(SDFretiming_LLW&&) = delete;
        SDFretiming_LLW& operator=(SDFretiming_LLW&&) = delete;


        bool feasibleRetiming(int dip) override;
        //compute Iteration Period of rg
        int computeIterationPeriod() override;
        int get_t(TimedSDFactor *a, int k);
        void init_t();

        int optimalRetiming();
        void printOptReVector();

};

std::unique_ptr<TimedSDFgraph> retime_LLW(const TimedSDFgraph& g, uint dip);
} // namespace SDF

#endif
