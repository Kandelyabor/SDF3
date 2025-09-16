/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   route.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   January 6, 2006
 *
 *  Function        :   Route
 *
 *  History         :
 *      06-01-06    :   Initial version.
 *
 * $Id: route.cc,v 1.1 2008-03-20 16:16:20 sander Exp $
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

#include "route.h"
#include "node.h"
#include <algorithm>
#include <utility>

namespace SDF {
/**
 * insertLink ()
 * Insert a link after the given iterator position into the route.
 */
void Route::insertLink(LinkRefsIter pos, Link* l) { links.insert(std::move(pos), l); }

/**
 * appendLink ()
 * Insert a link at the end of the route.
 */
void Route::appendLink(Link* l) { links.push_back(l); }

/**
 * removeLink ()
 * Remove a link from the route.
 */
void Route::removeLink(Link *l) {
    for (auto iter = links.begin(); iter != links.end(); iter++) {
        if ((*iter) == l) {
            links.erase(iter);
        }
    }
}

/**
 * containsNode ()
 * The function returns true if the route goes through the node n. Otherwise it
 * returns false.
 */
bool Route::containsNode(const Node *n) const {
    return std::any_of(links.begin(), links.end(), [n](const auto& l) {
        return l->getSrcNode() == n || l->getDstNode() == n;
    });
}

/**
 * operator==
 * Two routes are equal if all links are equal.
 */
bool Route::operator==(const Route &r) const {
    LinkRefsCIter iter;
    LinkRefsCIter iterR;

    // No equal length?
    if (length() != r.length()) {
        return false;
    }

    // All links equal?
    iter = linksBegin();
    iterR = r.linksBegin();
    do {
        if (*iter != *iterR) {
            return false;
        }
        iter++;
        iterR++;
    } while (iter != linksEnd());

    return true;
}

/**
 * print ()
 * Output a route to the given stream.
 */
std::ostream &Route::print(std::ostream &out) const {
    out << "(" << (*linksBegin())->getSrcNode()->getId();
    for (const auto& l: links) {
        out << " -> " << l->getDstNode()->getId();
    }
    out << ")";

    return out;
}
} // namespace SDF