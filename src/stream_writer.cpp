#include "stream_writer.hpp"

#include <utility>

StreamWriter::StreamWriter(std::string video_stream_url, std::string backend,
                           std::string codec, std::string resolution,
                           unsigned int fps, cv::Size sz)
    : stream_url_(std::move(video_stream_url)),
      backend_(std::move(backend)),
      codec_(std::move(codec)),
      resolution_(std::move(resolution)),
      fps_(fps),
      size_(std::move(sz)) {
  open(size_);
}

StreamWriter::~StreamWriter() { close(); }

bool StreamWriter::open(const cv::Size& sz) {
  if (sz == cv::Size(0, 0)) {
    if (size_ == cv::Size(0, 0)) {
      size_ = getValue(sizeByResolution(), resolution_, "Invalid resolution");
    }
  } else {
    size_ = sz;
  }
  //  LOGLN("Size: " << size_);
  //  LOGLN("FPS: " << fps_ << ", Frame size: " << sz);
  writer_ = createWriter(backend_, stream_url_, codec_, size_, fps_);
  if (!writer_) {
    ERRLN("Failed to create video writer");
    return false;
  }
  if (!writer_->isOpened()) {
    ERRLN("video writer is not opened");
    return false;
  }
  return true;
}

bool StreamWriter::isOpened() { return writer_->isOpened(); }

void StreamWriter::close() { writer_.release(); }

bool StreamWriter::write(cv::Mat& frame, const bool& resize_writer,
                         const bool& use_pre_process) {
  if (frame.empty()) {
    ERRLN("Skipping empty input frame");
    return false;
  }
  if (use_pre_process) {
    pre_process(frame);
  }
  if (resize_writer && frame.size() != size()) {
    open(frame.size());
  }
  *writer_ << frame;
  return true;
}

void StreamWriter::pre_process(cv::Mat& frame) {
  // Convert type from CV_16UC3 to CV_8UC3
  // frame.convertTo(frame, CV_8UC3);

  // Avoid odd width and height
  if (frame.size().height % 4 != 0 || frame.size().width % 4 != 0) {
    auto sz = frame.size();
    cv::Rect even_roi(0, 0, sz.width - sz.width % 4, sz.height - sz.height % 4);
    frame = frame(even_roi);
  }
}
