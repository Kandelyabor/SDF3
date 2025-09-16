/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   processor.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   May 12, 2009
 *
 *  Function        :   Tile processor.
 *
 *  History         :
 *      12-05-09    :   Initial version.
 *
 * $Id: processor.h,v 1.1 2009-12-23 13:37:22 sander Exp $
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

#ifndef FSMSADF_RESOURCE_ALLOCATION_PLATFORM_BINDING_PROCESSOR_H_INCLUDED
#define FSMSADF_RESOURCE_ALLOCATION_PLATFORM_BINDING_PROCESSOR_H_INCLUDED

#include "fsmsadf/resource_allocation/platform_graph/graph.h" // IWYU pragma: keep
#include "fsmsadf//base/graph.h" // IWYU pragma: keep
#include "fsmsadf/resource_allocation/scheduling/static_order/static_order_schedule.h"

namespace FSMSADF
{

    // Forward class definition
    class ProcessorBinding;
    class TileBinding;

    // List of processor binding pointers
    using ProcessorBindings = std::list<std::unique_ptr<ProcessorBinding>>;

    /**
     * ProcessorBinding
     * Container for processor binding.
     */
    class ProcessorBinding : public GraphComponent
    {
        public:

            // Constructor
            ProcessorBinding(const GraphComponent& c, Processor *p);

            // Destructor
            ~ProcessorBinding() override;

            // Delete copy and move constructors and assignment operators
            ProcessorBinding(const ProcessorBinding&) = delete;
            ProcessorBinding& operator=(const ProcessorBinding&) = delete;
            ProcessorBinding(ProcessorBinding&&) = delete;
            ProcessorBinding& operator=(ProcessorBinding&&) = delete;

            // Construct
            std::unique_ptr<ProcessorBinding> clone(const GraphComponent& c);

            // Construct
            void constructFromXML(const Scenario& ss,  CNodePtr processorNode);
            void constructResourceUsageFromXML( CNodePtr processorNode);

            // Convert
            void convertToXML(const Scenario& ss,  CNodePtr processorNode);
            void convertResourceUsageToXML( CNodePtr processorNode);

            // Processor
            [[nodiscard]] Processor *getProcessor() const
            {
                return processor;
            };

            // Actor bindings
            std::map<const Scenario *, ActorRefs> &getActorBindings()
            {
                return actorBindings;
            };
            void setActorBindings(const std::map<const Scenario *, ActorRefs> &a)
            {
                actorBindings = a;
            };
            bool addActorBinding(const Scenario& s, Actor *a);
            void removeActorBinding(const Scenario& s, const Actor *a);
            [[nodiscard]] bool hasActorBinding(const Scenario& s, const Actor *a) const;
            [[nodiscard]] bool hasActorBindingName(const Scenario& s, const CString& name) const;

            // Tile binding
            [[nodiscard]] TileBinding *getTileBinding() const;

            // Static order schedules
            std::map<ScenarioConstRef, StaticOrderSchedule> &getStaticOrderSchedules()
            {
                return staticOrderSchedules;
            };
            void setStaticOrderSchedules(const std::map<ScenarioConstRef, StaticOrderSchedule> &s)
            {
                staticOrderSchedules = s;
            };
            StaticOrderSchedule &getStaticOrderSchedule(const Scenario& s);
            void setStaticOrderSchedule(const Scenario& s, StaticOrderSchedule &so);
            void removeStaticOrderSchedule(const Scenario& s);
            [[nodiscard]] bool hasStaticOrderSchedule(const Scenario& s) const;

            // CompSOC-specific set of static-order schedules for each scenario
            [[nodiscard]] const std::map<CString, std::list<CString> > &getCompsocScenarioSchedules() const
            {
                return compsocScenarioSchedules;
            };
            void setCompsocScenarioSchedules(const std::map<CString, std::list<CString> > &s)
            {
                compsocScenarioSchedules = s;
            };
            void addCompsocScenarioSchedule(const CString& scenarioName, const std::list<CString>& sos)
            {
                compsocScenarioSchedules[scenarioName] = sos;
            };

            // Resource allocation
            std::map<const Scenario *, Time> &getWheelsizeAllocations()
            {
                return wheelsize;
            };
            void setWheelsizeAllocations(const std::map<const Scenario *, Time> &w)
            {
                wheelsize = w;
            };
            bool hasResourcesWheelSize(const Scenario& s, Time t) const;
            bool allocateWheelsize(const Scenario& s, Time t);

            // Resource occupancy
            [[nodiscard]] Time getAvailableWheelsize(const Scenario& s) const;
            [[nodiscard]] Time getAllocatedWheelsize(const Scenario& s) const;

            // Resource occupancy of application graph
            [[nodiscard]] Time getWheelsizeUsedForGraph() const;

            // Initial resource occupancy
            [[nodiscard]] Time getWheelsizeUsedForOtherGraphs() const
            {
                return wheelsizeUsedForOtherGraphs;
            };
            void setWheelsizeUsedForOtherGraphs(Time sz)
            {
                wheelsizeUsedForOtherGraphs = sz;
            };

        private:
            // Processor
            Processor *processor;

            // Actor bindings
            std::map<const Scenario *, ActorRefs> actorBindings;

            // Static-order schedules
            std::map<ScenarioConstRef, StaticOrderSchedule> staticOrderSchedules;

            // Collection of static-order schedules as list of strings, CompSOC-specific
            // format: map<scenario name, std::list<actor name>
            std::map<CString, std::list<CString> > compsocScenarioSchedules;

            // TDMA wheelsize allocation
            std::map<const Scenario *, Time> wheelsize;

            // TDMA wheelsize used for other graphs
            Time wheelsizeUsedForOtherGraphs;
    };

} // End namespace FSMSADF

#endif

