#pragma once
#include <string>
#include <vector>
#include "opencv2/opencv.hpp"

class Warper {
 public:
  explicit Warper(const std::string& filename);
  ~Warper();
  void calibrate();
  void help(char** argv);

 private:
  static void onMouse(int event, int x, int y, int, void*);

  std::string windowTitle = "Perspective Transformation Demo";
  std::string labels[4] = {"TL", "TR", "BR", "BL"};
  std::vector<cv::Point2f> roi_corners;
  std::vector<cv::Point2f> dst_corners{4};
  int roiIndex = 0;
  bool dragging = false;
  int selected_corner_index = 0;
  bool validation_needed = true;

  cv::Mat original_image;
  cv::Mat image;

  int num_images_;
};
