/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   exception.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   September 26, 2003
 *
 *  Function        :   Exception class
 *
 *  History         :
 *      26-09-03    :   Initial version.
 *
 * $Id: exception.h,v 1.1.1.1 2007-10-02 10:59:48 sander Exp $
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

#ifndef BASE_EXCEPTION_EXCEPTION_H
#define BASE_EXCEPTION_EXCEPTION_H

#include "base/basic_types.h" // IWYU pragma: keep
#include "base/string/cstring.h"
#include <exception>
#include <utility>

/*
 * CException
 * CException container class.
 */
class CException : std::exception {
private:
    CString message;
    CString cause;

public:
    // Constructor
    explicit CException(CString message) : message(std::move(message)) {}

    CException(CString message, CException &e) : message(std::move(message)) {
        cause = "    caused by: " + e.getMessage();
        if (!e.getCause().empty()) {
            cause += "\n" + e.getCause();
        }
    }

    CException(const CException &e) : std::exception(e), message(e.getMessage()), cause(e.getCause()) {}

    // Destructor
    ~CException() override = default;

    // Delete copy/move constructors and assignment operators
    CException& operator=(const CException&) = delete;
    CException(CException&&) = delete;
    CException& operator=(CException&&) = delete;

    // Message
    [[nodiscard]] CString getMessage() const { return message; }

    // Cause
    [[nodiscard]] CString getCause() const { return cause; }

    // Report
    std::ostream &report(std::ostream &stream) const {
        if (!getMessage().empty()) {
            stream << getMessage() << std::endl;
        }
        if (!getCause().empty()) {
            stream << getCause();
        }

        return stream;
    }

    friend std::ostream &operator<<(std::ostream &stream, const CException &e);
};

// NOLINTBEGIN (*macro-usage)

#undef ASSERT
#define ASSERT(condition, msg)                                                                     \
    {                                                                                              \
        if (!(condition))                                                                          \
            throw CException(CString(__FILE__) + CString(":") + CString(__LINE__) + CString(": ")  \
                             + CString(msg));                                                      \
    }

#undef EXCEPTION
#define EXCEPTION(msg, ...)                                                                        \
    {                                                                                              \
        char buf[1024];                                                                            \
        sprintf(&buf[0], msg, __VA_ARGS__);                                                        \
        throw CException(buf);                                                                     \
    }

#endif

// NOLINTEND (*macro-usage)
