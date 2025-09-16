/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   xhtml.h
 *
 *  Author          :   Martijn Koedam (m.l.p.j.koedam@tue.nl)
 *
 *  Date            :   November 03, 2011
 *
 *  Function        :   Helper class to crease xhtml files.
 *
 *  History         :
 *      03-11-11    :   Initial version.
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
#ifndef __BASE_XHTML_H__
#define __BASE_XHTML_H__

#include "base/xml/xml.h"
/**
 * Inherit from this class to easily make a document.
 */
class Xhtml {
public:
    Xhtml();

    void setPageTitle(const CString &title);

    void setPageFooter(const CString &footer);

    virtual ~Xhtml() {
        document.reset();
    }

    // Delete copy/move constructors and assignment operators
    Xhtml(const Xhtml &) = delete;
    Xhtml &operator=(const Xhtml &) = delete;
    Xhtml(Xhtml &&) = delete;
    Xhtml &operator=(Xhtml &&) = delete;

    void write(const CString &file) { document->saveFile(file, 1); }
    void write(std::ostream &out) { document->saveFile(out, 1); }
    CString serialize() { return document->serializeFile(1); }

    [[nodiscard]] CNode *get_head_node() const { return head_node; }
    [[nodiscard]] CNode *get_body_node() const { return body_node; }

private:
    std::unique_ptr<CDoc> document;
    CNode *root;

    CNode *head_node;
    CNode *title_node;
    CNode *body_node;
    CNode *footer_node;
    CNode *heading_node;

    void addBase();
    void addBaseStylesheet();
};

#endif
