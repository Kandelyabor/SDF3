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

#include "trace.h"

namespace SDF
{

	void TraceOfFirings::addActorFiring(const std::shared_ptr<ActorFiring>& acf)
	{
		acf->setId(this->nrActorFirings());
		actorFirings.push_back(acf);
	}
	void TraceOfFirings::addInputFiring(const std::shared_ptr<Firing>& inf)
	{
		// In tno trace, events (inputs or outputs) share the id
		inf->setId(this->nrInputFirings()+this->nrOutputFirings());
		inputFirings.push_back(inf);
	}
	void TraceOfFirings::addOutputFiring(const std::shared_ptr<Firing>& ouf)
	{
		// In tno trace, events (inputs or outputs) share the id
		ouf->setId(this->nrInputFirings()+this->nrOutputFirings());
		outputFirings.push_back(ouf);
	}

	std::shared_ptr<ActorFiring> TraceOfFirings::getActorFiring(const CString& name, uint iteration) const
	{
		for (const auto &acf: this->getActorFirings()) {
			if (acf->getName() == name && acf->getIteration() == iteration) {
				return acf;
			}
		}
		return nullptr;
	}
	std::shared_ptr<Firing> TraceOfFirings::getInputFiring(const CString& name, uint iteration) const
	{
		for (const auto &inf: this->getInputFirings()) {
			if (inf->getName() == name && inf->getIteration() == iteration) {
				return inf;
			}
		}
		return nullptr;
	}
	std::shared_ptr<Firing> TraceOfFirings::getOutputFiring(const CString& name, uint iteration) const
	{
		for (const auto &ouf: this->getOutputFirings()) {
			if (ouf->getName() == name && ouf->getIteration() == iteration) {
				return ouf;
			}
		}
		return nullptr;
	}

	std::shared_ptr<Dependency> TraceOfFirings::getDependency(
		DependencyType dependencyType,
        const std::shared_ptr<Firing>& dSrcFiring,
        const std::shared_ptr<Firing>& dDstFiring) const {

		for (const auto &dep: this->getDependencies()) {
			if (dep->getDType() == dependencyType &&
				dep->getSrcFiring() == dSrcFiring &&
				dep->getDstFiring() == dDstFiring) {
				return dep;
			}
		}
		return nullptr;
	}

	void TraceOfFirings::removeActorFiring(const CString& name, uint iteration)
	{
		actorFirings.remove_if([&](const std::shared_ptr<ActorFiring>& acf) {
			return acf->getName() == name && acf->getIteration() == iteration;
		});
	}
	void TraceOfFirings::removeInputFiring(const CString& name, uint iteration)
	{
		inputFirings.remove_if([&](const std::shared_ptr<Firing>& icf) {
			return icf->getName() == name && icf->getIteration() == iteration;
		});
	}
	void TraceOfFirings::removeOutputFiring(const CString& name, uint iteration)
	{
		outputFirings.remove_if([&](const std::shared_ptr<Firing>& ocf) {
			return ocf->getName() == name && ocf->getIteration() == iteration;
		});
	}

	void TraceOfFirings::removeDependency(
		DependencyType dependencyType,
        std::shared_ptr<Firing> dSrcFiring,
        std::shared_ptr<Firing> dDstFiring) {

		dependencies.remove_if([&](const std::shared_ptr<Dependency>& dep) {
			return 	dep->getDType() == dependencyType &&
					dep->getSrcFiring() == dSrcFiring &&
					dep->getDstFiring() == dDstFiring;
		});
	}

    void TraceOfFirings::xmlTrace(const CString& filename) const
    {
		CNode *traceNode = CNewNode("trace");
		CNode *firingsNode = CAddNode(traceNode, "firings");
		CNode *inputsNode = CAddNode(traceNode, "inputs");
		CNode *outputsNode = CAddNode(traceNode, "outputs");

		// add actor firings
		uint firingID = 0;
		for (const auto &acf : this->getActorFirings())
		{
			CNode *firingNode = CNewNode("firing");
			CAddAttribute(firingNode, "id", CString(firingID));
			CAddAttribute(firingNode, "start", MPString(acf->getTime()));
			CAddAttribute(firingNode, "end", MPString(acf->getTime() + acf->getDuration()));
			CAddAttribute(firingNode, "actor", (acf->getName()));
			CAddAttribute(firingNode, "iteration", CString(acf->getIteration()));
			CAddAttribute(firingNode, "scenario", "s"); // default set to "s", not used in SDF
			CAddNode(firingsNode, firingNode);
			firingID++;
		}

		// add input timestamps
		for (const auto &inf : this->getInputFirings())
		{
			CNode *inputNode = CNewNode("input");
			CAddAttribute(inputNode, "name", inf->getName());
			CAddAttribute(inputNode, "timestamp", MPString(inf->getTime()));
			CAddAttribute(inputNode, "iteration", MPString(inf->getIteration()));
			CAddNode(inputsNode, inputNode);
		}

		// add output timestamps
		for (const auto &ouf : this->getOutputFirings())
		{
			CNode *outputNode = CNewNode("output");
			CAddAttribute(outputNode, "name", ouf->getName());
			CAddAttribute(outputNode, "timestamp", MPString(ouf->getTime()));
			CAddAttribute(outputNode, "iteration", MPString(ouf->getIteration()));
			CAddNode(outputsNode, outputNode);
		}

		CAddNode(traceNode, firingsNode);
		CAddNode(traceNode, inputsNode);
		CAddNode(traceNode, outputsNode);

		// Create document and save it
		std::unique_ptr<CDoc> doc = CNewDoc(traceNode);
		doc->saveFile(filename);
	}

	void TraceOfFirings::tnoTrace(const CString& filename) const {
		std::ofstream traceFile(filename);

		// Global configurations
		traceFile
			<< "TU SECONDS" << std::endl
			<< "O 0" << std::endl
			<< "T name = " << filename << std::endl
			<< std::endl;

		// actor resources
		for (const auto &acf: this->getActorFirings()) {
			traceFile
				<< "R "
				<< acf->getId() << " "							// resource ID
				<< "100 false ; "								// capacity and user offset
				<< "name = " << acf->getId() << std::endl;		// attribute name
		}

		// actor firings
		for (const auto &acf: this->getActorFirings()) {
			MPTime startTime;
			MPTime endTime;

			if (acf->getDuration() < MPTime(0)) {
				startTime = acf->getTime() + acf->getDuration();
				endTime = acf->getTime();
			} else {
				startTime = acf->getTime();
				endTime = acf->getTime() + acf->getDuration();
			}

			traceFile
				<< "C "
				<< acf->getId() << " " 								// actor firing ID
				<< MPString(startTime) << " " 							// start time
				<< MPString(endTime) << " "								// end time
				<< acf->getId() << " "									// resource ID
				<< "100 ; "												// amount (not relevant but necessary)
				<< "name = " << acf->getName() << " , "					// attribute actor name
				<< "iteration = " << acf->getIteration() << std::endl;	// attribute iteration
		}

		// input events
		for (const auto &inf: this->getInputFirings()) {
			traceFile
				<< "E "
				<< inf->getId() << " "									// input firing ID
				<< MPString(inf->getTime()) << " ; "					// input event time
				<< "name = " << inf->getName() << " , "					// attribute input name
				<< "iteration = " << inf->getIteration() << std::endl;	// attribute iteration
		}

		// output events
		for (const auto &ouf: this->getOutputFirings()) {
			traceFile
				<< "E "
				<< ouf->getId() << " "									// output firing ID (also events)
				<< MPString(ouf->getTime()) << " ; "					// output event time
				<< "name = " << ouf->getName() << " , "					// attribute output name
				<< "iteration = " << ouf->getIteration() << std::endl;	// attribute iteration
		}

		// dependencies
		uint firingCounter = 0;
		for (const auto &dep: this->getDependencies()) {
			DependencyType deptype = dep->getDType();

			traceFile
				<< "D "
				<< firingCounter << " "
				<< static_cast<uint>(dep->getDType()) << " "
				<< dep->getSrcFiring()->getId() << " "
				<< dep->getDstFiring()->getId() << " ;" << std::endl;
			firingCounter++;
		}

		traceFile.close();
	}

} // namespace SDF
