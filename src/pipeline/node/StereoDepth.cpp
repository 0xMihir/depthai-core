#include "depthai/pipeline/node/StereoDepth.hpp"

// standard
#include <fstream>

#include "spdlog/spdlog.h"

namespace dai {
namespace node {

StereoDepth::StereoDepth(const std::shared_ptr<PipelineImpl>& par, int64_t nodeId) : Node(par, nodeId) {
    // 'properties' defaults already set
}

std::string StereoDepth::getName() const {
    return "StereoDepth";
}

std::vector<Node::Output> StereoDepth::getOutputs() {
    return {disparity, depth, syncedLeft, syncedRight, rectifiedLeft, rectifiedRight};
}

std::vector<Node::Input> StereoDepth::getInputs() {
    return {left, right};
}

nlohmann::json StereoDepth::getProperties() {
    nlohmann::json j;
    nlohmann::to_json(j, properties);
    return j;
}

std::shared_ptr<Node> StereoDepth::clone() {
    return std::make_shared<std::decay<decltype(*this)>::type>(*this);
}

void StereoDepth::loadCalibrationData(const std::vector<std::uint8_t>& data) {
    if(data.empty()) {
        // Will use EEPROM data
        properties.calibration.clear();
    } else {
        properties.calibration = data;
    }
}

void StereoDepth::loadCalibrationFile(const std::string& path) {
    std::vector<std::uint8_t> data;
    if(!path.empty()) {
        std::ifstream calib(path, std::ios::binary);
        if(!calib.is_open()) {
            throw std::runtime_error("StereoDepth node | Unable to open calibration file: " + path);
        }
        data = std::vector<std::uint8_t>(std::istreambuf_iterator<char>(calib), {});
    }
    loadCalibrationData(data);
}

void StereoDepth::setEmptyCalibration(void) {
    // Special case: a single element
    const std::vector<std::uint8_t> empty = {0};
    properties.calibration = empty;
}

void StereoDepth::loadMeshData(const std::vector<std::uint8_t>& dataLeft, const std::vector<std::uint8_t>& dataRight) {
    if (dataLeft.size() != dataRight.size()) {
        throw std::runtime_error("StereoDepth | left and right mesh sizes must match");
    }

    Asset meshAsset;
    std::string assetKey;
    meshAsset.alignment = 64;

    meshAsset.data = dataLeft;
    assetKey = "meshLeft";
    assetManager.set(assetKey, meshAsset);
    properties.mesh.meshLeftUri = std::string("asset:") + assetKey;

    meshAsset.data = dataRight;
    assetKey = "meshRight";
    assetManager.set(assetKey, meshAsset);
    properties.mesh.meshRightUri = std::string("asset:") + assetKey;

    properties.mesh.meshSize = meshAsset.data.size();
}

void StereoDepth::loadMeshFiles(const std::string& pathLeft, const std::string& pathRight) {
    std::ifstream streamLeft(pathLeft, std::ios::binary);
    if(!streamLeft.is_open()) {
        throw std::runtime_error("StereoDepth | Cannot open mesh at path: " + pathLeft);
    }
    std::vector<std::uint8_t> dataLeft = std::vector<std::uint8_t>(std::istreambuf_iterator<char>(streamLeft), {});

    std::ifstream streamRight(pathRight, std::ios::binary);
    if(!streamRight.is_open()) {
        throw std::runtime_error("StereoDepth | Cannot open mesh at path: " + pathRight);
    }
    std::vector<std::uint8_t> dataRight = std::vector<std::uint8_t>(std::istreambuf_iterator<char>(streamRight), {});

    loadMeshData(dataLeft, dataRight);
}

void StereoDepth::setMeshStep(int width, int height) {
    properties.mesh.stepWidth = width;
    properties.mesh.stepHeight = height;
}

void StereoDepth::setInputResolution(int width, int height) {
    properties.width = width;
    properties.height = height;
}
void StereoDepth::setMedianFilter(Properties::MedianFilter median) {
    properties.median = median;
}
void StereoDepth::setDepthAlign(Properties::DepthAlign align) {
    properties.depthAlign = align;
    // Unset 'depthAlignCamera', that would take precedence otherwise
    properties.depthAlignCamera = CameraBoardSocket::AUTO;
}
void StereoDepth::setDepthAlign(CameraBoardSocket camera) {
    properties.depthAlignCamera = camera;
}
void StereoDepth::setConfidenceThreshold(int confThr) {
    properties.confidenceThreshold = confThr;
}
void StereoDepth::setLeftRightCheck(bool enable) {
    properties.enableLeftRightCheck = enable;
}
void StereoDepth::setSubpixel(bool enable) {
    properties.enableSubpixel = enable;
}
void StereoDepth::setExtendedDisparity(bool enable) {
    properties.enableExtendedDisparity = enable;
}
void StereoDepth::setRectifyEdgeFillColor(int color) {
    properties.rectifyEdgeFillColor = color;
}
void StereoDepth::setRectifyMirrorFrame(bool enable) {
    properties.rectifyMirrorFrame = enable;
}
void StereoDepth::setOutputRectified(bool enable) {
    (void)enable;
    spdlog::warn("{} is deprecated. The output is auto-enabled if used", __func__);
}
void StereoDepth::setOutputDepth(bool enable) {
    (void)enable;
    spdlog::warn("{} is deprecated. The output is auto-enabled if used", __func__);
}

}  // namespace node
}  // namespace dai
