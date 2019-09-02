#include "video_provider.hpp"
#include <iostream>

VideoProvider::VideoProvider(const std::vector<std::string>& video_files) {
  open(video_files);
}

VideoProvider::~VideoProvider() { close(); }

bool VideoProvider::open(const std::vector<std::string>& video_files) {
  video_files_ = video_files;
  for (const auto& video_file : video_files) {
    video_readers_.emplace_back(video_file, cv::CAP_ANY);
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
  frames.resize(video_readers_.size());

#pragma omp parallel for
  for (size_t i = 0; i < video_readers_.size(); ++i) {
    // wait for a new frame from camera and store it into 'frame'
    bool ret = video_readers_[i].read(frames[i]);
    // check if we succeeded
    if (!ret) {
      std::cerr << "ERROR! frame not grabbed: " << video_files_[i] << std::endl;
      continue;
    } else if (frames[i].empty()) {
      std::cerr << "ERROR! blank frame grabbed: " << video_files_[i]
                << std::endl;
      continue;
    }
  }

  return true;
}
