#include "video_provider.hpp"
#include <iostream>

VideoProvider::VideoProvider(const std::vector<std::string>& video_files) {
  open(video_files_);
}

VideoProvider::~VideoProvider() { close(); }

bool VideoProvider::open(const std::vector<std::string>& video_files) {
  video_files_ = video_files;
  for (const auto& video_file : video_files) {
    video_readers_.emplace_back();
    video_readers_[-1].open(video_file);
  }
  return true;
}

bool VideoProvider::isOpened() {
  if (video_readers_.empty()) {
    return false;
  }
  for (size_t i = 0; i < video_readers_.size(); ++i) {
    if (!video_readers_[i].isOpened()) {
      std::cerr << "ERROR! frame not grabbed: " << video_files_[i] << std::endl;
      return false;
    }
  }
  return true;
}

void VideoProvider::close() {
  for (auto& video_reader : video_readers_) {
    video_reader.release();
  }
}

bool VideoProvider::read(std::vector<cv::Mat>& frames) {
  frames.clear();
  for (size_t i = 0; i < video_readers_.size(); ++i) {
    cv::Mat frame;
    // wait for a new frame from camera and store it into 'frame'
    auto ret = video_readers_[i].read(frame);
    // check if we succeeded
    if (!ret) {
      std::cerr << "ERROR! frame not grabbed: " << video_files_[i] << std::endl;
      continue;
    } else if (frame.empty()) {
      std::cerr << "ERROR! blank frame grabbed: " << video_files_[i]
                << std::endl;
      continue;
    }
    frames.push_back(frame);
  }
  return true;
}
