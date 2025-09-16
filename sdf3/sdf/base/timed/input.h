/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   input.h
 *
 *  Author          :   Joep van Wanrooij (j.c.p.van.wanrooij@tue.nl)
 *
 *  Date            :   May 1, 2025
 *
 *  Function        :   Timed SDF input
 *
 *  History         :
 *      01-05-2025  :   Initial version.
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

#ifndef SDF_BASE_TIMED_INPUT_H_INCLUDED
#define SDF_BASE_TIMED_INPUT_H_INCLUDED

#include "sdf/base/untimed/graph.h" // IWYU pragma: keeps
#include "timed_types.h"
namespace SDF {

/**
 * Input sequence class
 * Sequences of an SDF graph's input
 */
class EventSequence : public std::vector<SDFtime> {
public:
    EventSequence() = default;
    ~EventSequence() = default;

    explicit EventSequence(CString eSname) : name(std::move(eSname)) {}

    EventSequence(const EventSequence&) = default;
    EventSequence& operator=(const EventSequence&) = delete;
    EventSequence(EventSequence&&) = delete;
    EventSequence& operator=(EventSequence&&) = delete;


    [[nodiscard]] CString getName() { return name; }

    // Print
    std::ostream &print(std::ostream &out);
    friend std::ostream &operator<<(std::ostream &out, EventSequence &es) { return es.print(out); };

private:
    CString name;
};

// List of event sequences
using SDFSequences = std::list<std::unique_ptr<EventSequence>>;

/**
 * Timed SDFinput
 * Timed input in SDF graph
 */
class TimedSDFinput : public SDFinput {
public:

    // Constructor
    explicit TimedSDFinput(SDFcomponent &c);

    // Destructor
    ~TimedSDFinput() override;

    TimedSDFinput(const TimedSDFinput&) = delete;
    TimedSDFinput& operator=(const TimedSDFinput&) = delete;
    TimedSDFinput(TimedSDFinput&&) = delete;
    TimedSDFinput& operator=(TimedSDFinput&&) = delete;

    // Construct
    [[nodiscard]] std::unique_ptr<SDFinput> create(SDFcomponent &c) const override;
    [[nodiscard]] std::unique_ptr<SDFinput> createCopy(SDFcomponent &c) const override;
    [[nodiscard]] std::unique_ptr<SDFinput> clone(SDFcomponent &c) const override;

    // Properties
    void setProperties(CNodePtr propertiesNode);

    // Sequences
    [[nodiscard]] const SDFSequences &getEventSequences() const { return sequences; };
    void addEventSequence(std::unique_ptr<EventSequence> es) { sequences.push_back(std::move(es)); };
    void removeEventSequence(const CString &name);
    [[nodiscard]] uint eventSequencesSize() const { return static_cast<uint>(sequences.size()); };

    [[nodiscard]] bool isDiscreteTimeSequences() const { return isDiscreteTime; };

    // Print
    std::ostream &print(std::ostream &out);
    friend std::ostream &operator<<(std::ostream &out, TimedSDFinput &i) { return i.print(out); };

private:
    // Sequence
    SDFSequences sequences;

    // Flag possible changed during parsing to check if all execution times are integer
    bool isDiscreteTime = true;
};

} // namespace SDF
#endif
