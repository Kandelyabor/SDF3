#ifndef __XML_COMPARE__
#define __XML_COMPARE__

#include "base/xml/xml.h"

/**
 * Compare XML file a vs b.
 * Uses: xml library (build in)
 * Uses: libxml Xpath code. (TODO: windows?)
 */

class XmlCompare
{
    private:
        std::unique_ptr<CDoc> a;
        std::unique_ptr<CDoc> b;

    public:
        XmlCompare(CNode *node_a, CNode *node_b);
        ~XmlCompare();

                // Delete copy/move constructors and assignment operators
        XmlCompare(const XmlCompare&) = delete;
        XmlCompare& operator=(const XmlCompare&) = delete;
        XmlCompare(XmlCompare&&) = delete;
        XmlCompare& operator=(XmlCompare&&) = delete;


        // Check if they are equal.
        bool compare();

        // Purges the equal parts off the tree, this way
        // you can can get a 'diff'
        void purge();

        // Pass an xpath (with namespace sdf3) to ignore some
        // parts of the xml tree.
        void ignore_paths(CString& xpath);


        // Function to get internal docs.
        // use this to print diff.
        CDoc *getDocA()
        {
            return a.get();
        }

        CDoc *getDocB()
        {
            return b.get();
        }
    private:
        void CleanupHashCode(xmlNode *c);
        void purge_equal_parts(CNode *a_in, CNode *b_in);
        static std::size_t getSingleHash(xmlNode *a_root);
        std::size_t getHash(xmlNode *a_root);
        static void _ignore_paths(CString xpath, CDoc *doc);
};

#endif
