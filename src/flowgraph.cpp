#include "flowgraph.h"

#include <assert.h>

#include <charconv>
#include <fstream>
#include <string_view>
#include <yaml-cpp/yaml.h>

#include <fmt/format.h>

namespace ImChart {

namespace {

uint32_t nextId() {
    static uint32_t id = 0;
    return ++id;
}

} // namespace

Block::Block(std::string_view name, BlockType *type)
    : id(nextId())
    , type(type)
    , name(name) {
}

void Block::connectTo(uint32_t srcPort, ImChart::Block *dst, uint32_t dstPort) {
    if (srcPort < m_outputs.size() && dst->inputs().size() > dstPort) {
        m_outputs[srcPort].connections.push_back({ dst, dstPort });
    }
}

static std::string_view strview(auto &&s) {
    return { s.data(), s.size() };
}

static bool readFile(const std::filesystem::path &file, std::string &str) {
    // fmt::print("Reading {}\n", file.native());

    std::ifstream stream(file);
    if (!stream.is_open()) {
        return false;
    }

    stream.seekg(0, std::ios::end);
    size_t size = stream.tellg();

    str.resize(size);
    stream.seekg(0);
    stream.read(str.data(), size);
    return true;
}

void FlowGraph::loadBlockDefinitions(const std::filesystem::path &dir) {
    if (!std::filesystem::exists(dir)) {
        fmt::print("Cannot open directory '{}'\n", dir.native());
        return;
    }

    std::filesystem::directory_iterator iterator(dir);
    std::string                         str;

    for (const auto &entry : iterator) {
        const auto &path = entry.path();
        if (!path.native().ends_with(".block.yml")) {
            continue;
        }

        readFile(path, str);
        YAML::Node config = YAML::Load(str);

        auto       id     = config["id"].as<std::string>();
        // fmt::print("id {}\n",id);

        auto def        = m_types.insert({ id, std::make_unique<BlockType>() }).first->second.get();

        auto parameters = config["parameters"];
        for (const auto &p : parameters) {
            const auto &id = p["id"];
            if (!id || !id.IsScalar()) {
                continue;
            }

            BlockType::Parameter par;

            const auto          &opts = p["options"];
            if (opts) {
                std::string val;
                if (opts.IsSequence()) {
                    for (const auto &n : opts) {
                        val += n.as<std::string>();
                        val += " ";
                    }
                } else if (opts.IsScalar()) {
                    val = opts.as<std::string>();
                }

                par.options = val;
            }

            const auto &attrs = p["option_attributes"];
            if (attrs && attrs.IsMap()) {
                for (auto it = attrs.begin(); it != attrs.end(); ++it) {
                    auto        key = it->first.as<std::string>();
                    std::string val;
                    if (it->second.IsSequence()) {
                        for (const auto &n : it->second) {
                            val += n.as<std::string>();
                            val += " ";
                        }
                    } else if (it->second.IsScalar()) {
                        val = it->second.as<std::string>();
                    }

                    par.attributes.insert({ key, val });
                }
            }

            def->parameters.insert({ id.as<std::string>(), std::move(par) });
        }

        auto getParameter = [&](const std::string &str) -> std::string {
            std::string words[2];
            int         numWords = 0;

            int         start    = 2;
            int         last     = str.size() - 2;
            while (str[last] == ' ') {
                --last;
            }

            while (start < last && numWords <= 2) {
                int i = str.find('.', start);
                if (i == std::string::npos) {
                    i = last + 1;
                }

                int end = i - 1;

                while (str[start] == ' ' && start < end) {
                    ++start;
                }
                while (str[end] == ' ' && end > start) {
                    --end;
                }

                words[numWords++] = str.substr(start, end - start + 1);
                start             = i + 1;
            }

            if (numWords > 0) {
                auto it = def->parameters.find(words[0]);
                if (it != def->parameters.end()) {
                    const auto &par = it->second;
                    if (numWords == 1) {
                        return par.options;
                    } else {
                        auto attr = par.attributes.find(words[1]);
                        if (attr != par.attributes.end()) {
                            return attr->second;
                        }
                    }
                }
            }
            return {};
        };

        auto outputs = config["outputs"];
        for (const auto &o : outputs) {
            auto        n    = o["dtype"];
            std::string type = "unknown";
            if (n) {
                type = n.as<std::string>();

                if (type.starts_with("${") && type.ends_with("}")) {
                    type = getParameter(type);
                }
            }
            def->outputs.push_back({ type });
        }

        auto inputs = config["inputs"];
        for (const auto &o : inputs) {
            auto        n    = o["dtype"];
            std::string type = "unknown";
            if (n) {
                type = n.as<std::string>();
            }

            if (type.starts_with("${") && type.ends_with("}")) {
                type = getParameter(type);
            }

            def->inputs.push_back({ type });
        }
    }
}

void FlowGraph::parse(const std::filesystem::path &file) {
    m_blocks.clear();

    std::string str;
    if (!readFile(file, str)) {
        fmt::print("Cannot read file '{}'\n", file.native());
        return;
    }
    YAML::Node tree   = YAML::Load(str);

    auto       blocks = tree["blocks"];
    for (const auto &b : blocks) {
        auto n    = b["name"].as<std::string>();
        auto id   = b["id"].as<std::string>();

        auto type = m_types[id].get();
        if (!type) {
            fmt::print("Block type '{}' is unkown.\n", id);
        }
        m_blocks.push_back(std::make_unique<Block>(n, type));
        auto *block = m_blocks.back().get();

        if (type) {
            block->m_outputs.reserve(type->outputs.size());
            block->m_inputs.reserve(type->inputs.size());
            for (auto &o : type->outputs) {
                block->m_outputs.push_back({ nextId() });
            }
            for (auto &o : type->inputs) {
                block->m_inputs.push_back({ nextId() });
            }
        }
    }

    auto connections = tree["connections"];
    for (const auto &c : connections) {
        assert(c.size() == 4);

        auto srcBlockName = c[0].as<std::string>();
        auto srcPortStr   = c[1].as<std::string>();
        int  srcPort;
        std::from_chars(srcPortStr.data(), srcPortStr.data() + srcPortStr.size(), srcPort);

        auto dstBlockName = c[2].as<std::string>();
        auto dstPortStr   = c[3].as<std::string>();
        int  dstPort;
        std::from_chars(dstPortStr.data(), dstPortStr.data() + dstPortStr.size(), dstPort);

        auto srcBlock = findBlock(srcBlockName);
        assert(srcBlock);
        auto dstBlock = findBlock(dstBlockName);
        assert(dstBlock);

        srcBlock->connectTo(srcPort, dstBlock, dstPort);
    }
}

Block *FlowGraph::findBlock(std::string_view name) const {
    for (auto &b : m_blocks) {
        if (b->name == name) {
            return b.get();
        }
    }
    return nullptr;
}

} // namespace ImChart
