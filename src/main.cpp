#include <vector>
#include "opencv2/opencv.hpp"
#include "stitcher.hpp"

std::vector<std::string> img_names;

static void print_usage() {
  std::cout << "Rotation model images stitcher.\n\n"
               "stitching_detailed img1 img2 [...imgN]\n\n"
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
      img_names.emplace_back(argv[i]);
  }
  return 0;
}

int main(int argc, char* argv[]) {
  // Parse command line args
  int retval = parse_args(argc, argv);
  if (retval) {
    return retval;
  }

  int num_images = static_cast<int>(img_names.size());
  std::vector<cv::Mat> full_img(num_images);
  for (int i = 0; i < num_images; ++i) {
    full_img[i] = cv::imread(cv::samples::findFile(img_names[i]));
    if (full_img[i].empty()) {
      std::cerr << "Can't open image " << img_names[i] << std::endl;
      return -1;
    }
  }

  cv::Mat result, result_mask;
  Stitcher s(num_images);
  s.calibrate(full_img, result, result_mask);

  return 0;
}
