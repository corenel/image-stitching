#pragma once

#include "stream_helper.hpp"

class StreamWriter {
 public:
  explicit StreamWriter(std::string video_stream_url,
                        std::string backend = "gst-default",
                        std::string codec = "h264",
                        std::string resolution = "720p", unsigned fps = 30,
                        cv::Size sz = cv::Size(0, 0));
  ~StreamWriter();
  bool open(const cv::Size& sz = cv::Size(0, 0));
  bool isOpened();
  void close();
  bool write(const cv::Mat& frame);
  inline cv::Size& size() { return size_; };

 private:
  std::string stream_url_;  // stream url
  std::string backend_;     // video backend
  std::string codec_;       // codec type
  std::string resolution_;  // video resolution
  unsigned fps_;            // video fps
  cv::Size size_;
  cv::Ptr<cv::VideoWriter> writer_;
};
