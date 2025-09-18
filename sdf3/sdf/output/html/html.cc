/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   html.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   March 9, 2008
 *
 *  Function        :   Output SDF graph, architecture, mapping, etcetera in
 *                      HTML format.
 *
 *  History         :
 *      09-03-08    :   Initial version.
 *
 * $Id: html.cc,v 1.5.2.1 2009-03-23 08:07:16 sander Exp $
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

#include "html.h"
#include "sdf/output/dot/dot.h" // IWYU pragma: keep
namespace SDF {

/**
 * printStylesheet ()
 * Write the stylesheet to the supplied output stream.
 */
void SDFconvertToHTML::printStylesheet(std::ostream &out) {
    out << "        #header a {" << std::endl;
    out << "            text-decoration: none;" << std::endl;
    out << "            color: #6889b5;" << std::endl;
    out << "            font-size: 20pt;" << std::endl;
    out << "            font-weight: bold;" << std::endl;
    out << "        }" << std::endl;
    out << "        " << std::endl;
    out << "        #nav-main {" << std::endl;
    out << "            background-color: #4f6889;" << std::endl;
    out << "            width: 100%;" << std::endl;
    out << "            font-weight: bold;" << std::endl;
    out << "            font-family: 'Luxi Sans', verdana, helvetica, arial, sans-serif;"
        << std::endl;
    out << "            font-size: x-small;" << std::endl;
    out << "            margin-top: 5px;" << std::endl;
    out << "        }" << std::endl;
    out << "        " << std::endl;
    out << "        #nav-main-list {" << std::endl;
    out << "            height: 20px; /* size of navigation bar */" << std::endl;
    out << "            list-style: none;" << std::endl;
    out << "            margin: 0px;" << std::endl;
    out << "            padding: 0px;" << std::endl;
    out << "        }" << std::endl;
    out << "        " << std::endl;
    out << "        #nav-main-list li {" << std::endl;
    out << "            float: left;" << std::endl;
    out << "            display: inline;" << std::endl;
    out << "        }" << std::endl;
    out << "        " << std::endl;
    out << "        #nav-main-list a {" << std::endl;
    out << "            color: #fff;" << std::endl;
    out << "            text-decoration: none;" << std::endl;
    out << "            vertical-align: middle;" << std::endl;
    out << "            padding-left: 10px;" << std::endl;
    out << "            padding-right: 10px;" << std::endl;
    out << "            line-height: 20px;" << std::endl;
    out << "            display: block;" << std::endl;
    out << "        }" << std::endl;
    out << "         " << std::endl;
    out << "        #nav-main-list a:hover {" << std::endl;
    out << "            background-color: #7ca3d6;" << std::endl;
    out << "        } " << std::endl;
    out << "        " << std::endl;
    out << "        #nav-main-list li strong{" << std::endl;
    out << "            background-color: #6889b5;" << std::endl;
    out << "            display: block;" << std::endl;
    out << "        } " << std::endl;
    out << "        " << std::endl;
    out << "        * html .nav-corner-br {" << std::endl;
    out << "            margin-top: -12px;" << std::endl;
    out << "        }" << std::endl;
    out << "        " << std::endl;
    out << "        * html .nav-corner-bl {" << std::endl;
    out << "            margin-top: -12px;" << std::endl;
    out << "        }" << std::endl;
    out << "        " << std::endl;
    out << "        body {" << std::endl;
    out << "            margin: 0;" << std::endl;
    out << "            padding: 0;" << std::endl;
    out << "            text-align: center;" << std::endl;
    out << "            font-family: 'Luxi Sans', 'Bitstream Vera Sans', ";
    out << "'Lucida Grande', 'Trebuchet MS', helvetica, verdana, arial, sans-serif;" << std::endl;
    out << "            font-size: small;" << std::endl;
    out << "            color: #333;" << std::endl;
    out << "        }" << std::endl;
    out << "        " << std::endl;
    out << "        #mainone {" << std::endl;
    out << "            min-width: 720px;" << std::endl;
    out << "            padding-left: 30px;" << std::endl;
    out << "            padding-right: 30px;" << std::endl;
    out << "            margin-left: auto;" << std::endl;
    out << "            margin-right: auto;" << std::endl;
    out << "            text-align: left;" << std::endl;
    out << "        }" << std::endl;
    out << "        " << std::endl;
    out << "        #site-left {" << std::endl;
    out << "            width: 120px;" << std::endl;
    out << "            float: left;" << std::endl;
    out << "            margin-right: 20px;" << std::endl;
    out << "        }" << std::endl;
    out << "        " << std::endl;
    out << "        #site-middle-two {" << std::endl;
    out << "            margin-left: 140px;  /*width of site-left block */" << std::endl;
    out << "        }" << std::endl;
    out << "        " << std::endl;
    out << "        #content {" << std::endl;
    out << "            margin: 10px;" << std::endl;
    out << "        }" << std::endl;
    out << "        " << std::endl;
    out << "        img {" << std::endl;
    out << "            border: 0px;" << std::endl;
    out << "        }" << std::endl;
    out << "        " << std::endl;
    out << "        div.center {" << std::endl;
    out << "            text-align: center;" << std::endl;
    out << "        }" << std::endl;
    out << "        " << std::endl;
    out << "        .command {" << std::endl;
    out << "            font-family: 'Courier';" << std::endl;
    out << "        }" << std::endl;
    out << "        " << std::endl;
    out << "        div.command {" << std::endl;
    out << "            margin-top: 10px;" << std::endl;
    out << "            margin-bottom: 10px;" << std::endl;
    out << "            white-space: pre;" << std::endl;
    out << "        }" << std::endl;
    out << "        " << std::endl;
    out << "        div.example {" << std::endl;
    out << "            display: block;" << std::endl;
    out << "            padding: 10px;" << std::endl;
    out << "            border: 1px solid #6666cc;" << std::endl;
    out << "            color: #000;" << std::endl;
    out << "            overflow: auto;" << std::endl;
    out << "            margin: 1em 2em;" << std::endl;
    out << "        }" << std::endl;
    out << "        " << std::endl;
    out << "        code.screen, pre.screen {" << std::endl;
    out << "            font-family: 'Courier';" << std::endl;
    out << "            display: block;" << std::endl;
    out << "            padding: 10px;" << std::endl;
    out << "            border: 1px solid #bbb;" << std::endl;
    out << "            background-color: #eee;" << std::endl;
    out << "            color: #000;" << std::endl;
    out << "            overflow: auto;" << std::endl;
    out << "            margin: 1em 2em;" << std::endl;
    out << "        }" << std::endl;
    out << "        " << std::endl;
    out << "        pre {" << std::endl;
    out << "            font-size: 10pt;" << std::endl;
    out << "        }" << std::endl;
    out << "        " << std::endl;
    out << "        img.right {" << std::endl;
    out << "            display: block;" << std::endl;
    out << "            float: right;" << std::endl;
    out << "            margin: 10px;" << std::endl;
    out << "        }" << std::endl;
    out << "        " << std::endl;
    out << "        img.left {" << std::endl;
    out << "            display: block;" << std::endl;
    out << "            float: left;" << std::endl;
    out << "            margin: 10px;" << std::endl;
    out << "        }" << std::endl;
    out << "        " << std::endl;
    out << "        p.right {" << std::endl;
    out << "            text-align: right;" << std::endl;
    out << "        }" << std::endl;
    out << "        " << std::endl;
    out << "        div.caution {" << std::endl;
    out << "            display: block;" << std::endl;
    out << "            padding: 10px;" << std::endl;
    out << "            border: 1px solid #6666cc;" << std::endl;
    out << "            background-color: #ccccff;" << std::endl;
    out << "            color: #000;" << std::endl;
    out << "            overflow: auto;" << std::endl;
    out << "            margin: 1em 2em;" << std::endl;
    out << "        }" << std::endl;
    out << "        " << std::endl;
    out << "        div.info {" << std::endl;
    out << "            display: block;" << std::endl;
    out << "            padding: 10px;" << std::endl;
    out << "            border: 1px solid #6666cc;" << std::endl;
    out << "            background-color: #ccccff;" << std::endl;
    out << "            color: #000;" << std::endl;
    out << "            overflow: auto;" << std::endl;
    out << "            margin: 1em 2em;" << std::endl;
    out << "        }" << std::endl;
    out << "        " << std::endl;
    out << "        th {" << std::endl;
    out << "            color: #4f6889;" << std::endl;
    out << "        }" << std::endl;
    out << "        " << std::endl;
    out << "        .header {" << std::endl;
    out << "            color: #4f6889;" << std::endl;
    out << "        }" << std::endl;
    out << "        " << std::endl;
    out << "        ul.separate li {" << std::endl;
    out << "            margin-bottom: 10px;" << std::endl;
    out << "        }" << std::endl;
    out << "        " << std::endl;
    out << "        ol.separate li {" << std::endl;
    out << "            margin-bottom: 10px;" << std::endl;
    out << "        }" << std::endl;
    out << "        " << std::endl;
    out << "        li.obsolete {" << std::endl;
    out << "            color: #aaa;" << std::endl;
    out << "        }" << std::endl;
    out << "        " << std::endl;
    out << "        div.tabs {" << std::endl;
    out << "            width: 100%;" << std::endl;
    out << "            height: 20px; /* size of navigation bar */" << std::endl;
    out << "            list-style: none;" << std::endl;
    out << "            margin: 0px;" << std::endl;
    out << "            padding: 0px;" << std::endl;
    out << "            " << std::endl;
    out << "        }" << std::endl;
    out << "        " << std::endl;
    out << "        div.tabs ul {" << std::endl;
    out << "            margin: 0px;" << std::endl;
    out << "            padding: 0px;" << std::endl;
    out << "        }" << std::endl;
    out << "        " << std::endl;
    out << "        div.tabs li {" << std::endl;
    out << "            float: left;" << std::endl;
    out << "            display: inline;" << std::endl;
    out << "        }" << std::endl;
    out << "        " << std::endl;
    out << "        div.tabs a {" << std::endl;
    out << "            color: #6f6f6f;" << std::endl;
    out << "            text-decoration: none;" << std::endl;
    out << "            padding-left: 0px;" << std::endl;
    out << "            padding-right: 10px;" << std::endl;
    out << "            line-height: 20px;" << std::endl;
    out << "            display: block;" << std::endl;
    out << "        }" << std::endl;
    out << "         " << std::endl;
    out << "        div.tabs a:hover {" << std::endl;
    out << "            text-decoration: underline;" << std::endl;
    out << "        } " << std::endl;
    out << "        " << std::endl;
    out << "        #current{" << std::endl;
    out << "            font-weight: bold;" << std::endl;
    out << "            display: block;" << std::endl;
    out << "        } " << std::endl;
    out << "        " << std::endl;
    out << "        .center{" << std::endl;
    out << "            text-align: center;" << std::endl;
    out << "        } " << std::endl;
    out << "        td {" << std::endl;
    out << "            padding-right: 30px;" << std::endl;
    out << "        }" << std::endl;
    out << "        " << std::endl;
    out << "        th {" << std::endl;
    out << "            padding-right: 30px;" << std::endl;
    out << "        }" << std::endl;
    out << "        .footer {" << std::endl;
    out << "            font-size: x-small;" << std::endl;
    out << "            color: #777;" << std::endl;
    out << "            padding-top: 50px;" << std::endl;
    out << "            padding-bottom: 30px;" << std::endl;
    out << "            clear: both;" << std::endl;
    out << "        }" << std::endl;
    out << "        " << std::endl;
    out << "        a.footer {" << std::endl;
    out << "            text-decoration: none;" << std::endl;
    out << "            color: #666699;" << std::endl;
    out << "        }" << std::endl;
    out << "        " << std::endl;
    out << "        a.footer:visited {" << std::endl;
    out << "            color: #666699;" << std::endl;
    out << "        }" << std::endl;
    out << "        " << std::endl;
    out << "        a.footer:hover {" << std::endl;
    out << "            text-decoration: underline;" << std::endl;
    out << "            color: #666699;" << std::endl;
    out << "        }" << std::endl;
}

/**
 * printHeader ()
 * Write the header of the HTML file to the supplied output stream.
 */
void SDFconvertToHTML::printHeader(std::ostream &out, const CString &title) {
    out << "<!DOCTYPE html PUBLIC '-//W3C//DTD XHTML 1.0 Strict//EN' ";
    out << "'http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd'>" << std::endl;
    out << "<html xmlns='http://www.w3.org/1999/xhtml'>" << std::endl;
    out << "<head>" << std::endl;
    out << "    <title>SDF3 :: " << title << "</title>" << std::endl;
    out << "    <meta http-equiv='Content-type' content='text/html;charset=iso-8859-1' />"
        << std::endl;
    out << "    <style>";
    printStylesheet(out);
    out << "    </style>" << std::endl;
    out << "</head>" << std::endl;
    out << "<body>" << std::endl;
    out << "<a name='top'/>" << std::endl;
    out << "<div id='mainone'>" << std::endl;
    out << std::endl;
    out << "    <!-- header -->" << std::endl;
    out << "    <div id='header'>" << std::endl;
    out << "        <a href='https://www.es.ele.tue.nl/sdf3'>SDF3</a>" << std::endl;
    out << "    </div>" << std::endl;
    out << std::endl;
    out << "    <!-- navigation -->" << std::endl;
    out << "    <div id='nav-main'>" << std::endl;
    out << "        <ul id='nav-main-list'>" << std::endl;
    out << "            <li><a href='" << getFullName(sdfGraphFilename, "html");
    out << "'>Application</a></li>" << std::endl;

    if (getPlatformGraph() != nullptr) {
        out << "            <li><a href='";
        out << getFullName(platformGraphFilename, "html");
        out << "'>Platform</a></li>" << std::endl;
        out << "            <li><a href='";
        out << getFullName(platformMappingFilename, "html");
        out << "'>Platform Mapping</a></li>" << std::endl;
        out << "            <li><a href='";
        out << getFullName(platformUsageFilename, "html");
        out << "'>Platform Usage</a></li>" << std::endl;
    }

    if (getSetOfNoCScheduleProblems() != nullptr) {
        out << "            <li><a href='";
        out << getFullName(interconnectGraphFilename, "html");
        out << "'>Interconnect</a></li>" << std::endl;
        out << "            <li><a href='";
        out << getFullName(interconnectMappingFilename, "html");
        out << "'>Interconnect Mapping</a></li>" << std::endl;
        out << "            <li><a href='";
        out << getFullName(interconnectUsageFilename, "html");
        out << "'>Interconnect Usage</a></li>" << std::endl;
    }

    out << "        </ul>" << std::endl;
    out << "    </div>" << std::endl;
    out << std::endl;
    out << "    <div id='content'>";
}

/**
 * footerHeader ()
 * Write the footer of the HTML file to the supplied output stream.
 */
void SDFconvertToHTML::printFooter(std::ostream &out) {
    out << "    </div>" << std::endl;
    out << "</div>" << std::endl;
    out << "</body>" << std::endl;
    out << "</html>" << std::endl;
}

// NOLINTBEGIN(concurrency-mt-unsafe,cert-env33-c)

/**
 * convertSDFgraphToPNG ()
 * Convert the graph using dot to a PNG image. The function returns an image
 * map in XHTML format.
 */
CString SDFconvertToHTML::convertSDFgraphToPNG(TimedSDFgraph *g, const CString &filename) {
    CString tmpFilename;
    CString map;
    std::ofstream tmpFile;
    std::ifstream mapFile;

    // Temporary file
    tmpFilename = tempFileName("", "sdf3");
    tmpFile.open(tmpFilename);

    // Convert graph to dot format
    outputSDFgraphAsDot(g, tmpFile);

    // Close the temporary file
    tmpFile.close();

    // Run dot to convert the picture
    if (system((CString("dot -Tpng -o ") + filename + CString(" ") + tmpFilename).c_str()) != 0) {
        throw CException("Failed running dot.");
    }

    // Run dot to convert the picture
    if (system(("dot -Tcmapx -o " + tmpFilename + ".map " + tmpFilename).c_str())
        != 0) { // NOLINT(concurrency-mt-unsafe,cert-env33-c)
        throw CException("Failed running dot.");
    }

    // Read the image map from file
    mapFile.open(tmpFilename + ".map");

    // Read map from file
    while (!mapFile.eof()) {
        char c = 0;
        mapFile.get(c);

        if (!mapFile.eof()) {
            map += c;
        }
    }

    // Close the map
    mapFile.close();

    // Remove temporary file
    if (remove(tmpFilename.c_str()) != 0) {
        throw CException("Failed to remove temporary file: " + tmpFilename);
    }
    if (remove((tmpFilename + ".map").c_str()) != 0) {
        throw CException("Failed to remove temporary file: " + tmpFilename + ".map");
    }

    // Done
    return map;
}

/**
 * convertPlatformGraphToPNG ()
 * Convert the graph using dot to a PNG image.
 */
CString SDFconvertToHTML::convertPlatformGraphToPNG(PlatformGraph *g, const CString &filename) {
    CString tmpFilename;
    CString map;
    std::ofstream tmpFile;
    std::ifstream mapFile;

    // Temporary file
    tmpFilename = tempFileName("", "sdf3");
    tmpFile.open(tmpFilename);

    // Convert graph to dot format
    outputPlatformGraphAsDot(g, tmpFile);

    // Close the temporary file
    tmpFile.close();

    // Run dot to convert the picture
    if (system(("dot -Tpng -o " + filename + " " + tmpFilename).c_str())
        != 0) { // NOLINT(concurrency-mt-unsafe,cert-env33-c)
        throw CException("Failed running dot.");
    }

    // Run dot to convert the picture
    if (system(("dot -Tcmapx -o " + tmpFilename + ".map " + tmpFilename).c_str())
        != 0) { // NOLINT(concurrency-mt-unsafe,cert-env33-c)
        throw CException("Failed running dot.");
    }

    // Read the image map from file
    mapFile.open(tmpFilename + ".map");

    // Read map from file
    while (!mapFile.eof()) {
        char c = 0;
        mapFile.get(c);

        if (!mapFile.eof()) {
            map += c;
        }
    }

    // Close the map
    mapFile.close();

    // Remove temporary file
    if (remove(tmpFilename.c_str()) != 0) {
        throw CException("Failed to remove temporary file: " + tmpFilename);
    }
    if (remove((tmpFilename + ".map").c_str()) != 0) {
        throw CException("Failed to remove temporary file: " + tmpFilename + ".map");
    }

    // Done
    return map;
}

/**
 * convertPlatformMappingToPNG ()
 * Convert the mapping using dot to a PNG image.
 */
CString SDFconvertToHTML::convertPlatformMappingToPNG(TimedSDFgraph *sdfGraph,
                                                      PlatformGraph *platformGraph,
                                                      const CString &filename) {
    CString tmpFilename;
    CString map;
    std::ofstream tmpFile;
    std::ifstream mapFile;

    // Temporary file
    tmpFilename = tempFileName("", "sdf3");
    tmpFile.open(tmpFilename);

    // Convert graph to dot format
    outputPlatformMappingAsDot(sdfGraph, platformGraph, tmpFile);

    // Close the temporary file
    tmpFile.close();

    // Run dot to convert the picture
    if (system(("dot -Tpng -o " + filename + " " + tmpFilename).c_str())
        != 0) { // NOLINT(concurrency-mt-unsafe,cert-env33-c)
        throw CException("Failed running dot.");
    }

    // Run dot to convert the picture
    if (system(("dot -Tcmapx -o " + tmpFilename + ".map " + tmpFilename).c_str())
        != 0) { // NOLINT(concurrency-mt-unsafe,cert-env33-c)
        throw CException("Failed running dot.");
    }

    // Read the image map from file
    mapFile.open(tmpFilename + ".map");

    // Read map from file
    while (!mapFile.eof()) {
        char c = 0;
        mapFile.get(c);

        if (!mapFile.eof()) {
            map += c;
        }
    }

    // Close the map
    mapFile.close();

    // Remove temporary file
    if (remove(tmpFilename.c_str()) != 0) {
        throw CException("Failed to remove temporary file: " + tmpFilename);
    }
    if (remove((tmpFilename + ".map").c_str()) != 0) {
        throw CException("Failed to remove temporary file: " + tmpFilename + ".map");
    }

    // Done
    return map;
}

/**
 * convertInterconnectGraphToPNG ()
 * Convert the graph using dot to a PNG image. The function returns an image
 * map in XHTML format.
 */
CString SDFconvertToHTML::convertInterconnectGraphToPNG(InterconnectGraph *g,
                                                        const CString &filename) {
    CString tmpFilename;
    CString map;
    std::ofstream tmpFile;
    std::ifstream mapFile;

    // Temporary file
    tmpFilename = tempFileName("", "sdf3");
    tmpFile.open(tmpFilename);

    // Convert graph to dot format
    outputInterconnectGraphAsDot(g, tmpFile);

    // Close the temporary file
    tmpFile.close();

    // Run dot to convert the picture
    if (system(("dot -Tpng -o " + filename + " " + tmpFilename).c_str())
        != 0) { // NOLINT(concurrency-mt-unsafe,cert-env33-c)
        throw CException("Failed running dot.");
    }

    // Run dot to convert the picture
    if (system(("dot -Tcmapx -o " + tmpFilename + ".map " + tmpFilename).c_str())
        != 0) { // NOLINT(concurrency-mt-unsafe,cert-env33-c)
        throw CException("Failed running dot.");
    }

    // Read the image map from file
    mapFile.open(tmpFilename + ".map");

    // Read map from file
    while (!mapFile.eof()) {
        char c = 0;
        mapFile.get(c);

        if (!mapFile.eof()) {
            map += c;
        }
    }

    // Close the map
    mapFile.close();

    // Remove temporary file
    if (remove(tmpFilename.c_str()) != 0) {
        throw CException("Failed to remove temporary file: " + tmpFilename);
    }
    if (remove((tmpFilename + ".map").c_str()) != 0) {
        throw CException("Failed to remove temporary file: " + tmpFilename + ".map");
    }

    // Done
    return map;
}

// NOLINTEND(concurrency-mt-unsafe,cert-env33-c)

/**
 * outputSDFgraphAsDot ()
 * The function outputs an SDF graph in DOT format.
 */
void SDFconvertToHTML::outputSDFgraphAsDot(SDFgraph *g, std::ostream &out) {
    out << "digraph " << g->getName() << " {" << std::endl;
    out << "    size=\"7,10\";" << std::endl;

    // Output all actors
    for (const auto &a : g->getActors()) {
        out << "    " << a->getName() << " [ label=\"" << a->getName();
        out << "\", href=\"#actor_" << a->getName() << "\" ];" << std::endl;
    }
    out << std::endl;

    // Output all channels
    for (const auto &c : g->getChannels()) {
        // Initial tokens on channel?
        if (c->getInitialTokens() != 0) {
            SDFport *srcP = c->getSrcPort();
            SDFactor *srcA = srcP->getActor();
            SDFport *dstP = c->getDstPort();
            SDFactor *dstA = dstP->getActor();

            out << "    " << srcA->getName() << " -> " << dstA->getName();
            out << " [ label=\"" << c->getName() << "(";
            out << c->getInitialTokens() << ")"
                << "\", taillabel=\"";
            out << srcP->getRate() << "\", headlabel=\"";
            out << dstP->getRate() << "\" href=\"#channel_" << c->getName();
            out << "\" ];" << std::endl;
        } else {
            SDFport *srcP = c->getSrcPort();
            SDFactor *srcA = srcP->getActor();
            SDFport *dstP = c->getDstPort();
            SDFactor *dstA = dstP->getActor();

            out << "    " << srcA->getName() << " -> " << dstA->getName();
            out << " [ label=\"" << c->getName() << "\", taillabel=\"";
            out << srcP->getRate() << "\", headlabel=\"";
            out << dstP->getRate() << "\" href=\"#channel_" << c->getName();
            out << "\" ];" << std::endl;
        }
    }

    out << "}" << std::endl;
}

/**
 * outputPlatformGraphAsDot ()
 * The function outputs a platform graph in DOT format.
 */
void SDFconvertToHTML::outputPlatformGraphAsDot(PlatformGraph *g, std::ostream &out) {
    out << "digraph " << g->getName() << " {" << std::endl;
    out << "    size=\"7,10\";" << std::endl;

    // Output all tiles
    for (const auto &t : g->getTiles()) {
        out << "    " << t->getName() << " [ label=\"" << t->getName();
        out << R"(", shape="box", href="#tile_)" << t->getName() << "\" ];";
        out << std::endl;
    }
    out << std::endl;

    // Output all connections
    for (const auto &c : g->getConnections()) {
        out << "    " << c->getSrcTile()->getName() << " -> ";
        out << c->getDstTile()->getName();
        out << " [ label=\"" << c->getName() << "\", href=\"#connection_";
        out << c->getName() << "\" ];" << std::endl;
    }
    out << std::endl;

    out << "}" << std::endl;
}

/**
 * outputPlatformMappingAsDot ()
 * The function outputs a mapping in DOT format.
 */
void SDFconvertToHTML::outputPlatformMappingAsDot(TimedSDFgraph *sdfGraph,
                                                  PlatformGraph *platformGraph,
                                                  std::ostream &out) {
    out << "digraph " << platformGraph->getName() << " {" << std::endl;
    out << "    size=\"7,10\";" << std::endl;

    // Output all actors
    for (const auto &a : sdfGraph->getActors()) {
        out << "    " << a->getName() << " [ label=\"" << a->getName();
        out << "\", href=\"#actor_" << a->getName() << "\" ];" << std::endl;
    }
    out << std::endl;

    // Output all channels
    for (const auto &c : sdfGraph->getChannels()) {
        // Initial tokens on channel?
        if (c->getInitialTokens() != 0) {
            SDFport *srcP = c->getSrcPort();
            SDFactor *srcA = srcP->getActor();
            SDFport *dstP = c->getDstPort();
            SDFactor *dstA = dstP->getActor();

            out << "    " << srcA->getName() << " -> " << dstA->getName();
            out << " [ label=\"" << c->getName() << "(";
            out << c->getInitialTokens() << ")"
                << "\", taillabel=\"";
            out << srcP->getRate() << "\", headlabel=\"";
            out << dstP->getRate() << "\" href=\"#channel_" << c->getName();
            out << "\" ];" << std::endl;
        } else {
            SDFport *srcP = c->getSrcPort();
            SDFactor *srcA = srcP->getActor();
            SDFport *dstP = c->getDstPort();
            SDFactor *dstA = dstP->getActor();

            out << "    " << srcA->getName() << " -> " << dstA->getName();
            out << " [ label=\"" << c->getName() << "\", taillabel=\"";
            out << srcP->getRate() << "\", headlabel=\"";
            out << dstP->getRate() << "\" href=\"#channel_" << c->getName();
            out << "\" ];" << std::endl;
        }
    }

    // Output all tiles
    for (const auto &t : platformGraph->getTiles()) {
        out << "    " << t->getName() << " [ label=\"" << t->getName();
        out << R"(", shape="box", href="#tile_)" << t->getName() << "\" ];";
        out << std::endl;
    }
    out << std::endl;

    // Output all connections
    for (const auto &c : platformGraph->getConnections()) {
        out << "    " << c->getSrcTile()->getName() << " -> ";
        out << c->getDstTile()->getName();
        out << " [ label=\"" << c->getName() << "\", href=\"#connection_";
        out << c->getName() << "\" ];" << std::endl;
    }
    out << std::endl;

    // Output the mapping of actors to tiles
    for (const auto &t : platformGraph->getTiles()) {
        CompBindings *bindings = t->getProcessor()->getActorBindings();

        for (auto &b : *bindings) {
            if (b->getComponent() != nullptr) {
                out << "    " << b->getComponent()->getName() << " -> ";
                out << t->getName();
                out << " [ style=\"dashed\" ];" << std::endl;
            }
        }
    }

    out << "}" << std::endl;
}

/**
 * outputInterconnectGraphAsDot ()
 * The function outputs an interconnect graph in DOT format.
 */
void SDFconvertToHTML::outputInterconnectGraphAsDot(InterconnectGraph *g, std::ostream &out) {
    out << "digraph interconnect {" << std::endl;
    out << "    size=\"7,10\";" << std::endl;

    // Output all nodes
    for (const auto &n : g->getNodes()) {
        out << "    " << n->getName() << " [ label=\"" << n->getName();
        out << "\"";

        if (n->getType() == "tile") {
            out << ", shape=\"box\" ";
        } else {
            out << ", shape=\"diamond\" ";
        }

        out << "];" << std::endl;
    }
    out << std::endl;

    // Output all links
    for (const auto &l : g->getLinks()) {
        out << "    " << l->getSrcNode()->getName() << " -> ";
        out << l->getDstNode()->getName();
        out << " [ label=\"" << l->getName() << "\", ";
        out << "href=\"#link_" << l->getName();
        out << "\" ];" << std::endl;
    }

    out << "}" << std::endl;
}

/**
 * convertSDFgraph ()
 * Convert the SDF graph to HTML format.
 */
void SDFconvertToHTML::convertSDFgraph() {
    std::ofstream out;

    // No SDF graph given?
    if (getSDFgraph() == nullptr) {
        return;
    }

    // Create an output stream for the graph
    out.open(getFullName(sdfGraphFilename, "html"));
    if (!out.is_open()) {
        throw CException("Failed opening '" + getFullName(sdfGraphFilename, "html") + "'.");
    }

    // Write header to the file
    printHeader(out, "SDF graph");

    // Graph
    out << "<a name='actor_" << sdfGraph->getName() << "'/>" << std::endl;
    out << "<h2>Graph: " << sdfGraph->getName() << "</h2>" << std::endl;
    out << std::endl;

    // Picture
    out << convertSDFgraphToPNG(sdfGraph, getFullName(sdfGraphFilename, "png"));
    out << "<p class='center'><img src='";
    out << getFullName(sdfGraphFilename, "png") << "' usemap='#";
    out << sdfGraph->getName() << "'/></p>" << std::endl;
    out << std::endl;

    // Actors
    out << "<p><b>Actors: </b>" << std::endl;
    for (auto actorIter = sdfGraph->actorsBegin(); actorIter != sdfGraph->actorsEnd();
         actorIter++) {
        auto *a = dynamic_cast<TimedSDFactor *>((*actorIter).get());

        if (actorIter != sdfGraph->actorsBegin()) {
            out << ", ";
        }

        out << "<a href='#actor_" << a->getName() << "'>";
        out << a->getName() << "</a>";
    }
    out << "</p>" << std::endl;
    out << std::endl;

    // Channels
    out << "<p><b>Channel: </b>" << std::endl;
    for (auto channelIter = sdfGraph->channelsBegin(); channelIter != sdfGraph->channelsEnd();
         channelIter++) {
        auto *c = dynamic_cast<TimedSDFchannel *>((*channelIter).get());

        if (channelIter != sdfGraph->channelsBegin()) {
            out << ", ";
        }

        out << "<a href='#channel_" << c->getName() << "'>";
        out << c->getName() << "</a>";
    }
    out << "</p>" << std::endl;
    out << std::endl;

    // Timing constraints
    out << "<p><b class='header'>Throughput constraint:</b> " << std::endl;
    out << sdfGraph->getThroughputConstraint();
    out << "</p>" << std::endl;
    out << std::endl;

    // Actors
    for (const auto &actorIter : sdfGraph->getActors()) {
        auto *a = dynamic_cast<TimedSDFactor *>(actorIter.get());

        // Header of actor
        out << "<hr/><a name='actor_" << a->getName() << "'/>" << std::endl;
        out << "<h3>Actor: " << a->getName() << "</h3>" << std::endl;
        out << std::endl;

        // Ports
        out << "<table class='center'>" << std::endl;
        out << "    <tr>" << std::endl;
        out << "        <th>port name</th>" << std::endl;
        out << "        <th>direction</th>" << std::endl;
        out << "        <th>rate</th>" << std::endl;
        out << "        <th>channel</th>" << std::endl;
        out << "    </tr>" << std::endl;
        for (const auto &p : a->getPorts()) {
            out << "    <tr>" << std::endl;
            out << "        <td>" << p->getName() << "</td>" << std::endl;
            out << "        <td>" << p->getTypeAsString() << "</td>" << std::endl;
            out << "        <td>" << p->getRate() << "</td>" << std::endl;
            out << "        <td><a href='#channel_" << p->getChannel()->getName();
            out << "'>" << p->getChannel()->getName() << "</a></td>" << std::endl;
            out << "    </tr>" << std::endl;
        }
        out << "</table>" << std::endl;

        out << std::endl;
        out << "<br/>" << std::endl;
        out << std::endl;

        // Properties
        out << "<table class='center'>" << std::endl;
        out << "    <tr>" << std::endl;
        out << "        <th>processor type</th>" << std::endl;
        out << "        <th>execution time</th>" << std::endl;
        out << "        <th>statesize</th>" << std::endl;
        out << "        <th>default</th>" << std::endl;
        out << "    </tr>" << std::endl;
        for (const auto &p : a->getProcessors()) {
            out << "    <tr>" << std::endl;
            out << "        <td>" << p->type << "</td>" << std::endl;
            out << "        <td>" << p->execTime << "</td>" << std::endl;
            out << "        <td>" << p->stateSize << "</td>" << std::endl;
            if (a->getDefaultProcessor() == p->type) {
                out << "        <td>true</td>" << std::endl;
            } else {
                out << "        <td>false</td>" << std::endl;
            }
            out << "    </tr>" << std::endl;
        }
        out << "</table>" << std::endl;
        out << std::endl;

        out << "<div><a class='footer' href='#top'>Back to graph</a></div>" << std::endl;
        out << std::endl;
    }

    // Channels
    for (const auto &channelIter : sdfGraph->getChannels()) {
        auto *c = dynamic_cast<TimedSDFchannel *>(channelIter.get());

        // Header of channel
        out << "<hr/><a name='channel_" << c->getName() << "'/>" << std::endl;
        out << "<h3>Channel: " << c->getName() << "</h3>" << std::endl;
        out << std::endl;

        // Structure
        out << "<table class='center'>" << std::endl;
        out << "    <tr>" << std::endl;
        out << "        <th>src actor</th>" << std::endl;
        out << "        <th>src port</th>" << std::endl;
        out << "        <th>dst actor</th>" << std::endl;
        out << "        <th>dst port</th>" << std::endl;
        out << "        <th>#initial tokens</th>" << std::endl;
        out << "    </tr>" << std::endl;
        out << "    <tr>" << std::endl;
        out << "        <td><a href='#actor_" << c->getSrcActor()->getName();
        out << "'>" << c->getSrcActor()->getName() << "</a></td>" << std::endl;
        out << "        <td>" << c->getSrcPort()->getName() << "</td>" << std::endl;
        out << "        <td><a href='#actor_" << c->getDstActor()->getName();
        out << "'>" << c->getDstActor()->getName() << "</a></td>" << std::endl;
        out << "        <td>" << c->getDstPort()->getName() << "</td>" << std::endl;
        out << "        <td>" << c->getInitialTokens() << "</td>" << std::endl;
        out << "    </tr>" << std::endl;
        out << "</table>" << std::endl;

        out << std::endl;
        out << "<br/>" << std::endl;
        out << std::endl;

        // Properties
        out << "<table class='center'>" << std::endl;
        out << "    <tr>" << std::endl;
        out << "        <th>tokensize</th>" << std::endl;
        out << "        <th>latency</th>" << std::endl;
        out << "        <th>bandwidth</th>" << std::endl;
        out << "        <th>buffersize (sz/src/dst/mem)</th>" << std::endl;
        out << "    </tr>" << std::endl;
        out << "    <tr>" << std::endl;
        out << "        <td>" << c->getTokenSize() << "</td>" << std::endl;
        out << "        <td>" << c->getMinLatency() << "</td>" << std::endl;
        out << "        <td>" << c->getMinBandwidth() << "</td>" << std::endl;
        out << "        <td>";
        if (c->getBufferSize().sz != SDF_INFINITE_SIZE) {
            out << c->getBufferSize().sz;
        } else {
            out << "inf";
        }
        out << "/";
        if (c->getBufferSize().src != SDF_INFINITE_SIZE) {
            out << c->getBufferSize().src;
        } else {
            out << "inf";
        }
        out << "/";
        if (c->getBufferSize().dst != SDF_INFINITE_SIZE) {
            out << c->getBufferSize().dst;
        } else {
            out << "inf";
        }
        out << "/";
        if (c->getBufferSize().mem != SDF_INFINITE_SIZE) {
            out << c->getBufferSize().mem;
        } else {
            out << "inf";
        }
        out << "</td>";
        out << "    </tr>" << std::endl;
        out << "</table>" << std::endl;
        out << std::endl;

        out << "<div><a class='footer' href='#top'>Back to graph</a></div>" << std::endl;
        out << std::endl;
    }
    out << std::endl;

    // Write footer to the file
    printFooter(out);
}

/**
 * convertPlatformGraph ()
 * Convert the platform graph to HTML format.
 */
void SDFconvertToHTML::convertPlatformGraph() {
    std::ofstream out;

    // No SDF graph given?
    if (getPlatformGraph() == nullptr) {
        return;
    }

    // Create an output stream for the graph
    out.open(getFullName(platformGraphFilename, "html"));
    if (!out.is_open()) {
        throw CException("Failed opening '" + getFullName(platformGraphFilename, "html") + "'.");
    }

    // Write header to the file
    printHeader(out, "Platform");

    // Graph
    out << "<a name='platform_" << platformGraph->getName() << "'/>" << std::endl;
    out << "<h2>Platform: " << platformGraph->getName() << "</h2>" << std::endl;
    out << std::endl;

    // Picture
    out << convertPlatformGraphToPNG(platformGraph, getFullName(platformGraphFilename, "png"));
    out << "<p class='center'><img src='";
    out << getFullName(platformGraphFilename, "png") << "' usemap='#";
    out << platformGraph->getName() << "'/></p>" << std::endl;
    out << std::endl;

    // Tiles
    out << "<p><b>Tiles: </b>" << std::endl;
    for (auto tileIter = platformGraph->tilesBegin(); tileIter != platformGraph->tilesEnd();
         tileIter++) {
        Tile *t = (*tileIter).get();

        if (tileIter != platformGraph->tilesBegin()) {
            out << ", ";
        }

        out << "<a href='#tile_" << t->getName() << "'>";
        out << t->getName() << "</a>";
    }
    out << "</p>" << std::endl;
    out << std::endl;

    // Connections
    out << "<p><b>Connections: </b>" << std::endl;
    for (auto connectionIter = platformGraph->connectionsBegin();
         connectionIter != platformGraph->connectionsEnd();
         connectionIter++) {
        Connection *c = (*connectionIter).get();

        if (connectionIter != platformGraph->connectionsBegin()) {
            out << ", ";
        }

        out << "<a href='#connections_" << c->getName() << "'>";
        out << c->getName() << "</a>";
    }
    out << "</p>" << std::endl;
    out << std::endl;

    // Tiles
    out << "<hr/><h3>Tiles</h3>" << std::endl;
    out << "<table class='center'>" << std::endl;
    out << "    <tr>" << std::endl;
    out << "        <th>name</th>" << std::endl;
    out << "        <th>processor type</th>" << std::endl;
    out << "        <th>TDMA wheelsize</th>" << std::endl;
    out << "        <th>memory size</th>" << std::endl;
    out << "        <th>#connections</th>" << std::endl;
    out << "        <th>input bandwidth</th>" << std::endl;
    out << "        <th>output bandwidth</th>" << std::endl;
    out << "    </tr>" << std::endl;

    for (const auto &t : platformGraph->getTiles()) {
        out << "    <tr>" << std::endl;
        out << "        <td><a name='tile_" << t->getName() << "'/>";
        out << t->getName() << "</td>" << std::endl;
        out << "        <td>" << t->getProcessor()->getType() << "</td>" << std::endl;
        out << "        <td>" << t->getProcessor()->getTimewheelSize() << "</td>" << std::endl;
        out << "        <td>" << t->getMemory()->getSize() << "</td>" << std::endl;
        out << "        <td>" << t->getNetworkInterface()->getNrConnections() << "</td>"
            << std::endl;
        out << "        <td>" << t->getNetworkInterface()->getInBandwidth() << "</td>" << std::endl;
        out << "        <td>" << t->getNetworkInterface()->getOutBandwidth() << "</td>"
            << std::endl;
        out << "    </tr>" << std::endl;
    }

    out << "</table>" << std::endl;
    out << std::endl;
    out << "<p class='footer'><a href='#top'>Back to top</a></p>" << std::endl;
    out << std::endl;

    // Connections
    out << "<hr/><h3>Connections</h3>" << std::endl;
    out << "<table class='center'>" << std::endl;
    out << "    <tr>" << std::endl;
    out << "        <th>name</th>" << std::endl;
    out << "        <th>src tile</th>" << std::endl;
    out << "        <th>dst tile</th>" << std::endl;
    out << "        <th>delay</th>" << std::endl;
    out << "    </tr>" << std::endl;

    for (const auto &c : platformGraph->getConnections()) {
        out << "    <tr>" << std::endl;
        out << "        <td><a name='connection_" << c->getName() << "'/>";
        out << c->getName() << "</td>" << std::endl;
        out << "        <td><a name='#tile_" << c->getSrcTile()->getName() << "'/>";
        out << c->getSrcTile()->getName() << "</td>" << std::endl;
        out << "        <td><a name='#tile_" << c->getDstTile()->getName() << "'/>";
        out << c->getDstTile()->getName() << "</td>" << std::endl;
        out << "        <td>" << c->getLatency() << "</td>" << std::endl;
        out << "    </tr>" << std::endl;
    }

    out << "</table>" << std::endl;
    out << "<div><a class='footer' href='#top'>Back to graph</a></div>" << std::endl;
    out << std::endl;

    // Write footer to the file
    printFooter(out);
}

/**
 * convertPlatformMapping ()
 * Convert the mapping to HTML format.
 */
void SDFconvertToHTML::convertPlatformMapping() {
    std::ofstream out;

    // No SDF graph given or platform graph?
    if (getPlatformGraph() == nullptr || getSDFgraph() == nullptr) {
        return;
    }

    // Create an output stream for the graph
    out.open(getFullName(platformMappingFilename, "html"));
    if (!out.is_open()) {
        throw CException("Failed opening '" + getFullName(platformMappingFilename, "html") + "'.");
    }

    // Write header to the file
    printHeader(out, "Mapping");

    // Graph
    out << "<a name='mapping_" << platformGraph->getName() << "'/>" << std::endl;
    out << "<h2>Mapping: " << sdfGraph->getName() << " on ";
    out << platformGraph->getName() << "</h2>" << std::endl;
    out << std::endl;

    // Picture
    out << convertPlatformMappingToPNG(
            sdfGraph, platformGraph, getFullName(platformMappingFilename, "png"));
    out << "<p class='center'><img src='";
    out << getFullName(platformMappingFilename, "png") << "' usemap='#";
    out << platformGraph->getName() << "'/></p>" << std::endl;
    out << std::endl;

    // Actors
    out << "<p><b>Actors: </b>" << std::endl;
    for (auto actorIter = sdfGraph->actorsBegin(); actorIter != sdfGraph->actorsEnd();
         actorIter++) {
        auto *a = dynamic_cast<TimedSDFactor *>((*actorIter).get());

        if (actorIter != sdfGraph->actorsBegin()) {
            out << ", ";
        }

        out << "<a href='#actor_" << a->getName() << "'>";
        out << a->getName() << "</a>";
    }
    out << "</p>" << std::endl;
    out << std::endl;

    // Channels
    out << "<p><b>Channel: </b>" << std::endl;
    for (auto channelIter = sdfGraph->channelsBegin(); channelIter != sdfGraph->channelsEnd();
         channelIter++) {
        auto *c = dynamic_cast<TimedSDFchannel *>((*channelIter).get());

        if (channelIter != sdfGraph->channelsBegin()) {
            out << ", ";
        }

        out << "<a href='#channel_" << c->getName() << "'>";
        out << c->getName() << "</a>";
    }
    out << "</p>" << std::endl;
    out << std::endl;

    // Tiles
    out << "<p><b>Tiles: </b>" << std::endl;
    for (auto tileIter = platformGraph->tilesBegin(); tileIter != platformGraph->tilesEnd();
         tileIter++) {
        Tile *t = (*tileIter).get();

        if (tileIter != platformGraph->tilesBegin()) {
            out << ", ";
        }

        out << "<a href='#tile_" << t->getName() << "'>";
        out << t->getName() << "</a>";
    }
    out << "</p>" << std::endl;
    out << std::endl;

    // Connections
    out << "<p><b>Connections: </b>" << std::endl;
    for (auto connectionIter = platformGraph->connectionsBegin();
         connectionIter != platformGraph->connectionsEnd();
         connectionIter++) {
        Connection *c = (*connectionIter).get();

        if (connectionIter != platformGraph->connectionsBegin()) {
            out << ", ";
        }

        out << "<a href='#connections_" << c->getName() << "'>";
        out << c->getName() << "</a>";
    }
    out << "</p>" << std::endl;
    out << std::endl;

    // Tiles
    for (const auto &t : platformGraph->getTiles()) {
        StaticOrderSchedule &s = t->getProcessor()->getSchedule();
        CompBindings *bindings = nullptr;

        out << "<hr/><a name='tile_" << t->getName() << "'/>";
        out << "<h3>Tile: " << t->getName() << "</h3>" << std::endl;

        // No actors on the tile?
        if (s.empty()) {
            out << "<p>No actors mapped to this tile.</p>" << std::endl;
            out << "<div><a class='footer' href='#top'>Back to graph</a></div>" << std::endl;
            out << std::endl;
        } else {
            out << "<p><b class='header'>Actors:</b> " << std::endl;
            bindings = t->getProcessor()->getActorBindings();
            for (auto bindingIter = bindings->begin(); bindingIter != bindings->end();
                 bindingIter++) {
                ComponentBinding *b = (*bindingIter).get();

                if (b->getComponent() != nullptr) {
                    if (bindingIter != bindings->begin()) {
                        out << ", ";
                    }

                    out << "<a name='actor_" << b->getComponent()->getName();
                    out << "'/>" << b->getComponent()->getName();
                }
            }
            out << "</p>" << std::endl;
            out << std::endl;

            out << "<p><b class='header'>Static-order schedule:</b> " << std::endl;
            for (uint i = 0; i != s.size(); i++) {
                if (s.getStartPeriodicSchedule() == i) {
                    out << "<b>(</b>";
                }

                out << s.getScheduleEntry(i)->actor->getName();

                if (i + 1 == s.size()) {
                    out << "<b>)</b>";
                } else {
                    out << ", ";
                }
            }
            out << "</p>" << std::endl;
            out << std::endl;

            out << "<p><b class='header'>TDMA slice:</b> ";
            out << t->getProcessor()->getReservedTimeSlice() << std::endl;
            out << " (";
            out << 100.0 * static_cast<double>(t->getProcessor()->getReservedTimeSlice())
                            / static_cast<double>(t->getProcessor()->getTimewheelSize());
            out << "%)</p>" << std::endl;

            bindings = t->getMemory()->getActorBindings();
            out << "<table class='center'>" << std::endl;
            out << "    <tr>" << std::endl;
            out << "        <th>actor / channel</th>" << std::endl;
            out << "        <th>memory usage</th>" << std::endl;
            out << "    </tr>" << std::endl;
            for (auto &b : *bindings) {
                if (b->getComponent() != nullptr) {
                    out << "    <tr>" << std::endl;
                    out << "        <td>" << b->getComponent()->getName() << "</td>" << std::endl;
                    out << "        <td>" << b->getValue(0) << "</td>" << std::endl;
                    out << "    </tr>" << std::endl;
                }
            }
            bindings = t->getMemory()->getChannelBindings();
            for (auto &b : *bindings) {
                if (b->getComponent() != nullptr) {
                    out << "    <tr>" << std::endl;
                    out << "        <td>" << b->getComponent()->getName() << "</td>" << std::endl;
                    out << "        <td>" << b->getValue(0) << "</td>" << std::endl;
                    out << "    </tr>" << std::endl;
                }
            }
            out << "</table>" << std::endl;
            out << "</p>" << std::endl;

            out << "<table class='center'>" << std::endl;
            out << "    <tr>" << std::endl;
            out << "        <th>channel</th>" << std::endl;
            out << "        <th>#connections</th>" << std::endl;
            out << "        <th>input bandwidth</th>" << std::endl;
            out << "        <th>output bandwidth</th>" << std::endl;
            out << "    </tr>" << std::endl;
            bindings = t->getNetworkInterface()->getBindings();
            for (auto &b : *bindings) {
                if (b->getComponent() != nullptr) {
                    out << "    <tr>" << std::endl;
                    out << "        <td>" << b->getComponent()->getName() << "</td>" << std::endl;
                    out << "        <td>" << b->getValue(NetworkInterface::nrConn) << "</td>"
                        << std::endl;
                    out << "        <td>" << b->getValue(NetworkInterface::inBw) << "</td>"
                        << std::endl;
                    out << "        <td>" << b->getValue(NetworkInterface::outBw) << "</td>"
                        << std::endl;
                    out << "    </tr>" << std::endl;
                }
            }
            out << "</table>" << std::endl;

            out << "<div><a class='footer' href='#top'>Back to graph</a></div>" << std::endl;
            out << std::endl;

            out << "</p>" << std::endl;
        }
    }

    // Connections
    out << "<hr/><h3>Connections</h3>" << std::endl;
    out << "<table class='center'>" << std::endl;
    out << "    <tr>" << std::endl;
    out << "        <th>name</th>" << std::endl;
    out << "        <th>src tile</th>" << std::endl;
    out << "        <th>dst tile</th>" << std::endl;
    out << "        <th>channels</th>" << std::endl;
    out << "    </tr>" << std::endl;

    for (const auto &c : platformGraph->getConnections()) {
        CompBindings *bindings = c->getChannelBindings();

        out << "    <tr>" << std::endl;
        out << "        <td><a name='connection_" << c->getName() << "'/>";
        out << c->getName() << "</td>" << std::endl;
        out << "        <td><a href='#tile_" << c->getSrcTile()->getName();
        out << "'/>" << c->getSrcTile()->getName() << "</a></td>" << std::endl;
        out << "        <td><a href='#tile_" << c->getDstTile()->getName();
        out << "'/>" << c->getDstTile()->getName() << "</a></td>" << std::endl;
        out << "        <td>";
        for (auto bindingIter = bindings->begin(); bindingIter != bindings->end(); bindingIter++) {
            ComponentBinding *b = (*bindingIter).get();

            if (b->getComponent() != nullptr) {
                if (bindingIter != bindings->begin()) {
                    out << ", ";
                }

                out << "<a name='#channel_" << b->getComponent()->getName() << "'/>";
                out << b->getComponent()->getName() << "</a>";
            }
        }
        out << "</td>" << std::endl;
        out << "    </tr>" << std::endl;
    }

    out << "</table>" << std::endl;
    out << "<div><a class='footer' href='#top'>Back to graph</a></div>" << std::endl;
    out << std::endl;

    // Write footer to the file
    printFooter(out);
}

/**
 * convertPlatformUsage ()
 * Convert the platform usage to HTML format.
 */
void SDFconvertToHTML::convertPlatformUsage() {
    std::ofstream out;

    // No SDF graph given or platform graph?
    if (getPlatformGraph() == nullptr || getSDFgraph() == nullptr) {
        return;
    }

    // Create an output stream for the graph
    out.open(getFullName(platformUsageFilename, "html"));
    if (!out.is_open()) {
        throw CException("Failed opening '" + getFullName(platformUsageFilename, "html") + "'.");
    }

    // Write header to the file
    printHeader(out, "Usage");

    // Graph
    out << "<a name='usage_" << platformGraph->getName() << "'/>" << std::endl;
    out << "<h2>Platform usage: " << platformGraph->getName() << "</h2>" << std::endl;
    out << std::endl;

    // Picture
    out << convertPlatformGraphToPNG(platformGraph, getFullName(platformUsageFilename, "png"));
    out << "<p class='center'><img src='";
    out << getFullName(platformUsageFilename, "png");
    out << "' usemap='#";
    out << platformGraph->getName() << "'/></p>" << std::endl;
    out << std::endl;

    // Tiles
    out << "<p><b>Tiles: </b>" << std::endl;
    for (auto tileIter = platformGraph->tilesBegin(); tileIter != platformGraph->tilesEnd();
         tileIter++) {
        Tile *t = (*tileIter).get();

        if (tileIter != platformGraph->tilesBegin()) {
            out << ", ";
        }

        out << "<a href='#tile_" << t->getName() << "'>";
        out << t->getName() << "</a>";
    }
    out << "</p>" << std::endl;
    out << std::endl;

    // Tiles
    out << "<hr/><h3>Tiles</h3>" << std::endl;
    out << "<table class='center'>" << std::endl;
    out << "    <tr>" << std::endl;
    out << "        <th>name</th>" << std::endl;
    out << "        <th>TDMA wheel</th>" << std::endl;
    out << "        <th>memory</th>" << std::endl;
    out << "        <th>#connections</th>" << std::endl;
    out << "        <th>input bandwidth</th>" << std::endl;
    out << "        <th>output bandwidth</th>" << std::endl;
    out << "    </tr>" << std::endl;

    for (const auto &t : platformGraph->getTiles()) {
        out << "    <tr>" << std::endl;
        out << "        <td><a name='tile_" << t->getName() << "'/>";
        out << t->getName() << "</td>" << std::endl;

        out << "        <td>";
        out << 100.0
                        * static_cast<double>(t->getProcessor()->getReservedTimeSlice()
                                              + t->getProcessor()->getOccupiedTimeSlice())
                        / static_cast<double>(t->getProcessor()->getTimewheelSize());
        out << "%</td>" << std::endl;

        out << "        <td>";
        out << 100.0
                        * static_cast<double>(t->getMemory()->getSize()
                                              - t->getMemory()->availableMemorySize())
                        / static_cast<double>(t->getMemory()->getSize());
        out << "%</td>" << std::endl;

        out << "        <td>";
        out << 100.0
                        * static_cast<double>(t->getNetworkInterface()->getNrConnections()
                                              - t->getNetworkInterface()->availableNrConnections())
                        / static_cast<double>(t->getNetworkInterface()->getNrConnections());
        out << "%</td>" << std::endl;

        out << "        <td>";
        out << 100.0
                        * (t->getNetworkInterface()->getInBandwidth()
                           - t->getNetworkInterface()->availableInBandwidth())
                        / (t->getNetworkInterface()->getInBandwidth());
        out << "%</td>" << std::endl;

        out << "        <td>";
        out << 100.0
                        * (t->getNetworkInterface()->getOutBandwidth()
                           - t->getNetworkInterface()->availableOutBandwidth())
                        / (t->getNetworkInterface()->getOutBandwidth());
        out << "%</td>" << std::endl;

        out << "    </tr>" << std::endl;
    }

    out << "</table>" << std::endl;
    out << "<div><a class='footer' href='#top'>Back to graph</a></div>" << std::endl;
    out << std::endl;

    // Write footer to the file
    printFooter(out);
}

/**
 * convertInterconnectGraph ()
 * Convert the interconnect mapping to HTML format.
 */
void SDFconvertToHTML::convertInterconnectGraph() {
    InterconnectGraph *interconnectGraph = nullptr;
    std::ofstream out;

    // No interconnect mapping?
    if (getSetOfNoCScheduleProblems() == nullptr
        || getSetOfNoCScheduleProblems()->nrScheduleProblems() == 0) {
        return;
    }

    // Create an output stream for the interconnect
    out.open(getFullName(interconnectGraphFilename, "html"));
    if (!out.is_open()) {
        throw CException("Failed opening '" + getFullName(interconnectGraphFilename, "html")
                         + "'.");
    }

    // Write header to the file
    printHeader(out, "Interconnect");

    // Graph
    out << "<a name='interconnect_graph'/>" << std::endl;
    out << "<h2>Interconnect</h2>" << std::endl;
    out << std::endl;

    // Interconnect graph (structure is equal for all problems)
    interconnectGraph =
            (*getSetOfNoCScheduleProblems()->scheduleProblemsBegin())->getInterconnectGraph();

    // Picture
    out << convertInterconnectGraphToPNG(interconnectGraph,
                                         getFullName(interconnectGraphFilename, "png"));
    out << "<p class='center'><img src='";
    out << getFullName(interconnectGraphFilename, "png");
    out << "' ";
    out << "usemap='#interconnect'/></p>" << std::endl;
    out << std::endl;

    // Links
    out << "<p><b class='header'>Links: </b>" << std::endl;
    for (auto linkIter = interconnectGraph->linksBegin(); linkIter != interconnectGraph->linksEnd();
         linkIter++) {
        Link *l = (*linkIter).get();

        if (linkIter != interconnectGraph->linksBegin()) {
            out << ", ";
        }

        out << l->getName();
    }
    out << "</p>" << std::endl;
    out << std::endl;

    // NoC properties
    out << "<p><b class='header'>Slot-table size: </b>";
    out << interconnectGraph->getSlotTableSize();
    out << "</p>" << std::endl;

    out << "<p><b class='header'>Packet header size: </b>";
    out << interconnectGraph->getPacketHeaderSize();
    out << "</p>" << std::endl;

    out << "<p><b class='header'>Flit size: </b>";
    out << interconnectGraph->getFlitSize();
    out << "</p>" << std::endl;

    out << "<p><b class='header'>Reconfiguration time NI: </b>";
    out << interconnectGraph->getReconfigurationTimeNI();
    out << "</p>" << std::endl;

    // Write footer to the file
    printFooter(out);
}

/**
 * convertInterconnectMapping ()
 * Convert the interconnect mapping to HTML format.
 */
void SDFconvertToHTML::convertInterconnectMapping() {
    InterconnectGraph *interconnectGraph = nullptr;
    std::ofstream out;

    // No interconnect mapping?
    if (getSetOfNoCScheduleProblems() == nullptr
        || getSetOfNoCScheduleProblems()->nrScheduleProblems() == 0) {
        return;
    }

    // Create an output stream for the interconnect
    out.open(getFullName(interconnectMappingFilename, "html"));
    if (!out.is_open()) {
        throw CException("Failed opening '" + getFullName(interconnectMappingFilename, "html")
                         + "'.");
    }

    // Write header to the file
    printHeader(out, "Interconnect mapping");

    // Graph
    out << "<a name='interconnect_graph'/>" << std::endl;
    out << "<h2>Interconnect mapping</h2>" << std::endl;
    out << std::endl;

    // Interconnect graph (structure is equal for all problems)
    interconnectGraph =
            (*getSetOfNoCScheduleProblems()->scheduleProblemsBegin())->getInterconnectGraph();

    // Picture
    out << convertInterconnectGraphToPNG(interconnectGraph,
                                         getFullName(interconnectMappingFilename, "png"));
    out << "<p class='center'><img src='";
    out << getFullName(interconnectMappingFilename, "png");
    out << "' ";
    out << "usemap='#interconnect'/></p>" << std::endl;
    out << std::endl;

    // Links
    out << "<p><b>Links: </b>" << std::endl;
    for (auto linkIter = interconnectGraph->linksBegin(); linkIter != interconnectGraph->linksEnd();
         linkIter++) {
        Link *l = (*linkIter).get();

        if (linkIter != interconnectGraph->linksBegin()) {
            out << ", ";
        }

        out << "<a href='#link_" << l->getName() << "'>";
        out << l->getName() << "</a>";
    }
    out << "</p>" << std::endl;
    out << std::endl;

    // Scheduling problems
    out << "<p><b>Scheduling problems: </b>" << std::endl;

    for (auto problemIter = getSetOfNoCScheduleProblems()->scheduleProblemsBegin();
         problemIter != getSetOfNoCScheduleProblems()->scheduleProblemsEnd();
         problemIter++) {
        NoCScheduleProblem *problem = (*problemIter).get();

        if (problemIter != getSetOfNoCScheduleProblems()->scheduleProblemsBegin()) {
            out << ", ";
        }

        out << "<a href='#problem_" << problem->getName() << "'>";
        out << problem->getName() << "</a>";
    }
    out << "</p>" << std::endl;
    out << std::endl;

    // Scheduling problem switches
    out << "<hr/><h3>Switches between scheduling problems</h3>" << std::endl;
    out << "<table class='center'>" << std::endl;
    out << "    <tr>" << std::endl;
    out << "        <th>from</th>" << std::endl;
    out << "        <th>to</th>" << std::endl;
    out << "        <th>overlap</th>" << std::endl;
    out << "    </tr>" << std::endl;

    // Iterate over all scheduling problems
    for (auto problemIter = getSetOfNoCScheduleProblems()->scheduleProblemsBegin();
         problemIter != getSetOfNoCScheduleProblems()->scheduleProblemsEnd();
         problemIter++) {
        NoCScheduleProblem *problem = (*problemIter).get();

        // Iterate over all constraints
        for (auto constraintIter = problem->scheduleSwitchConstraintsBegin();
             constraintIter != problem->scheduleSwitchConstraintsEnd();
             constraintIter++) {
            NoCScheduleSwitchConstraint constraint = *constraintIter;

            // Constraint starts from this problem
            if (constraint.from->getName() == problem->getName()) {
                out << "    <tr>" << std::endl;
                out << "        <td>" << constraint.from->getName() << "</td>" << std::endl;
                out << "        <td>" << constraint.to->getName() << "</td>" << std::endl;
                out << "        <td>" << constraint.overlap << "</td>" << std::endl;
                out << "    </tr>" << std::endl;
            }
        }
    }
    out << "</table>" << std::endl;
    out << "<div><a class='footer' href='#top'>Back to graph</a></div>" << std::endl;
    out << std::endl;

    // Iterate over the scheduling problems
    for (auto problemIter = getSetOfNoCScheduleProblems()->scheduleProblemsBegin();
         problemIter != getSetOfNoCScheduleProblems()->scheduleProblemsEnd();
         problemIter++) {
        NoCScheduleProblem *problem = (*problemIter).get();

        out << "<hr/><a name='problem_" << problem->getName() << "'/>" << std::endl;
        ;
        out << "<h3>Scheduling problem: " << problem->getName() << "</h3>" << std::endl;
        out << "<table class='center'>" << std::endl;
        out << "    <tr>" << std::endl;
        out << "        <th>link</th>" << std::endl;

        for (uint i = 0; i < interconnectGraph->getSlotTableSize(); i++) {
            out << "        <th>slot " << i << "</th>" << std::endl;
        }

        out << "    </tr>" << std::endl;

        for (const auto &l : interconnectGraph->getLinks()) {
            SlotReservations slotReservations = l->getUsedSlotsInSchedule();

            out << "    <tr>" << std::endl;
            out << "        <td>" << l->getName() << "</td>" << std::endl;

            for (uint i = 0; i < interconnectGraph->getSlotTableSize(); i++) {
                if (slotReservations[i]) {
                    out << "        <td>x</td>" << std::endl;
                } else {
                    out << "        <td>&nbsp;</td>" << std::endl;
                }
            }

            out << "    </tr>" << std::endl;
        }
        out << "</table>" << std::endl;
        out << "<div><a class='footer' href='#top'>Back to graph</a></div>";
        out << std::endl << std::endl;
    }

    // Write footer to the file
    printFooter(out);
}

/**
 * convertInterconnectUsage ()
 * Convert the interconnect usage to HTML format.
 */
void SDFconvertToHTML::convertInterconnectUsage() {
    InterconnectGraph *interconnectGraph = nullptr;
    std::ofstream out;

    // No interconnect?
    if (getSetOfNoCScheduleProblems() == nullptr
        || getSetOfNoCScheduleProblems()->nrScheduleProblems() == 0) {
        return;
    }

    // Create an output stream for the interconnect
    out.open(getFullName(interconnectUsageFilename, "html"));
    if (!out.is_open()) {
        throw CException("Failed opening '" + getFullName(interconnectUsageFilename, "html")
                         + "'.");
    }

    // Write header to the file
    printHeader(out, "Interconnect usage");

    // Graph
    out << "<a name='interconnect_graph'/>" << std::endl;
    out << "<h2>Interconnect usage</h2>" << std::endl;
    out << std::endl;

    // Interconnect graph (structure is equal for all problems)
    interconnectGraph =
            (*getSetOfNoCScheduleProblems()->scheduleProblemsBegin())->getInterconnectGraph();

    // Picture
    out << convertInterconnectGraphToPNG(interconnectGraph,
                                         getFullName(interconnectUsageFilename, "png"));
    out << "<p class='center'><img src='";
    out << getFullName(interconnectUsageFilename, "png ") << "' ";
    out << "usemap='#interconnect'/></p>" << std::endl;
    out << std::endl;

    // Links
    out << "<p><b>Links: </b>" << std::endl;
    for (auto linkIter = interconnectGraph->linksBegin(); linkIter != interconnectGraph->linksEnd();
         linkIter++) {
        Link *l = (*linkIter).get();

        if (linkIter != interconnectGraph->linksBegin()) {
            out << ", ";
        }

        out << "<a href='#link_" << l->getName() << "'>";
        out << l->getName() << "</a>";
    }
    out << "</p>" << std::endl;
    out << std::endl;

    // Links
    out << "<hr/><h3>Links</h3>" << std::endl;
    out << "<table class='center'>" << std::endl;
    out << "    <tr>" << std::endl;
    out << "        <th>link</th>" << std::endl;
    for (uint i = 0; i < interconnectGraph->getSlotTableSize(); i++) {
        out << "        <th>slot " << i << "</th>" << std::endl;
    }
    out << "    </tr>" << std::endl;

    // Iterate over all links
    for (const auto &l : interconnectGraph->getLinks()) {
        SlotReservations freeSlots;
        SlotReservations freeSlotsOnLinkInProblem;

        // Initialize free slot
        freeSlots.resize(interconnectGraph->getSlotTableSize());
        for (uint i = 0; i < interconnectGraph->getSlotTableSize(); i++) {
            freeSlots[i] = true;
        }

        // Iterate over all problems
        for (auto problemIter = getSetOfNoCScheduleProblems()->scheduleProblemsBegin();
             problemIter != getSetOfNoCScheduleProblems()->scheduleProblemsEnd();
             problemIter++) {
            NoCScheduleProblem *problem = (*problemIter).get();
            Link *linkInProblem = nullptr;

            // Find link l in this problem
            linkInProblem = problem->getInterconnectGraph()->getLink(l->getId());

            // Free slots in this problem on link l (iterate over all slot tables)
            for (auto iter = linkInProblem->slotTableSeqBegin();
                 iter != linkInProblem->slotTableSeqEnd();
                 iter++) {
                SlotTable &s = *iter;

                freeSlotsOnLinkInProblem = s.getSlotReservations(nullptr);

                for (uint i = 0; i < interconnectGraph->getSlotTableSize(); i++) {
                    freeSlots[i] = freeSlots[i] && freeSlotsOnLinkInProblem[i];
                }
            }
        }

        // Iterate over all slots in the link
        out << "    <tr>" << std::endl;
        out << "        <td><a name='link_" << l->getName() << "'/>";
        out << l->getName() << "</td>" << std::endl;

        for (uint i = 0; i < interconnectGraph->getSlotTableSize(); i++) {
            if (freeSlots[i]) {
                out << "        <td>&nbsp;</td>" << std::endl;
            } else {
                out << "        <td>x</td>" << std::endl;
            }
        }

        out << "    </tr>" << std::endl;
    }
    out << "</table>" << std::endl;
    out << "<div><a class='footer' href='#top'>Back to graph</a></div>";
    out << std::endl << std::endl;

    // Write footer to the file
    printFooter(out);
}

/**
 * convert ()
 * Convert the SDF graph and platform graph to HTML format. When mapping
 * or resource usage information is present, this information is also
 * exported to HTML.
 */
void SDFconvertToHTML::convert(const CString &prefix) {
    // Filename prefix
    fileNamePrefix = prefix;

    // No SDF graph given?
    if (getSDFgraph() == nullptr) {
        return;
    }

    // Convert the SDF graph to HTML
    convertSDFgraph();

    // No platform graph given?
    if (getPlatformGraph() == nullptr) {
        return;
    }

    // Convert the platform graph to HTML
    convertPlatformGraph();

    // Convert the mapping to HTML
    convertPlatformMapping();

    // Convert platform usage to HTML
    convertPlatformUsage();

    // Interconnect mapping
    if (getSetOfNoCScheduleProblems() != nullptr) {
        convertInterconnectGraph();
        convertInterconnectMapping();
        convertInterconnectUsage();
    }
}

} // namespace SDF
