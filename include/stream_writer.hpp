#pragma once

#include "stream_helper.hpp"

class StreamWriter {
 public:
  explicit StreamWriter(const std::string &video_stream_url);

 private:
  std::string backend_ = "gst-default";  // video backend
  std::string codec_ = "h264";           // codec type
  std::string stream_url_;               // stream url
  std::string resolution_ = "720p";      // video resolution
};
