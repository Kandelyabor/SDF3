/*
*  TU Eindhoven
*  Eindhoven, The Netherlands
*
*  Name            :   shellnix.cc
*
*  Author          :   Marc Geilen (m.c.w.geilen@tue.nl)
*
*  Date            :   August 12, 2010
*
*  Function        :   Linux support functions for
*                       running shell scripts
*
*  History         :
*      31-10-11    :   Adjusted to support timeout and redirecting output
*                  :   via pipes. (M.Koedam)
*      12-08-10    :   Initial version.
*
* $Id: shellnix.cc,v 1.1.4.12 2010-08-18 20:20:08 mgeilen Exp $
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

#include "shellnix.h"
#include "dirs.h"
#include "base/string/cstring.h"
#include "base/tempfile/tempfile.h"

#include "stdlib.h"
#include <cstring>
#include <memory>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <errno.h>
#include <unistd.h>
#include <sstream>
#include <algorithm>

// should hold the working directory
#define ENVBUFSIZE  4096


/**
* constructor
*/
ShellNix::ShellNix(): Shell()
{
}

/**
 * ShellNix::execute
 * execute bash script named script in directory dir
 * return stdout and stderr output of run in redstdout and redstderr
 * return false in case of timeout, true otherwise
 */

 int ShellNix::execute(const CString &dir, const CString &script, std::unique_ptr<CString> *redstdout,
                            std::unique_ptr<CString> *redstderr, uint timeout, uint memoryLimit)
{
    bool hasTimedout = false;

    // buffer for current working directory
    char cwdbuf[ENVBUFSIZE];
    if (getcwd(cwdbuf, ENVBUFSIZE) != cwdbuf)
        throw new CShellException("Could not read current working dir.");

    // change current directory to dir
    if (chdir(dir.c_str()) != 0)
        throw new CShellException("Could not change the working directory.");

    // Create output redirect pipes.
    int out[2];
    int err[2];
    // STDout.
    int c  = pipe(out);
    if (c < 0)
    {
        throw CException(CString("Failed to create pipe: ") + (const char *)strerror(errno));
    }
    // STDerr
    c  = pipe(err);
    if (c < 0)
    {
        close(out[0]);
        close(out[1]);
        throw CException(CString("Failed to create pipe: ") + (const char *)strerror(errno));
    }

    // Fork and start execute script.
    pid_t pid = fork();
    if (pid == 0)
    {
        CStrings args = script.split(' ');
        char **argv = new char *[args.size() + 1];
        int i = 0;
        for (CStrings::iterator iter = args.begin(); iter != args.end(); iter++)
        {
            CString arg = *iter;
            argv[i] = strdup(arg.c_str());
            i++;
        }
        argv[i] = NULL;
        // Set memory limit.
        struct rlimit mem_limit;
        mem_limit.rlim_cur = 1024 * 1024 * memoryLimit;
        mem_limit.rlim_max = 1024 * 1024 * memoryLimit;
        setrlimit(RLIMIT_AS, &mem_limit);

        // child
        close(out[0]);
        close(err[0]);
        dup2(out[1], 1);
        dup2(err[1], 2);
        execvp(argv[0], argv);
        delete [] argv;
        exit(EXIT_FAILURE);
    }
    else if (pid > 0)
    {
        //parent
        close(out[1]);
        close(err[1]);
    }
    else
    {
        // Error
        close(err[0]);
        close(err[1]);
        close(out[0]);
        close(out[1]);
        throw CException(CString("Failed to fork: ") + (const char *)strerror(errno));
    }

    // Start listening to pipes.
    // Read the pipes and put them in the CStrings.
    fd_set  rfds;

    struct timeval stop, now;
    struct timeval tv;


    gettimeofday(&stop, NULL);
    stop.tv_sec += timeout / 1000;
    stop.tv_usec += timeout % 1000;

    *redstdout = std::make_unique<CString>();
    *redstderr = std::make_unique<CString>();
    while (true)
    {
        // Set fd's to watch.
        FD_ZERO(&rfds);
        if (out[0] >= 0)
            FD_SET(out[0], &rfds);
        if (err[0] >= 0)
            FD_SET(err[0], &rfds);

        // Recalculate timeout.
        gettimeofday(&now, NULL);
        timersub(&stop, &now, &tv);

        // Check if we are past our timeout.
        if (tv.tv_sec < 0 || tv.tv_usec < 0)
        {
            close(out[0]);
            close(err[0]);
            out[0] = err[0] = -1;
            kill(pid, SIGTERM);
            hasTimedout = true;
            break;
        }
        // Wait for input of pipe.
        int ret = select(std::max(out[0], err[0]) + 1, &rfds, NULL, NULL, &tv);
        if (ret > 0)
        {
            // data for reading
            if ( out[0] >= 0 && FD_ISSET(out[0], &rfds))
            {
                char buffer[128];
                ssize_t t = read(out[0], buffer, 127);
                buffer[t] = '\0';
                if (t > 0)
                {
                    (**redstdout) += buffer;
                }
                else
                {
                    // Means end of file or error, bail out then.
                    close(out[0]);
                    out[0] = -1;
                }
            }
            if ( err[0] >= 0 && FD_ISSET(err[0], &rfds))
            {
                char buffer[128];
                ssize_t t = read(err[0], buffer, 127);
                buffer[t] = '\0';
                if (t > 0)
                {
                    (**redstderr) += buffer;
                }
                else
                {
                    // Means end of file or error, bail out then.
                    close(err[0]);
                    err[0] = -1;
                }
            }
        }
        else if (ret == -1)
        {
            // Error
            close(out[0]);
            close(err[0]);
            out[0] = err[0] = -1;
            kill(pid, SIGTERM);
            break;
        }
        else
        {
            // timeout.
            close(out[0]);
            close(err[0]);
            out[0] = err[0] = -1;
            kill(pid, SIGTERM);
            hasTimedout = true;
            break;
        }

        // Check if there are FD's left.
        if (out[0] < 0 && err[0] < 0)
        {
            break;
        }
    }
    assert(out[0] == -1 && err[0] == -1);


    int res = 0;
    // Get child process exit status.
    waitpid(pid, &res, 0);


    if (hasTimedout)
    {
        return Shell::RESULT_TIMEOUT;
    }
    // Check if we exited normally, or with error.
    if (WIFSIGNALED(res))
    {
        struct rusage usage;
        // Abort signal is called when passing memory limit.
        if (WTERMSIG(res) == 6)
            return Shell::RESULT_MEMORY_LIMIT;

        // If memory usage was > 90% off maximum,
        // Assume we fail because of memory failure.
        // Sometimes a failure to allocate memory results
        // in a segfault.
        getrusage(RUSAGE_CHILDREN, &usage);
        if (usage.ru_maxrss >= memoryLimit * 1024 * 0.90)
        {
            return Shell::RESULT_MEMORY_LIMIT;
        }
        return res;
    }
    if (!WIFEXITED(res) || WEXITSTATUS(res) == 127)
    {
        //throw CShellException("execution of script failed: " + CString(res));
    }
    res = WEXITSTATUS(res);
    return res;
}

/**
 * ShellNix::initialize
 * opportunity to do any necessary initialization
 */
void ShellNix::initialize(void)
{
}

/**
 * ShellNix::readFile
 * read a file into a string
 */
std::unique_ptr<CString> ShellNix::readFile(const CString &fullPath) const
{
    // buffer to hold single line
    CString line;
    // stream for the file
    std::ifstream thefile;

    // string stream to generate resulting string
    std::stringstream out(std::stringstream::out);

    // open the file
    thefile.open(fullPath, std::ifstream::in);

    // if successful
    if (thefile.is_open())
    {
        // read all lines
        while (! thefile.eof())
        {
            getline(thefile, line);
            out << line << std::endl;
        }
        thefile.close();
    }
    else
    {
        // there was a problem
        throw CShellException("cannot open file: " + fullPath);
    }

    // return result
    return std::make_unique<CString>(out.str());

}
