/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   output.h
 *
 *  Author          :   Joep van Wanrooij (j.c.p.van.wanrooij@tue.nl)
 *
 *  Date            :   July 13, 2005
 *
 *  Function        :   SDF actor
 *
 *  History         :
 *      30-01-25    :   Initial version.
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

#ifndef SDF_BASE_UNTIMED_OUTPUT_H_INCLUDED
#define SDF_BASE_UNTIMED_OUTPUT_H_INCLUDED

#include "port.h"
#include <list>
namespace SDF {

/**
 * SDFoutput
 * output in SDF graph
 */
class SDFoutput : public SDFcomponent {
public:
    // Constructor
    explicit SDFoutput(SDFcomponent &c);

    // Destructor
    ~SDFoutput() override;

    SDFoutput(const SDFoutput&) = delete;
    SDFoutput& operator=(const SDFoutput&) = delete;
    SDFoutput(SDFoutput&&) = delete;
    SDFoutput& operator=(SDFoutput&&) = delete;

    // Construct
    [[nodiscard]] virtual std::unique_ptr<SDFoutput> create(SDFcomponent &c) const;
    [[nodiscard]] virtual std::unique_ptr<SDFoutput> createCopy(SDFcomponent &c) const;
    [[nodiscard]] virtual std::unique_ptr<SDFoutput> clone(SDFcomponent &c) const;
    void construct(CNodePtr outputNode);

    // Ports
    [[nodiscard]] SDFport *getPort() const { return port.get(); }
    [[nodiscard]] SDFport *getPort(CId id);
    [[nodiscard]] SDFport *getPort(const CString &name);
    void setPort(std::unique_ptr<SDFport> p);

    // Properties
    [[nodiscard]] bool isConnected() const;

    // Graph
    [[nodiscard]] SDFgraph *getGraph() const;

    // Print
    std::ostream &print(std::ostream &out);
    friend std::ostream &operator<<(std::ostream &out, SDFoutput &a) { return a.print(out); };

private:
    // Port
    std::unique_ptr<SDFport> port;
};

using SDFoutputs = std::list<std::unique_ptr<SDFoutput>>;
using SDFoutputsIter = SDFoutputs::iterator;
using SDFoutputsCIter = SDFoutputs::const_iterator;

} // namespace SDF
#endif
