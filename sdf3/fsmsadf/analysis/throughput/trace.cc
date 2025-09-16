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

#include <memory>
#include <utility>

namespace FSMSADF
{
    void TraceOfFirings::Trace4CPSTrace(const CStrings& filenames)
    {
        // Get filenames
        const CString& configFilename = filenames.front();
        const CString& traceFilename = filenames.back();

        // Generate Trace4CPS.TRACE configuration file
        std::ofstream Configfile(configFilename);
        Configfile << "ConfigurationVersion	V0.1\n"
            << "ConfigurationName	Config\n"
            << "ConfigurationID	" << configFilename << std::endl
            << "TimeScaleShift	0\n"
            << "TimeScaleUnit	s\n"
            << "TimeDisplayFormat	%5g\n"
            << "Attribute	0	Name	String	Describing	Grouping	Coloring\n"
            << "Attribute	1	Scenario	String	Describing	Grouping	Coloring\n"
            << "Attribute	2	Iteration	String	Describing	Grouping	Coloring\n"
            << "Context	0	Processor	0\n"
            << "Context	1	Firings	0	1	2\n";
        Configfile.close();

        // Generate Trace4CPS.TRACE trace file
        // Initialize the trace file
        std::ofstream Tracefile(traceFilename);
        Tracefile << "TraceVersion	V0.1\nConfigurationID	"
            << configFilename
            << "\nConfigurationSource	"
            << configFilename << std::endl << std::endl;

        // Generate a resource for each actor firing
        unsigned int firingCounter = 0;
        int untracedFirings = 0;
        auto i = this->begin();
        while (i != this->end())
        {
            if ((*i)->getStartTime().isMinusInfinity())
            {
                i = this->erase(i);
                untracedFirings++;
            }
            else
            {
                Tracefile << "R	" << firingCounter << "	100	%	0	0	" << (*i)->getResource() << firingCounter << std::endl; //Claim Full
                firingCounter++;
                i++;
            }
        }
        Tracefile << std::endl;

        // Generate TRACE format traces
        firingCounter = 0;
        for (auto i = this->begin(); i != this->end(); i++, firingCounter++)
        {
            CString str = "C	";//Claim
            str += static_cast<int>(firingCounter); //Claim ID
            str += "	";
            str += static_cast<int>(firingCounter);// Resource ID It is assumed that each claim has a resource with the same ID
            str += "	";
            str += MPString((*i)->getStartTime());// Start Time
            str += "	";
            str += MPString((*i)->getEndTime());// End Time
            str += "	0	";// Limitation
            str += "1	";// Context ID
            str += (*i)->getActorName(); //Actor Name
            str += "	";
            str += (*i)->getScenarioName();// Scenario Name
            str += "	";
            str += (*i)->getIteration();//Number of the iteration
            Tracefile << str << std::endl;
        }
        Tracefile.close();
        if (untracedFirings != 0){ std::cout << "[WARNING] " << untracedFirings << " firings consumed only non-persistent tokens and not traced." << std::endl; }
    }

    void TraceOfFirings::xmlTrace(const CString& filename)
    {
		CNode *traceNode = CNewNode("trace");

		CNode *firingsNode = CAddNode(traceNode, "firings");

		// add firings
		uint firingCounter = 0;
		for (auto i = this->begin(); i != this->end(); i++, firingCounter++)
		{
			const std::unique_ptr<TraceFiring>& c = (*i);
			CNode *firingNode = CNewNode("firing");
			CAddAttribute(firingNode, "id", CString(firingCounter));
			CAddAttribute(firingNode, "start", MPString(c->getStartTime()));
			CAddAttribute(firingNode, "end", MPString(c->getEndTime()));
			CAddAttribute(firingNode, "actor", (c->getActorName()));
			CAddAttribute(firingNode, "scenario", (c->getScenarioName()));
			CAddAttribute(firingNode, "iteration", CString(c->getIteration()));
			CAddNode(firingsNode, firingNode);
		}

		CAddNode(traceNode, firingsNode);

		// Create document and save it
		std::unique_ptr<CDoc> doc = CNewDoc(traceNode);
		doc->saveFile(filename);
	}

    std::unique_ptr<Trace> TraceOfFirings::scheduleTrace()
    {
        auto trace = std::make_unique<Trace>();

        // Generate trace
        uint iteration = -1;
        std::pair<CString, std::list<CString>> *scenarioTracePtr = nullptr;
        for (const auto& firing: *this)
        {
            if (static_cast<int>(firing->getIteration()) != iteration)
            {
                auto scenarioTrace = std::make_unique<std::pair<CString, std::list<CString>>>();
                scenarioTracePtr = scenarioTrace.get();
                scenarioTrace->first = firing->getScenarioName();
                trace->push_back(std::move(scenarioTrace));
                iteration = firing->getIteration();
            }
            scenarioTracePtr->second.push_back(firing->getActorName());
        }
        // Return trace;
        return trace;
    }

    TraceVector::TraceVector(CStrings token_names, Vector state_vector):
		tokenNames(std::move(token_names)),
		stateVector(std::move(state_vector))
    {
    }

    const CStrings& TraceVector::getTokenNames()
    {
        return this->tokenNames;
    }

    const Vector& TraceVector::getStateVector()
    {
        return this->stateVector;
    }

    void TraceOfVectors::Trace4CPSTrace(const CStrings&  /*filenames*/) // NOLINT(*static)
    {
		throw ::CException("Generation of a vector trace in Trace4CPS Trace format is not implemented (yet).");
    }

    void TraceOfVectors::xmlTrace(const CString& filename)
    {
		CNode *traceNode = CNewNode("vectortrace");

		CNode *vectorsNode = CAddNode(traceNode, "vectors");

		// add vectors
		uint vectorCounter = 0;
		for (auto i = this->begin(); i != this->end(); i++, vectorCounter++)
		{
			const std::unique_ptr<TraceVector>& v = (*i);
			CNode *vectorNode = CNewNode("vector");
			CAddAttribute(vectorNode, "id", CString(vectorCounter));
			CStrings::const_iterator j;
			unsigned int k = 0;
			for (j = v->getTokenNames().begin(), k = 0; j != v->getTokenNames().end(); j++, k++) {
				const CString& tn = *j;
				CNode *tokenNode = CNewNode("token");
				CAddAttribute(tokenNode, "name", tn);
				CAddAttribute(tokenNode, "timestamp", MPString(v->getStateVector().get(k)));
				CAddNode(vectorNode, tokenNode);
			}
			CAddNode(vectorsNode, vectorNode);
		}

		CAddNode(traceNode, vectorsNode);

		// Create document and save it
		std::unique_ptr<CDoc> doc = CNewDoc(traceNode);
		doc->saveFile(filename);
	}


} // namespace FSMSADF
