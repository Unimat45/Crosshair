#ifndef PROCESSCONFIG_H
#define PROCESSCONFIG_H

#include <string>
#include <vector>
#include <memory>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

class ProcessConfig
{

private:
    std::string processName;
    std::string widthRule;
    std::string heightRule;
    int x;
    int y;
    int w;
    int h;

    static bool runScan;
    static std::vector<std::shared_ptr<ProcessConfig>> configs;

public:
    static std::shared_ptr<ProcessConfig> activeConfig;

public:
    ProcessConfig(json process, int w, int h);
    ProcessConfig(int w, int h) :
        widthRule("%w / 2"),
        heightRule("%h / 2"),
        w(w), h(h),
        x(-1), y(-1) {
            this->widthRule.replace(widthRule.find("%w"), 2, std::to_string(this->w));
            this->heightRule.replace(heightRule.find("%h"), 2, std::to_string(this->h));
        }
    ~ProcessConfig() {}

    int calcX();
    int calcY();

    inline static void stopLoop() { ProcessConfig::runScan = false; }
    static void scanForProcesses();
    static void addToConfigs(ProcessConfig *cfg);
};

#endif // PROCESSCONFIG_H