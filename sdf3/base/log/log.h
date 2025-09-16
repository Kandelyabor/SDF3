/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   log.h
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
 * $Id: log.h,v 1.1.1.1 2007-10-02 10:59:48 sander Exp $
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

#ifndef BASE_LOG_LOG_H
#define BASE_LOG_LOG_H

#include "base/string/cstring.h"
#include "base/basic_types.h" // IWYU pragma: keep

/**
 * logHasLevel ()
 * The function returns 'true' if the current log level will log a
 * message of the supplied level.
 */
bool logHasLevel(int level = 0);

/**
 * getLogLevel ()
 * The function return the level used for logging messages.
 */
int getLogLevel();

/**
 * setLogLevel ()
 * The function sets the level used for logging messages.
 */
void setLogLevel(int level);

/**
 * logMsg ()
 * Output the message to the stream out (default: std::cerr) when the
 * level is equal or larger than the logging level.
 */
void logMsg(const CString &msg, int level = 0, std::ostream &out = std::cerr);
void logMsg(const char *msg, int level = 0, std::ostream &out = std::cerr);

/**
 * logInfo ()
 * Output the message to the stream out (default: std::cerr) when the
 * level is equal or larger than the logging level. The function
 * prepends a "[INFO] " string and ends the message with an end-of-line.
 */
void logInfo(const CString &msg, int level = 0, std::ostream &out = std::cerr);
void logInfo(const char *msg, int level = 0, std::ostream &out = std::cerr);
void logInfo(const std::string& msg, int level = 0, std::ostream &out = std::cerr);

/**
 * logWarning ()
 * Output the message to the stream out (default: std::cerr) when the
 * level is equal or larger than the logging level. The function
 * prepends a "[WARNING] " string and ends the message with an end-of-line.
 */
void logWarning(const CString &msg, int level = 0, std::ostream &out = std::cerr);
void logWarning(const char *msg, int level = 0, std::ostream &out = std::cerr);
void logWarning(const std::string& msg, int level = 0, std::ostream &out = std::cerr);

/**
 * logError ()
 * Output the message to the stream out (default: clog) when the
 * level is equal or larger than the logging level.The function
 * prepends a "[ERROR] " string and ends the message with an end-of-line.
 */
void logError(const CString &msg, int level = 0, std::ostream &out = std::cerr);
void logError(const char *msg, int level = 0, std::ostream &out = std::cerr);

#endif
