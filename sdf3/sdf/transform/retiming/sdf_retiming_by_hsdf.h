/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sdf_retiming_by_hsdf.h
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


#ifndef SDFretiming_byHSDF_H_
#define SDFretiming_byHSDF_H_

#include "sdf_retiming.h"
#include <memory>

namespace SDF
{

class SDFretiming_byHSDF: public SDFretiming
{
    public:
        explicit SDFretiming_byHSDF(const TimedSDFgraph& sdfg);
         ~SDFretiming_byHSDF() override;

        // Prevent copying and moving
        SDFretiming_byHSDF(const SDFretiming_byHSDF&) = delete;
        SDFretiming_byHSDF& operator=(const SDFretiming_byHSDF&) = delete;
        SDFretiming_byHSDF(SDFretiming_byHSDF&&) = delete;
        SDFretiming_byHSDF& operator=(SDFretiming_byHSDF&&) = delete;

        bool isRetimable(int dip) override;

        TimedSDFgraph* getRetimedSDFG() override;
        void printFeasibleRetimingVector() override;
    private:
        void computeRetimingVector();
        //reVector is a feasible retiming if one exists
        bool feasibleRetiming(int dip) override;


        std::unique_ptr<TimedSDFgraph> h;
        RetimingVector reVectorOfH;
};

} // namespace SDF

#endif /* SDFretiming_byHSDF_H_ */
