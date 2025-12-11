#pragma once

#include "models/Device.h"
#include "models/SignalMapping.h"
#include <map>
#include <mutex>
#include <optional>
#include <yaml-cpp/yaml.h>

class IOSignalService
{
public:
    void applyMappings(const std::string &deviceName, const std::vector<SignalMapping> &mappings);
    std::vector<SignalMapping> mappings(const std::string &deviceName);
    std::vector<SignalValue> snapshot(const std::string &deviceName);
    struct AssemblyData
    {
        std::vector<uint8_t> input;
        std::vector<uint8_t> output;
    };
    std::optional<AssemblyData> assemblies(const std::string &deviceName) const;
    bool applyOutputBytes(const std::string &deviceName, const std::vector<uint8_t> &bytes);
    bool setOutputValue(const std::string &deviceName, const std::string &signalName, double engineeringValue);
    void consumeInputBytes(const std::string &deviceName, const std::vector<uint8_t> &data);
    void fillOutputBytes(const std::string &deviceName, std::vector<uint8_t> &buffer);

    std::string exportMappingsYaml(const std::string &deviceName) const;
    Json::Value exportMappingsJson(const std::string &deviceName) const;
    std::vector<SignalMapping> importMappings(const std::string &serialized, bool yaml);

private:
    struct DeviceSignals
    {
        std::vector<SignalMapping> mappings;
        std::map<std::string, double> outputs;
        std::map<std::string, double> inputs;
        std::vector<uint8_t> lastInput;
        std::vector<uint8_t> lastOutput;
    };

    mutable std::mutex mutex_;
    std::map<std::string, DeviceSignals> devices_;

    static double decodeValue(const SignalMapping &mapping, const std::vector<uint8_t> &data);
    static void encodeValue(const SignalMapping &mapping, double engineeringValue, std::vector<uint8_t> &buffer);
};

class IOSignalServiceProvider
{
public:
    static IOSignalService *instance()
    {
        static IOSignalService service;
        return &service;
    }
};

