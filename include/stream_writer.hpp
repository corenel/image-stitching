#pragma once

#include "stream_helper.hpp"

class StreamWriter {
 public:
  explicit StreamWriter(std::string video_stream_url,
                        std::string backend = "gst-default",
                        std::string codec = "h264",
                        std::string resolution = "720p", unsigned fps = 30);
  ~StreamWriter();
  bool open();
  bool isOpened();
  void close();
  bool write(const cv::Mat& frame);

 private:
  std::string stream_url_;  // stream url
  std::string backend_;     // video backend
  std::string codec_;       // codec type
  std::string resolution_;  // video resolution
  cv::Size size_;
  unsigned fps_;  // video fps
  cv::Ptr<cv::VideoWriter> writer_;
};
