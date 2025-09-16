/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   shell.h
 *
 *  Author          :   Marc Geilen (m.c.w.geilen@tue.nl)
 *
 *  Date            :   August 12, 2010
 *
 *  Function        :   Cross-platform support functions
 *                      running shell scripts
 *
 *  History         :
 *      12-08-10    :   Initial version.
 *
 * $Id: shell.h,v 1.1.4.7 2010-08-18 07:51:34 mgeilen Exp $
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

#ifndef BASE_SHELL_SHELL_H_INCLUDED
#define BASE_SHELL_SHELL_H_INCLUDED

#include "base/string/cstring.h"
#include <memory>

constexpr size_t MemoryLimit = 4095;

class ShellWin;

/**
 * Shell
 * Abstract class representing the platform dependent implementations for shell functionality.
 * Has platform specific subclasses ShellWin and ShellNix
 */
class Shell {
public:
    Shell() = default;
    virtual ~Shell() = default;

    // Prevent copying and moving
    Shell(const Shell&) = delete;
    Shell& operator=(const Shell&) = delete;
    Shell(Shell&&) = delete;
    Shell& operator=(Shell&&) = delete;


    /**
     * Special return values.
     */
    static const int RESULT_TIMEOUT = -9999;
    static const int RESULT_MEMORY_LIMIT = -19999;
    static const int RESULT_OK = 0;
    /**
     * do the necessary initializations
     */
    virtual void initialize() {};

    /**
     * execute bash script named script in directory dir
     * return stdout and stderr output of run in redstdout and redstderr
     * return false in case of timeout, true otherwise
     */
    virtual int execute(const CString &dir,
                        const CString &script,
                        std::unique_ptr<CString> *redstdout,
                        std::unique_ptr<CString> *redstderr,
                        uint timeout,
                        uint memoryLimit) = 0;

    /**
     * read a file into a string
     */
    [[nodiscard]] virtual std::unique_ptr<CString> readFile(const CString &fullPath) const = 0;
};

/**
 * function to act as a factory to create the shell appropriate for the platform being built.
 */
std::unique_ptr<Shell> createShell();

#endif
