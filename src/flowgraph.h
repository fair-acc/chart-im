#pragma once

#include <filesystem>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace ImChart {

class BlockType;

class Block {
public:
    class Connection {
    public:
        Block   *block;
        uint32_t portNumber;
    };

    class Port {
    public:
        const uint32_t          id;
        std::vector<Connection> connections;
    };

    Block(std::string_view name, BlockType *type);

    void              connectTo(uint32_t srcPort, Block *dst, uint32_t dstPort);

    const auto       &inputs() const { return m_inputs; }
    const auto       &outputs() const { return m_outputs; }

    const uint32_t    id;
    const BlockType  *type;
    const std::string name;
    std::vector<Port> m_inputs;
    std::vector<Port> m_outputs;
};

class BlockType {
public:
    struct PortDefinition {
        std::string type;
    };

    struct Parameter {
        std::string                                  options;
        std::unordered_map<std::string, std::string> attributes;
    };

    std::unordered_map<std::string, Parameter> parameters;
    std::vector<PortDefinition>                inputs;
    std::vector<PortDefinition>                outputs;
};

class FlowGraph {
public:
    void               loadBlockDefinitions(const std::filesystem::path &dir);
    void               parse(const std::filesystem::path &file);

    Block             *findBlock(std::string_view name) const;

    inline const auto &blocks() const { return m_blocks; }

private:
    std::vector<std::unique_ptr<Block>>                         m_blocks;
    std::unordered_map<std::string, std::unique_ptr<BlockType>> m_types;
};

} // namespace ImChart
