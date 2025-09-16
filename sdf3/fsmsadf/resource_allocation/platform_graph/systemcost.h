/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   systemcost.h
 *
 *  Author          :   Reinier van Kampenhout (j.r.v.kampenhout@tue.nl)
 *
 *  Date            :   July 1, 2015
 *
 *  Function        :   System cost.
 *
 *  History         :
 *      01-07-15    :   Initial version.
 *      10-03-17    :   Updated to cover POSE and COMIK
 *
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

#ifndef FSMSADF_RESOURCE_ALLOCATION_PLATFORM_GRAPH_SYSTEMCOST_H_INCLUDED
#define FSMSADF_RESOURCE_ALLOCATION_PLATFORM_GRAPH_SYSTEMCOST_H_INCLUDED

#include "fsmsadf/base/type.h"

namespace FSMSADF {

// Forward class definition
class SystemCost;

// List of SystemCost pointers
using SystemCosts = std::list<std::unique_ptr<SystemCost>>;

/**
 * SystemCost
 * Container for SystemCost.
 */
class SystemCost : public GraphComponent {
public:
    // Constructor
    explicit SystemCost(const GraphComponent &c);

    // Destructor
    ~SystemCost() override;

    SystemCost(const SystemCost&) = delete;
    SystemCost& operator=(const SystemCost&) = delete;
    SystemCost(SystemCost&&) = delete;
    SystemCost& operator=(SystemCost&&) = delete;

    // Construct
    [[nodiscard]] static std::unique_ptr<SystemCost> create(const GraphComponent &c);
    [[nodiscard]] std::unique_ptr<SystemCost> createCopy(const GraphComponent &c) const;
    [[nodiscard]] std::unique_ptr<SystemCost> clone(const GraphComponent &c) const;

    // Construct
    void constructFromXML(CNodePtr systemCost);

    // Convert
    void convertToXML(CNodePtr systemCostNode) const;

    [[nodiscard]] Time getLoadScheduleTime() const { return loadScheduleTime; };
    void setLoadScheduleTime(const Time sz) { loadScheduleTime = sz; };

    [[nodiscard]] Size getLoadScheduleInstrMem() const { return loadScheduleInstrMem; };
    void setLoadScheduleInstrMem(const Size size) { loadScheduleInstrMem = size; };

    [[nodiscard]] Size getLoadScheduleDataMem() const { return loadScheduleDataMem; };
    void setLoadScheduleDataMem(const Size size) { loadScheduleDataMem = size; };

    [[nodiscard]] Time getSwitchSelectTimeConst() const { return switchSelectTimeConst; };
    void setSwitchSelectTimeConst(const Time sz) { switchSelectTimeConst = sz; };

    [[nodiscard]] Time getSwitchSelectTimePerUnit() const { return switchSelectTimePerUnit; };
    void setSwitchSelectTimePerUnit(const Time sz) { switchSelectTimePerUnit = sz; };

    [[nodiscard]] int getSwitchSelectInstrMem() const {
        return static_cast<int>(switchSelectInstrMem);
    };
    void setSwitchSelectInstrMem(const Size size) { switchSelectInstrMem = size; };

    [[nodiscard]] int getSwitchSelectDataMem() const {
        return static_cast<int>(switchSelectDataMem);
    };
    void setSwitchSelectDataMem(const Size size) { switchSelectDataMem = size; };

private:
    // Time required for loading a schedule
    Time loadScheduleTime;

    // Instruction memory size required for the schedule loader
    Size loadScheduleInstrMem;

    // Data memory size required for the schedule loader
    Size loadScheduleDataMem;

    // Constant time required for the switchSelect actor
    Time switchSelectTimeConst;

    // Additional time that switchSelect actor takes for each unit
    Time switchSelectTimePerUnit;

    // Instruction Memory size required for the schedule loader
    Size switchSelectInstrMem;

    // Data Memory size required for the schedule loader
    Size switchSelectDataMem;
};

} // End namespace FSMSADF

#endif
