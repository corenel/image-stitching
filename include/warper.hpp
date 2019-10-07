#pragma once
#include <string>
#include <utility>
#include <vector>
#include "opencv2/opencv.hpp"

struct WarpConfig {
  WarpConfig(std::string  input_url, const float& input_left,
             const float& input_top, const float& input_right,
             const float& input_bottom)
      : url(std::move(input_url)),
        left(input_left),
        top(input_top),
        right(input_right),
        bottom(input_bottom){};
  std::string url;
  float left, top, right, bottom;
};

struct WarpTransform {
  cv::Matx33f M;
  float width{}, height{};
  cv::Point2f orig;
};

class Warper {
 public:
  explicit Warper();
  ~Warper();
  WarpTransform calibrate_single(const WarpConfig& config);
  WarpTransform calibrate_single(const cv::Mat& frame,
                                 const WarpConfig& config);
  WarpTransform calibrate_single(const std::string& filename, const float& left,
                                 const float& top, const float& right,
                                 const float& bottom);
  WarpTransform calibrate_single(const cv::Mat& frame, const float& left,
                                 const float& top, const float& right,
                                 const float& bottom);
  std::vector<WarpTransform> calibrate(const std::vector<WarpConfig>& configs);
  std::vector<WarpTransform> calibrate(const std::vector<cv::Mat>& frames,
                                       const std::vector<WarpConfig>& configs);
  void help(char** argv);

 private:
  WarpTransform calibrate_single(const float& left, const float& top,
                                 const float& right, const float& bottom);
  static void onCalibrationMouse(int event, int x, int y, int, void*);
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
