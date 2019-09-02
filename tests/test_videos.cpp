#include <vector>
#include "opencv2/opencv.hpp"

#include "stitcher.hpp"
#include "video_provider.hpp"

std::vector<std::string> video_files;

static void print_usage() {
  std::cout << "Rotation model video stitcher.\n\n"
               "stitching_detailed video1 video2 [...videoN]\n\n"
            << std::endl;
}

static int parse_args(int argc, char** argv) {
  if (argc == 1) {
    print_usage();
    return -1;
  }

  for (int i = 1; i < argc; ++i) {
    if (std::string(argv[i]) == "--help" || std::string(argv[i]) == "-h") {
      print_usage();
      return -1;
    } else
      video_files.emplace_back(argv[i]);
  }
  return 0;
}

int main(int argc, char* argv[]) {
  // Parse command line args
  int retval = parse_args(argc, argv);
  if (retval) {
    return retval;
  }

  // Load images
  int num_images = static_cast<int>(video_files.size());
  VideoProvider providers(video_files);

  // Process
  cv::Mat result, result_mask;
  std::vector<cv::Mat> frames;

  if (providers.isOpened()) {
    LOGLN("------- Calibrating -------");
    providers.read(frames);
    LOGLN(frames.size());
    LOGLN(frames[0].size());
    Stitcher s(num_images, frames[0].size());
    s.calibrate(frames, result, result_mask);
    cv::imwrite("result_0.jpg", result);

    LOGLN("------- Processing -------");
    providers.read(frames);
    s.process(frames, result, result_mask);
    cv::imwrite("result_1.jpg", result);

    return 0;
  } else {
    return 1;
  }
}
