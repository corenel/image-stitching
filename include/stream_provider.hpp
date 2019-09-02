#pragma once

#include <map>
#include <string>
#include <vector>
#include "opencv2/opencv.hpp"

template <typename M>
inline typename M::mapped_type getValue(const M &dict,
                                        const typename M::key_type &key,
                                        const std::string &errorMessage) {
  typename M::const_iterator it = dict.find(key);
  if (it == dict.end()) {
    CV_Error(cv::Error::StsBadArg, errorMessage);
  }
  return it->second;
}

inline std::map<std::string, cv::Size> sizeByResolution() {
  std::map<std::string, cv::Size> res;
  res["720p"] = cv::Size(1280, 720);
  res["1080p"] = cv::Size(1920, 1080);
  res["4k"] = cv::Size(3840, 2160);
  return res;
}

inline std::map<std::string, int> fourccByCodec() {
  std::map<std::string, int> res;
  res["h264"] = cv::VideoWriter::fourcc('H', '2', '6', '4');
  res["h265"] = cv::VideoWriter::fourcc('H', 'E', 'V', 'C');
  res["mpeg2"] = cv::VideoWriter::fourcc('M', 'P', 'E', 'G');
  res["mpeg4"] = cv::VideoWriter::fourcc('M', 'P', '4', '2');
  res["mjpeg"] = cv::VideoWriter::fourcc('M', 'J', 'P', 'G');
  res["vp8"] = cv::VideoWriter::fourcc('V', 'P', '8', '0');
  return res;
}

inline std::map<std::string, std::string> defaultEncodeElementByCodec() {
  std::map<std::string, std::string> res;
  res["h264"] = "x264enc";
  res["h265"] = "x265enc";
  res["mpeg2"] = "mpeg2enc";
  res["mjpeg"] = "jpegenc";
  res["vp8"] = "vp8enc";
  return res;
}

inline std::map<std::string, std::string> VAAPIEncodeElementByCodec() {
  std::map<std::string, std::string> res;
  res["h264"] = "parsebin ! vaapih264enc";
  res["h265"] = "parsebin ! vaapih265enc";
  res["mpeg2"] = "parsebin ! vaapimpeg2enc";
  res["mjpeg"] = "parsebin ! vaapijpegenc";
  res["vp8"] = "parsebin ! vaapivp8enc";
  return res;
}

inline std::map<std::string, std::string> mfxDecodeElementByCodec() {
  std::map<std::string, std::string> res;
  res["h264"] = "parsebin ! mfxh264dec";
  res["h265"] = "parsebin ! mfxhevcdec";
  res["mpeg2"] = "parsebin ! mfxmpeg2dec";
  res["mjpeg"] = "parsebin ! mfxjpegdec";
  return res;
}

inline std::map<std::string, std::string> mfxEncodeElementByCodec() {
  std::map<std::string, std::string> res;
  res["h264"] = "mfxh264enc";
  res["h265"] = "mfxhevcenc";
  res["mpeg2"] = "mfxmpeg2enc";
  res["mjpeg"] = "mfxjpegenc";
  return res;
}

inline std::map<std::string, std::string> libavDecodeElementByCodec() {
  std::map<std::string, std::string> res;
  res["h264"] = "parsebin ! avdec_h264";
  res["h265"] = "parsebin ! avdec_h265";
  res["mpeg2"] = "parsebin ! avdec_mpeg2video";
  res["mpeg4"] = "parsebin ! avdec_mpeg4";
  res["mjpeg"] = "parsebin ! avdec_mjpeg";
  res["vp8"] = "parsebin ! avdec_vp8";
  return res;
}

inline std::map<std::string, std::string> libavEncodeElementByCodec() {
  std::map<std::string, std::string> res;
  res["h264"] = "avenc_h264";
  res["h265"] = "avenc_h265";
  res["mpeg2"] = "avenc_mpeg2video";
  res["mpeg4"] = "avenc_mpeg4";
  res["mjpeg"] = "avenc_mjpeg";
  res["vp8"] = "avenc_vp8";
  return res;
}

inline std::map<std::string, std::string> demuxPluginByContainer() {
  std::map<std::string, std::string> res;
  res["avi"] = "avidemux";
  res["mp4"] = "qtdemux";
  res["mov"] = "qtdemux";
  res["mkv"] = "matroskademux";
  return res;
}

inline std::map<std::string, std::string> muxPluginByContainer() {
  std::map<std::string, std::string> res;
  res["avi"] = "avimux";
  res["mp4"] = "qtmux";
  res["mov"] = "qtmux";
  res["mkv"] = "matroskamux";
  return res;
}

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
