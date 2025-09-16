/*
*  TU Eindhoven
*  Eindhoven, The Netherlands
*
*  Name            :   trace.cc
*
*  Author          :   Hadi Seyedalizadeh Ara(s.h.seyyed.alizadeh@tue.nl)
*
*  Date            :   Jul 11, 2015
*
*  Function        :   Provides primitives to generate execution
*                      traces for FSMSADFs
*
*  History         :
*      11-07-15    :   Initial version.
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

#include "fsmsadf/base/graph.h" // IWYU pragma: keep
#include "fsmsadf/base/fsm.h" // IWYU pragma: keep
#include <memory>

namespace FSMSADF
{

    using namespace MaxPlus;

	enum TraceFormat { Trace4CPS, XML };

    class TraceFiring
    {
    public:
        void create(const CString& actorName, const CString& scenarioName, const CString& resource, unsigned int iteration, MPTime starttime, MPTime endtime)
        {
            ActorName = actorName;
            ScenarioName = scenarioName;
            Iteration = iteration;
            Resource = resource;
            StartTime = starttime;
            EndTime = endtime;
        }
        CString getActorName(){ return ActorName; }
        CString getScenarioName(){ return ScenarioName; }
        CString getResource(){ return Resource; }
        [[nodiscard]] unsigned int getIteration() const{ return Iteration; }
        MPTime getStartTime(){ return StartTime; }
        MPTime getEndTime(){ return EndTime; }
    private:
        CString ActorName;
        CString ScenarioName;
        CString Resource;
        unsigned int Iteration;
        MPTime StartTime;
        MPTime EndTime;
    };

    class TraceVector
    {
    public:
        TraceVector(CStrings token_names, Vector state_vector);
        const CStrings& getTokenNames();
        const Vector& getStateVector();
    private:
        CStrings tokenNames;
        Vector stateVector;
    };


    using Trace = std::list<std::unique_ptr<std::pair<CString, std::list<CString>>>>;

    class TraceOfFirings : public std::list<std::unique_ptr<TraceFiring>>
    {
    public:
        TraceOfFirings() = default;
        ~TraceOfFirings() = default;
        TraceOfFirings(const TraceOfFirings&) = delete;
        TraceOfFirings& operator=(const TraceOfFirings&) = delete;
        TraceOfFirings(TraceOfFirings&&) = delete;
        TraceOfFirings& operator=(TraceOfFirings&&) = delete;

        void Trace4CPSTrace(const CStrings& filenames);
        void xmlTrace(const CString& filename);

        std::unique_ptr<Trace> scheduleTrace();
    };

    class TraceOfVectors : public std::list<std::unique_ptr<TraceVector>>
    {
    public:
        TraceOfVectors() = default;
        ~TraceOfVectors() = default;
        TraceOfVectors(const TraceOfVectors&) = delete;
        TraceOfVectors& operator=(const TraceOfVectors&) = delete;
        TraceOfVectors(TraceOfVectors&&) = delete;
        TraceOfVectors& operator=(TraceOfVectors&&) = delete;

        void Trace4CPSTrace(const CStrings& filenames);
        void xmlTrace(const CString& filename);
    };

} // namespace FSMSADF
