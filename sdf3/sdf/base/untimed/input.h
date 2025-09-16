/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   input.h
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

#ifndef SDF_BASE_UNTIMED_INPUT_H_INCLUDED
#define SDF_BASE_UNTIMED_INPUT_H_INCLUDED

#include "port.h"
#include <list>
namespace SDF {

/**
 * SDFinput
 * Input in SDF graph
 */
class SDFinput : public SDFcomponent {
public:
    // Constructor
    explicit SDFinput(SDFcomponent &c);

    // Destructor
    ~SDFinput() override;

    SDFinput(const SDFinput&) = delete;
    SDFinput& operator=(const SDFinput&) = delete;
    SDFinput(SDFinput&&) = delete;
    SDFinput& operator=(SDFinput&&) = delete;

    // Construct
    [[nodiscard]] virtual std::unique_ptr<SDFinput> create(SDFcomponent &c) const;
    [[nodiscard]] virtual std::unique_ptr<SDFinput> createCopy(SDFcomponent &c) const;
    [[nodiscard]] virtual std::unique_ptr<SDFinput> clone(SDFcomponent &c) const;
    void construct(CNodePtr inputNode);

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
    friend std::ostream &operator<<(std::ostream &out, SDFinput &i) { return i.print(out); };

private:
    // Port
    std::unique_ptr<SDFport> port;
};

using SDFinputs = std::list<std::unique_ptr<SDFinput>>;
using SDFinputsIter = SDFinputs::iterator;
using SDFinputsCIter = SDFinputs::const_iterator;

} // namespace SDF
#endif
