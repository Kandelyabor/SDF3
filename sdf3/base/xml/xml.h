/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   xml.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   March 29, 2002
 *
 *  Function        :   XML interface functions
 *
 *  History         :
 *      29-03-02    :   Initial version.
 *      26-08-06    :   Additions by Bart Theelen.
 *            08    :   ported to MS compatibility by Marc Geilen
 *      11-04-11    :   Add libxml/xmlschemas.h for validation support.
 *
 * $Id: xml.h,v 1.3.2.6 2010-08-15 19:30:04 mgeilen Exp $
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

#ifndef BASE_XML_XML_H_INCLUDED
#define BASE_XML_XML_H_INCLUDED

#include "base/string/cstring.h"



#include <libxml/parser.h>
#include <libxml/xmlschemas.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

// Types
using CNode = xmlNode;
using CNodePtr = xmlNodePtr;

using CAttr = xmlAttr;
using CAttrPtr = xmlAttrPtr;

class CDoc {
public:
    explicit CDoc(xmlDoc* d) : theXmlDoc(d){};
    ~CDoc() {
        xmlFreeDoc(this->theXmlDoc);
    };

    CDoc(const CDoc&) = delete;
    CDoc& operator=(const CDoc&) = delete;
    CDoc(CDoc&&) = delete;
    CDoc& operator=(CDoc&&) = delete;


    [[nodiscard]] xmlDoc* getXmlDoc() const {return this->theXmlDoc;};

    // Set doc root node
    void setRootNode(CNode *rootNode) const;

    // Output a document to a file
    void saveFile(const CString &filename, int format = 0) const;

    // Output a document to a stream
    void saveFile(std::ostream &out, int format = 0) const;

    [[nodiscard]] CString serializeFile(int format = 0) const;

    // Get the root node of the tree
    [[nodiscard]] CNode *getRootNode() const;

    // Validate the current document.
    bool validate();


private:
    xmlDoc* theXmlDoc;
};

using CDocPtr = xmlDocPtr;

using CNodes = std::list<CNode *>;

/*
 * Functions
 */
// Open a file and parse it into a tree structure
std::unique_ptr<CDoc> CParseFile(const CString &filename);

void CSetPrivateDTD(CDoc *doc, const CString &root_element, const CString &uri);

// Create a new document
std::unique_ptr<CDoc> CNewDoc();

// Create a new document with root node
std::unique_ptr<CDoc> CNewDoc(CNode *rootNode);


// Return name of node
CString CIsNode(const CNode *n);

// Check that node is this node
bool CIsNode(const CNode *n, const CString &name);

// Return pointer to parent node
CNode *CGetParentNode(CNode *n);

// Get node with name name
CNode *CGetChildNode(CNode *n, const CString &name = "");

// Check that node has a child with name name
bool CHasChildNode(CNode *n, const CString &name = CString(""));

// Return content of node as string
CString CGetNodeContent(CNode *n);

// Add a sub-tree
CNode *CAddNode(CNode *n, const CString &name,
                const CString &content = CString(""));
CNode *CAddNode(CNode *n, const CString &name,
                double content);
CNode *CAddNode(CNode *n, CNode *child);

// Create a copy of a node (including attributes and children)
CNode *CCopyNode(CNode *n);

// Create a new node
CNode *CNewNode(const CString &name);

// Remove node and all child nodes
void CRemoveNode(CNode *n);

// Replace node with new node (old node is returned)
CNode *CReplaceNode(CNode *oldNode, CNode *newNode);

// Find node with name (search in childs of node)
CNode *CFindNode(CNode *n, const CString &name);

// Get next node on same level (if name is not nullptr,
// next node must have name 'name')
CNode *CNextNode(const CNode *n, const CString &name = CString(""));

// Get previous node on same level (if name is not nullptr,
// previous node must have name 'name')
CNode *CPreviousNode(const CNode *n, const CString &name = CString(""));

// Get value of attribute
CString CGetAttribute(CNode *n, const CString &name);

// Check that attribute is present
bool CHasAttribute(CNode *n, const CString &name);

// Add an attribute to the node
void CAddAttribute(CNode *n, const CString &name, const CString &value);

// Set or reset an attribute
void CSetAttribute(CNode *n, const CString &name, const CString &value);

// Remove attribute from node
void CRemoveAttribute(CNode *n, const CString &name);

// Set content of node
void CSetContent(CNode *n, const CString &cont);

// Get number of child nodes with name name
CId CGetNumberOfChildNodes(CNode *n, const CString &name);

// call at the end to release xml related resources.
void xmlCleanup();

// Adds a processing instuction to the document such as a stylesheet
void CAddProcessingInstruction(CDoc& doc, const CString &pi, const CString &piArgs);

CNode *CFindNodeWithAttribute(CNode *parent,
                              const CString& name,
                              const CString& field,
                              const CString& value);
// Get the attributes.
CStrings CGetAttributes(CNode *n);

CNodes CGetNodesXPath(CDoc *doc, CString& xpath);
#endif

