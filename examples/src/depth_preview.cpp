#include <cstdio>
#include <iostream>

// Inludes common necessary includes for development using depthai library
#include "depthai/depthai.hpp"

// Closer-in minimum depth, disparity range is doubled (from 95 to 190):
static std::atomic<bool> extended_disparity{false};
// Better accuracy for longer distance, fractional disparity 32-levels:
static std::atomic<bool> subpixel{false};
// Better handling for occlusions:
static std::atomic<bool> lr_check{false};

int main() {
    using namespace std;

    dai::Pipeline pipeline;

    auto monoLeft = pipeline.create<dai::node::MonoCamera>();
    auto monoRight = pipeline.create<dai::node::MonoCamera>();
    auto depth = pipeline.create<dai::node::StereoDepth>();
    auto xout = pipeline.create<dai::node::XLinkOut>();

    xout->setStreamName("disparity");

    monoLeft->setResolution(dai::MonoCameraProperties::SensorResolution::THE_400_P);
    monoLeft->setBoardSocket(dai::CameraBoardSocket::LEFT);
    monoRight->setResolution(dai::MonoCameraProperties::SensorResolution::THE_400_P);
    monoRight->setBoardSocket(dai::CameraBoardSocket::RIGHT);

    // Create a node that will produce the depth map (using disparity output as it's easier to visualize depth this way)
    depth->setConfidenceThreshold(200);
    // Options: MEDIAN_OFF, KERNEL_3x3, KERNEL_5x5, KERNEL_7x7 (default)
    depth->setMedianFilter(dai::StereoDepthProperties::MedianFilter::KERNEL_7x7);
    depth->setLeftRightCheck(lr_check);

    // Normal disparity values range from 0..95, will be used for normalization
    int max_disparity = 95;

    if(extended_disparity) max_disparity *= 2;  // Double the range
    depth->setExtendedDisparity(extended_disparity);

    if(subpixel) max_disparity *= 32;  // 5 fractional bits, x32
    depth->setSubpixel(subpixel);

    // When we get disparity to the host, we will multiply all values with the multiplier
    // for better visualization
    float multiplier = 255 / max_disparity;

    // Linking
    monoLeft->out.link(depth->left);
    monoRight->out.link(depth->right);
    depth->disparity.link(xout->input);

    // Connect to device and start pipeline
    dai::Device device(pipeline);

    // Output queue will be used to get the disparity frames from the outputs defined above
    auto q = device.getOutputQueue("disparity", 4, false);

    while(true) {
        auto inDepth = q->get<dai::ImgFrame>();
        auto frame = inDepth->getFrame();
        frame.convertTo(frame, CV_8UC1, multiplier);

        cv::imshow("disparity", frame);

        // Available color maps: https://docs.opencv.org/3.4/d3/d50/group__imgproc__colormap.html
        cv::applyColorMap(frame, frame, cv::COLORMAP_JET);
        cv::imshow("disparity_color", frame);

        int key = cv::waitKey(1);
        if(key == 'q' || key == 'Q') {
            return 0;
        }
    }
    return 0;
}
