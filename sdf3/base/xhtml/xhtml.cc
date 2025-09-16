/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   xhtml.cc
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

#include "base/xml/xml.h"
#include "xhtml.h"

void Xhtml::setPageTitle(const CString &title)
{
    CSetContent(title_node, title);
    CSetContent(heading_node, title);
}
void Xhtml::setPageFooter(const CString &footer)
{
    CSetContent(footer_node, footer);
}
void Xhtml::addBaseStylesheet()
{
    CNode *base_stylesheet = CNewNode(CString("style"));
    CAddAttribute(base_stylesheet, CString("type"), CString("text/css"));
    CString sheet = CString(""
                    "body {\n"
                    "  width: 100%;\n"
                    "  margin-top:  0;\n"
                    "  margin-left: auto;\n"
                    "  margin-right: auto;\n"
                    "}\n");
    sheet += CString(""
             "div.footer { \n"
             "   margin-top: 1em;\n"
             "	padding:   .2em;\n"
             "	border-top: 1px dotted grey;\n"
             "	border-bottom: 1px dotted grey;\n"
             "   background-color: #EEEEEE;\n"
             "}\n");

    sheet += CString(""
             "div.heading{ \n"
             "	padding:   .2em;\n"
             "	border-bottom: 1px dotted grey;\n"
             "   background-color: #EEEEEE;\n"
             "   font-size: 2em;\n"
             "}\n");

    sheet += CString(""
             "div.contents{ \n"
             "	padding:   .5em;\n"
             "}\n");
    CSetContent(base_stylesheet, sheet);
    CAddNode(head_node, base_stylesheet);
}

/**
 * addBase()
 *
 * This function creates the base document structure.
 * <head> (head_node)
 *  <title></title> (title_node)
 * <head>
 * <body>
 *  <div class="content"> (body_node)
 *  </div>
 *  <div class="footer"> (footer_node)
 *  </div>
 * </body>
 */
void Xhtml::addBase()
{
    // Add head
    head_node = CNewNode(CString("head"));
    addBaseStylesheet();
    // title
    title_node = CNewNode(CString("title"));
    // default title
    CSetContent(title_node, CString("SDF3 program output"));
    CAddNode(head_node, title_node);
    CAddNode(root, head_node);

    // Add body
    CNode *body_node_real = CNewNode(CString("body"));
    // Heading.
    heading_node = CNewNode(CString("div"));
    CAddAttribute(heading_node, CString("class"), CString("heading"));
    CAddNode(body_node_real, heading_node);
    // Body diff
    body_node = CNewNode(CString("div"));
    CAddAttribute(body_node, CString("class"), CString("contents"));
    CAddNode(body_node_real, body_node);

    // add Footer
    footer_node = CNewNode(CString("div"));
    CAddAttribute(footer_node, CString("class"), CString("footer"));
    CAddNode(body_node_real, footer_node);
    // Set default footer:
    constexpr size_t buffer_size = 256;
    char buffer[buffer_size];
    time_t t_time = time(nullptr);
    const struct tm *lt = localtime(&t_time); // NOLINT(*concurrency-mt-unsafe)
    strftime(buffer, buffer_size, "SDF3 Generated: %c", lt); // NOLINT(cert-err33-c,*array-to-pointer-decay,*array-decay)
    setPageFooter(CString(buffer));// NOLINT(*array-to-pointer-decay,*array-decay)

    CAddNode(root, body_node_real);

    setPageTitle(CString("SDF3"));
}
/**
 * Xhtml()
 * Constructor. Creates a base document. (calls addBase()).
 */
Xhtml::Xhtml()
    : document(CNewDoc()),
      head_node(nullptr),
      title_node(nullptr),
      body_node(nullptr),
      footer_node(nullptr),
      heading_node(nullptr)
{
    CSetPrivateDTD(document.get(), CString("html"), CString("http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd"));
    // Create root node
    root = CNewNode(CString("html"));
    // Create body structure.
    addBase();
    // add root node.
    document->setRootNode(root);
}
