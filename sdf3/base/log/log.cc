/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   log.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   February 16, 2007
 *
 *  Function        :   Logging functions
 *
 *  History         :
 *      16-02-07    :   Initial version.
 *
 * $Id: log.cc,v 1.1.1.1 2007-10-02 10:59:48 sander Exp $
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

#include "log.h"

namespace{


/**
 * logLevel
 * Global indicating the level from which messages are outputted.
 */
int logLevel = 0; // NOLINT(*global-variables)

} // namespace

/**
 * logHasLevel ()
 * The function returns 'true' if the current log level will log a
 * message of the supplied level.
 */
bool logHasLevel(int level) { return logLevel >= level; }

/**
 * getLogLevel ()
 * The function return the level used for logging messages.
 */
int getLogLevel() { return logLevel; }

/**
 * setLogLevel ()
 * The function sets the level used for logging messages.
 */
void setLogLevel(int level) { logLevel = level; }

/**
 * logMsg ()
 * Output the message to the stream out (default: cerr) when the
 * level is equal or larger than the logging level.
 */
void logMsg(const CString &msg, int level, std::ostream &out) {
    if (logLevel >= level) {
        out << "          " << msg << std::endl;
    }
}

/**
 * logMsg ()
 * Output the message to the stream out (default: cerr) when the
 * level is equal or larger than the logging level.
 */
void logMsg(const char *msg, int level, std::ostream &out) {
    if (logLevel >= level) {
        out << "          " << msg << std::endl;
    }
}

/**
 * logInfo ()
 * Output the message to the stream out (default: cerr) when the
 * level is equal or larger than the logging level. The function
 * prepends a "[INFO   ] " string and ends the message with an end-of-line.
 */
void logInfo(const CString &msg, int level, std::ostream &out) {
    if (logLevel >= level) {
        out << "[INFO   ] " << msg << std::endl;
    }
}

/**
 * logInfo ()
 * Output the message to the stream out (default: cerr) when the
 * level is equal or larger than the logging level. The function
 * prepends a "[INFO   ] " string and ends the message with an end-of-line.
 */
void logInfo(const char *msg, int level, std::ostream &out) {
    if (logLevel >= level) {
        out << "[INFO   ] " << msg << std::endl;
    }
}

/**
 * logInfo ()
 * Output the message to the stream out (default: cerr) when the
 * level is equal or larger than the logging level. The function
 * prepends a "[INFO   ] " string and ends the message with an end-of-line.
 */
void logInfo(const std::string &msg, int level, std::ostream &out) {
    if (logLevel >= level) {
        out << "[INFO   ] " << msg << std::endl;
    }
}

/**
 * logWarning ()
 * Output the message to the stream out (default: cerr) when the
 * level is equal or larger than the logging level. The function
 * prepends a "[WARNING] " string and ends the message with an end-of-line.
 */
void logWarning(const CString &msg, int level, std::ostream &out) {
    if (logLevel >= level) {
        out << "[WARNING] " << msg << std::endl;
    }
}

/**
 * logWarning ()
 * Output the message to the stream out (default: cerr) when the
 * level is equal or larger than the logging level. The function
 * prepends a "[WARNING] " string and ends the message with an end-of-line.
 */
void logWarning(const char *msg, int level, std::ostream &out) {
    if (logLevel >= level) {
        out << "[WARNING] " << msg << std::endl;
    }
}

/**
 * logWarning ()
 * Output the message to the stream out (default: cerr) when the
 * level is equal or larger than the logging level. The function
 * prepends a "[WARNING] " string and ends the message with an end-of-line.
 */
void logWarning(const std::string &msg, int level, std::ostream &out) {
    if (logLevel >= level) {
        out << "[WARNING] " << msg << std::endl;
    }
}
/**
 * logError ()
 * Output the message to the stream out (default: clog) when the
 * level is equal or larger than the logging level.The function
 * prepends a "[ERROR] " string and ends the message with an end-of-line.
 */
void logError(const CString &msg, int level, std::ostream &out) {
    if (logLevel >= level) {
        out << "[ERROR] " << msg << std::endl;
    }
}

/**
 * logError ()
 * Output the message to the stream out (default: clog) when the
 * level is equal or larger than the logging level.The function
 * prepends a "[ERROR] " string and ends the message with an end-of-line.
 */
void logError(const char *msg, int level, std::ostream &out) {
    if (logLevel >= level) {
        out << "[ERROR] " << msg << std::endl;
    }
}
