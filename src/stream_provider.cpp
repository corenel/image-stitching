#include "stream_provider.hpp"

#include <utility>

StreamProvider::StreamProvider(std::string video_stream_url,
                               std::string backend, std::string codec,
                               std::string resolution, unsigned fps, bool fast)
    : stream_url_(std::move(video_stream_url)),
      backend_(std::move(backend)),
      codec_(std::move(codec)),
      resolution_(std::move(resolution)),
      fps_(fps),
      fast_measure_(fast) {
  //  LOGLN("Stream Provider");
  //  LOGLN("  Url: " << stream_url_);
  //  LOGLN("  Backend: " << backend_);
  //  LOGLN("  Codec: " << codec_);
  //  LOGLN("  Resolution: " << resolution_);
  //  LOGLN("  FPS: " << fps_);
  open();
}

StreamProvider::~StreamProvider() {}

bool StreamProvider::open() {
  cap_ = createCapture(backend_, stream_url_, codec_);
  if (!cap_) {
    LOGLN("Failed to create video capture: " << stream_url_);
    return false;
  }
  if (!cap_->isOpened()) {
    LOGLN("Capture is not opened" << stream_url_);
    return false;
  }
  return true;
}

bool StreamProvider::isOpened() { return cap_->isOpened(); }

void StreamProvider::close() { cap_->release(); }

bool StreamProvider::read(cv::Mat& frame) {
  if (!cap_->grab()) {
    LOGLN("No more frames");
    return false;
  }
  if (!cap_->retrieve(frame)) {
    LOGLN("Failed to retrieve frame");
    return false;
  }
  if (frame.empty()) {
    LOGLN("Empty frame received");
    return false;
  }
  return true;
}

MultipleStreamProvider::MultipleStreamProvider(
    const std::vector<std::string>& video_stream_urls) {}

MultipleStreamProvider::~MultipleStreamProvider() {}

bool MultipleStreamProvider::open() { return false; }

bool MultipleStreamProvider::isOpened() { return false; }

void MultipleStreamProvider::close() {}

bool MultipleStreamProvider::read(std::vector<cv::Mat>& frames) {
  return false;
}
