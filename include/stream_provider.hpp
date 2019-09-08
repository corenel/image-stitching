#pragma once

#include <map>
#include <string>
#include <vector>
#include "opencv2/opencv.hpp"

#include "stream_helper.hpp"

class StreamProvider {
 public:
  explicit StreamProvider(std::string video_stream_url,
                          std::string backend = "gst-default",
                          std::string codec = "h264",
                          std::string resolution = "720p", unsigned fps = 30,
                          bool fast = false);
  ~StreamProvider();

  bool open();
  bool isOpened();
  void close();
  bool read(cv::Mat &frame);

 private:
  std::string stream_url_;
  // video backend (supported: 'gst-default', 'gst-basic',
  // 'gst-vaapi', 'gst-libav', 'gst-mfx', 'ffmpeg')
  std::string backend_;
  // codec name (supported: 'h264', 'h265', 'mpeg2',
  // 'mpeg4', 'mjpeg', 'vp8')
  std::string codec_;
  // video resolution for encoding (supported: '720p', '1080p', '4k')
  std::string resolution_;
  // fix frame per second for encoding (supported: fps > 0)
  unsigned fps_;
  // fast measure fps
  bool fast_measure_;
  cv::Ptr<cv::VideoCapture> cap_;
};

class MultipleStreamProvider {
 public:
  explicit MultipleStreamProvider(
      const std::vector<std::string> &video_stream_urls);
  ~MultipleStreamProvider();

  bool open();
  bool isOpened();
  void close();
  bool read(std::vector<cv::Mat> &frames);

 private:
  std::vector<cv::Ptr<StreamProvider>> streamers_;
};
