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
 *  Function        :   Timed CSDF actor
 *
 *  History         :
 *      18-07-05    :   Initial version.
 *
 * $Id: actor.h,v 1.2.2.1 2010-04-25 01:21:16 mgeilen Exp $
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

#ifndef CSDF_BASE_TIMED_ACTOR_H_INCLUDED
#define CSDF_BASE_TIMED_ACTOR_H_INCLUDED

#include "csdf/base/untimed/actor.h"
#include "timed_types.h"
namespace CSDF
{
    /**
     * TimedCSDFactor
     * Timed actor in CSDF graph
     */
    class TimedCSDFactor : public CSDFactor
    {
        public:
            // Processor type specific properties
            using Processor = struct Processor
            {
                CString             type;
                CSDFtimeSequence    execTime;
                CSize               stateSize;
            };

            using Processors = std::list<std::unique_ptr<Processor>>;
            using ProcessorsIter = Processors::iterator;
            using ProcessorsCIter = Processors::const_iterator;


            // Constructor
            explicit TimedCSDFactor(CSDFcomponent &c);

            // Destructor
            ~TimedCSDFactor() override;

            // Prevent copying and moving
            TimedCSDFactor(const TimedCSDFactor&) = delete;
            TimedCSDFactor& operator=(const TimedCSDFactor&) = delete;
            TimedCSDFactor(TimedCSDFactor&&) = delete;
            TimedCSDFactor& operator=(TimedCSDFactor&&) = delete;


            // Construct
            std::unique_ptr<CSDFactor> create(CSDFcomponent &c) const override;
            std::unique_ptr<CSDFactor> createCopy(CSDFcomponent &c) const override;
            std::unique_ptr<CSDFactor> clone(CSDFcomponent &c) const override;

            // Execution time
            [[nodiscard]] CSDFtimeSequence &getExecutionTime() const;
            [[nodiscard]] CSDFdiscreteTimeSequence getDiscreteExecutionTime() const;

            // State size
            CSize getStateSize();
            CSize getStateSize(const CString &proc);

            // Processor
            const Processor *getProcessor(const CString &proc);
            Processor *addProcessor(const CString &proc);
            void addProcessor(const Processor& p);
            void removeProcessor(const CString &proc);
            [[nodiscard]] uint nrProcessors() const
            {
                return static_cast<uint>(processors.size());
            };
            ProcessorsIter processorsBegin()
            {
                return processors.begin();
            };
            [[nodiscard]] ProcessorsCIter processorsBegin() const
            {
                return processors.begin();
            };
            ProcessorsIter processorsEnd()
            {
                return processors.end();
            };
            [[nodiscard]] ProcessorsCIter processorsEnd() const
            {
                return processors.end();
            };
            Processors& getProcessors()
            {
                return processors;
            };

            // Default processor
            [[nodiscard]] CString getDefaultProcessor() const
            {
                return defaultProcessor;
            };
            void setDefaultProcessor(const CString &p)
            {
                defaultProcessor = p;
            };

            // Print
            std::ostream &print(std::ostream &out) const;
            friend std::ostream &operator<<(std::ostream &out, TimedCSDFactor &a)
            {
                return a.print(out);
            };

        private:
            // Processor
            Processors processors;
            CString defaultProcessor;
    };

    using TimedCSDFactors = std::list<TimedCSDFactor *>;
    using TimedCSDFactorsIter = TimedCSDFactors::iterator;
    using TimedCSDFactorsCIter = TimedCSDFactors::const_iterator;

}//namespace CSDF
#endif
