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

#ifndef SDF_OUTPUT_TRACE_TRACE_H_INCLUDED
#define SDF_OUTPUT_TRACE_TRACE_H_INCLUDED

#include <utility>

#include "sdf/base/untimed/graph.h" // IWYU pragma: keep
#include "maxplus/algebra/mptype.h"

namespace SDF
{

    using namespace MaxPlus;

    enum DependencyType {
        CstartTOCstart = 0,
        CstartTOCend = 1,
        CendTOCstart = 2,
        CendTOCend = 3,
        EventToEvent = 4,
        CstartToEvent = 5,
        CendToEvent = 6,
        EventToCStart = 7,
        EventToCEnd = 8
    };

    class Firing
    {
    public:
    Firing() = default;
    Firing(uint fId, CString  fName, uint fIteration, MPTime fTime) :
            id(fId), name(std::move(fName)), iteration(fIteration), time(fTime) {};

            [[nodiscard]] uint getId() const{ return id; }
            CString getName(){ return name; }
            [[nodiscard]] uint getIteration() const{ return iteration; }
            MPTime getTime(){ return time; }

            void setId(uint fId) { id = fId; }
            void setName(const CString& fName) { name = fName; }
            void setIteration(uint fIteration) { iteration = fIteration; }
            void setTime(MPTime fTime) { time = fTime; }

    private:
        uint id{};
        CString name;
        uint iteration{};
        MPTime time;
    };

    class ActorFiring : public Firing
    {
    public:
        ActorFiring() = default;
        ActorFiring(
            uint fId,
            const CString& fName,
            uint fIteration,
            MPTime fTime,
            const CString& fResource,
            MPTime fDuration) :
            Firing(fId, fName, fIteration, fTime),
            resource(fResource),
            duration(fDuration) {};

        CString getResource(){ return resource; }
        MPTime getDuration(){ return duration; }

        void setResource(const CString& fResource){ resource = fResource; }
        void setDuration(MPTime fDuration){ duration = fDuration; }
        private:
        CString resource;
        MPTime duration;
    };

    class Dependency
    {
    public:
        Dependency() : dType(CstartTOCstart) {} // Initialize dType with a default value
        Dependency(
            DependencyType dependencyType,
            std::shared_ptr<Firing> dSrcFiring,
            std::shared_ptr<Firing> dDstFiring) :
            dType(dependencyType),
            srcFiring(std::move(dSrcFiring)),
            dstFiring(std::move(dDstFiring)) {};

        DependencyType getDType() { return dType; }
        std::shared_ptr<Firing> getSrcFiring() { return srcFiring; }
        std::shared_ptr<Firing> getDstFiring() { return dstFiring; }

        void setDType(DependencyType dependencyType) { dType = dependencyType; }
        void setSrcFiring(std::shared_ptr<Firing> dSrcFiring) { srcFiring = std::move(dSrcFiring); }
        void setDstFiring(std::shared_ptr<Firing> dDstFiring) { dstFiring = std::move(dDstFiring); }

    private:
        DependencyType dType;
        std::shared_ptr<Firing> srcFiring;
        std::shared_ptr<Firing> dstFiring;
    };

    using ActorFiringList = std::list<std::shared_ptr<ActorFiring>>;
    using InputFiringList = std::list<std::shared_ptr<Firing>>;
    using OutputFiringList = std::list<std::shared_ptr<Firing>>;
    using DependencyList = std::list<std::shared_ptr<Dependency>>;

    class TraceOfFirings
    {
    public:

    [[nodiscard]] const ActorFiringList &getActorFirings() const { return actorFirings; };
    [[nodiscard]] const InputFiringList &getInputFirings() const { return inputFirings; };
    [[nodiscard]] const OutputFiringList &getOutputFirings() const { return outputFirings; };
    [[nodiscard]] const DependencyList &getDependencies() const { return dependencies; };

    void xmlTrace(const CString& filename) const;
    void tnoTrace(const CString& filename) const;

    [[nodiscard]] uint nrActorFirings() const { return static_cast<uint>(actorFirings.size()); };
    [[nodiscard]] uint nrInputFirings() const { return static_cast<uint>(inputFirings.size()); };
    [[nodiscard]] uint nrOutputFirings() const { return static_cast<uint>(outputFirings.size()); };
    [[nodiscard]] uint nrDependencies() const { return static_cast<uint>(dependencies.size()); };

    static std::shared_ptr<ActorFiring> createActorFiring() { return std::make_shared<ActorFiring>(); };
    static std::shared_ptr<Firing> createFiring() { return std::make_shared<Firing>(); };
    static std::shared_ptr<Dependency> createDependency() { return std::make_shared<Dependency>(); };

    void addActorFiring(const std::shared_ptr<ActorFiring>& acf);
    void addInputFiring(const std::shared_ptr<Firing>& inf);
    void addOutputFiring(const std::shared_ptr<Firing>& ouf);
    void addDependency(const std::shared_ptr<Dependency>& dep) { dependencies.push_back(dep); };

    [[nodiscard]] std::shared_ptr<ActorFiring> getActorFiring(const CString& actorName, uint iteration) const;
    std::shared_ptr<Firing> getInputFiring(const CString& inputName, uint iteration) const;
    std::shared_ptr<Firing> getOutputFiring(const CString& outputName, uint iteration) const;
    std::shared_ptr<Dependency> getDependency(
        DependencyType dependencyType,
        const std::shared_ptr<Firing>& dSrcFiring,
        const std::shared_ptr<Firing>& dDstFiring) const;

    void removeActorFiring(const CString& actorName, uint iteration);
    void removeInputFiring(const CString& inputName, uint iteration);
    void removeOutputFiring(const CString& outputName, uint iteration);
    void removeDependency(
        DependencyType dependencyType,
        std::shared_ptr<Firing> dSrcFiring,
        std::shared_ptr<Firing> dDstFiring);

    private:
    ActorFiringList actorFirings;
    InputFiringList inputFirings;
    OutputFiringList outputFirings;
    DependencyList dependencies;
};

} // namespace SDF

#endif