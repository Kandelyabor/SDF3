/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sequence.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   May 7, 2007
 *
 *  Function        :   Sequence class
 *
 *  History         :
 *      07-05-07    :   Initial version.
 *
 * $Id: sequence.h,v 1.5 2008-11-01 16:03:11 sander Exp $
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

#ifndef BASE_SEQUENCE_SEQUENCE_H_INCLUDED
#define BASE_SEQUENCE_SEQUENCE_H_INCLUDED

#include "base/basic_types.h"
#include "base/exception/exception.h"
#include "base/string/cstring.h"

/**
 * CSequence
 * Sequence class template
 */
template <class T> class CSequence : public std::vector<T> {
public:
    // Constructor
    CSequence() = default;
    explicit CSequence(T t) { this->push_back(t); };
    CSequence(size_t n, const T &t) { this->resize(n, t); };
    explicit CSequence(const CString &t) {
        CStrings ta;

        // Split string on comma
        ta = t.split(',');

        // Create vector
        for (const auto &iter : ta) {
            CString element = CString(iter).trim();
            CString counter = "";

            char c = 0;
            bool counterFound = false;

            do {
                c = element[0];
                element = CString(element.substr(1));

                if (c == '*') {
                    counterFound = true;
                    break;
                }

                counter += c;

            } while (!element.empty());

            if (counterFound && element.trim().empty()) {
                throw CException("[Error] Invalid rate sequence.");
            }

            if (counterFound) {
                constexpr int RadixTen = 10;
                for (uint i = 0;
                     i != static_cast<uint>(strtol(static_cast<const char *>(counter), nullptr, RadixTen));
                     i++) {
                    if (!element.isNNInteger()) {
                        throw CException("[Error] Execution time must be non-negative integer.");
                    }
                    this->push_back(static_cast<T>(element));
                }
            } else {
                if (!counter.isNNInteger()) {
                    throw CException("[Error] Execution time must be non-negative integer.");
                }
                this->push_back(static_cast<T>(counter));
            }
        }
    };

    // Destructor
    ~CSequence() = default;

        // copying and moving
    CSequence(const CSequence&) = default;
    CSequence& operator=(const CSequence&) = default;
    CSequence(CSequence&&) = default;
    CSequence& operator=(CSequence&&) = default;

    // Size
    [[nodiscard]] size_t size() const {
        return std::vector<T>::size();
    };

    // operator[]
    T &operator[](const int i) {
        return std::vector<T>::at((i + size()) % size());
    }

    T at(const int i) const {
        return std::vector<T>::at((i + size()) % size());
    }


    // Print sequence
    std::ostream &print(std::ostream &out) const {
        for (uint i = 0; i < size(); i++) {
            if (i != 0) {
                out << ",";
            }
            out << (*this).at(i);
        }

        return out;
    }

    friend std::ostream &operator<<(std::ostream &out, const CSequence &s) { return s.print(out); };
};

#endif
