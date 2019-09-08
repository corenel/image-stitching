#include "video_provider.hpp"
#include <iostream>

MultipleVideoProvider::MultipleVideoProvider(
    const std::vector<std::string>& video_files) {
  open(video_files);
}

MultipleVideoProvider::~MultipleVideoProvider() { close(); }

bool MultipleVideoProvider::open(const std::vector<std::string>& video_files) {
  video_files_ = video_files;
  for (const auto& video_file : video_files) {
    video_readers_.emplace_back(
        cv::makePtr<cv::VideoCapture>(video_file, cv::CAP_ANY));
  }
  return true;
}

bool MultipleVideoProvider::isOpened() {
  if (video_readers_.empty()) {
    return false;
  }
  for (size_t i = 0; i < video_readers_.size(); ++i) {
    if (!video_readers_[i]->isOpened()) {
      std::cerr << "ERROR! frame not grabbed: " << video_files_[i] << std::endl;
      return false;
    }
  }
  return true;
}

void MultipleVideoProvider::close() {
  for (auto& video_reader : video_readers_) {
    video_reader.release();
  }
}

bool MultipleVideoProvider::read(std::vector<cv::Mat>& frames) {
  frames.clear();
  frames.resize(video_readers_.size());

#ifdef USE_OPENMP
#pragma omp parallel for
#endif
  for (size_t i = 0; i < video_readers_.size(); ++i) {
    // wait for a new frame from camera and store it into 'frame'
    bool ret = video_readers_[i]->read(frames[i]);
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
