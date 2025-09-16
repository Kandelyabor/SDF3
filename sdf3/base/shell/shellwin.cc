/*
*  TU Eindhoven
*  Eindhoven, The Netherlands
*
*  Name            :   shell.cc
*
*  Author          :   Marc Geilen (m.c.w.geilen@tue.nl)
*
*  Date            :   August 12, 2010
*
*  Function        :   Windows support functions
*                       running shell scripts
*
*  History         :
*      12-08-10    :   Initial version.
*
* $Id: shellwin.cc,v 1.1.4.11 2010-08-18 15:38:20 mgeilen Exp $
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

// platform includes
// NOLINTBEGIN(*include-order)
#include <windows.h>
#include <psapi.h>

#include "dirs.h"
#include "shellwin.h"
#include "base/string/cstring.h"
#include "base/tempfile/tempfile.h"
// NOLINTEND(*include-order)


#define ENVBUFSIZE  4096
// note: script output should not exceed this size:
#define FILEBUFSIZE 65536

// Avoid VS complaints about unsafe (but standard) string operations
#pragma warning( disable : 4995 )



/**
* constructor
*/
ShellWin::ShellWin() = default;

/**
 * ShellWin::execute
 * execute bash script named script in directory dir
 * return stdout and stderr output of run in redstdout and redstderr
 * return false in case of timeout, true otherwise
 */
int ShellWin::execute(const CString &dir, const CString &script,
                      std::unique_ptr<CString> *redstdout, std::unique_ptr<CString> *redstderr, uint timeout,
                      uint memoryLimit) {
    // temporary files to hold stdout and stderr
    //NOLINTBEGIN(*-decay)
    char tempbuf[ENVBUFSIZE];
    char *filepart = nullptr;
    CString tmpStdOutR = tempFileName(dir, "output");
    GetFullPathName(tmpStdOutR.c_str(), ENVBUFSIZE, tempbuf, &filepart);
    CString tmpstdout = tempbuf;

    CString tmpStdErrR = tempFileName(dir, "output");
    GetFullPathName(tmpStdErrR.c_str(), ENVBUFSIZE, tempbuf, &filepart);
    CString tmpstderr = tempbuf;

    //NOLINTEND(*-decay)

    // create and run the child process to run script in directory dir
    int completed = ShellWin::createChildProcess(dir, script, tmpstdout, tmpstderr,
                                            timeout, memoryLimit);

    // read the file contents to a string
    (*redstdout) = this->readFile(tmpstdout);
    (*redstderr) = this->readFile(tmpstderr);

    // delete the temporary files
    if ((!DeleteFile(tmpstdout.c_str())) || (!DeleteFile(tmpstderr.c_str()))) {
        throw CShellException("Could not delete temporary output file.");
}

    return completed;
}

/**
 * ShellWin::initialize
 * opportunity to do any necessary initialization
 */
void ShellWin::initialize()
{
}

/**
 * ShellWin::createChildProcess
 * create a child process to execute the script
 * return false in case of timeout
 */
int ShellWin::createChildProcess(const CString &dir, const CString &script,
                                 const CString &tmpstdout,
                                 const CString &tmpstderr, uint timeout,
                                 uint memoryLimit) {
    JOBOBJECT_EXTENDED_LIMIT_INFORMATION jeLi = {0};
    HANDLE hJob = CreateJobObject(nullptr, nullptr);

    if (hJob == nullptr) {
        throw CShellException("CreateJobObject");
}

    jeLi.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_JOB_MEMORY;
    constexpr size_t BytesPerKilobyte = 1024;
    constexpr size_t KilobytesPerMegabyte = 1024;
    jeLi.JobMemoryLimit = BytesPerKilobyte * KilobytesPerMegabyte * memoryLimit;

    if (SetInformationJobObject(hJob, JobObjectExtendedLimitInformation, &jeLi,
                                 sizeof(jeLi)) == 0) {
        throw CShellException("SetInformationJobObject");
    }

    int completed = -1;

    // copy to a buffer which may be modified in the call
    auto szCmdline = std::make_unique<TCHAR[]>(script.length() + 1);
    strcpy(szCmdline.get(), script.c_str()); // NOLINT(*deprecated-declarations)

    SECURITY_ATTRIBUTES saAttr;
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = nullptr;

    // Create output handles
    HANDLE hOutputFile = CreateFile(
        tmpstdout.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ,
        &saAttr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hOutputFile == INVALID_HANDLE_VALUE) {
        throw CShellException("CreateStdOutFile");
    }

    HANDLE hErrorFile = CreateFile(
        tmpstderr.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ,
        &saAttr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hErrorFile == INVALID_HANDLE_VALUE) {
        throw CShellException("CreateStdErrFile");
    }

    // process information and startup information for the new child process
    // Set up members of the PROCESS_INFORMATION structure.
    PROCESS_INFORMATION piProcInfo;
    ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

    // Set up members of the STARTUPINFO structure.
    STARTUPINFO siStartInfo = {0};
    ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
    siStartInfo.cb = sizeof(STARTUPINFO);
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES;
    siStartInfo.hStdInput = nullptr;
    siStartInfo.hStdOutput = hOutputFile;
    siStartInfo.hStdError = hErrorFile;

    // Create the child process.
    BOOL bSuccess = CreateProcess(nullptr,
                                  szCmdline.get(),    // command line
                                  nullptr,      // process security attributes
                                  nullptr,       // primary thread security attributes
                                  TRUE,             // handles are inherited
                                  CREATE_SUSPENDED, // creation flags
                                  nullptr,            // use parent's environment
                                  dir.c_str(),      // set current directory
                                  &siStartInfo,       // STARTUPINFO pointer
                                  &piProcInfo);// receives PROCESS_INFORMATION

    // If an error occurred, raise an exception
    if (bSuccess == 0) {
        throw CShellException("CreateProcess");
    }

    if (AssignProcessToJobObject(hJob, piProcInfo.hProcess) == 0) {
        throw CShellException("AssignProcessToJobObject");
    }

    ResumeThread(piProcInfo.hThread);

    // otherwise, wait for child process to end
    DWORD result = WaitForSingleObject(piProcInfo.hProcess, timeout);

    if (result == WAIT_TIMEOUT) {
        completed = Shell::RESULT_TIMEOUT;

        // The process did not complete within the timeout period. Kill it.
        if (TerminateJobObject(hJob, 1) == 0) {
            throw CShellException("TerminateJobObject");
        }
    }

    PROCESS_MEMORY_COUNTERS pmc;
    if (!GetProcessMemoryInfo(piProcInfo.hProcess, &pmc, sizeof(pmc))) {
        throw CShellException("GetProcessMemoryInfo");
    }

    // If memory usage was > 90% off maximum, assume we fail because of memory failure.
    // Sometimes a failure to allocate memory results in a segfault.
    constexpr double MemoryUsageThreshold = 0.90;
    if (pmc.PeakPagefileUsage > static_cast<size_t>(static_cast<double>(BytesPerKilobyte * KilobytesPerMegabyte * memoryLimit) * MemoryUsageThreshold)) {
        completed = Shell::RESULT_MEMORY_LIMIT;
    }

    DWORD exitCode = 0;
    if (GetExitCodeProcess(piProcInfo.hProcess, &exitCode) == 0) {
        throw CShellException("GetExitCodeProcess");
    }

    // Close handles to the child process and its primary thread.
    CloseHandle(piProcInfo.hProcess);
    CloseHandle(piProcInfo.hThread);
    CloseHandle(hOutputFile);
    CloseHandle(hErrorFile);

    if (completed == -1) {
        return static_cast<int>(exitCode);
    }

    return completed;
}

/**
 * ShellWin::readFile
 * read a file into a string
 */
std::unique_ptr<CString> ShellWin::readFile(const CString &fullPath) const
{

    // open the file for reading
    HANDLE hFile = CreateFile(fullPath.c_str(),      // file to open
                              GENERIC_READ,          // open for reading
                              FILE_SHARE_READ,       // share for reading
                              nullptr,               // default security
                              OPEN_EXISTING,         // existing file only
                              FILE_ATTRIBUTE_NORMAL, // normal file
                              nullptr);              // no attr. template

    // was there a problem?
    if (hFile == INVALID_HANDLE_VALUE)
    {
        throw CShellException("Could not open file: " + fullPath);
    }

    // Read one character less than the buffer size to save room for
    // the terminating nullptr character.
    DWORD  dwBytesRead = 0;
    char   ReadBuffer[FILEBUFSIZE] = {0};
    if (ReadFile(hFile, ReadBuffer, FILEBUFSIZE - 1, &dwBytesRead, nullptr) == FALSE) { // NOLINT(*decay)
        CloseHandle(hFile);
        throw CShellException("Error reading file: " + fullPath);
    }

    // close the file
    CloseHandle(hFile);

    // does the number of bytes read make sense?
    if (dwBytesRead <= FILEBUFSIZE)
    {
        ReadBuffer[dwBytesRead] = '\0'; // nullptr character NOLINT(*array-index)
        return std::make_unique<CString>(ReadBuffer);
    }
    throw CShellException("Unexpected value for dwBytesRead");
}


