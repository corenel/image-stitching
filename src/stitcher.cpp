#include "stitcher.hpp"

Stitcher::Stitcher(const int& num_images_given) : num_images(num_images_given) {
  if (features_type == "orb") match_conf = 0.3f;
  if (preview) {
    compose_megapix = 0.6;
  }
  corners.resize(num_images);
  masks_warped.resize(num_images);
  images_warped.resize(num_images);
  sizes.resize(num_images);
  masks.resize(num_images);

  img_warped.resize(num_images);
  img_warped_s.resize(num_images);
  dilated_mask.resize(num_images);
  seam_mask.resize(num_images);
  mask.resize(num_images);
  mask_warped.resize(num_images);
}

int Stitcher::calibrate(const std::vector<std::string>& img_names, cv::Mat result, cv::Mat result_mask) {
#if ENABLE_LOG
  int64 app_start_time = cv::getTickCount();
#endif

  // Check if have enough images
  if (num_images != static_cast<int>(img_names.size())) {
    LOGLN("Invalid number of images");
    return -1;
  } else if (num_images < 2) {
    LOGLN("Need more images");
    return -1;
  }

  // Loading images
#if ENABLE_LOG
  LOGLN("Loading images...");
  int64 t = cv::getTickCount();
#endif
  std::vector<cv::Mat> full_img(num_images), img(num_images);
  std::vector<cv::Size> full_img_sizes(num_images);
  for (int i = 0; i < num_images; ++i) {
    full_img[i] = cv::imread(cv::samples::findFile(img_names[i]));
    if (full_img[i].empty()) {
      LOGLN("Can't open image " << img_names[i]);
      return -1;
    }
    full_img_sizes[i] = full_img[i].size();
  }
#if ENABLE_LOG
  LOGLN("Loading image, time: "
        << ((cv::getTickCount() - t) / cv::getTickFrequency()) << " sec");
#endif

  // finding features
#if ENABLE_LOG
  LOGLN("finding features...");
  t = cv::getTickCount();
#endif

#if (CV_VERSION_MAJOR >= 4)
  cv::Ptr<cv::Feature2D> finder;
  if (features_type == "orb") {
    finder = cv::ORB::create();
  } else if (features_type == "akaze") {
    finder = cv::AKAZE::create();
  }
#ifdef HAVE_OPENCV_XFEATURES2D
  else if (features_type == "surf") {
    finder = cv::xfeatures2d::SURF::create();
  } else if (features_type == "sift") {
    finder = cv::xfeatures2d::SIFT::create();
  }
#endif
  else {
    LOGLN("Unknown 2D features type: '" << features_type);
    return -1;
  }
#else
  cv::Ptr<cv::detail::FeaturesFinder> finder;
  if (features_type == "surf") {
#ifdef HAVE_OPENCV_XFEATURES2D
    if (try_cuda && cv::cuda::getCudaEnabledDeviceCount() > 0)
      finder = cv::makePtr<cv::detail::SurfFeaturesFinderGpu>();
    else
#endif
      finder = cv::makePtr<cv::detail::SurfFeaturesFinder>();
  } else if (features_type == "orb") {
    finder = cv::makePtr<cv::detail::OrbFeaturesFinder>();
  } else if (features_type == "sift") {
    finder = cv::makePtr<cv::detail::SiftFeaturesFinder>();
  } else {
    LOGLN("Unknown 2D features type: '" << features_type);
    return -1;
  }
#endif

  std::vector<cv::detail::ImageFeatures> features(num_images);
  std::vector<cv::Mat> images(num_images);
  double seam_work_aspect = 1;

#pragma omp parallel for
  for (int i = 0; i < num_images; ++i) {
    if (work_megapix < 0) {
      img[i] = full_img[i];
      work_scale = 1;
      is_work_scale_set = true;
    } else {
      if (!is_work_scale_set) {
        work_scale = std::min(
            1.0, std::sqrt(work_megapix * 1e6 / full_img[i].size().area()));
        is_work_scale_set = true;
      }
      cv::resize(full_img[i], img[i], cv::Size(), work_scale, work_scale,
                 cv::INTER_LINEAR_EXACT);
    }
    if (!is_seam_scale_set) {
      seam_scale = std::min(
          1.0, std::sqrt(seam_megapix * 1e6 / full_img[i].size().area()));
      seam_work_aspect = seam_scale / work_scale;
      is_seam_scale_set = true;
    }
#if (CV_VERSION_MAJOR >= 4)
    computeImageFeatures(finder, img[i], features[i]);
#else
    (*finder)(img[i], features[i]);
#endif
    features[i].img_idx = i;
#if ENABLE_LOG
    LOGLN("Features in image #" << i + 1 << ": "
                                << features[i].keypoints.size());
#endif

    resize(full_img[i], img[i], cv::Size(), seam_scale, seam_scale,
           cv::INTER_LINEAR_EXACT);
    images[i] = img[i].clone();
    // full_img.release();
    // img.release();
  }
#if (CV_VERSION_MAJOR < 4)
  finder->collectGarbage();
#endif
#if ENABLE_LOG
  LOGLN("Finding features, time: "
        << ((cv::getTickCount() - t) / cv::getTickFrequency()) << " sec");
#endif

// Pairwise matching
#if ENABLE_LOG
  LOGLN("Pairwise matching");
  t = cv::getTickCount();
#endif

  std::vector<cv::detail::MatchesInfo> pairwise_matches;
  cv::Ptr<cv::detail::FeaturesMatcher> matcher;
  if (matcher_type == "affine")
    matcher = cv::makePtr<cv::detail::AffineBestOf2NearestMatcher>(
        false, try_cuda, match_conf);
  else if (range_width == -1)
    matcher =
        cv::makePtr<cv::detail::BestOf2NearestMatcher>(try_cuda, match_conf);
  else
    matcher = cv::makePtr<cv::detail::BestOf2NearestRangeMatcher>(
        range_width, try_cuda, match_conf);

  cv::UMat matchMask(features.size(), features.size(), CV_8U, cv::Scalar(0));

#pragma omp parallel for
  for (int i = 0; i < num_images - 1; ++i) {
    matchMask.getMat(cv::ACCESS_READ).at<char>(i, i + 1) = 1;
  }

  (*matcher)(features, pairwise_matches, matchMask);
  matcher->collectGarbage();

#if ENABLE_LOG
  LOGLN("Pairwise matching, time: "
        << ((cv::getTickCount() - t) / cv::getTickFrequency()) << " sec");
#endif

  // Estimating
#if ENABLE_LOG
  LOGLN("Estimating");
  t = cv::getTickCount();
#endif
  cv::Ptr<cv::detail::Estimator> estimator;
  if (estimator_type == "affine")
    estimator = cv::makePtr<cv::detail::AffineBasedEstimator>();
  else
    estimator = cv::makePtr<cv::detail::HomographyBasedEstimator>();

  std::vector<cv::detail::CameraParams> cameras;
  if (!(*estimator)(features, pairwise_matches, cameras)) {
    LOGLN("Homography estimation failed.");
    return -1;
  }
  for (auto& camera : cameras) {
    cv::Mat R;
    camera.R.convertTo(R, CV_32F);
    camera.R = R;
  }

  cv::Ptr<cv::detail::BundleAdjusterBase> adjuster;
  if (ba_cost_func == "reproj")
    adjuster = cv::makePtr<cv::detail::BundleAdjusterReproj>();
  else if (ba_cost_func == "ray")
    adjuster = cv::makePtr<cv::detail::BundleAdjusterRay>();
  else if (ba_cost_func == "affine")
    adjuster = cv::makePtr<cv::detail::BundleAdjusterAffinePartial>();
  else if (ba_cost_func == "no")
    adjuster = cv::makePtr<cv::detail::NoBundleAdjuster>();
  else {
    LOGLN("Unknown bundle adjustment cost function: '" << ba_cost_func);
    return -1;
  }
  adjuster->setConfThresh(conf_thresh);
  cv::Mat_<uchar> refine_mask = cv::Mat::zeros(3, 3, CV_8U);
  if (ba_refine_mask[0] == 'x') refine_mask(0, 0) = 1;
  if (ba_refine_mask[1] == 'x') refine_mask(0, 1) = 1;
  if (ba_refine_mask[2] == 'x') refine_mask(0, 2) = 1;
  if (ba_refine_mask[3] == 'x') refine_mask(1, 1) = 1;
  if (ba_refine_mask[4] == 'x') refine_mask(1, 2) = 1;
  adjuster->setRefinementMask(refine_mask);
  if (!(*adjuster)(features, pairwise_matches, cameras)) {
    LOGLN("Camera parameters adjusting failed.");
    return -1;
  }
  // Find median focal length
  std::vector<double> focals;
  focals.reserve(cameras.size());
  for (size_t i = 0; i < cameras.size(); ++i) {
    focals[i] = cameras[i].focal;
  }
  std::sort(focals.begin(), focals.end());
  float warped_image_scale;
  if (focals.size() % 2 == 1)
    warped_image_scale = static_cast<float>(focals[focals.size() / 2]);
  else
    warped_image_scale = static_cast<float>(focals[focals.size() / 2 - 1] +
                                            focals[focals.size() / 2]) *
                         0.5f;
  if (do_wave_correct) {
    std::vector<cv::Mat> rmats;
    rmats.reserve(cameras.size());
    for (const auto& camera : cameras) {
      rmats.push_back(camera.R.clone());
    }
    cv::detail::waveCorrect(rmats, wave_correct);
    for (size_t i = 0; i < cameras.size(); ++i) {
      cameras[i].R = rmats[i];
    }
  }
#if ENABLE_LOG
  LOGLN("Estimating, time: "
        << ((cv::getTickCount() - t) / cv::getTickFrequency()) << " sec");
#endif

  // Warping images
#if ENABLE_LOG
  LOGLN("Warping images (auxiliary)... ");
  t = cv::getTickCount();
#endif

  // Prepare images masks
#pragma omp parallel for
  for (int i = 0; i < num_images; ++i) {
    masks[i].create(images[i].size(), CV_8U);
    masks[i].setTo(cv::Scalar::all(255));
  }

  // Warp images and their masks
  cv::Ptr<cv::WarperCreator> warper_creator;
#ifdef HAVE_OPENCV_CUDAWARPING
  if (try_cuda && cv::cuda::getCudaEnabledDeviceCount() > 0) {
    if (warp_type == "plane")
      warper_creator = cv::makePtr<cv::PlaneWarperGpu>();
    else if (warp_type == "cylindrical")
      warper_creator = cv::makePtr<cv::CylindricalWarperGpu>();
    else if (warp_type == "spherical")
      warper_creator = cv::makePtr<cv::SphericalWarperGpu>();
  } else
#endif
  {
    if (warp_type == "plane")
      warper_creator = cv::makePtr<cv::PlaneWarper>();
    else if (warp_type == "affine")
      warper_creator = cv::makePtr<cv::AffineWarper>();
    else if (warp_type == "cylindrical")
      warper_creator = cv::makePtr<cv::CylindricalWarper>();
    else if (warp_type == "spherical")
      warper_creator = cv::makePtr<cv::SphericalWarper>();
    else if (warp_type == "fisheye")
      warper_creator = cv::makePtr<cv::FisheyeWarper>();
    else if (warp_type == "stereographic")
      warper_creator = cv::makePtr<cv::StereographicWarper>();
    else if (warp_type == "compressedPlaneA2B1")
      warper_creator = cv::makePtr<cv::CompressedRectilinearWarper>(2.0f, 1.0f);
    else if (warp_type == "compressedPlaneA1.5B1")
      warper_creator = cv::makePtr<cv::CompressedRectilinearWarper>(1.5f, 1.0f);
    else if (warp_type == "compressedPlanePortraitA2B1")
      warper_creator =
          cv::makePtr<cv::CompressedRectilinearPortraitWarper>(2.0f, 1.0f);
    else if (warp_type == "compressedPlanePortraitA1.5B1")
      warper_creator =
          cv::makePtr<cv::CompressedRectilinearPortraitWarper>(1.5f, 1.0f);
    else if (warp_type == "paniniA2B1")
      warper_creator = cv::makePtr<cv::PaniniWarper>(2.0f, 1.0f);
    else if (warp_type == "paniniA1.5B1")
      warper_creator = cv::makePtr<cv::PaniniWarper>(1.5f, 1.0f);
    else if (warp_type == "paniniPortraitA2B1")
      warper_creator = cv::makePtr<cv::PaniniPortraitWarper>(2.0f, 1.0f);
    else if (warp_type == "paniniPortraitA1.5B1")
      warper_creator = cv::makePtr<cv::PaniniPortraitWarper>(1.5f, 1.0f);
    else if (warp_type == "mercator")
      warper_creator = cv::makePtr<cv::MercatorWarper>();
    else if (warp_type == "transverseMercator")
      warper_creator = cv::makePtr<cv::TransverseMercatorWarper>();
  }

  if (!warper_creator) {
    LOGLN("Can't create the following warper '" << warp_type);
    return -1;
  }

  std::vector<cv::Ptr<cv::detail::RotationWarper>> warpers(num_images);
#pragma omp parallel for
  for (int i = 0; i < num_images; ++i) {
    warpers[i] = warper_creator->create(
        static_cast<float>(warped_image_scale * seam_work_aspect));
  }

  std::vector<cv::UMat> images_warped_f(num_images);
#pragma omp parallel for
  for (int i = 0; i < num_images; ++i) {
    cv::Ptr<cv::detail::RotationWarper> warper = warper_creator->create(
        static_cast<float>(warped_image_scale * seam_work_aspect));
    cv::Mat_<float> K;
    cameras[i].K().convertTo(K, CV_32F);
    auto swa = (float)seam_work_aspect;
    K(0, 0) *= swa;
    K(0, 2) *= swa;
    K(1, 1) *= swa;
    K(1, 2) *= swa;

    corners[i] = warpers[i]->warp(images[i], K, cameras[i].R, cv::INTER_LINEAR,
                                  cv::BORDER_REFLECT, images_warped[i]);
    sizes[i] = images_warped[i].size();

    warpers[i]->warp(masks[i], K, cameras[i].R, cv::INTER_NEAREST,
                     cv::BORDER_CONSTANT, masks_warped[i]);
    images_warped[i].convertTo(images_warped_f[i], CV_32F);
  }

#if ENABLE_LOG
  LOGLN("Warping images, time: "
        << ((cv::getTickCount() - t) / cv::getTickFrequency()) << " sec");
#endif

  // Compensating exposure
#if ENABLE_LOG
  LOGLN("Compensating exposure...");
  t = cv::getTickCount();
#endif

  cv::Ptr<cv::detail::ExposureCompensator> compensator =
      cv::detail::ExposureCompensator::createDefault(expos_comp_type);

#if (CV_VERSION_MAJOR >= 4)
  if (dynamic_cast<cv::detail::GainCompensator*>(compensator.get())) {
    auto* gcompensator =
        dynamic_cast<cv::detail::GainCompensator*>(compensator.get());
    gcompensator->setNrFeeds(expos_comp_nr_feeds);
  }

  if (dynamic_cast<cv::detail::ChannelsCompensator*>(compensator.get())) {
    auto* ccompensator =
        dynamic_cast<cv::detail::ChannelsCompensator*>(compensator.get());
    ccompensator->setNrFeeds(expos_comp_nr_feeds);
  }

  if (dynamic_cast<cv::detail::BlocksCompensator*>(compensator.get())) {
    auto* bcompensator =
        dynamic_cast<cv::detail::BlocksCompensator*>(compensator.get());
    bcompensator->setNrFeeds(expos_comp_nr_feeds);
    bcompensator->setNrGainsFilteringIterations(expos_comp_nr_filtering);
    bcompensator->setBlockSize(expos_comp_block_size, expos_comp_block_size);
  }
#endif

  compensator->feed(corners, images_warped, masks_warped);

#if ENABLE_LOG
  LOGLN("Compensating exposure, time: "
        << ((cv::getTickCount() - t) / cv::getTickFrequency()) << " sec");
#endif

  // Finding seams
#if ENABLE_LOG
  LOGLN("Finding seams...");
  t = cv::getTickCount();
#endif

  cv::Ptr<cv::detail::SeamFinder> seam_finder;
  if (seam_find_type == "no")
    seam_finder = cv::makePtr<cv::detail::NoSeamFinder>();
  else if (seam_find_type == "voronoi")
    seam_finder = cv::makePtr<cv::detail::VoronoiSeamFinder>();
  else if (seam_find_type == "gc_color") {
    //#ifdef HAVE_OPENCV_CUDALEGACY
    //    if (try_cuda && cuda::getCudaEnabledDeviceCount() > 0)
    //      seam_finder = makePtr<detail::GraphCutSeamFinderGpu>(
    //          GraphCutSeamFinderBase::COST_COLOR);
    //    else
    //#endif
    seam_finder = cv::makePtr<cv::detail::GraphCutSeamFinder>(
        cv::detail::GraphCutSeamFinderBase::COST_COLOR);
  } else if (seam_find_type == "gc_colorgrad") {
    //#ifdef HAVE_OPENCV_CUDALEGACY
    //    if (try_cuda && cuda::getCudaEnabledDeviceCount() > 0)
    //      seam_finder = makePtr<detail::GraphCutSeamFinderGpu>(
    //          GraphCutSeamFinderBase::COST_COLOR_GRAD);
    //    else
    //#endif
    seam_finder = cv::makePtr<cv::detail::GraphCutSeamFinder>(
        cv::detail::GraphCutSeamFinderBase::COST_COLOR_GRAD);
  } else if (seam_find_type == "dp_color")
    seam_finder =
        cv::makePtr<cv::detail::DpSeamFinder>(cv::detail::DpSeamFinder::COLOR);
  else if (seam_find_type == "dp_colorgrad")
    seam_finder = cv::makePtr<cv::detail::DpSeamFinder>(
        cv::detail::DpSeamFinder::COLOR_GRAD);
  if (!seam_finder) {
    LOGLN("Can't create the following seam finder '" << seam_find_type);
    return -1;
  }

  seam_finder->find(images_warped_f, corners, masks_warped);

  // FIXME Release unused memory
  images.clear();
  images_warped.clear();
  images_warped_f.clear();
  masks.clear();

#if ENABLE_LOG
  LOGLN("Finding seams, time: "
        << ((cv::getTickCount() - t) / cv::getTickFrequency()) << " sec");
#endif

  // Compositing
  LOGLN("Compositing...");
#if ENABLE_LOG
  t = cv::getTickCount();
#endif

  double compose_work_aspect = 1;

  if (!is_compose_scale_set) {
    if (compose_megapix > 0)
      compose_scale = std::min(
          1.0, std::sqrt(compose_megapix * 1e6 / full_img[0].size().area()));
    is_compose_scale_set = true;

    // Compute relative scales
    compose_work_aspect = compose_scale / work_scale;

    // Update warped image scale
    warped_image_scale *= static_cast<float>(compose_work_aspect);

#pragma omp parallel for
    for (int img_idx = 0; img_idx < num_images; ++img_idx) {
      warpers[img_idx] = warper_creator->create(warped_image_scale);
    }

    // Update corners and sizes
#pragma omp parallel for
    for (int i = 0; i < num_images; ++i) {
      // Update intrinsics
      cameras[i].focal *= compose_work_aspect;
      cameras[i].ppx *= compose_work_aspect;
      cameras[i].ppy *= compose_work_aspect;

      // Update corner and size
      cv::Size sz = full_img_sizes[i];
      if (std::abs(compose_scale - 1) > 1e-1) {
        sz.width = cvRound(full_img_sizes[i].width * compose_scale);
        sz.height = cvRound(full_img_sizes[i].height * compose_scale);
      }

      cv::Mat K;
      cameras[i].K().convertTo(K, CV_32F);
      cv::Rect roi = warpers[i]->warpRoi(sz, K, cameras[i].R);
      corners[i] = roi.tl();
      sizes[i] = roi.size();
    }
  }

  if (!blender) {
    blender = cv::detail::Blender::createDefault(blend_type, try_cuda);
    cv::Size dst_sz = cv::detail::resultRoi(corners, sizes).size();
    float blend_width =
        std::sqrt(static_cast<float>(dst_sz.area())) * blend_strength / 100.f;
    if (blend_width < 1.f)
      blender =
          cv::detail::Blender::createDefault(cv::detail::Blender::NO, try_cuda);
    else if (blend_type == cv::detail::Blender::MULTI_BAND) {
      auto* mb = dynamic_cast<cv::detail::MultiBandBlender*>(blender.get());
      mb->setNumBands(
          static_cast<int>(std::ceil(log(blend_width) / log(2.)) - 1.));
      LOGLN("Multi-band blender, number of bands: " << mb->numBands());
    } else if (blend_type == cv::detail::Blender::FEATHER) {
      auto* fb = dynamic_cast<cv::detail::FeatherBlender*>(blender.get());
      fb->setSharpness(1.f / blend_width);
      LOGLN("Feather blender, sharpness: " << fb->sharpness());
    }
    blender->prepare(corners, sizes);
  }

#pragma omp parallel for
  for (int img_idx = 0; img_idx < num_images; ++img_idx) {
    // Read image and resize it if necessary
    if (std::abs(compose_scale - 1) > 1e-1)
      cv::resize(full_img[img_idx], img[img_idx], cv::Size(), compose_scale,
                 compose_scale, cv::INTER_LINEAR_EXACT);
    else
      img[img_idx] = full_img[img_idx];
    cv::Size img_size = img[img_idx].size();

    cv::Mat K;
    cameras[img_idx].K().convertTo(K, CV_32F);

    // Warp the current image
    warpers[img_idx]->warp(img[img_idx], K, cameras[img_idx].R,
                           cv::INTER_LINEAR, cv::BORDER_REFLECT,
                           img_warped[img_idx]);

    // Warp the current image mask
    mask[img_idx].create(img_size, CV_8U);
    mask[img_idx].setTo(cv::Scalar::all(255));
    warpers[img_idx]->warp(mask[img_idx], K, cameras[img_idx].R,
                           cv::INTER_NEAREST, cv::BORDER_CONSTANT,
                           mask_warped[img_idx]);

    // Compensate exposure
    compensator->apply(img_idx, corners[img_idx], img_warped[img_idx],
                       mask_warped[img_idx]);

    img_warped[img_idx].convertTo(img_warped_s[img_idx], CV_16S);

    cv::dilate(masks_warped[img_idx], dilated_mask[img_idx], cv::Mat());
    cv::resize(dilated_mask[img_idx], seam_mask[img_idx],
               mask_warped[img_idx].size(), 0, 0, cv::INTER_LINEAR_EXACT);
    mask_warped[img_idx] = seam_mask[img_idx] & mask_warped[img_idx];

    // Blend the current image
    blender->feed(img_warped_s[img_idx], mask_warped[img_idx],
                  corners[img_idx]);
  }

  blender->blend(result, result_mask);
#if ENABLE_LOG
  LOGLN("Compositing, time: " << ((cv::getTickCount() - t) / cv::getTickFrequency())
                              << " sec");
  LOGLN("Finished, total time: "
        << ((cv::getTickCount() - app_start_time) / cv::getTickFrequency()) << " sec");
#endif
  return 0;
}
