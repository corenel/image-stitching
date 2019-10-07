#include <iostream>
#include "csv.h"
#include "warper.hpp"

int main(int argc, char** argv) {
  // parse arguments
  cv::CommandLineParser parser(argc, argv, "{@input| input.csv |}");
  std::string filename =
      cv::samples::findFile(parser.get<std::string>("@input"));

  // parse configs
  io::CSVReader<5> in(filename);
  in.read_header(io::ignore_extra_column, "url", "left", "top", "right",
                 "bottom");
  std::string url;
  float left, top, right, bottom;
  std::vector<WarpConfig> configs;
  while (in.read_row(url, left, top, right, bottom)) {
    configs.emplace_back(url, left, top, right, bottom);
  }

  // calibrate
  Warper warper;
  warper.help(argv);
  auto transforms = warper.calibrate(configs);

  for (const auto& transform : transforms) {
    std::cout << transform.orig << std::endl;
  }

  float final_left{}, final_top{}, final_right{}, final_bottom{};
  for (const auto& t : transforms) {
    final_left = std::fmin(final_left, t.orig.x);
    final_top = std::fmin(final_top, t.orig.y);
    final_right = std::fmax(final_right, t.orig.x + t.width);
    final_bottom = std::fmax(final_bottom, t.orig.y + t.height);
  }

  cv::Mat output(int(final_bottom - final_top) + 1,
                 int(final_right - final_left) + 1, CV_8UC4);
  for (size_t i = 0; i < transforms.size(); ++i) {
    cv::Mat original_image = cv::imread(configs[i].url);
    cv::Mat warped_image;
    cv::warpPerspective(original_image, warped_image, transforms[i].M,
                        cv::Size(transforms[i].width, transforms[i].height),
                        cv::WARP_INVERSE_MAP);

    // remove the black background
    cv::Mat warped_image_bgra;
#if (CV_VERSION_MAJOR >= 4)
    cv::cvtColor(warped_image, warped_image_bgra, cv::COLOR_BGR2BGRA);
#else
    cv::cvtColor(warped_image, warped_image_bgra, CV_BGR2BGRA);
#endif
    // find all black pixel and set alpha value to zero:

    for (int y = 0; y < warped_image_bgra.rows; ++y) {
      for (int x = 0; x < warped_image_bgra.cols; ++x) {
        auto& pixel = warped_image_bgra.at<cv::Vec4b>(y, x);
        // if pixel is black
        if (pixel[0] == 0 && pixel[1] == 0 && pixel[2] == 0) {
          // set alpha to zero:
          pixel[3] = 0;
        }
      }
    }
    auto roi = cv::Rect(transforms[i].orig.x + (-final_left + final_right) / 2,
                        -transforms[i].orig.y + (-final_top + final_bottom) / 2,
                        transforms[i].width, transforms[i].height);
    cv::add(warped_image_bgra, output(roi), output(roi));
  }
  cv::imwrite("merged.png", output);
  cv::cvtColor(output, output, cv::COLOR_BGRA2BGR);
  cv::imwrite("merged.jpg", output);

  return 0;
}
