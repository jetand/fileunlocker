#include "pch.h"
#include <iostream>
#include <filesystem>
#include <chrono>
#include <cstdio>
#include <vector>

#include <windows.h>
#include <RestartManager.h>

#pragma comment(lib, "Rstrtmgr.lib")

static wchar_t *convertCharArrayToLPCWSTR(const char* charArray)
{
    wchar_t* wString = new wchar_t[4096];
    MultiByteToWideChar(CP_ACP, 0, charArray, -1, wString, 4096);
    return wString;
}

static std::vector<DWORD> getFileLockingProcesses(const std::string& filePath, bool debugMode=false)
{
    std::vector<DWORD> result;
    DWORD dwSession;
    WCHAR szSessionKey[CCH_RM_SESSION_KEY + 1] = { 0 };
    DWORD dwError = RmStartSession(&dwSession, 0, szSessionKey);
    if (debugMode)
    {
        wprintf(L"RmStartSession returned %d\n", dwError);
    }

    if (dwError == ERROR_SUCCESS) {
        PCWSTR pszFile = convertCharArrayToLPCWSTR(filePath.c_str());
        dwError = RmRegisterResources(dwSession, 1, &pszFile, 0, NULL, 0, NULL);
        if (debugMode)
        {
            wprintf(L"RmRegisterResources(%ls) returned %d\n", pszFile, dwError);
        }
        if (dwError == ERROR_SUCCESS)
        {
            DWORD dwReason;
            UINT i;
            UINT nProcInfoNeeded;
            UINT nProcInfo = 10;
            RM_PROCESS_INFO rgpi[10] = { 0 };
            dwError = RmGetList(dwSession, &nProcInfoNeeded, &nProcInfo, rgpi, &dwReason);
            if (debugMode)
            {
                wprintf(L"RmGetList returned %d\n", dwError);
            }
            if (dwError == ERROR_SUCCESS)
            {
                if (debugMode)
                {
                    wprintf(L"RmGetList returned %d infos (%d needed)\n", nProcInfo, nProcInfoNeeded);
                }
                if (nProcInfo == 0)
                {
                    RmEndSession(dwSession);
                    return result;
                }
                for (i = 0; i < nProcInfo; i++)
                {
                    if (debugMode)
                    {
                        wprintf(L"%d.ApplicationType = %d\n", i, rgpi[i].ApplicationType);
                        wprintf(L"%d.strAppName = %ls\n", i, rgpi[i].strAppName);
                        wprintf(L"%d.Process.dwProcessId = %d\n", i, rgpi[i].Process.dwProcessId);
                    }
                    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION,
                        FALSE, rgpi[i].Process.dwProcessId);
                    if (hProcess)
                    {
                        FILETIME ftCreate, ftExit, ftKernel, ftUser;
                        if (GetProcessTimes(hProcess, &ftCreate, &ftExit,
                            &ftKernel, &ftUser) &&
                            CompareFileTime(&rgpi[i].Process.ProcessStartTime,
                                &ftCreate) == 0) {
                            WCHAR sz[MAX_PATH];
                            DWORD cch = MAX_PATH;
                            if (QueryFullProcessImageNameW(hProcess, 0, sz, &cch) &&
                                cch <= MAX_PATH) {
                                wprintf(L"  = %ls\n", sz);
                            }
                        }
                        CloseHandle(hProcess);
                    }
                    result.push_back(rgpi[i].Process.dwProcessId);
                }
            }
        }
        RmEndSession(dwSession);
    }
    return result;
}

static void closeFileLockingProcesses(const std::string& filePath, bool debugMode=false)
{
    std::vector<DWORD> lockers = getFileLockingProcesses(filePath, false);
    for (auto id : lockers)
    {
        auto handle = OpenProcess(PROCESS_ALL_ACCESS, TRUE, id);
        if (handle)
        {
            if (debugMode)
            {
                wprintf(L"About to terminate process id %d\n", id);
            }
            if (TerminateProcess(handle, 0))
            {
                wprintf(L"Terminated process id %d\n", id);
            }
            else
            {
                wprintf(L"Failed to terminate process %did \n", id);
            }
        }
    }
}

static void closeLockedFilesRecursive(const std::filesystem::path& path, bool debugMode=false)
{
    std::cout << "Closing processes\r";
    int dots = 0;
    std::vector<std::string> files;
    auto start = std::chrono::system_clock::now();
    std::cout << "Listing files             \r";
    for (const auto& p : std::filesystem::recursive_directory_iterator(path))
    {
        if (!std::filesystem::is_directory(p))
        {
            files.push_back(p.path().string());
            if (std::chrono::system_clock::now() >= (start + std::chrono::seconds(1)))
            {
                dots++;
                if (dots > 3)
                {
                    dots = 0;
                }
                std::cout << "Listing files                 " << '\r';
                std::cout << "Listing files " << files.size() << '\r';
                start = std::chrono::system_clock::now();
            }
        }
    }
    std::cout << "\nListing files completed. Found " << files.size() << " files total\n";
    std::cout << "Closing open files                                \r";
    int count = 0;
    for (const auto& filePath : files)
    {
        if (std::chrono::system_clock::now() >= (start + std::chrono::milliseconds(100)))
        {
            dots++;
            if (dots > 3)
            {
                dots = 0;
            }
            std::cout << "Closing open files                                                     \r";
            std::cout << "Closing open files " << count << " out of " << files.size() << ' ' << std::string(dots, '.') << '\r';
            start = std::chrono::system_clock::now();
        }

        closeFileLockingProcesses(filePath, debugMode);
        count++;
    }
    std::cout << "Closing open files                                                     \r";
    std::cout << "Closing open files " << count << " out of " << files.size() << ' ' << std::string(dots, '.') << '\r';
    std::cout << "\nDone.\n";
}

int main(int argc, const char* argv[])
{
    if (argc < 2)
    {
        std::cout << "Usage: " << argv[0] << " path-to-folder\n";
        return 0;
    }
    closeLockedFilesRecursive(std::string(argv[1]), true);
    std::cout << "Press enter finish\n";
    (void)std::getchar();

    return 0;
}

