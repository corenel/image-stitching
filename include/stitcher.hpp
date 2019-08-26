#pragma once

#include <iostream>
#include <string>
#include "opencv2/core/utility.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/opencv_modules.hpp"
#include "opencv2/stitching/detail/blenders.hpp"
#include "opencv2/stitching/detail/camera.hpp"
#include "opencv2/stitching/detail/exposure_compensate.hpp"
#include "opencv2/stitching/detail/matchers.hpp"
#include "opencv2/stitching/detail/motion_estimators.hpp"
#include "opencv2/stitching/detail/seam_finders.hpp"
#include "opencv2/stitching/detail/warpers.hpp"
#include "opencv2/stitching/warpers.hpp"

#ifdef HAVE_OPENCV_XFEATURES2D
#include "opencv2/xfeatures2d/nonfree.hpp"
#endif

#define ENABLE_LOG 1
#define LOG(msg) std::cout << msg
#define LOGLN(msg) std::cout << msg << std::endl

class Stitcher {
 public:
  Stitcher(const int& num_images);
  int calibrate(const std::vector<std::string>& img_names, cv::Mat result, cv::Mat result_mask);

 private:
  int num_images;
  bool preview = false;
  bool try_cuda = false;
  double work_megapix = 0.08;
  double seam_megapix = 0.08;
  double compose_megapix = -1;
  double work_scale = 1;
  double seam_scale = 1;
  double compose_scale = 1;
  bool is_work_scale_set = false;
  bool is_seam_scale_set = false;
  bool is_compose_scale_set = false;
  float conf_thresh = 0.5f;
#ifdef HAVE_OPENCV_XFEATURES2D
  std::string features_type = "surf";
#else
  std::string features_type = "orb";
#endif
  std::string matcher_type = "homography";
  std::string estimator_type = "homography";
  std::string ba_cost_func = "ray";
  std::string ba_refine_mask = "xxxxx";
  bool do_wave_correct = true;
  cv::detail::WaveCorrectKind wave_correct = cv::detail::WAVE_CORRECT_HORIZ;
  bool save_graph = false;
  std::string save_graph_to;
  std::string warp_type = "cylindrical";
  int expos_comp_type = cv::detail::ExposureCompensator::GAIN;
  int expos_comp_nr_feeds = 1;
  int expos_comp_nr_filtering = 2;
  int expos_comp_block_size = 32;
  float match_conf = 0.3f;
  std::string seam_find_type = "dp_colorgrad";
  int blend_type = cv::detail::Blender::MULTI_BAND;
  float blend_strength = 5;
  int range_width = -1;

  std::vector<cv::Point> corners;
  std::vector<cv::UMat> masks_warped;
  std::vector<cv::UMat> images_warped;
  std::vector<cv::Size> sizes;
  std::vector<cv::UMat> masks;

  std::vector<cv::Mat> img_warped;
  std::vector<cv::Mat> img_warped_s;
  std::vector<cv::Mat> dilated_mask;
  std::vector<cv::Mat> seam_mask;
  std::vector<cv::Mat> mask;
  std::vector<cv::Mat> mask_warped;

  cv::Ptr<cv::detail::Blender> blender;
};
