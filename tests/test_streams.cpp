#include <tclap/CmdLine.h>
#include <vector>
#include "opencv2/opencv.hpp"

#include "stitcher.hpp"
#include "util.hpp"
#include "video_provider.hpp"

int main(int argc, char* argv[]) {
  // Parse arguments
  std::string path_to_calib_list;
  std::string path_to_input_list;
  try {
    TCLAP::CmdLine cmd("Command description message", ' ', "0.9");

    TCLAP::ValueArg<std::string> arg_calib(
        "c", "calib",
        "Path to list of calib images"
        "(one path per line as format of plain text)",
        true, "calib_list.txt", "string");
    TCLAP::ValueArg<std::string> arg_input(
        "i", "input",
        "Path to list of input videos"
        "(one path per line as format of plain text)",
        true, "input_list.txt", "string");

    cmd.add(arg_calib);
    cmd.add(arg_input);

    // Parse the argv array.
    cmd.parse(argc, argv);

    // Get the value parsed by each arg.
    path_to_calib_list = arg_calib.getValue();
    path_to_input_list = arg_input.getValue();
  } catch (TCLAP::ArgException& e) {
    std::cerr << "Parse error: " << e.error() << " for arg " << e.argId()
              << std::endl;
  }

  // Read lists
  std::vector<std::string> calib_list;
  std::vector<std::string> input_list;
  get_file_content(path_to_calib_list, calib_list);
  get_file_content(path_to_input_list, input_list);
  std::cout << "calibration list:" << std::endl;
  for (const auto& e : calib_list) {
    std::cout << e << std::endl;
  }
  std::cout << "input list:" << std::endl;
  for (const auto& e : input_list) {
    std::cout << e << std::endl;
  }

  // Load calibration images
  int num_images = static_cast<int>(input_list.size());
  std::vector<cv::Mat> calib_images(num_images);
  load_images(calib_list, calib_images);

  // Initialize video providers
  MultipleVideoProvider providers(input_list);

  // Process
  cv::Mat result, result_mask;
  std::vector<cv::Mat> frames;

  if (providers.isOpened()) {
    LOGLN("------- Calibrating -------");
    providers.read(frames);
    Stitcher s(num_images, frames[0].size());
    s.calibrate(frames, result, result_mask);
    cv::imwrite("result_0.jpg", result);

    LOGLN("------- Processing -------");
    providers.read(frames);
    s.process(frames, result, result_mask);
    cv::imwrite("result_1.jpg", result);

    LOGLN("------- Re-calibrating -------");
    providers.read(frames);
    s.calibrate(frames, result, result_mask);
    cv::imwrite("result_2.jpg", result);

    LOGLN("------- Processing -------");
    providers.read(frames);
    s.process(frames, result, result_mask);
    cv::imwrite("result_3.jpg", result);

    return 0;
  } else {
    return 1;
  }
}
