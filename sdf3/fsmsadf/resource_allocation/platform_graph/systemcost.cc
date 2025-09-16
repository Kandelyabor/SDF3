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

#include "systemcost.h"

namespace FSMSADF {

/**
 * SystemCost()
 * Constructor.
 */
SystemCost::SystemCost(const GraphComponent &c) :
    GraphComponent(c),
    loadScheduleTime(0),
    loadScheduleInstrMem(0),
    loadScheduleDataMem(0),
    switchSelectTimeConst(0),
    switchSelectTimePerUnit(0),
    switchSelectInstrMem(0),
    switchSelectDataMem(0) {}

/**
 * ~SystemCost()
 * Destructor.
 */
SystemCost::~SystemCost() = default;

/**
 * create()
 * The function returns a new systemCost object.
 */
std::unique_ptr<SystemCost> SystemCost::create(const GraphComponent& c) { return std::make_unique<SystemCost>(c); }

/**
 * createCopy()
 * The function returns a new systemCost object and copies all properties.
 */
std::unique_ptr<SystemCost> SystemCost::createCopy(const GraphComponent& c) const {
    std::unique_ptr<SystemCost> p = create(c);
    p->setLoadScheduleTime(loadScheduleTime);
    p->setLoadScheduleInstrMem(loadScheduleInstrMem);
    p->setLoadScheduleDataMem(loadScheduleDataMem);
    p->setSwitchSelectTimeConst(switchSelectTimeConst);
    p->setSwitchSelectTimePerUnit(switchSelectTimePerUnit);
    p->setSwitchSelectInstrMem(switchSelectInstrMem);
    p->setSwitchSelectDataMem(switchSelectDataMem);

    // Properties

    return p;
}

/**
 * clone()
 * The function returns a new systemCost object and copies all properties, but
 * when the parent of the graph component c and this object are different, all
 * pointers are relocated.
 */
std::unique_ptr<SystemCost> SystemCost::clone(const GraphComponent& c) const {
    std::unique_ptr<SystemCost> p = createCopy(c);

    return p;
}

/**
 * constructFromXML()
 * Load all properties of this systemCost from the supplied XML node.
 */
void SystemCost::constructFromXML(const CNodePtr systemCostNode) {

    // loadScheduleTime
    if (!CHasAttribute(systemCostNode, "loadScheduleTime")) {
        throw CException("Invalid graph, missing loadScheduleTime.");
}
    setLoadScheduleTime(static_cast<Time>(CGetAttribute(systemCostNode, "loadScheduleTime")));

    // loadScheduleInstrMem
    if (!CHasAttribute(systemCostNode, "loadScheduleInstrMem")) {
        throw CException("Invalid graph, missing loadScheduleInstrMem.");
}
    setLoadScheduleInstrMem(static_cast<Size>(CGetAttribute(systemCostNode, "loadScheduleInstrMem")));

    // loadScheduleDataMem
    if (!CHasAttribute(systemCostNode, "loadScheduleDataMem")) {
        throw CException("Invalid graph, missing loadScheduleDataMem.");
}
    setLoadScheduleDataMem(static_cast<Size>(CGetAttribute(systemCostNode, "loadScheduleDataMem")));

    // switchSelectTimeConst
    if (!CHasAttribute(systemCostNode, "switchSelectTimeConst")) {
        throw CException("Invalid graph, missing switchSelectTimeConst.");
}
    setSwitchSelectTimeConst(static_cast<Time>(CGetAttribute(systemCostNode, "switchSelectTimeConst")));

    // switchSelectTimePerUnit
    if (!CHasAttribute(systemCostNode, "switchSelectTimePerUnit")) {
        throw CException("Invalid graph, missing switchSelectTimePerUnit.");
}
    setSwitchSelectTimePerUnit(static_cast<Time>(CGetAttribute(systemCostNode, "switchSelectTimePerUnit")));

    // switchSelectInstrMem
    if (!CHasAttribute(systemCostNode, "switchSelectInstrMem")) {
        throw CException("Invalid graph, missing switchSelectInstrMem.");
}
    setSwitchSelectInstrMem(static_cast<Size>(CGetAttribute(systemCostNode, "switchSelectInstrMem")));

    // switchSelectDataMem
    if (!CHasAttribute(systemCostNode, "switchSelectDataMem")) {
        throw CException("Invalid graph, missing switchSelectDataMem.");
}
    setSwitchSelectDataMem(static_cast<Size>(CGetAttribute(systemCostNode, "switchSelectDataMem")));
}

/**
 * convertToXML()
 * Add all properties of this systemCost to the supplied XML node.
 */
void SystemCost::convertToXML(const CNodePtr systemCostNode) const {
    // loadScheduleTime
    CAddAttribute(systemCostNode, "loadScheduleTime", CString(getLoadScheduleTime()));

    // loadScheduleInstrMem
    CAddAttribute(systemCostNode, "loadScheduleInstrMem", CString(getLoadScheduleInstrMem()));

    // loadScheduleDataMem
    CAddAttribute(systemCostNode, "loadScheduleDataMem", CString(getLoadScheduleDataMem()));

    // switchSelectTimeConst
    CAddAttribute(systemCostNode, "switchSelectTimeConst", CString(getSwitchSelectTimeConst()));

    // switchSelectTimePerUnit
    CAddAttribute(systemCostNode, "switchSelectTimePerUnit", CString(getSwitchSelectTimePerUnit()));

    // switchSelectInstrMem
    CAddAttribute(systemCostNode, "switchSelectInstrMem", CString(getSwitchSelectInstrMem()));

    // switchSelectDataMem
    CAddAttribute(systemCostNode, "switchSelectDataMem", CString(getSwitchSelectDataMem()));
}

} // End namespace FSMSADF
