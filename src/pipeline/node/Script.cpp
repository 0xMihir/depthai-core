#include "depthai/pipeline/node/Script.hpp"

#include "depthai/pipeline/Pipeline.hpp"
#include "openvino/BlobReader.hpp"

namespace dai {
namespace node {

Script::Script(const std::shared_ptr<PipelineImpl>& par, int64_t nodeId)
    : Node(par, nodeId),
      inputs(Input(*this, "", Input::Type::SReceiver, {{DatatypeEnum::Buffer, true}})),
      outputs(Output(*this, "", Output::Type::MSender, {{DatatypeEnum::Buffer, true}})) {
    properties.scriptUri = "";
    properties.scriptName = "<script>";
    properties.processor = ProcessorType::LEON_MSS;
}

std::string Script::getName() const {
    return "Script";
}

std::vector<Node::Output> Script::getOutputs() {
    std::vector<Node::Output> vecOutputs;
    for(const auto& kv : outputs) {
        vecOutputs.push_back(kv.second);
    }
    return vecOutputs;
}

std::vector<Node::Input> Script::getInputs() {
    std::vector<Node::Input> vecInputs;
    for(const auto& kv : inputs) {
        vecInputs.push_back(kv.second);
    }
    return vecInputs;
}

nlohmann::json Script::getProperties() {
    nlohmann::json j;
    nlohmann::to_json(j, properties);
    return j;
}

std::shared_ptr<Node> Script::clone() {
    return std::make_shared<std::decay<decltype(*this)>::type>(*this);
}

void Script::setScriptPath(const std::string& path) {
    properties.scriptUri = assetManager.add("__script", path)->getRelativeUri();
    scriptPath = path;
    properties.scriptName = path;
}

void Script::setScriptData(const std::string& script, const std::string& name) {
    std::vector<std::uint8_t> data{script.begin(), script.end()};
    properties.scriptUri = assetManager.add("__script", std::move(data))->getRelativeUri();
    scriptPath = "";
    if(name.empty()) {
        properties.scriptName = "<script>";
    } else {
        properties.scriptName = name;
    }
}

void Script::setScriptData(const std::vector<std::uint8_t>& data, const std::string& name) {
    properties.scriptUri = assetManager.add("__script", std::move(data))->getRelativeUri();
    scriptPath = "";
    if(name.empty()) {
        properties.scriptName = "<script>";
    } else {
        properties.scriptName = name;
    }
}

void Script::setProcessor(ProcessorType proc) {
    properties.processor = proc;
}

std::string Script::getScriptPath() const {
    return scriptPath;
}

std::string Script::getScriptName() const {
    return properties.scriptName;
}

ProcessorType Script::getProcessor() const {
    return properties.processor;
}

}  // namespace node
}  // namespace dai