/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   message.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   January 6, 2006
 *
 *  Function        :   Message
 *
 *  History         :
 *      06-01-06    :   Initial version.
 *
 * $Id: message.h,v 1.1 2008-03-20 16:16:20 sander Exp $
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

#ifndef SDF_RESOURCE_ALLOCATION_NOC_ALLOCATION_PROBLEM_MESSAGE_H_INCLUDED
#define SDF_RESOURCE_ALLOCATION_NOC_ALLOCATION_PROBLEM_MESSAGE_H_INCLUDED


#include "interconnect_graph.h" // IWYU pragma: keep
#include "sdf/resource_allocation/noc_allocation/problem/schedulingentity.h"

namespace SDF
{
    // Forward class definition
    // class NoCSchedulingEntity;

    /**
     * Message
     * Message class used for NoC scheduling
     */
    class Message
    {
        public:
            // Constructor
            explicit Message(CId id) :
				id(id)
			{};

            // Destructor
            ~Message() = default;

            // Prevent copying and moving
            Message(const Message&) = delete;
            Message& operator=(const Message&) = delete;
            Message(Message&&) = delete;
            Message& operator=(Message&&) = delete;

            // Properties
            [[nodiscard]] CId getId() const
            {
                return id;
            };
            void setSrcNodeId(const CId n)
            {
                srcNodeId = n;
            };
            [[nodiscard]] CId getSrcNodeId() const
            {
                return srcNodeId;
            };
            void setDstNodeId(const CId n)
            {
                dstNodeId = n;
            };
            [[nodiscard]] CId getDstNodeId() const
            {
                return dstNodeId;
            };
            void setStreamId(const CId s)
            {
                streamId = s;
            };
            [[nodiscard]] CId getStreamId() const
            {
                return streamId;
            };
            void setSeqNr(const ulong n)
            {
                seqNr = n;
            };
            [[nodiscard]] ulong getSeqNr() const
            {
                return seqNr;
            };
            void setStartTime(const TTime t)
            {
                startTime = t;
            };
            [[nodiscard]] TTime getStartTime() const
            {
                return startTime;
            };
            void setDuration(const TTime d)
            {
                duration = d;
            };
            [[nodiscard]] TTime getDuration() const
            {
                return duration;
            };
            void setSize(const CSize s)
            {
                size = s;
            };
            [[nodiscard]] CSize getSize() const
            {
                return size;
            };

            // Stream
            [[nodiscard]] Message *getNextMessageInStream() const
            {
                return nextMsgInStream;
            };
            void setNextMessageInStream(Message *m)
            {
                nextMsgInStream = m;
            };
            [[nodiscard]] Message *getPreviousMessageInStream() const
            {
                return prevMsgInStream;
            };
            void setPreviousMessageInStream(Message *m)
            {
                prevMsgInStream = m;
            };

            // Scheduling entity
            [[nodiscard]] NoCSchedulingEntity *getSchedulingEntity() const
            {
                return schedulingEntity.get();
            };
            void setSchedulingEntity(std::shared_ptr<NoCSchedulingEntity> n)
            {
                schedulingEntity = std::move(n);
            };

            // Cost
            void setCost(double c)
            {
                cost = c;
            };
            [[nodiscard]] double getCost() const
            {
                return cost;
            };
            bool operator<(const Message &m) const
            {
                return getCost() < m.getCost();
            };

            // Print
            std::ostream &print(std::ostream &out) const;

        private:
            // Properties
            CId id;
            CId srcNodeId{};
            CId dstNodeId{};
            CId streamId{};
            ulong seqNr{};
            TTime startTime{};
            TTime duration{};
            CSize size{};

            // Stream
            Message *nextMsgInStream{};
            Message *prevMsgInStream{};

            // Scheduling entity
            std::shared_ptr<NoCSchedulingEntity> schedulingEntity = nullptr;

            // Cost
            double cost{};
    };

    using Messages = std::list<std::unique_ptr<Message>>;
    using MessagesIter = Messages::iterator;
    using MessagesCIter = Messages::const_iterator;
    using MessageRef = Message*;

    struct MessageLess
    {
        bool operator()(const std::unique_ptr<Message>& x, const std::unique_ptr<Message>& y)
        {
            return x->getCost() < y->getCost();
        }
    };
}//namespace SDF
#endif
