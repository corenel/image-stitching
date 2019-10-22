#include "stream_provider.hpp"

#include <utility>

StreamProvider::StreamProvider(std::string video_stream_url,
                               std::string backend, std::string codec,
                               std::string resolution, unsigned fps, bool fast,
                               bool sync)
    : stream_url_(std::move(video_stream_url)),
      backend_(std::move(backend)),
      codec_(std::move(codec)),
      resolution_(std::move(resolution)),
      fps_(fps),
      fast_measure_(fast),
      sync_(sync) {
  //  LOGLN("Stream Provider");
  //  LOGLN("  Url: " << stream_url_);
  //  LOGLN("  Backend: " << backend_);
  //  LOGLN("  Codec: " << codec_);
  //  LOGLN("  Resolution: " << resolution_);
  //  LOGLN("  FPS: " << fps_);
  open();
}

StreamProvider::~StreamProvider() { close(); }

bool StreamProvider::open() {
  cap_ = createCapture(backend_, stream_url_, codec_, sync_);
  if (!cap_) {
    ERRLN("Failed to create video capture: " << stream_url_);
    return false;
  }
  if (!cap_->isOpened()) {
    ERRLN("Capture is not opened: " << stream_url_);
    return false;
  }
  return true;
}

bool StreamProvider::isOpened() { return cap_->isOpened(); }

void StreamProvider::close() { cap_->release(); }

bool StreamProvider::read(cv::Mat& frame) {
  if (!cap_->grab()) {
    ERRLN("No more frames");
    return false;
  }
  if (!cap_->retrieve(frame)) {
    ERRLN("Failed to retrieve frame");
    return false;
  }
  if (frame.empty()) {
    ERRLN("Empty frame received");
    return false;
  }
  return true;
}

MultipleStreamProvider::MultipleStreamProvider(
    std::vector<std::string> video_stream_urls, std::string backend,
    std::string codec, std::string resolution, unsigned fps)
    : stream_urls_(std::move(video_stream_urls)),
      backend_(std::move(backend)),
      codec_(std::move(codec)),
      resolution_(std::move(resolution)),
      fps_(fps) {
  open();
}

MultipleStreamProvider::~MultipleStreamProvider() { close(); }

bool MultipleStreamProvider::open() {
  for (const auto& stream_url : stream_urls_) {
    streamers_.emplace_back(cv::makePtr<StreamProvider>(
        stream_url, backend_, codec_, resolution_, fps_));
  }
  return true;
}

bool MultipleStreamProvider::isOpened() {
  if (streamers_.empty()) {
    return false;
  }
  for (size_t i = 0; i < streamers_.size(); ++i) {
    if (!streamers_[i]->isOpened()) {
      ERRLN("ERROR! frame not grabbed: " << stream_urls_[i]);
      return false;
    }
  }
  return true;
}

void MultipleStreamProvider::close() {
  for (auto& streamer : streamers_) {
    streamer.release();
  }
}

bool MultipleStreamProvider::read(std::vector<cv::Mat>& frames) {
  frames.clear();
  frames.resize(streamers_.size());

#ifdef USE_OPENMP
#pragma omp parallel for
#endif
  for (size_t i = 0; i < streamers_.size(); ++i) {
    // wait for a new frame from camera and store it into 'frame'
    bool ret = streamers_[i]->read(frames[i]);
    // check if we succeeded
    if (!ret) {
      ERRLN("ERROR! frame not grabbed: " << stream_urls_[i]);
      continue;
    } else if (frames[i].empty()) {
      ERRLN("ERROR! blank frame grabbed: " << stream_urls_[i]);
      continue;
    }
  }

  return true;
}
