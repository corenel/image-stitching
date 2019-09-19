#include "stitcher.hpp"

Stitcher::Stitcher(const int& num_images_given, cv::Size image_size_given)
    : num_images_(num_images_given), image_size_(std::move(image_size_given)) {
  reset();
}

void Stitcher::reset() {
  if (features_type_ == "orb") match_conf_ = 0.2f;
  if (preview_) {
    compose_megapix_ = 0.6;
  }

  features_.clear();
  images_.clear();
  cameras_.clear();
  warpers_.clear();
  features_.resize(num_images_);
  images_.resize(num_images_);
  cameras_.resize(num_images_);
  warpers_.resize(num_images_);

  img_.clear();
  corners_.clear();
  masks_warped_.clear();
  images_warped_.clear();
  images_warped_f_.clear();
  sizes_.clear();
  masks_.clear();
  img_.resize(num_images_);
  corners_.resize(num_images_);
  masks_warped_.resize(num_images_);
  images_warped_.resize(num_images_);
  images_warped_f_.resize(num_images_);
  sizes_.resize(num_images_);
  masks_.resize(num_images_);

  img_warped_.clear();
  img_warped_s_.clear();
  dilated_mask_.clear();
  seam_mask_.clear();
  mask_.clear();
  mask_warped_.clear();
  img_warped_.resize(num_images_);
  img_warped_s_.resize(num_images_);
  dilated_mask_.resize(num_images_);
  seam_mask_.resize(num_images_);
  mask_.resize(num_images_);
  mask_warped_.resize(num_images_);

  // feature finder_
#if (CV_VERSION_MAJOR >= 4)
  if (features_type_ == "orb") {
    finder_ = cv::ORB::create();
  } else if (features_type_ == "akaze") {
    finder_ = cv::AKAZE::create();
  }
#ifdef HAVE_OPENCV_XFEATURES2D
  else if (features_type_ == "surf") {
    finder_ = cv::xfeatures2d::SURF::create();
  } else if (features_type_ == "sift") {
    finder_ = cv::xfeatures2d::SIFT::create();
  }
#endif
  else {
    LOGLN("Unknown 2D features_ type: '" << features_type_);
    // return -1;
  }
#else
  if (features_type_ == "surf") {
#ifdef HAVE_OPENCV_XFEATURES2D
    if (try_cuda_ && cv::cuda::getCudaEnabledDeviceCount() > 0)
      finder_ = cv::makePtr<cv::detail::SurfFeaturesFinderGpu>();
    else
#endif
      finder_ = cv::makePtr<cv::detail::SurfFeaturesFinder>();
  } else if (features_type_ == "orb") {
    finder_ = cv::makePtr<cv::detail::OrbFeaturesFinder>();
  } else if (features_type_ == "sift") {
    finder_ = cv::makePtr<cv::detail::SiftFeaturesFinder>();
  } else {
    LOGLN("Unknown 2D features_ type: '" << features_type_);
    // return -1;
  }
#endif

  // feature matcher
  if (matcher_type_ == "affine") {
    matcher_ = cv::makePtr<cv::detail::AffineBestOf2NearestMatcher>(
        false, try_cuda_, match_conf_);
  } else if (range_width_ == -1) {
    matcher_ =
        cv::makePtr<cv::detail::BestOf2NearestMatcher>(try_cuda_, match_conf_);
  } else {
    matcher_ = cv::makePtr<cv::detail::BestOf2NearestRangeMatcher>(
        range_width_, try_cuda_, match_conf_);
  }

  // camera estimator
  if (estimator_type_ == "affine") {
    estimator_ = cv::makePtr<cv::detail::AffineBasedEstimator>();
  } else {
    estimator_ = cv::makePtr<cv::detail::HomographyBasedEstimator>();
  }

  // adjuster
  if (ba_cost_func_ == "reproj")
    adjuster_ = cv::makePtr<cv::detail::BundleAdjusterReproj>();
  else if (ba_cost_func_ == "ray")
    adjuster_ = cv::makePtr<cv::detail::BundleAdjusterRay>();
  else if (ba_cost_func_ == "affine")
    adjuster_ = cv::makePtr<cv::detail::BundleAdjusterAffinePartial>();
  else if (ba_cost_func_ == "no")
    adjuster_ = cv::makePtr<cv::detail::NoBundleAdjuster>();
  else {
    LOGLN("Unknown bundle adjustment cost function: '" << ba_cost_func_);
    // return -1;
  }
  adjuster_->setConfThresh(conf_thresh_);
  cv::Mat_<uchar> refine_mask = cv::Mat::zeros(3, 3, CV_8U);
  if (ba_refine_mask_[0] == 'x') refine_mask(0, 0) = 1;
  if (ba_refine_mask_[1] == 'x') refine_mask(0, 1) = 1;
  if (ba_refine_mask_[2] == 'x') refine_mask(0, 2) = 1;
  if (ba_refine_mask_[3] == 'x') refine_mask(1, 1) = 1;
  if (ba_refine_mask_[4] == 'x') refine_mask(1, 2) = 1;
  adjuster_->setRefinementMask(refine_mask);

  // warper creator
#ifdef HAVE_OPENCV_CUDAWARPING
  if (try_cuda_ && cv::cuda::getCudaEnabledDeviceCount() > 0) {
    if (warp_type_ == "plane")
      warper_creator_ = cv::makePtr<cv::PlaneWarperGpu>();
    else if (warp_type_ == "cylindrical")
      warper_creator_ = cv::makePtr<cv::CylindricalWarperGpu>();
    else if (warp_type_ == "spherical")
      warper_creator_ = cv::makePtr<cv::SphericalWarperGpu>();
  } else
#endif
  {
    if (warp_type_ == "plane")
      warper_creator_ = cv::makePtr<cv::PlaneWarper>();
    else if (warp_type_ == "affine")
      warper_creator_ = cv::makePtr<cv::AffineWarper>();
    else if (warp_type_ == "cylindrical")
      warper_creator_ = cv::makePtr<cv::CylindricalWarper>();
    else if (warp_type_ == "spherical")
      warper_creator_ = cv::makePtr<cv::SphericalWarper>();
    else if (warp_type_ == "fisheye")
      warper_creator_ = cv::makePtr<cv::FisheyeWarper>();
    else if (warp_type_ == "stereographic")
      warper_creator_ = cv::makePtr<cv::StereographicWarper>();
    else if (warp_type_ == "compressedPlaneA2B1")
      warper_creator_ =
          cv::makePtr<cv::CompressedRectilinearWarper>(2.0f, 1.0f);
    else if (warp_type_ == "compressedPlaneA1.5B1")
      warper_creator_ =
          cv::makePtr<cv::CompressedRectilinearWarper>(1.5f, 1.0f);
    else if (warp_type_ == "compressedPlanePortraitA2B1")
      warper_creator_ =
          cv::makePtr<cv::CompressedRectilinearPortraitWarper>(2.0f, 1.0f);
    else if (warp_type_ == "compressedPlanePortraitA1.5B1")
      warper_creator_ =
          cv::makePtr<cv::CompressedRectilinearPortraitWarper>(1.5f, 1.0f);
    else if (warp_type_ == "paniniA2B1")
      warper_creator_ = cv::makePtr<cv::PaniniWarper>(2.0f, 1.0f);
    else if (warp_type_ == "paniniA1.5B1")
      warper_creator_ = cv::makePtr<cv::PaniniWarper>(1.5f, 1.0f);
    else if (warp_type_ == "paniniPortraitA2B1")
      warper_creator_ = cv::makePtr<cv::PaniniPortraitWarper>(2.0f, 1.0f);
    else if (warp_type_ == "paniniPortraitA1.5B1")
      warper_creator_ = cv::makePtr<cv::PaniniPortraitWarper>(1.5f, 1.0f);
    else if (warp_type_ == "mercator")
      warper_creator_ = cv::makePtr<cv::MercatorWarper>();
    else if (warp_type_ == "transverseMercator")
      warper_creator_ = cv::makePtr<cv::TransverseMercatorWarper>();
  }

  if (!warper_creator_) {
    LOGLN("Can't create the following warper '" << warp_type_);
    // return -1;
  }

  // compensator
  compensator_ =
      cv::detail::ExposureCompensator::createDefault(expos_comp_type_);
#if (CV_VERSION_MAJOR >= 4)
  if (dynamic_cast<cv::detail::GainCompensator*>(compensator_.get())) {
    auto* gcompensator =
        dynamic_cast<cv::detail::GainCompensator*>(compensator_.get());
    gcompensator->setNrFeeds(expos_comp_nr_feeds_);
  }

  if (dynamic_cast<cv::detail::ChannelsCompensator*>(compensator_.get())) {
    auto* ccompensator =
        dynamic_cast<cv::detail::ChannelsCompensator*>(compensator_.get());
    ccompensator->setNrFeeds(expos_comp_nr_feeds_);
  }

  if (dynamic_cast<cv::detail::BlocksCompensator*>(compensator_.get())) {
    auto* bcompensator =
        dynamic_cast<cv::detail::BlocksCompensator*>(compensator_.get());
    bcompensator->setNrFeeds(expos_comp_nr_feeds_);
    bcompensator->setNrGainsFilteringIterations(expos_comp_nr_filtering_);
    bcompensator->setBlockSize(expos_comp_block_size_, expos_comp_block_size_);
  }
#endif

  // seam finder_
  if (seam_find_type_ == "no")
    seam_finder_ = cv::makePtr<cv::detail::NoSeamFinder>();
  else if (seam_find_type_ == "voronoi")
    seam_finder_ = cv::makePtr<cv::detail::VoronoiSeamFinder>();
  else if (seam_find_type_ == "gc_color") {
    //#ifdef HAVE_OPENCV_CUDALEGACY
    //    if (try_cuda_ && cuda::getCudaEnabledDeviceCount() > 0)
    //      seam_finder = makePtr<detail::GraphCutSeamFinderGpu>(
    //          GraphCutSeamFinderBase::COST_COLOR);
    //    else
    //#endif
    seam_finder_ = cv::makePtr<cv::detail::GraphCutSeamFinder>(
        cv::detail::GraphCutSeamFinderBase::COST_COLOR);
  } else if (seam_find_type_ == "gc_colorgrad") {
    //#ifdef HAVE_OPENCV_CUDALEGACY
    //    if (try_cuda_ && cuda::getCudaEnabledDeviceCount() > 0)
    //      seam_finder = makePtr<detail::GraphCutSeamFinderGpu>(
    //          GraphCutSeamFinderBase::COST_COLOR_GRAD);
    //    else
    //#endif
    seam_finder_ = cv::makePtr<cv::detail::GraphCutSeamFinder>(
        cv::detail::GraphCutSeamFinderBase::COST_COLOR_GRAD);
  } else if (seam_find_type_ == "dp_color")
    seam_finder_ =
        cv::makePtr<cv::detail::DpSeamFinder>(cv::detail::DpSeamFinder::COLOR);
  else if (seam_find_type_ == "dp_colorgrad")
    seam_finder_ = cv::makePtr<cv::detail::DpSeamFinder>(
        cv::detail::DpSeamFinder::COLOR_GRAD);
  if (!seam_finder_) {
    LOGLN("Can't create the following seam finder_ '" << seam_find_type_);
    // return -1;
  }
}

int Stitcher::calibrate(const std::vector<cv::Mat>& full_img, cv::Mat& result,
                        cv::Mat& result_mask) {
  //////////////
  // Checking //
  //////////////
#if ENABLE_LOG
  int64 app_start_time = cv::getTickCount();
#endif

  reset();

  // Check if have enough images
  if (num_images_ != static_cast<int>(full_img.size())) {
    LOGLN("Invalid number of images");
    return -1;
  } else if (num_images_ < 2) {
    LOGLN("Need more images");
    return -1;
  }

  // Check image size
  for (const auto& image : full_img) {
    if (image_size_ != image.size()) {
      LOGLN("Invalid resolution of input images");
      return -1;
    }
  }

  //////////////////////
  // Finding features_ //
  ////////////////////

#if ENABLE_LOG
  LOGLN("finding features_...");
  auto t = cv::getTickCount();
#endif

#ifdef USE_OPENMP
#pragma omp parallel for
#endif
  for (int i = 0; i < num_images_; ++i) {
    if (work_megapix_ < 0) {
      img_[i] = full_img[i];
      work_scale_ = 1;
      is_work_scale_set_ = true;
    } else {
      if (!is_work_scale_set_) {
        work_scale_ = std::min(
            1.0, std::sqrt(work_megapix_ * 1e6 / full_img[i].size().area()));
        is_work_scale_set_ = true;
      }
      cv::resize(full_img[i], img_[i], cv::Size(), work_scale_, work_scale_,
                 cv::INTER_LINEAR_EXACT);
    }
    if (!is_seam_scale_set_) {
      seam_scale_ = std::min(
          1.0, std::sqrt(seam_megapix_ * 1e6 / full_img[i].size().area()));
      seam_work_aspect_ = seam_scale_ / work_scale_;
      is_seam_scale_set_ = true;
    }
#if (CV_VERSION_MAJOR >= 4)
    computeImageFeatures(finder_, img_[i], features_[i]);
#else
    (*finder_)(img_[i], features_[i]);
#endif
    features_[i].img_idx = i;
#if ENABLE_LOG
    LOGLN("Features in image #" << i + 1 << ": "
                                << features_[i].keypoints.size());
#endif

    resize(full_img[i], img_[i], cv::Size(), seam_scale_, seam_scale_,
           cv::INTER_LINEAR_EXACT);
    images_[i] = img_[i].clone();
  }
#if (CV_VERSION_MAJOR < 4)
  finder_->collectGarbage();
#endif
#if ENABLE_LOG
  LOGLN("Finding features_, time: "
        << ((cv::getTickCount() - t) / cv::getTickFrequency()) << " sec");
#endif

  ///////////////////////
  // Pairwise matching //
  ///////////////////////

#if ENABLE_LOG
  LOGLN("Pairwise matching");
  t = cv::getTickCount();
#endif

  std::vector<cv::detail::MatchesInfo> pairwise_matches;

  cv::UMat matchMask(features_.size(), features_.size(), CV_8U, cv::Scalar(0));
  // mask frames from The adjacent cameras
#ifdef USE_OPENMP
#pragma omp parallel for
#endif
  for (int i = 0; i < num_images_ - 1; ++i) {
    matchMask.getMat(cv::ACCESS_READ).at<char>(i, i + 1) = 1;
  }
  for (int i = 1; i < num_images_; ++i) {
    matchMask.getMat(cv::ACCESS_READ).at<char>(i, i - 1) = 1;
  }
  // mask frames from the first and the last camera
  matchMask.getMat(cv::ACCESS_READ).at<char>(0, num_images_ - 1) = 1;
  matchMask.getMat(cv::ACCESS_READ).at<char>(num_images_ - 1, 0) = 1;
  (*matcher_)(features_, pairwise_matches, matchMask);
  //  (*matcher_)(features_, pairwise_matches);

  matcher_->collectGarbage();

#if ENABLE_LOG
  LOGLN("Pairwise matching, time: "
        << ((cv::getTickCount() - t) / cv::getTickFrequency()) << " sec");
#endif

  /////////////////
  // Estimating //
  ////////////////

#if ENABLE_LOG
  LOGLN("Estimating");
  t = cv::getTickCount();
#endif

  if (!(*estimator_)(features_, pairwise_matches, cameras_)) {
    LOGLN("Homography estimation failed.");
    return -1;
  }
  for (auto& camera : cameras_) {
    cv::Mat R;
    camera.R.convertTo(R, CV_32F);
    camera.R = R;
  }

  if (!(*adjuster_)(features_, pairwise_matches, cameras_)) {
    LOGLN("Camera parameters adjusting failed.");
    return -1;
  }
  // Find median focal length
  std::vector<double> focals;
  focals.reserve(cameras_.size());
  for (size_t i = 0; i < cameras_.size(); ++i) {
    focals[i] = cameras_[i].focal;
  }
  std::sort(focals.begin(), focals.end());
  float warped_image_scale;
  if (focals.size() % 2 == 1)
    warped_image_scale = static_cast<float>(focals[focals.size() / 2]);
  else
    warped_image_scale = static_cast<float>(focals[focals.size() / 2 - 1] +
                                            focals[focals.size() / 2]) *
                         0.5f;
  if (do_wave_correct_) {
    std::vector<cv::Mat> rmats;
    rmats.reserve(cameras_.size());
    for (const auto& camera : cameras_) {
      rmats.push_back(camera.R.clone());
    }
    cv::detail::waveCorrect(rmats, wave_correct_);
    for (size_t i = 0; i < cameras_.size(); ++i) {
      cameras_[i].R = rmats[i];
    }
  }
#if ENABLE_LOG
  LOGLN("Estimating, time: "
        << ((cv::getTickCount() - t) / cv::getTickFrequency()) << " sec");
#endif

  ////////////////////
  // Warping images //
  ////////////////////

#if ENABLE_LOG
  LOGLN("Warping images (auxiliary)... ");
  t = cv::getTickCount();
#endif

  // Prepare images masks
#ifdef USE_OPENMP
#pragma omp parallel for
#endif
  for (int i = 0; i < num_images_; ++i) {
    masks_[i].create(images_[i].size(), CV_8U);
    masks_[i].setTo(cv::Scalar::all(255));
  }

  // Warp images and their masks

#ifdef USE_OPENMP
#pragma omp parallel for
#endif
  for (int i = 0; i < num_images_; ++i) {
    warpers_[i] = warper_creator_->create(
        static_cast<float>(warped_image_scale * seam_work_aspect_));
  }

#ifdef USE_OPENMP
#pragma omp parallel for
#endif
  for (int i = 0; i < num_images_; ++i) {
    cv::Mat_<float> K;
    cameras_[i].K().convertTo(K, CV_32F);
    auto swa = (float)seam_work_aspect_;
    K(0, 0) *= swa;
    K(0, 2) *= swa;
    K(1, 1) *= swa;
    K(1, 2) *= swa;

    corners_[i] =
        warpers_[i]->warp(images_[i], K, cameras_[i].R, cv::INTER_LINEAR,
                          cv::BORDER_REFLECT, images_warped_[i]);
    sizes_[i] = images_warped_[i].size();

    warpers_[i]->warp(masks_[i], K, cameras_[i].R, cv::INTER_NEAREST,
                      cv::BORDER_CONSTANT, masks_warped_[i]);
    images_warped_[i].convertTo(images_warped_f_[i], CV_32F);
  }

#if ENABLE_LOG
  LOGLN("Warping images, time: "
        << ((cv::getTickCount() - t) / cv::getTickFrequency()) << " sec");
#endif

  ///////////////////////////
  // Compensating exposure //
  ///////////////////////////

#if ENABLE_LOG
  LOGLN("Compensating exposure...");
  t = cv::getTickCount();
#endif

  compensator_->feed(corners_, images_warped_, masks_warped_);

#if ENABLE_LOG
  LOGLN("Compensating exposure, time: "
        << ((cv::getTickCount() - t) / cv::getTickFrequency()) << " sec");
#endif

  ///////////////////
  // Finding seams //
  ///////////////////

#if ENABLE_LOG
  LOGLN("Finding seams...");
  t = cv::getTickCount();
#endif

  seam_finder_->find(images_warped_f_, corners_, masks_warped_);

  // FIXME Release unused memory
  images_.clear();
  images_warped_.clear();
  images_warped_f_.clear();
  masks_.clear();

#if ENABLE_LOG
  LOGLN("Finding seams, time: "
        << ((cv::getTickCount() - t) / cv::getTickFrequency()) << " sec");
#endif

  /////////////////
  // Compositing //
  /////////////////

#if ENABLE_LOG
  LOGLN("Compositing...");
  t = cv::getTickCount();
#endif

  //  if (!is_compose_scale_set_) {
  if (true) {
    if (compose_megapix_ > 0)
      compose_scale_ = std::min(
          1.0, std::sqrt(compose_megapix_ * 1e6 / full_img[0].size().area()));
    //    is_compose_scale_set_ = true;

    // Compute relative scales
    compose_work_aspect_ = compose_scale_ / work_scale_;

    // Update warped image scale
    warped_image_scale *= static_cast<float>(compose_work_aspect_);

#ifdef USE_OPENMP
#pragma omp parallel for
#endif
    for (int img_idx = 0; img_idx < num_images_; ++img_idx) {
      warpers_[img_idx] = warper_creator_->create(warped_image_scale);
    }

    // Update corners and sizes
#ifdef USE_OPENMP
#pragma omp parallel for
#endif
    for (int i = 0; i < num_images_; ++i) {
      // Update intrinsics
      cameras_[i].focal *= compose_work_aspect_;
      cameras_[i].ppx *= compose_work_aspect_;
      cameras_[i].ppy *= compose_work_aspect_;

      // Update corner and size
      cv::Size sz = image_size_;
      if (std::abs(compose_scale_ - 1) > 1e-1) {
        sz.width = cvRound(image_size_.width * compose_scale_);
        sz.height = cvRound(image_size_.height * compose_scale_);
      }

      cv::Mat K;
      cameras_[i].K().convertTo(K, CV_32F);
      cv::Rect roi = warpers_[i]->warpRoi(sz, K, cameras_[i].R);
      corners_[i] = roi.tl();
      sizes_[i] = roi.size();
    }
  }

  //  if (!blender_) {
#if ENABLE_LOG
  LOGLN("Create blender");
#endif
  blender_ = cv::detail::Blender::createDefault(blend_type_, try_cuda_);
  cv::Size dst_sz = cv::detail::resultRoi(corners_, sizes_).size();
  float blend_width =
      std::sqrt(static_cast<float>(dst_sz.area())) * blend_strength_ / 100.f;
  if (blend_width < 1.f)
    blender_ =
        cv::detail::Blender::createDefault(cv::detail::Blender::NO, try_cuda_);
  else if (blend_type_ == cv::detail::Blender::MULTI_BAND) {
    auto* mb = dynamic_cast<cv::detail::MultiBandBlender*>(blender_.get());
    mb->setNumBands(
        static_cast<int>(std::ceil(log(blend_width) / log(2.)) - 1.));
#if ENABLE_LOG
    LOGLN("Multi-band blender, number of bands: " << mb->numBands());
#endif
  } else if (blend_type_ == cv::detail::Blender::FEATHER) {
    auto* fb = dynamic_cast<cv::detail::FeatherBlender*>(blender_.get());
    fb->setSharpness(1.f / blend_width);
#if ENABLE_LOG
    LOGLN("Feather blender, sharpness: " << fb->sharpness());
#endif
  }
  blender_->prepare(corners_, sizes_);
  //  }

#ifdef USE_OPENMP
#pragma omp parallel for
#endif
  for (int img_idx = 0; img_idx < num_images_; ++img_idx) {
    // Read image and resize it if necessary

    if (std::abs(compose_scale_ - 1) > 1e-1)
      cv::resize(full_img[img_idx], img_[img_idx], cv::Size(), compose_scale_,
                 compose_scale_, cv::INTER_LINEAR_EXACT);
    else
      img_[img_idx] = full_img[img_idx];
    cv::Size img_size = img_[img_idx].size();

    cv::Mat K;
    cameras_[img_idx].K().convertTo(K, CV_32F);

    // Warp the current image
    warpers_[img_idx]->warp(img_[img_idx], K, cameras_[img_idx].R,
                            cv::INTER_LINEAR, cv::BORDER_REFLECT,
                            img_warped_[img_idx]);

    // Warp the current image mask
    mask_[img_idx].create(img_size, CV_8U);
    mask_[img_idx].setTo(cv::Scalar::all(255));
    warpers_[img_idx]->warp(mask_[img_idx], K, cameras_[img_idx].R,
                            cv::INTER_NEAREST, cv::BORDER_CONSTANT,
                            mask_warped_[img_idx]);

    // Compensate exposure
    compensator_->apply(img_idx, corners_[img_idx], img_warped_[img_idx],
                        mask_warped_[img_idx]);

    img_warped_[img_idx].convertTo(img_warped_s_[img_idx], CV_16S);

    cv::dilate(masks_warped_[img_idx], dilated_mask_[img_idx], cv::Mat());
    cv::resize(dilated_mask_[img_idx], seam_mask_[img_idx],
               mask_warped_[img_idx].size(), 0, 0, cv::INTER_LINEAR_EXACT);
    mask_warped_[img_idx] = seam_mask_[img_idx] & mask_warped_[img_idx];

    // Blend the current image
    blender_->feed(img_warped_s_[img_idx], mask_warped_[img_idx],
                   corners_[img_idx]);
  }
  blender_->blend(result, result_mask);

#if ENABLE_LOG
  LOGLN("Compositing, time: "
        << ((cv::getTickCount() - t) / cv::getTickFrequency()) << " sec");
  LOGLN("Finished, total time: "
        << ((cv::getTickCount() - app_start_time) / cv::getTickFrequency())
        << " sec");
#endif

  post_process(result);
  post_process(result_mask);

  return 0;
}

int Stitcher::process(const std::vector<cv::Mat>& full_img, cv::Mat& result,
                      cv::Mat& result_mask) {
#if ENABLE_LOG
  auto app_start_time = cv::getTickCount();
  auto t = cv::getTickCount();
#endif
  img_.clear();
  img_warped_.clear();
  img_warped_s_.clear();
  dilated_mask_.clear();
  seam_mask_.clear();
  mask_.clear();
  mask_warped_.clear();
#if ENABLE_LOG
  LOGLN("Clear state, time: "
        << ((cv::getTickCount() - t) / cv::getTickFrequency()) << " sec");
#endif

#if ENABLE_LOG
  t = cv::getTickCount();
#endif
  blender_->prepare(corners_, sizes_);
#if ENABLE_LOG
  LOGLN("Prepare blender, time: "
        << ((cv::getTickCount() - t) / cv::getTickFrequency()) << " sec");
#endif

#if ENABLE_LOG
  t = cv::getTickCount();
#endif
  for (int img_idx = 0; img_idx < num_images_; ++img_idx) {
    // Resize images if necessary
    if (std::abs(compose_scale_ - 1) > 1e-1) {
      cv::resize(full_img[img_idx], img_[img_idx], cv::Size(), compose_scale_,
                 compose_scale_, cv::INTER_LINEAR_EXACT);
    } else {
      img_[img_idx] = full_img[img_idx];
    }
    cv::Size img_size = img_[img_idx].size();

    cv::Mat K;
    cameras_[img_idx].K().convertTo(K, CV_32F);

    // Warp the current image
    warpers_[img_idx]->warp(img_[img_idx], K, cameras_[img_idx].R,
                            cv::INTER_LINEAR, cv::BORDER_REFLECT,
                            img_warped_[img_idx]);

    // Warp the current image mask
    mask_[img_idx].create(img_size, CV_8U);
    mask_[img_idx].setTo(cv::Scalar::all(255));
    warpers_[img_idx]->warp(mask_[img_idx], K, cameras_[img_idx].R,
                            cv::INTER_NEAREST, cv::BORDER_CONSTANT,
                            mask_warped_[img_idx]);

    // Compensate exposure
    compensator_->apply(img_idx, corners_[img_idx], img_warped_[img_idx],
                        mask_warped_[img_idx]);

    img_warped_[img_idx].convertTo(img_warped_s_[img_idx], CV_16S);

    cv::dilate(masks_warped_[img_idx], dilated_mask_[img_idx], cv::Mat());
    cv::resize(dilated_mask_[img_idx], seam_mask_[img_idx],
               mask_warped_[img_idx].size(), 0, 0, cv::INTER_LINEAR_EXACT);
    mask_warped_[img_idx] = seam_mask_[img_idx] & mask_warped_[img_idx];

    // Blend the current image
    blender_->feed(img_warped_s_[img_idx], mask_warped_[img_idx],
                   corners_[img_idx]);
  }
  blender_->blend(result, result_mask);
#if ENABLE_LOG
  LOGLN(
      "Blending, time: " << ((cv::getTickCount() - t) / cv::getTickFrequency())
                         << " sec");
  LOGLN("Finished, total time: "
        << ((cv::getTickCount() - app_start_time) / cv::getTickFrequency())
        << " sec");
#endif

  post_process(result);
  post_process(result_mask);

  return 0;
}

void Stitcher::post_process(cv::Mat& frame) {
  // Convert type from CV_16UC3 to CV_8UC3
  frame.convertTo(frame, CV_8UC3);

  // Avoid odd width and height
  if (frame.size().height % 4 != 0 || frame.size().width % 4 != 0) {
    auto sz = frame.size();
    cv::Rect even_roi(0, 0, sz.width - sz.width % 4, sz.height - sz.height % 4);
    frame = frame(even_roi);
  }
}
