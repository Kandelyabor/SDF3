#include "xmlcompare.h"
#include <libxml/parser.h>

/*************************************************
 * class XmlCompare {
 * public:
 */

/**
 * XmlCompare(), constructor
 * @param an The root node of the first xml file to compare.
 * @param bn The root node of the second xml file to compare.
 *
 * Constructs the XmlCompare font. \a an, \a bn trees are
 * internally copied.
 */
XmlCompare::XmlCompare(CNode *node_a, CNode *node_b):
    a(CNewDoc(CCopyNode(node_a))), b(CNewDoc(CCopyNode(node_b)))
{
}

XmlCompare::~XmlCompare()
{
    a.reset();
    b.reset();
}

/**
 * compare()
 *
 * Compares the different nodes based on the content of the trees.
 * node names, attributes and contents.
 *
 * @returns true if the trees are identical.
 */
bool XmlCompare::compare()
{
    return this->getHash(a->getRootNode()) == this->getHash(b->getRootNode());
}

/**
 * purge()
 *
 * Trims the equal branches of the internal trees. (can be slow).
 * After this is done only the nodes that are different should remain.
 * This can then generate a 'diff'
 */
void XmlCompare::purge()
{
    purge_equal_parts(a->getRootNode(), b->getRootNode());
}


/**
 * ignore_paths()
 * @param xpath The XPath to ignore. (namespace is sdf3)
 *
 * The XPath marked nodes (and subnodes) are removed/ignored in the tree.
 * Run this before running compare or purge.
 * Multiple calls to this function is allowed.
 */
void XmlCompare::ignore_paths(CString& xpath)
{
    _ignore_paths(xpath, a.get());
    _ignore_paths(xpath, b.get());
}

namespace{


/*************************************************
 * private:
 */


std::size_t hash_combine(std::size_t& h, const CString& s) {
    std::size_t h2 = std::hash<std::string>{}(s);
    h = h ^ (h2 << 1); // NOLINT(*signed-bitwise)
    return h;
}

} // namespace

std::size_t XmlCompare::getSingleHash(xmlNode *a_root)
{
    // Compare all the attributes.
    std::list<std::size_t> attr_list;
    CStrings attr = CGetAttributes(a_root);
    for (const auto& name: attr)
    {
        std::size_t size = 0;
        size = 0;
        hash_combine(size, name);

        xmlChar *prop = xmlGetProp(a_root, BAD_CAST name.c_str()); // NOLINT(*-cast)
        CString temp = CString(reinterpret_cast<const char *>(prop)); // NOLINT(*-cast)
        hash_combine(size, temp);
        xmlFree(prop);
        attr_list.push_back(size);
    }
    attr_list.sort();
    std::size_t retV = 0;
    for (auto val: attr_list)
    {
        hash_combine(retV, CString(val));
    }
    // Hash magic, with thank to C++ and libxml
    std::string a = reinterpret_cast<const char *>(a_root->name); // NOLINT(*-cast)
    hash_combine(retV, CString(a));

    return retV;
}

std::size_t XmlCompare::getHash(xmlNode *a_root)
{
    if (a_root == nullptr)
    {
        return 0;
    }

    // HAsh attributes.
    // Compare all the attributes.
    std::list<std::size_t> attr_list;
    CStrings attr = CGetAttributes(a_root);
    for (const auto& name: attr)
    {
        // Create a hash of key::value pair.
        std::size_t size = 0;
        hash_combine(size, name);

        xmlChar *prop = xmlGetProp(a_root, BAD_CAST name.c_str()); // NOLINT(*-cast)
        CString temp = CString(reinterpret_cast<const char *>(prop)); // NOLINT(*-cast)
        hash_combine(size, temp);
        xmlFree(prop);
        attr_list.push_back(size);
    }

    // Hash children
    std::list<std::size_t> child_list;
    for (xmlNode *ch = CGetChildNode(a_root); ch != nullptr; ch = CNextNode(ch))
    {
        std::size_t v = getHash(ch);
        child_list.push_back(v);
    }

    attr_list.sort();
    child_list.sort();
    std::size_t retV = 0;
    for (auto val: attr_list)
    {
        hash_combine(retV, CString(val));
    }
    for (auto val: child_list)
    {
        hash_combine(retV, CString(val));
    }
    // Hash magic, with thanks to C++ and libxml
    CString a = CString(reinterpret_cast<const char *>(a_root->name)); // NOLINT(*-cast)
    hash_combine(retV, a);

    xmlChar *cont = xmlNodeGetContent(a_root);
    if (cont != nullptr)
    {
        a = CString(reinterpret_cast<const char *>(cont)); // NOLINT(*-cast)
        a.trim();
        if (!a.empty())
        {
            hash_combine(retV, a);
        }
    }
    xmlFree(cont);
    return retV;
}

void XmlCompare::_ignore_paths(CString xpath, CDoc *doc)
{
    std::list<CNode *> list = CGetNodesXPath(doc, xpath);
    for (auto *i: list)
    {
        CRemoveNode(i);
    }
}
void XmlCompare::purge_equal_parts(CNode *a_in, CNode *b_in)
{
    // Compare the children.
    CNode *child_a = CGetChildNode(a_in);
    while (child_a != nullptr)
    {
        bool found = false;
        CNode *child_b = CGetChildNode(b_in);
        for (; child_b != nullptr ; child_b = CNextNode(child_b))
        {
            if (getHash(child_a) == getHash(child_b))
            {
                found = true;
                CRemoveNode(child_b);
                break;
            }
        }
        if (!found)
        {
            child_a = CNextNode(child_a);
        }
        else
        {
            // Purge the found node from the Tree
            CRemoveNode(child_a);
            child_a = CGetChildNode(a_in);
        }
    }

    child_a = CGetChildNode(a_in);
    while (child_a != nullptr)
    {
        bool found = false;
        CNode *child_b = CGetChildNode(b_in);
        for (; !found && child_b != nullptr ; child_b = CNextNode(child_b))
        {
            if (getSingleHash(child_a) == getSingleHash(child_b))
            {
                found = true;
                purge_equal_parts(child_a, child_b);
            }
        }
        child_a = CNextNode(child_a);
    }
}
