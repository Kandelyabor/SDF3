/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   dot.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   July 22, 2005
 *
 *  Function        :   Output SDF graph in dot format
 *
 *  History         :
 *      22-07-05    :   Initial version.
 *
 * $Id: dot.cc,v 1.1.1.1 2007-10-02 10:59:46 sander Exp $
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

#include "dot.h"
namespace SDF
{


    /**
     * outputSDFasDot ()
     * The function outputs a SDF graph in DOT format.
     */
    void outputSDFasDot(const SDFgraph& g, std::ostream &out)
    {

        out << "digraph " << g.getName() << " {" << std::endl;
        out << "    size=\"7,10\";" << std::endl;

        // Output all actors
        for (const auto& a: g.getActors())
        {
            out << "    " << a->getName() << " [ label=\"" << a->getName();
            out << "\" ];" << std::endl;
        }
        out << std::endl;

        // Output all channels
        for (const auto& c: g.getChannels())
        {
            // Initial tokens on channel?
            if (c->getInitialTokens() != 0)
            {
#ifdef __DOT_EXPLICIT_INITIAL_TOKENS
                SDFport *srcP = c->getSrcPort();
                SDFactor *srcA = srcP->getActor();
                SDFport *dstP = c->getDstPort();
                SDFactor *dstA = dstP->getActor();

                // Source actor to initial token actor
                out << "    " << srcA->getName() << " -> " << c->getName();
                out << " [ label=\"" << c->getName() << "\", taillabel=\"";
                out << srcP->getRate() << "\", arrowhead=dot, headlabel=\"";
                out << c->getInitialTokens() << "\" ];" << std::endl;

                // initial token actor to destination actor
                out << "    " << c->getName() << " -> " << dstA->getName();
                out << " [ headlabel=\"" << dstP->getRate() << "\" ];" << std::endl;

                // set properties initial token actor
                out << "    " << c->getName();
                out << " [ label=\"\", width=0, height=0, fixedsize=true ];";
                out << std::endl;
#else // __DOT_EXPLICIT_INITIAL_TOKENS
                SDFport *srcP = c->getSrcPort();
                SDFactor *srcA = srcP->getActor();
                SDFport *dstP = c->getDstPort();
                SDFactor *dstA = dstP->getActor();

                out << "    " << srcA->getName() << " -> " << dstA->getName();
                out << " [ label=\"" << c->getName() << "(";
                out << c->getInitialTokens() << ")" << "\", taillabel=\"";
                out << srcP->getRate() << "\", headlabel=\"";
                out << dstP->getRate() << "\" ];" << std::endl;
#endif // __DOT_EXPLICIT_INITIAL_TOKENS
            }
            else
            {
                SDFport *srcP = c->getSrcPort();
                SDFactor *srcA = srcP->getActor();
                SDFport *dstP = c->getDstPort();
                SDFactor *dstA = dstP->getActor();

                out << "    " << srcA->getName() << " -> " << dstA->getName();
                out << " [ label=\"" << c->getName() << "\", taillabel=\"";
                out << srcP->getRate() << "\", headlabel=\"";
                out << dstP->getRate() << "\" ];" << std::endl;
            }
        }


        for (const auto& c: g.getInputChannels())
        {
                SDFport* srcP = c->getSrcPort();
                SDFport* dstP = c->getDstPort();
                SDFactor* dstA = dstP->getActor();

                out << " Input -> " << dstA->getName();
                out << " [ label=\"" << c->getName() << "\", taillabel=\"";
                out << srcP->getRate() << "\", headlabel=\"";
                out << dstP->getRate() << "\" ];" << std::endl;

        }

        for (const auto& c: g.getOutputChannels())
        {
            SDFport* srcP = c->getSrcPort();
            SDFport* dstP = c->getDstPort();
            SDFactor* srcA = srcP->getActor();

            out << srcA->getName() << "-> output";
            out << " [ label=\"" << c->getName() << "\", taillabel=\"";
            out << srcP->getRate() << "\", headlabel=\"";
            out << dstP->getRate() << "\" ];" << std::endl;

        }
        out << "}" << std::endl;
    }
}//namespace SDF
