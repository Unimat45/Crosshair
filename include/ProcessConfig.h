#ifndef PROCESSCONFIG_H
#define PROCESSCONFIG_H

#include <memory>
#include <mutex>
#include <string>
#include <vector>


class ProcessConfig
{
  public:
    std::string processName;
    int x;
    int y;
    int w;
    int h;

  private:
    static bool runScan;
    static std::vector<std::shared_ptr<ProcessConfig>> configs;

  public:
    static std::shared_ptr<ProcessConfig> activeConfig;
    static std::mutex cfgMutex;

  public:
    ProcessConfig(int w, int h) : x(w / 2), y(h / 2), w(w), h(h) {}

    inline static void stopLoop() { ProcessConfig::runScan = false; }
    static void scanForProcesses();
    static void addToConfigs(ProcessConfig *cfg);
};

#endif // PROCESSCONFIG_H