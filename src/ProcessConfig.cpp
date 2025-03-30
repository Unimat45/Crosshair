#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifdef UNICODE
#undef UNICODE
#endif

#include <Windows.h>
#include <TlHelp32.h>
#include <algorithm>

#include "ProcessConfig.h"

bool ProcessConfig::runScan = true;
std::mutex ProcessConfig::cfgMutex = std::mutex();
std::shared_ptr<ProcessConfig> ProcessConfig::activeConfig = nullptr;
std::vector<std::shared_ptr<ProcessConfig>> ProcessConfig::configs =
    std::vector<std::shared_ptr<ProcessConfig>>();

bool GetProcessByName(std::string name)
{
    DWORD pid = 0;

    // Create toolhelp snapshot.
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 process;
    ZeroMemory(&process, sizeof(process));
    process.dwSize = sizeof(process);

    // Walkthrough all processes.
    if (Process32First(snapshot, &process))
    {
        do
        {
            // Compare process.szExeFile to name both as lowercase strings
            std::string exeFile(process.szExeFile);
            std::ranges::transform(exeFile, exeFile.begin(),
                                   [](uint8_t c) { return std::tolower(c); });
            std::ranges::transform(name, name.begin(),
                                   [](uint8_t c) { return std::tolower(c); });

            if (exeFile == name)
            {
                // If found, sets the pid and exits loop
                pid = process.th32ProcessID;
                break;
            }
        } while (Process32Next(snapshot, &process));
    }

    CloseHandle(snapshot);

    return pid != 0;
}

void ProcessConfig::scanForProcesses()
{
    while (runScan)
    {
        // If there are no configs, no need to search for processes
        if (ProcessConfig::configs.size() == 0)
            break;

        if (activeConfig == nullptr)
        {
            // If no configs are loaded, search for a match in running processes
            for (std::shared_ptr<ProcessConfig> &cfg : configs)
            {
                if (GetProcessByName(cfg->processName))
                {
                    cfgMutex.lock();
                    activeConfig = cfg;
                    cfgMutex.unlock();

                    // Keep the first found config, but stay in main loop
                    break;
                }
            }
        }
        else
        {
            // If a config is already loaded, check if the process is still
            // running, or unloads the config
            if (!GetProcessByName(activeConfig->processName))
            {
                cfgMutex.lock();
                activeConfig = nullptr;
                cfgMutex.unlock();
            }
        }

        // 1 Sec
        Sleep(1000);
    }
}

void ProcessConfig::addToConfigs(ProcessConfig *cfg)
{
    // Converts the pointer to a shared ptr for easy management
    ProcessConfig::configs.push_back(std::shared_ptr<ProcessConfig>(cfg));
}
