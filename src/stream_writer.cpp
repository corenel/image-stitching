#include "stream_writer.hpp"

#include <utility>

StreamWriter::StreamWriter(std::string video_stream_url, std::string backend,
                           std::string codec, std::string resolution,
                           unsigned int fps)
    : stream_url_(std::move(video_stream_url)),
      backend_(std::move(backend)),
      codec_(std::move(codec)),
      resolution_(std::move(resolution)),
      fps_(fps) {
  open();
}

StreamWriter::~StreamWriter() { close(); }

bool StreamWriter::open() {
  size_ = getValue(sizeByResolution(), resolution_, "Invalid resolution");
  //  LOGLN("FPS: " << fps_ << ", Frame size: " << sz);
  writer_ = createWriter(backend_, stream_url_, codec_, size_, fps_);
  if (!writer_) {
    ERRLN("Failed to create synthetic video source or video writer");
    return false;
  }
  if (!writer_->isOpened()) {
    ERRLN("Synthetic video source or video writer is not opened");
    return false;
  }
  return true;
}

bool StreamWriter::isOpened() { return writer_->isOpened(); }

void StreamWriter::close() { writer_.release(); }

bool StreamWriter::write(const cv::Mat& frame) {
  if (frame.empty()) {
    ERRLN("Skipping empty input frame");
    return false;
  }
  *writer_ << frame;
  return true;
}
