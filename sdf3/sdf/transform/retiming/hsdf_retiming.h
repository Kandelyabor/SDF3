/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   hsdf_retiming.h
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


#ifndef HSDFRETIMING_H_
#define HSDFRETIMING_H_

#include "sdf_retiming.h"

namespace SDF
{

class HSDFretiming: public SDFretiming
{
    public:
        explicit HSDFretiming(const TimedSDFgraph& hsdfg);
        ~HSDFretiming() override = default;

        // Prevent copying and moving
        HSDFretiming(const HSDFretiming&) = delete;
        HSDFretiming& operator=(const HSDFretiming&) = delete;
        HSDFretiming(HSDFretiming&&) = delete;
        HSDFretiming& operator=(HSDFretiming&&) = delete;


        //private:
        //compute Iteration Period of rg
        int computeIterationPeriod() override;
        //reVector is a feasible retiming if one exists
        bool feasibleRetiming(int dip) override;


};

}  // namespace SDF

#endif /*_ */
