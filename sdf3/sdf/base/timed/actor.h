/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   actor.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   July 18, 2005
 *
 *  Function        :   Timed SDF actor
 *
 *  History         :
 *      18-07-05    :   Initial version.
 *
 * $Id: actor.h,v 1.1.1.1 2007-10-02 10:59:46 sander Exp $
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

#ifndef SDF_BASE_TIMED_ACTOR_H_INCLUDED
#define SDF_BASE_TIMED_ACTOR_H_INCLUDED

#include "sdf/base/untimed/graph.h" // IWYU pragma: keep
#include "timed_types.h"
#include <memory>
namespace SDF {

/**
 * Timed SDFactor
 * Timed actor in SDF graph
 */
class TimedSDFactor : public SDFactor {
public:
    // Rule of Five: declare copy/move constructors and assignment operators
    TimedSDFactor(const TimedSDFactor& other) = delete;
    TimedSDFactor& operator=(const TimedSDFactor& other) = delete;
    TimedSDFactor(TimedSDFactor&& other) noexcept = delete;
    TimedSDFactor& operator=(TimedSDFactor&& other) noexcept = delete;

    // Processor type specific properties
    using Processor = struct Processor {
        CString type;
        SDFtime execTime;
        CSize stateSize;
    };

    using Processors = std::list<std::unique_ptr<Processor>>;
    using ProcessorsIter = Processors::iterator;
    using ProcessorsCIter = Processors::const_iterator;

    // Constructor
    explicit TimedSDFactor(SDFcomponent &c);

    // Destructor
    ~TimedSDFactor() override;

    // Construct
    [[nodiscard]] std::unique_ptr<SDFactor> create(SDFcomponent &c) const override;
    [[nodiscard]] std::unique_ptr<SDFactor> createCopy(SDFcomponent &c) const override;
    [[nodiscard]] std::unique_ptr<SDFactor> clone(SDFcomponent &c) const override;

    // Properties
    void setProperties(CNodePtr propertiesNode);

    // Execution time
    [[nodiscard]] SDFtime getExecutionTime() const;
    [[nodiscard]] SDFtime getExecutionTime(const CString &proc) const;
    [[nodiscard]] SDFdiscreteTime getDiscreteExecutionTime() const;

    // State size
    [[nodiscard]] CSize getStateSize() const;
    [[nodiscard]] CSize getStateSize(const CString &proc) const;

    // Processor
    [[nodiscard]] Processor *getProcessor(const CString &proc) const;
    Processor *addProcessor(const CString &proc);
    void addProcessor(const Processor *p);
    void removeProcessor(const CString &proc);
    [[nodiscard]] uint nrProcessors() const { return static_cast<uint>(processors.size()); };
    ProcessorsIter processorsBegin() { return processors.begin(); };
    [[nodiscard]] ProcessorsCIter processorsBegin() const { return processors.begin(); };
    ProcessorsIter processorsEnd() { return processors.end(); };
    [[nodiscard]] ProcessorsCIter processorsEnd() const { return processors.end(); };
    [[nodiscard]] const Processors &getProcessors() const { return processors; };

    [[nodiscard]] bool isDiscreteTimeActor() const;

    // Default processor
    [[nodiscard]] CString getDefaultProcessor() const { return defaultProcessor; };
    void setDefaultProcessor(const CString &p) { defaultProcessor = p; };

    // dynamic cast
    static std::unique_ptr<TimedSDFactor> dynamicCast(std::unique_ptr<SDFactor> a);

    // Print
    std::ostream &print(std::ostream &out);
    friend std::ostream &operator<<(std::ostream &out, TimedSDFactor &a) { return a.print(out); };

private:
    // Processor
    Processors processors;
    CString defaultProcessor;
    // Flag possible changed during parsing to check if all execution times are integer
    bool isDiscreteTime = true;
};

using TimedSDFactorRef = TimedSDFactor*;

} // namespace SDF
#endif
