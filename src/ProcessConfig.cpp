#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <Windows.h>
#include <TlHelp32.h>

#include "eval.h"
#include "ProcessConfig.h"

bool ProcessConfig::runScan = true;
std::shared_ptr<ProcessConfig> ProcessConfig::activeConfig = nullptr;
std::vector<std::shared_ptr<ProcessConfig>> ProcessConfig::configs = std::vector<std::shared_ptr<ProcessConfig>>();

HANDLE GetProcessByName(std::string name)
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
// Compare process.szExeFile based on format of name, i.e., trim file path
// trim .exe if necessary, etc.
#ifdef UNICODE
            std::wstring exeFile(process.szExeFile);
            std::wstring largeName(name.begin(), name.end());
            std::ranges::transform(exeFile, exeFile.begin(), [](uint8_t c) { return std::tolower(c); });
            std::ranges::transform(largeName, largeName.begin(), [](uint8_t c) { return std::tolower(c); });
            bool comp = exeFile == largeName;
#else
            std::string exeFile(process.szExeFile);
            std::ranges::transform(exeFile, exeFile.begin(), [](uint8_t c) { return std::tolower(c); });
            std::ranges::transform(name, name.begin(), [](uint8_t c) { return std::tolower(c); });
            bool comp = exeFile == name;
#endif
            if (comp)
            {
                pid = process.th32ProcessID;
                break;
            }
        } while (Process32Next(snapshot, &process));
    }

    CloseHandle(snapshot);

    if (pid != 0)
    {
        return OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    }

    // Not found

    return NULL;
}

ProcessConfig::ProcessConfig(json process, int w, int h): w(w), h(h)
{
    this->processName = process.at("processName").template get<std::string>();
    if (process.contains("widthRule"))
    {
        this->widthRule = process.at("widthRule").template get<std::string>();
        this->widthRule.replace(widthRule.find("%w"), 2, std::to_string(this->w));
    }
    if (process.contains("heightRule"))
    {
        this->heightRule = process.at("heightRule").template get<std::string>();
        this->heightRule.replace(heightRule.find("%h"), 2, std::to_string(this->h));
    }
    if (process.contains("x"))
    {
        this->x = process.at("x").template get<int>();
    }
    else
    {
        this->x = -1;
    }
    if (process.contains("y"))
    {
        this->y = process.at("y").template get<int>();
    }
    else
    {
        this->y = -1;
    }
}

int ProcessConfig::calcX()
{
    if (this->x < 0 && this->widthRule.size() > 0)
    {
        return Math::eval(this->widthRule);
    }
    return std::max(x, 0);
}

int ProcessConfig::calcY()
{
    if (this->y < 0 && this->heightRule.size() > 0)
    {
        return Math::eval(this->heightRule);
    }
    return std::max(y, 0);
}

void ProcessConfig::scanForProcesses()
{
    while (runScan)
    {
        if (ProcessConfig::configs.size() == 0)
            break;

        if (activeConfig == nullptr)
        {
            for (std::shared_ptr<ProcessConfig> &cfg : configs)
            {
                HANDLE ps = GetProcessByName(cfg->processName);

                if (ps)
                {
                    activeConfig = cfg;
                }
            }
        }
        else
        {
            HANDLE ps = GetProcessByName(activeConfig->processName);
            if (!ps)
            {
                activeConfig = nullptr;
            }
        }

        // 1 Sec
        Sleep(1000);
    }
}

void ProcessConfig::addToConfigs(ProcessConfig *cfg)
{
    ProcessConfig::configs.push_back(std::shared_ptr<ProcessConfig>(cfg));
}
