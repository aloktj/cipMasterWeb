#include "IOSignalService.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <memory>
#include <sstream>

namespace
{
uint16_t readLE16(const std::vector<uint8_t> &data, size_t offset)
{
    if (offset + 2 > data.size())
    {
        return 0;
    }
    return static_cast<uint16_t>(data[offset] | (data[offset + 1] << 8));
}

uint32_t readLE32(const std::vector<uint8_t> &data, size_t offset)
{
    if (offset + 4 > data.size())
    {
        return 0;
    }
    return static_cast<uint32_t>(data[offset] | (data[offset + 1] << 8) | (data[offset + 2] << 16) |
                                 (data[offset + 3] << 24));
}

void writeLE16(uint16_t value, std::vector<uint8_t> &buffer, size_t offset)
{
    if (offset + 2 > buffer.size())
    {
        buffer.resize(offset + 2, 0);
    }
    buffer[offset] = static_cast<uint8_t>(value & 0xFF);
    buffer[offset + 1] = static_cast<uint8_t>((value >> 8) & 0xFF);
}

void writeLE32(uint32_t value, std::vector<uint8_t> &buffer, size_t offset)
{
    if (offset + 4 > buffer.size())
    {
        buffer.resize(offset + 4, 0);
    }
    buffer[offset] = static_cast<uint8_t>(value & 0xFF);
    buffer[offset + 1] = static_cast<uint8_t>((value >> 8) & 0xFF);
    buffer[offset + 2] = static_cast<uint8_t>((value >> 16) & 0xFF);
    buffer[offset + 3] = static_cast<uint8_t>((value >> 24) & 0xFF);
}
}

void IOSignalService::applyMappings(const std::string &deviceName, const std::vector<SignalMapping> &mappings)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto &device = devices_[deviceName];
    device.mappings = mappings;
    device.inputs.clear();
    device.outputs.clear();
    for (const auto &mapping : mappings)
    {
        if (mapping.direction == SignalDirection::Output)
        {
            device.outputs[mapping.name] = 0.0;
        }
        else
        {
            device.inputs[mapping.name] = 0.0;
        }
    }
}

std::vector<SignalMapping> IOSignalService::mappings(const std::string &deviceName)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = devices_.find(deviceName);
    if (it == devices_.end())
    {
        return {};
    }
    return it->second.mappings;
}

std::vector<SignalValue> IOSignalService::snapshot(const std::string &deviceName)
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<SignalValue> values;
    auto it = devices_.find(deviceName);
    if (it == devices_.end())
    {
        return values;
    }

    values.reserve(it->second.mappings.size());
    for (const auto &mapping : it->second.mappings)
    {
        SignalValue value;
        value.mapping = mapping;
        if (mapping.direction == SignalDirection::Output)
        {
            value.engineeringValue = it->second.outputs[mapping.name];
        }
        else
        {
            value.engineeringValue = it->second.inputs[mapping.name];
        }
        value.rawValue = mapping.scale != 0 ? (value.engineeringValue - mapping.engineeringOffset) / mapping.scale
                                             : value.engineeringValue;
        values.push_back(value);
    }
    return values;
}

bool IOSignalService::setOutputValue(const std::string &deviceName, const std::string &signalName, double engineeringValue)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = devices_.find(deviceName);
    if (it == devices_.end())
    {
        return false;
    }

    for (const auto &mapping : it->second.mappings)
    {
        if (mapping.name == signalName && mapping.direction == SignalDirection::Output)
        {
            it->second.outputs[signalName] = engineeringValue;
            return true;
        }
    }
    return false;
}

void IOSignalService::consumeInputBytes(const std::string &deviceName, const std::vector<uint8_t> &data)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto &device = devices_[deviceName];
    device.lastInput = data;
    for (const auto &mapping : device.mappings)
    {
        if (mapping.direction == SignalDirection::Input)
        {
            device.inputs[mapping.name] = decodeValue(mapping, data);
        }
    }
}

void IOSignalService::fillOutputBytes(const std::string &deviceName, std::vector<uint8_t> &buffer)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto &device = devices_[deviceName];
    for (const auto &mapping : device.mappings)
    {
        if (mapping.direction == SignalDirection::Output)
        {
            const double eng = device.outputs[mapping.name];
            encodeValue(mapping, eng, buffer);
        }
    }
    device.lastOutput = buffer;
}

std::string IOSignalService::exportMappingsYaml(const std::string &deviceName) const
{
    std::lock_guard<std::mutex> lock(mutex_);
    YAML::Node root;
    auto it = devices_.find(deviceName);
    if (it == devices_.end())
    {
        return "";
    }

    for (const auto &mapping : it->second.mappings)
    {
        YAML::Node node;
        node["name"] = mapping.name;
        node["direction"] = mapping.direction == SignalDirection::Input ? "input" : "output";
        switch (mapping.type)
        {
        case SignalType::Bool:
            node["type"] = "bool";
            break;
        case SignalType::UInt8:
            node["type"] = "uint8";
            break;
        case SignalType::UInt16:
            node["type"] = "uint16";
            break;
        case SignalType::UInt32:
            node["type"] = "uint32";
            break;
        case SignalType::SInt:
            node["type"] = "sint";
            break;
        case SignalType::Real32:
            node["type"] = "real32";
            break;
        }
        node["byteOffset"] = mapping.byteOffset;
        if (mapping.bitOffset.has_value())
        {
            node["bitOffset"] = *mapping.bitOffset;
        }
        node["scale"] = mapping.scale;
        node["engineeringOffset"] = mapping.engineeringOffset;
        node["units"] = mapping.units;
        if (!mapping.enums.empty())
        {
            YAML::Node enumsNode;
            for (const auto &opt : mapping.enums)
            {
                YAML::Node optNode;
                optNode["value"] = opt.value;
                optNode["label"] = opt.label;
                enumsNode.push_back(optNode);
            }
            node["enums"] = enumsNode;
        }
        root.push_back(node);
    }

    YAML::Emitter emitter;
    emitter << root;
    return emitter.c_str();
}

Json::Value IOSignalService::exportMappingsJson(const std::string &deviceName) const
{
    Json::Value root(Json::arrayValue);
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = devices_.find(deviceName);
    if (it == devices_.end())
    {
        return root;
    }
    for (const auto &mapping : it->second.mappings)
    {
        root.append(mapping.toJson());
    }
    return root;
}

std::vector<SignalMapping> IOSignalService::importMappings(const std::string &serialized, bool yaml)
{
    std::vector<SignalMapping> mappings;
    if (serialized.empty())
    {
        return mappings;
    }

    if (yaml)
    {
        YAML::Node root = YAML::Load(serialized);
        for (const auto &entry : root)
        {
            Json::Value json;
            for (const auto &kv : entry)
            {
                const auto key = kv.first.as<std::string>();
                const auto value = kv.second;
                if (value.IsScalar())
                {
                    try
                    {
                        json[key] = value.as<int>();
                    }
                    catch (...)
                    {
                        try
                        {
                            json[key] = value.as<double>();
                        }
                        catch (...)
                        {
                            json[key] = value.as<std::string>();
                        }
                    }
                }
                else if (value.IsSequence())
                {
                    Json::Value arr(Json::arrayValue);
                    for (const auto &inner : value)
                    {
                        Json::Value obj;
                        for (const auto &innerKv : inner)
                        {
                            const auto innerKey = innerKv.first.as<std::string>();
                            const auto scalar = innerKv.second;
                            try
                            {
                                obj[innerKey] = scalar.as<int>();
                            }
                            catch (...)
                            {
                                try
                                {
                                    obj[innerKey] = scalar.as<double>();
                                }
                                catch (...)
                                {
                                    obj[innerKey] = scalar.as<std::string>();
                                }
                            }
                        }
                        arr.append(obj);
                    }
                    json[key] = arr;
                }
            }
            mappings.push_back(SignalMapping::fromJson(json));
        }
    }
    else
    {
        Json::CharReaderBuilder builder;
        std::string errs;
        Json::Value root;
        std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
        if (!reader->parse(serialized.data(), serialized.data() + serialized.size(), &root, &errs))
        {
            throw std::runtime_error("Failed to parse mapping payload: " + errs);
        }
        if (root.isArray())
        {
            for (const auto &entry : root)
            {
                mappings.push_back(SignalMapping::fromJson(entry));
            }
        }
    }

    return mappings;
}

double IOSignalService::decodeValue(const SignalMapping &mapping, const std::vector<uint8_t> &data)
{
    const auto offset = mapping.byteOffset;
    switch (mapping.type)
    {
    case SignalType::Bool:
    {
        if (offset >= data.size())
        {
            return 0.0;
        }
        uint8_t byte = data[offset];
        if (mapping.bitOffset.has_value())
        {
            const uint8_t bit = *mapping.bitOffset;
            if (bit < 8)
            {
                byte = (byte >> bit) & 0x01;
            }
        }
        return byte ? 1.0 : 0.0;
    }
    case SignalType::UInt8:
        return offset < data.size() ? static_cast<double>(data[offset]) * mapping.scale + mapping.engineeringOffset : 0.0;
    case SignalType::UInt16:
        return (offset + 1 < data.size())
                   ? static_cast<double>(readLE16(data, offset)) * mapping.scale + mapping.engineeringOffset
                   : 0.0;
    case SignalType::UInt32:
        return (offset + 3 < data.size())
                   ? static_cast<double>(readLE32(data, offset)) * mapping.scale + mapping.engineeringOffset
                   : 0.0;
    case SignalType::SInt:
        return offset < data.size()
                   ? static_cast<double>(static_cast<int8_t>(data[offset])) * mapping.scale + mapping.engineeringOffset
                   : 0.0;
    case SignalType::Real32:
        if (offset + 3 >= data.size())
        {
            return 0.0;
        }
        float realVal = 0.0f;
        std::memcpy(&realVal, data.data() + offset, sizeof(float));
        return static_cast<double>(realVal) * mapping.scale + mapping.engineeringOffset;
    }
    return 0.0;
}

void IOSignalService::encodeValue(const SignalMapping &mapping, double engineeringValue, std::vector<uint8_t> &buffer)
{
    const double raw = mapping.scale != 0.0 ? (engineeringValue - mapping.engineeringOffset) / mapping.scale : engineeringValue;
    const auto offset = mapping.byteOffset;
    if (buffer.size() < offset + mapping.widthBytes())
    {
        buffer.resize(offset + mapping.widthBytes(), 0);
    }
    switch (mapping.type)
    {
    case SignalType::Bool:
    {
        uint8_t byte = buffer[offset];
        const bool set = std::fabs(raw) >= 0.5;
        if (mapping.bitOffset.has_value())
        {
            const uint8_t bit = *mapping.bitOffset;
            if (bit < 8)
            {
                if (set)
                {
                    byte |= static_cast<uint8_t>(1u << bit);
                }
                else
                {
                    byte &= static_cast<uint8_t>(~(1u << bit));
                }
                buffer[offset] = byte;
            }
        }
        else
        {
            buffer[offset] = set ? 1 : 0;
        }
        break;
    }
    case SignalType::UInt8:
        buffer[offset] = static_cast<uint8_t>(std::clamp(raw, 0.0, 255.0));
        break;
    case SignalType::UInt16:
        writeLE16(static_cast<uint16_t>(std::clamp(raw, 0.0, 65535.0)), buffer, offset);
        break;
    case SignalType::UInt32:
        writeLE32(static_cast<uint32_t>(std::max(0.0, raw)), buffer, offset);
        break;
    case SignalType::SInt:
        buffer[offset] = static_cast<uint8_t>(static_cast<int8_t>(std::round(raw)));
        break;
    case SignalType::Real32:
    {
        float real = static_cast<float>(raw);
        std::memcpy(buffer.data() + offset, &real, sizeof(float));
        break;
    }
    }
}

