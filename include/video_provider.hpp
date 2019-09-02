#pragma once

#include <string>
#include <vector>
#include "opencv2/opencv.hpp"

class MultipleVideoProvider {
 public:
  explicit MultipleVideoProvider(const std::vector<std::string> &video_files);

  ~MultipleVideoProvider();

  bool open(const std::vector<std::string> &video_files);
  bool isOpened();
  void close();
  bool read(std::vector<cv::Mat> &frames);

 private:
  std::vector<std::string> video_files_;
  std::vector<cv::Ptr<cv::VideoCapture>> video_readers_;
};
