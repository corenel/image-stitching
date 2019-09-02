#pragma once

#include <string>
#include <vector>
#include "opencv2/opencv.hpp"

class VideoProvider {
 public:
  explicit VideoProvider(const std::vector<std::string> &video_files);
  ~VideoProvider();

  bool open(const std::vector<std::string> &video_files);
  bool isOpened();
  void close();
  bool read(std::vector<cv::Mat> &frames);

 private:
  std::vector<std::string> video_files_;
  std::vector<cv::VideoCapture> video_readers_;
};
