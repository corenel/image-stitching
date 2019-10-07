#pragma once
#include <string>
#include <vector>
#include "opencv2/opencv.hpp"

class Warper {
 public:
  explicit Warper();
  ~Warper();
  void calibrate_single(const std::string& filename, const float& left,
                        const float& top, const float& right,
                        const float& bottom);
  void help(char** argv);

 private:
  static void onMouse(int event, int x, int y, int, void*);
  cv::Point2f adjust_point(const int& x, const int& y);

  // params for calibration
  std::string window_title_ = "Perspective Transformation Demo";
  std::string labels_[4] = {"TL", "TR", "BR", "BL"};
  std::vector<cv::Point2f> roi_corners_;
  std::vector<cv::Point2f> dst_corners_{4};
  int roi_index_ = 0;
  bool is_dragging_ = false;
  int selected_corner_index_ = 0;
  bool validation_needed_ = true;
  float side_length = 200.0f;

  // params for corner detection
  bool use_corner_detection_ = true;
  int roi_range_ = 25;
  int max_corners_ = 1;
  double quality_level_ = 0.05;
  double min_distance_ = 2.0;
  int block_size_ = 3;
  bool user_harris_detector_ = true;
  double k_ = 0.04;
  std::vector<cv::Point2f> corners_;

  cv::Mat original_image_;
  cv::Mat image_;

  int num_images_;
};
