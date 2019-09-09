#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include "opencv2/opencv.hpp"

#ifndef LOG
#define LOG(msg) std::cout << msg
#endif
#ifndef LOGLN
#define LOGLN(msg) std::cout << msg << std::endl
#endif

template <typename M>
inline typename M::mapped_type getValue(const M &dict,
                                        const typename M::key_type &key,
                                        const std::string &errorMessage) {
  auto it = dict.find(key);
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

inline std::map<std::string, std::string> depayPluginByContainer() {
  std::map<std::string, std::string> res;
  res["h264"] = "rtph264depay";
  res["h265"] = "rtph265depay";
  return res;
}

inline std::string containerByName(const std::string &name) {
  size_t found = name.rfind('.');
  if (found != std::string::npos) {
    return name.substr(found + 1);  // container type
  }
  return std::string();
}

inline cv::Ptr<cv::VideoCapture> createSynthSource(const cv::Size &sz,
                                                   unsigned fps) {
  std::ostringstream line;
  line << "videotestsrc pattern=smpte";
  line << " ! video/x-raw";
  line << ",width=" << sz.width << ",height=" << sz.height;
  if (fps > 0) line << ",framerate=" << fps << "/1";
  line << " ! appsink sync=false";
  std::cout << "Created synthetic video source ( " << line.str() << " )"
            << std::endl;
  return cv::makePtr<cv::VideoCapture>(line.str(), cv::CAP_GSTREAMER);
}

inline cv::Ptr<cv::VideoCapture> createCapture(const std::string &backend,
                                               const std::string &file_name,
                                               const std::string &codec) {
  if (backend == "gst-default") {
    std::cout << "Created GStreamer capture ( " << file_name << " )"
              << std::endl;
    return cv::makePtr<cv::VideoCapture>(file_name, cv::CAP_GSTREAMER);
  } else if (backend.find("gst") == 0) {
    std::ostringstream line;
    if (file_name.find("rtsp") == 0) {
      line << "rtspsrc latency=0 location=\"" << file_name << "\"";
      line << " ! ";
      line << getValue(depayPluginByContainer(), codec, "Invalid codec");
      line << " ! queue";
    } else {
      line << "filesrc location=\"" << file_name << "\"";
      line << " ! ";
      line << getValue(demuxPluginByContainer(), containerByName(file_name),
                       "Invalid container");
    }
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
    line << " ! videoconvert n-threads=" << cv::getNumThreads();
    line << " ! appsink sync=false";
    std::cout << "Created GStreamer capture  ( " << line.str() << " )"
              << std::endl;
    return cv::makePtr<cv::VideoCapture>(line.str(), cv::CAP_GSTREAMER);
  } else if (backend == "ffmpeg") {
    std::cout << "Created FFmpeg capture ( " << file_name << " )" << std::endl;
    return cv::makePtr<cv::VideoCapture>(file_name, cv::CAP_FFMPEG);
  }
  return cv::Ptr<cv::VideoCapture>();
}

inline cv::Ptr<cv::VideoWriter> createWriter(const std::string &backend,
                                             const std::string &file_name,
                                             const std::string &codec,
                                             const cv::Size &sz, unsigned fps) {
  if (backend == "gst-default") {
    std::cout << "Created GStreamer writer ( " << file_name << ", FPS=" << fps
              << ", Size=" << sz << ")" << std::endl;
    return cv::makePtr<cv::VideoWriter>(
        file_name, cv::CAP_GSTREAMER,
        getValue(fourccByCodec(), codec, "Invalid codec"), fps, sz, true);
  } else if (backend.find("gst") == 0) {
    std::ostringstream line;
    line << "appsrc ! videoconvert n-threads=" << cv::getNumThreads() << " ! ";
    if (backend.find("basic") == 4)
      line << getValue(defaultEncodeElementByCodec(), codec, "Invalid codec");
    else if (backend.find("vaapi") == 4)
      line << getValue(VAAPIEncodeElementByCodec(), codec, "Invalid codec");
    else if (backend.find("libav") == 4)
      line << getValue(libavEncodeElementByCodec(), codec, "Invalid codec");
    else if (backend.find("mfx") == 4)
      line << getValue(mfxEncodeElementByCodec(), codec, "Invalid codec");
    else
      return cv::Ptr<cv::VideoWriter>();
    line << " ! ";
    line << getValue(muxPluginByContainer(), containerByName(file_name),
                     "Invalid container");
    line << " ! ";
    line << "filesink location=\"" << file_name << "\"";
    std::cout << "Created GStreamer writer ( " << line.str() << " )"
              << std::endl;
    return cv::makePtr<cv::VideoWriter>(line.str(), cv::CAP_GSTREAMER, 0, fps,
                                        sz, true);
  } else if (backend == "ffmpeg") {
    std::cout << "Created FFMpeg writer ( " << file_name << ", FPS=" << fps
              << ", Size=" << sz << " )" << std::endl;
    return cv::makePtr<cv::VideoWriter>(
        file_name, cv::CAP_FFMPEG,
        getValue(fourccByCodec(), codec, "Invalid codec"), fps, sz, true);
  }
  return cv::Ptr<cv::VideoWriter>();
}
