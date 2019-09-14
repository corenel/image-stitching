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

#define ENABLE_LOG 0
#ifndef LOG
#define LOG(msg) std::cout << msg
#endif
#ifndef LOGLN
#define LOGLN(msg) std::cout << msg << std::endl
#endif

class Stitcher {
 public:
  explicit Stitcher(const int& num_images, cv::Size image_size);
  int calibrate(const std::vector<cv::Mat>& full_img, cv::Mat& result,
                cv::Mat& result_mask);
  int process(const std::vector<cv::Mat>& images, cv::Mat& result,
              cv::Mat& result_mask);

 private:
  int num_images_;
  cv::Size image_size_;

  bool preview_ = false;
  bool try_cuda_ = false;
  double work_megapix_ = 0.6;
  double seam_megapix_ = 0.1;
  double compose_megapix_ = -1;
  double work_scale_ = 1;
  double seam_scale_ = 1;
  double compose_scale_ = 1;
  double compose_work_aspect_ = 1;
  bool is_work_scale_set_ = false;
  bool is_seam_scale_set_ = false;
  //  bool is_compose_scale_set_ = false;
  // decrease this if adjuster raise errors
  float conf_thresh_ = 0.2f;
  //#ifdef HAVE_OPENCV_XFEATURES2D
  std::string features_type_ = "surf";
  //#else
  //  std::string features_type_ = "orb";
  //#endif
  // Use "affine" if there exists distortion
  // Or just "homography"
  std::string matcher_type_ = "affine";
  std::string estimator_type_ = "homography";
  std::string ba_cost_func_ = "ray";
  std::string ba_refine_mask_ = "xxxxx";
  bool do_wave_correct_ = true;
  cv::detail::WaveCorrectKind wave_correct_ = cv::detail::WAVE_CORRECT_HORIZ;
  std::string warp_type_ = "cylindrical";
  int expos_comp_type_ = cv::detail::ExposureCompensator::GAIN;
  int expos_comp_nr_feeds_ = 1;
  int expos_comp_nr_filtering_ = 2;
  int expos_comp_block_size_ = 32;
  float match_conf_ = 0.3f;
  std::string seam_find_type_ = "gc_color";
  int blend_type_ = cv::detail::Blender::MULTI_BAND;
  float blend_strength_ = 5;
  int range_width_ = -1;

#if (CV_VERSION_MAJOR >= 4)
  cv::Ptr<cv::Feature2D> finder_;
#else
  cv::Ptr<cv::detail::FeaturesFinder> finder_;
#endif

  std::vector<cv::detail::ImageFeatures> features_;
  std::vector<cv::Mat> images_;
  double seam_work_aspect_ = 1;

  cv::Ptr<cv::detail::FeaturesMatcher> matcher_;
  std::vector<cv::detail::CameraParams> cameras_;
  cv::Ptr<cv::detail::Estimator> estimator_;
  cv::Ptr<cv::detail::BundleAdjusterBase> adjuster_;
  cv::Ptr<cv::WarperCreator> warper_creator_;
  std::vector<cv::Ptr<cv::detail::RotationWarper>> warpers_;
  cv::Ptr<cv::detail::ExposureCompensator> compensator_;
  cv::Ptr<cv::detail::SeamFinder> seam_finder_;

  std::vector<cv::Mat> img_;
  std::vector<cv::Point> corners_;
  std::vector<cv::UMat> masks_warped_;
  std::vector<cv::UMat> images_warped_;
  std::vector<cv::UMat> images_warped_f_;
  std::vector<cv::Size> sizes_;
  std::vector<cv::UMat> masks_;

  std::vector<cv::Mat> img_warped_;
  std::vector<cv::Mat> img_warped_s_;
  std::vector<cv::Mat> dilated_mask_;
  std::vector<cv::Mat> seam_mask_;
  std::vector<cv::Mat> mask_;
  std::vector<cv::Mat> mask_warped_;

  cv::Ptr<cv::detail::Blender> blender_;

  void reset();
  static void post_process(cv::Mat& frame);
};
