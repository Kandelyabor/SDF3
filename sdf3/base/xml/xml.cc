/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   xml.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   March 29, 2002
 *
 *  Function        :   XML interface functions
 *
 *  History         :
 *      29-03-02    :   Initial version.
 *      27-05-04    :   Changed datatypes to CAST datatypes.
 *      26-08-06    :   Additions by Bart Theelen.
 *      12-04-11    :   Add validation to libxml version.
 *
 * $Id: xml.cc,v 1.4.2.11 2010-08-15 19:30:04 mgeilen Exp $
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

#include "xml.h"
#include "base/exception/exception.h"
#include "base/log/log.h"
#include "base/tempfile/tempfile.h"
#include <cstddef>

namespace {

/**
 * Error handler
 */
constexpr size_t TMP_BUF_SIZE = 256;
void err(void * /*ctx*/, const char *msg, ...) { // NOLINT(cert-dcl50-cpp)
    char cstring[TMP_BUF_SIZE];
    int len = 0;
    va_list arg_ptr; // NOLINT(*init-variables,*-vararg)
    va_start(arg_ptr, msg);
    len = vsnprintf(cstring, TMP_BUF_SIZE, msg, arg_ptr); // NOLINT(*decay)

    // strip of trailing newline.
    if (len > 0 && cstring[len - 1] == '\n') { // NOLINT(*array-index)
        cstring[len - 1] = '\0'; // NOLINT(*array-index)
    }
    va_end(arg_ptr);
    // log the error only in debug builds.
#ifdef _DEBUG_
    logWarning(CString("XML: ") + cstring);

    // print the error to stderr
    if (ctx != nullptr) {
        std::cerr << cstring << std::endl;
    } else {
        std::cerr << "XML: " << cstring << std::endl;
    }
#endif

}

/**
 * CIsElementNode ()
 * Check if the node is an element node.
 * This check is needed if you f.e. search for a node
 * with name "text" or "comment"
 * Also iterating it is useful to skip 'special' nodes.
 */
bool CIsElementNode(CNode *n) { return (n->type == XML_ELEMENT_NODE); }

} // namespace

/**
 * validate()
 *
 * First validates using DTD the xml file, and or schema file.
 *
 * Make libxml validate the xml file using the specified schema (xi:schema)
 * Errors are printed out to stderr using the default libxml error callback.
 * validate() also reports final result to stderr.
 *
 * @return true if document is valid, false if invalid
 */
bool CDoc::validate() {

    if (this->theXmlDoc == nullptr) {
        logWarning("XML: No document specified (doc == nullptr)");
        return false;
    }

    int res = 0;
    if (xmlGetIntSubset(this->getXmlDoc()) != nullptr) {
        /* Validate DTD */
        xmlValidCtxtPtr ctx = xmlNewValidCtxt();
        if (xmlValidateDocument(ctx, this->getXmlDoc()) == 0) {
            xmlFreeValidCtxt(ctx);
            return false;
        }
        xmlFreeValidCtxt(ctx);
    }

    CNode *root = this->getRootNode();
    /* nothing to validate */
    if (root == nullptr) {
        return true;
    }

    // hack to check if there is a schema file specified.
    // This hacks avoid that there is a warning.
    xmlChar *nsl = xmlGetProp(root, BAD_CAST "noNamespaceSchemaLocation"); // NOLINT(*cstyle-cast)
    xmlChar *sl = xmlGetProp(root, BAD_CAST "schemaLocation"); // NOLINT(*cstyle-cast)
    if (nsl == nullptr && sl == nullptr) {
        return true;
    }
    if (nsl != nullptr) {
        xmlFree(nsl);
    }
    if (sl != nullptr) {
        xmlFree(sl);
    }

    /* the XSD validation context */
    xmlSchemaValidCtxtPtr validationCtxt = nullptr;

    /* A validation context created with nullptr as the
     * @schema argument will validate the instance against the schemata
     * indicated by xsi:schemaLocation and xsi:noNamespaceSchemaLocation.
     */
    validationCtxt = xmlSchemaNewValidCtxt(nullptr);
    if (validationCtxt == nullptr) {
        logWarning("XML: Failed to create the validation context.");
        return false;
    }

    /* Validate the instance. */
    res = xmlSchemaValidateDoc(validationCtxt, this->getXmlDoc());
    if (res == -1) {
        logWarning("XML: Internal error during validation.");
    }
    /* Cleanup */
    xmlSchemaFreeValidCtxt(validationCtxt);

    /* return result.*/
    return (res == 0);
}

/**
 * CParseFile ()
 * Open a file and parse it into a tree structure.
 */
std::unique_ptr<CDoc> CParseFile(const CString &filename) {
    xmlSetGenericErrorFunc(nullptr, err);
    std::unique_ptr<CDoc> doc = std::make_unique<CDoc>(xmlReadFile(filename.c_str(), nullptr, 0));

    if (doc->getXmlDoc() != nullptr) {

        if (!doc->validate()) {
            // disable validation warning, the schema files are not available
            //logWarning("The document '" + filename + "' is not valid.");
        }
    }
    return std::move(doc);
}

/**
 * CSetPrivateDTD
 *
 * Set private (SYSTEM) dtd header on document.
 */
void CSetPrivateDTD(CDoc *doc, const CString &root_element, const CString &uri) {
    xmlCreateIntSubset(
            doc->getXmlDoc(), BAD_CAST(const char *) root_element, nullptr, BAD_CAST(const char *) uri); // NOLINT(*cast*)
}
/**
 * CNewDoc ()
 * Create a new document.
 */
std::unique_ptr<CDoc> CNewDoc() {
    xmlDocPtr doc = nullptr;

    doc = xmlNewDoc(BAD_CAST "1.0"); // NOLINT(*cstyle-cast)
    return std::make_unique<CDoc>(doc);
}

/**
 * CNewDoc ()
 * Create a new document.
 */
std::unique_ptr<CDoc> CNewDoc(CNode *rootNode) {
    xmlDocPtr doc = nullptr;

    doc = xmlNewDoc(BAD_CAST "1.0"); // NOLINT(*cstyle-cast)
    xmlDocSetRootElement(doc, rootNode);

    return std::make_unique<CDoc>(doc);
}

/**
 * saveFile ()
 * Output a document to a file.
 */
void CDoc::saveFile(const CString &filename, int format) const {
    xmlSaveFormatFile(filename.c_str(), this->getXmlDoc(), format);
}

/**
 * saveFile ()
 * Output a document to a stream.
 */
void CDoc::saveFile(std::ostream &out, int format) const {
    char c = 0;

    // Temporary file
    CString tmpfile = tempFileName(CString(""), CString("sdf3"));

    // Store XML document in temp file
    xmlSaveFormatFile(tmpfile.c_str(), this->getXmlDoc(), format);

    // Read temp file to stream
    std::ifstream f(tmpfile);
    while (f.get(c) && !f.eof()) {
        out << c;
    };
    f.close();

    // Remove temporary file
    remove(tmpfile.c_str()); // NOLINT(cert-err33-c)
}
/**
 * serializeFile()
 * Serialize a XML Document to a CString
 */
CString CDoc::serializeFile(int format) const {
    xmlChar *xmlbuff = nullptr;
    int buffersize = 0;
    CString ret_v;
    xmlDocDumpFormatMemory(this->getXmlDoc(), &xmlbuff, &buffersize, format);
    ret_v = CString(reinterpret_cast<const char *>(xmlbuff)); // NOLINT(*-cast)
    xmlFree(xmlbuff);
    return ret_v;
}

/**
 * CGetRootNode ()
 * Get the root node of the document.
 */
CNode *CDoc::getRootNode() const {
    if (this->getXmlDoc()== nullptr) {
        return nullptr;
    }
    return xmlDocGetRootElement(this->getXmlDoc());
}

/**
 * CIsNode ()
 * Return name of node.
 */
CString CIsNode(const CNode *n) { return CString {(char *)n->name}; } // NOLINT(*cast*)

/**
 * CIsNode ()
 * Check that node has name 'name'.
 */
bool CIsNode(const CNode *n, const CString &name) {
    return xmlStrcmp(n->name, reinterpret_cast<const xmlChar *>(name.c_str())) == 0; // NOLINT(*-cast)
}

/**
 * CGetParentNode ()
 * Return pointer to parent of node.
 */
CNode *CGetParentNode(CNode *n) { return n->parent; }

/**
 * CGetChildNode ()
 * Return pointer to node with name 'name' (if exists).
 */
CNode *CGetChildNode(CNode *n, const CString &name) {
    if (n == nullptr) {
        return nullptr;
    }

    // Check all children of node
    for (CNode *t = n->children; t != nullptr; t = t->next) {
        // Compare name of node and searched name
        if (CIsElementNode(t) && (name.empty() || CIsNode(t, name))) {
            // Found a child with correct name
            return t;
        }
    }

    // Did not find a child with correct name
    return nullptr;
}

/**
 * CHasChildNode ()
 * Check that the node has a child with name 'name'.
 */
bool CHasChildNode(CNode *n, const CString &name) {
    if (n == nullptr) {
        return false;
    }

    // Check all children of node
    for (CNode *t = n->children; t != nullptr; t = t->next) {
        // Compare name of node and searched name
        if (CIsElementNode(t) && (name.empty() || CIsNode(t, name))) {
            // Found a child with correct name
            return true;
        }
    }

    // Did not find a child with correct name
    return false;
}

/**
 * CGetNodeContent ()
 * Return the content of a node as a string
 */
CString CGetNodeContent(CNode *n) {
    CString contentStr;
    xmlChar *cont = xmlNodeGetContent(n);
    contentStr = CString(reinterpret_cast<const char *>(cont)); // NOLINT(*-cast)
    xmlFree(cont);
    return contentStr;
}

/**
 * CAddNode ()
 * Add a child node to given node.
 */
CNode *CAddNode(CNode *n, const CString &name, const CString &content) {
    CNode *r = nullptr;

    if (n == nullptr) {
        r = xmlNewNode(nullptr, reinterpret_cast<const xmlChar *>(name.c_str())); // NOLINT(*-cast)
        xmlNodeSetContent(r, reinterpret_cast<const xmlChar *>(content.c_str())); // NOLINT(*-cast)
        return r;
    }

    // Add a child to the tree below node n
    r = xmlNewChild(n,
                    nullptr,
                    reinterpret_cast<const xmlChar *>(name.c_str()), // NOLINT(*-cast)
                    reinterpret_cast<const xmlChar *>(content.c_str())); // NOLINT(*-cast)

    return r;
}

/**
 * CAddNode ()
 * Add a child node to given node.
 */
CNode *CAddNode(CNode *n, const CString &name, const double content) {
    constexpr size_t buf_size = 256;
    // Create a buffer to hold the content
    char buf[buf_size];

    // Convert content to string
    sprintf(buf, "%.10e", content); // NOLINT(cert-err33-c,*vararg,*decay)

    // Add node
    return CAddNode(n, name, CString(buf)); // NOLINT(*decay)
}

/**
 * CAddNode ()
 * Add a child node to given node.
 */
CNode *CAddNode(CNode *n, CNode *child) { return xmlAddChild(n, child); }

/**
 * CCopyNode ()
 * Create a copy of a node (including attributes and children).
 */
CNode *CCopyNode(CNode *n) { return xmlCopyNode(n, 1); }

/**
 * CNewNode ()
 * Create a new node.
 */
CNode *CNewNode(const CString &name) {
    return xmlNewNode(nullptr, reinterpret_cast<const xmlChar *>(name.c_str())); // NOLINT(*-cast)
}

/**
 * CRemoveNode ()
 * Remove a node and its children from the database.
 */
void CRemoveNode(CNode *n) {
    // Unlink the node from the tree
    xmlUnlinkNode(n);

    // Free the memory for the subtree starting in this node
    xmlFreeNode(n);
}

/**
 * CReplaceNode ()
 * Replace node with new node (old node is returned).
 */
CNode *CReplaceNode(CNode *oldNode, CNode *newNode) { return xmlReplaceNode(oldNode, newNode); }

/**
 * CFindNode ()
 * Get a child node with name 'name'. The whole subtree is searched for
 * the node and not only the direct children as with the getNode function.
 */
CNode *CFindNode(CNode *n, const CString &name) {
    CNode *node = nullptr;

    if (n == nullptr) {
        return nullptr;
    }

    // Try to get node at this level
    node = CGetChildNode(n, name);

    // End of recursion
    if (node != nullptr) {
        return node;
    }

    // Recursion - search the children
    for (CNode *t = n->children; t != nullptr && node == nullptr; t = t->next) {
        if (CIsElementNode(t)) {
            node = CFindNode(t, name);
        }
    }

    return node;
}

/**
 * CNextNode ()
 * Return pointer to next node on same level. If name is not nullptr, next
 * node must have name 'name'.
 */
CNode *CNextNode(const CNode *n, const CString &name) {
    CNode *t = nullptr;

    for (t = n->next; t != nullptr; t = t->next) {
        if (CIsElementNode(t) && (name.empty() || CIsNode(t, name))) {
            return t;
        }
    }

    return nullptr;
}

/**
 * CPreviousNode ()
 * Return pointer to previous node on same level. If name is not nullptr, previous
 * node must have name 'name'.
 */
CNode *CPreviousNode(const CNode *n, const CString &name) {
    CNode *t = nullptr;

    for (t = n->prev; t != nullptr; t = t->prev) {
        if (CIsElementNode(t) && (name.empty() || CIsNode(t, name))) {
            return t;
        }
    }
    return nullptr;
}

/**
 * CGetAttribute ()
 * Get the value of an attribute.
 */
CString CGetAttribute(CNode *n, const CString &name) {
    if (CHasAttribute(n, name)) {
        xmlChar *prop = xmlGetProp(n, reinterpret_cast<const xmlChar *>(name.c_str())); // NOLINT(*-cast)
        CString attr = CString(reinterpret_cast<const char *>(prop)); // NOLINT(*-cast)
        xmlFree(prop);
        return attr;
    }
    std::cerr << CIsNode(n) << " " << name << std::endl;
    throw CException(CString("(CGetAttribute) Node does not have requested attribute."));

    return "";
}

/**
 * CHasAttribute ()
 * Check that attribute is present.
 */
bool CHasAttribute(CNode *n, const CString &name) {
    xmlAttr *attr = nullptr;

    // Find the attribute
    attr = xmlHasProp(n, reinterpret_cast<const xmlChar *>(name.c_str())); // NOLINT(*-cast)

    // Check attribute
    return attr != nullptr;
}

/**
 * CAddAttribute ()
 * Add an attribute to the node.
 */
void CAddAttribute(CNode *n, const CString &name, const CString &value) {
    // Check that attribute is already present
    if (CHasAttribute(n, name)) {
        // Update existing attribute
        xmlSetProp(n,
                   reinterpret_cast<const xmlChar *>(name.c_str()), // NOLINT(*-cast)
                   reinterpret_cast<const xmlChar *>(value.c_str())); // NOLINT(*-cast)
    } else {
        // Create new attribute
        xmlNewProp(n,
                   reinterpret_cast<const xmlChar *>(name.c_str()), // NOLINT(*-cast)
                   reinterpret_cast<const xmlChar *>(value.c_str())); // NOLINT(*-cast)
    }
}

/**
 * CSetAttribute ()
 * Set or reset an attribute of the node.
 */
void CSetAttribute(CNode *n, const CString &name, const CString &value) {
    xmlSetProp(n,
               reinterpret_cast<const xmlChar *>(name.c_str()), // NOLINT(*-cast)
               reinterpret_cast<const xmlChar *>(value.c_str())); // NOLINT(*-cast)
}

/**
 * CRemoveAttribute ()
 * Remove attribute from node.
 */
void CRemoveAttribute(CNode *n, const CString &name) {
    xmlAttr *prop = nullptr;

    // Find the attribute
    prop = xmlHasProp(n, reinterpret_cast<const xmlChar *>(name.c_str())); // NOLINT(*-cast)

    if (prop == nullptr) {
        return;
    }

    // Remove the attribute
    xmlRemoveProp(prop);
}

/**
 * CSetContent ()
 * Set content of node.
 */
void CSetContent(CNode *n, const CString &cont) {
    xmlNodeAddContent(n, reinterpret_cast<const xmlChar *>(cont.c_str())); // NOLINT(*-cast)
}

/**
 * CGetNumberOfChildNodes ()
 * Return the number of child nodes with name 'name'.
 */
CId CGetNumberOfChildNodes(CNode *n, const CString &name) {
    CId NumberOfChildNodes = 0;

    if (n != nullptr) {
        for (CNode *t = n->children; t != nullptr; t = t->next) {
            if (CIsElementNode(t) && (name.empty() || CIsNode(t, name))) {
                NumberOfChildNodes++;
            }
        }
    }

    return NumberOfChildNodes;
}

CStrings CGetAttributes(CNode *n) {
    CStrings retV;
    if (n != nullptr) {
        for (xmlAttr *attr = n->properties; attr != nullptr; attr = attr->next) {
            retV.emplace_back(reinterpret_cast<const char *>(attr->name)); // NOLINT(*-cast)
        }
    }
    return retV;
}

/**
 * Create a processing instruction.
 */
void CAddProcessingInstruction(CDoc& doc, const CString &pi, const CString &piArgs) {
    xmlNodePtr newPI = xmlNewDocPI(doc.getXmlDoc(), (xmlChar *)pi.c_str(), (xmlChar *)piArgs.c_str()); // NOLINT(*cast*)
    xmlNodePtr rootNode = xmlDocGetRootElement(doc.getXmlDoc());
    xmlAddPrevSibling(rootNode, newPI);
}

/**
 * setRootNode ()
 * Set the document root node
 */
void CDoc::setRootNode(CNode *rootNode) const {
    xmlDocSetRootElement(this->getXmlDoc(), rootNode);
}

/**
 * xmlCleanup()
 * call at the end to release xml related resources.
 */
void xmlCleanup() { xmlCleanupParser(); }

/**
 * CGetNodesXPath()
 * @param CDoc The document to apply the xpath on.
 * @param xpath The query.
 *
 * @returns a list of CNode * matching the path.
 * @throws a CException on error.
 */
CNodes CGetNodesXPath(CDoc *doc, CString &xpath) {
    CNodes retV;
    int size = 0;
    xmlXPathContextPtr xpathCtx = nullptr;
    xmlXPathObjectPtr xpathObj = nullptr;
    /* Create xpath evaluation context */
    xpathCtx = xmlXPathNewContext(doc->getXmlDoc());
    if (xpathCtx == nullptr) {
        throw CException(CString(__FUNCTION__) + ": unable to create new XPath context.");
        return retV;
    }
    // Todo: Load this from the file?
    /// @TODO: This is hardcoded now? how to properly fix it.
    if (xmlXPathRegisterNs(xpathCtx, BAD_CAST "sdf3", BAD_CAST "uri:sdf3") != 0) { // NOLINT(*-cast)
        throw CException(CString(__FUNCTION__) + ": Unable to register NS with prefix");
        return retV;
    }
    /* Evaluate xpath expression */
    xpathObj = xmlXPathEvalExpression(reinterpret_cast<const xmlChar *>(xpath.c_str()), xpathCtx); // NOLINT(*-cast)
    if (xpathObj == nullptr) {
        throw CException(CString(__FUNCTION__) + ": unable to evaluate xpath expression: " + xpath);
        xmlXPathFreeContext(xpathCtx);
        return retV;
    }
    if (xmlXPathNodeSetIsEmpty(xpathObj->nodesetval)) {
        xmlXPathFreeObject(xpathObj);
        xmlXPathFreeContext(xpathCtx);
        // No results
        return retV;
    }
    xmlNodeSet *nodes = xpathObj->nodesetval;
    size = nodes != nullptr ? nodes->nodeNr : 0;
    //
    // NOTE: the nodes are processed in reverse order, i.e. reverse document
    //       order because xmlNodeSetContent can actually free up descendant
    //       of the node and such nodes may have been selected too ! Handling
    //       in reverse order ensure that descendant are accessed first, before
    //       they get removed. Mixing XPath and modifications on a tree must be
    //       done carefully !
    //
    for (int i = size - 1; i >= 0; i--) {
        CNode *node = nodes->nodeTab[i];
        if (node->type == XML_ELEMENT_NODE) {
            retV.push_back(node);
        }
    }

    // Cleanup of XPath data
    xmlXPathFreeObject(xpathObj);
    xmlXPathFreeContext(xpathCtx);

    // Return the list.
    return retV;
}

/**
 * Find a node with name %name under parent where attribute %field == %value
 *
 * @param parent the parent #CNode
 * @param name the name of the node to find.
 * @param field the name of the attribute to mach
 * @param value the value of the attribute.
 * @return a CNode or an exception if not found.
 */
CNode *CFindNodeWithAttribute(CNode *parent, const CString& name, const CString& field, const CString& value) {
    CNode *node = CFindNode(parent, name);

    for (; node != nullptr; node = CNextNode(node, name)) {
        if (CHasAttribute(node, field)) {
            CString node_id = CGetAttribute(node, field);

            if (node_id == value) {
                return node;
            }
        }
    }
    throw CException(CString(CString(__FUNCTION__) + ": No node '" + name + "' found where " + field
                             + "==" + value));
}
