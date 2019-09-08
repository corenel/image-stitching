#pragma once

#include <map>
#include <string>
#include <vector>
#include "opencv2/opencv.hpp"
#include "stream_helper.hpp"

class StreamProvider {
 public:
  explicit StreamProvider(const std::string &video_stream_url);
  ~StreamProvider();

  bool open(const std::string &video_stream_url);
  bool isOpened();
  void close();
  bool read(cv::Mat &frame);

 private:
  std::string backend = "gst-default";  // video backend
  std::string codec = "h264";           // codec type
  std::string stream_url;               // stream url
  std::string resolution = "720p";      // video resolution

  inline cv::Ptr<cv::VideoCapture> createCapture(const std::string &backend,
                                         const std::string &file_name,
                                         const std::string &codec) {
    if (backend == "gst-default") {
      std::cout << "Created GStreamer capture ( " << file_name << " )" << std::endl;
      return cv::makePtr<cv::VideoCapture>(file_name, cv::CAP_GSTREAMER);
    } else if (backend.find("gst") == 0) {
      std::ostringstream line;
      line << "filesrc location=\"" << file_name << "\"";
      line << " ! ";
      line << getValue(demuxPluginByContainer(), containerByName(file_name),
                       "Invalid container");
      line << " ! ";
      if (backend.find("basic") == 4)
        line << "decodebin";
      else if (backend.find("vaapi") == 4)
        line << "vaapidecodebin";
      else if (backend.find("libav") == 4)
        line << getValue(libavDecodeElementByCodec(), codec, "Invalid codec");
      else if (backend.find("mfx") == 4)
        line << getValue(mfxDecodeElementByCodec(), codec,
                         "Invalid or unsupported codec");
      else
        return cv::Ptr<cv::VideoCapture>();
      line << " ! videoconvert n-threads=" << getNumThreads();
      line << " ! appsink sync=false";
      std::cout << "Created GStreamer capture  ( " << line.str() << " )" << std::endl;
      return cv::makePtr<cv::VideoCapture>(line.str(), cv::CAP_GSTREAMER);
    } else if (backend == "ffmpeg") {
      std::cout << "Created FFmpeg capture ( " << file_name << " )" << std::endl;
      return cv::makePtr<cv::VideoCapture>(file_name, cv::CAP_FFMPEG);
    }
    return Ptr<VideoCapture>();
  }
};

class MultipleStreamProvider {
 public:
  explicit MultipleStreamProvider(
      const std::vector<std::string> &video_stream_urls);
  ~MultipleStreamProvider()();

  bool open(const std::vector<std::string> &video_stream_urls);
  bool isOpened();
  void close();
  bool read(std::vector<cv::Mat> &frames);

 private:
  std::vector<cv::Ptr<StreamProvider>> streamers_;
};
