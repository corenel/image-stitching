#include "warper.hpp"

Warper::Warper() {}

Warper::~Warper() { cv::setMouseCallback(window_title_, nullptr, 0); }

WarpTransform Warper::calibrate_single(const WarpConfig& config) {
  return calibrate_single(config.url, config.left, config.top, config.right,
                          config.bottom);
}

WarpTransform Warper::calibrate_single(const cv::Mat& frame,
                                       const WarpConfig& config) {
  return calibrate_single(frame, config.left, config.top, config.right,
                          config.bottom);
}

WarpTransform Warper::calibrate_single(const std::string& filename,
                                       const float& left, const float& top,
                                       const float& right,
                                       const float& bottom) {
  original_image_ = cv::imread(filename);
  return calibrate_single(left, top, right, bottom);
}

WarpTransform Warper::calibrate_single(const cv::Mat& frame, const float& left,
                                       const float& top, const float& right,
                                       const float& bottom) {
  original_image_ = frame;
  return calibrate_single(left, top, right, bottom);
}

WarpTransform Warper::calibrate_single(const float& left, const float& top,
                                       const float& right,
                                       const float& bottom) {
  auto t = WarpTransform();
  roi_corners_.clear();

  cv::namedWindow(window_title_, cv::WINDOW_NORMAL);
  cv::namedWindow("Warped Image", cv::WINDOW_NORMAL);
  cv::moveWindow("Warped Image", 20, 20);
  cv::moveWindow(window_title_, 330, 20);

  cv::setMouseCallback(window_title_, onCalibrationMouse, this);

  bool endProgram = false;
  while (!endProgram) {
    if (validation_needed_ & (roi_corners_.size() < 4)) {
      validation_needed_ = false;
      image_ = original_image_.clone();

      for (size_t i = 0; i < roi_corners_.size(); ++i) {
        cv::circle(image_, roi_corners_[i], 5, cv::Scalar(0, 255, 0), 3);

        if (i > 0) {
          cv::line(image_, roi_corners_[i - 1], roi_corners_[(i)],
                   cv::Scalar(0, 0, 255), 2);
          cv::circle(image_, roi_corners_[i], 5, cv::Scalar(0, 255, 0), 3);
          cv::putText(image_, labels_[i].c_str(), roi_corners_[i],
                      cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(255, 255, 0),
                      2);
        }
      }
      cv::imshow(window_title_, image_);
    }

    cv::Mat warped_image;
    if (validation_needed_ & (roi_corners_.size() == 4)) {
      image_ = original_image_.clone();
      for (int i = 0; i < 4; ++i) {
        cv::line(image_, roi_corners_[i], roi_corners_[(i + 1) % 4],
                 cv::Scalar(0, 0, 255), 2);
        cv::circle(image_, roi_corners_[i], 5, cv::Scalar(0, 255, 0), 3);
        cv::putText(image_, labels_[i].c_str(), roi_corners_[i],
                    cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(255, 255, 0), 2);
      }

      cv::imshow(window_title_, image_);

      dst_corners_[0].x = 0;
      dst_corners_[0].y = 0;
      dst_corners_[1].x = side_length * (right - left);
      dst_corners_[1].y = 0;
      dst_corners_[2].x = side_length * (right - left);
      dst_corners_[2].y = side_length * (top - bottom);
      dst_corners_[3].x = 0;
      dst_corners_[3].y = side_length * (top - bottom);

      // calculate transformation
      t.M = cv::getPerspectiveTransform(dst_corners_, roi_corners_);

      // calculate warped position of all corners
      cv::Point3f a = t.M.inv() * cv::Point3f(0, 0, 1);
      a = a * (1.0 / a.z);

      cv::Point3f b = t.M.inv() * cv::Point3f(0, image_.rows, 1);
      b = b * (1.0 / b.z);

      cv::Point3f c = t.M.inv() * cv::Point3f(image_.cols, image_.rows, 1);
      c = c * (1.0 / c.z);

      cv::Point3f d = t.M.inv() * cv::Point3f(image_.cols, 0, 1);
      d = d * (1.0 / d.z);

      // to make sure all corners are in the image
      // every position must be > (0, 0)
      float x =
          std::ceil(std::abs(std::min(std::min(a.x, b.x), std::min(c.x, d.x))));
      float y =
          std::ceil(std::abs(std::min(std::min(a.y, b.y), std::min(c.y, d.y))));

      // and also < (width, height)
      t.width = std::ceil(std::abs(
                    std::max(std::max(a.x, b.x), std::max(c.x, d.x)))) +
                x;
      t.height = std::ceil(std::abs(
                     std::max(std::max(a.y, b.y), std::max(c.y, d.y)))) +
                 y;

      // adjust target points accordingly
      for (int i = 0; i < 4; i++) {
        dst_corners_[i] += cv::Point2f(x, y);
      }

      // recalculate transformation
      t.M = cv::getPerspectiveTransform(dst_corners_, roi_corners_);

      // get result
      cv::warpPerspective(original_image_, warped_image, t.M,
                          cv::Size(t.width, t.height), cv::WARP_INVERSE_MAP);

      cv::imshow("Warped Image", warped_image);
    }

    char c = (char)cv::waitKey(10);

    // quit
    if (((c == 'q') | (c == 'Q') | (c == 27)) & validation_needed_ &
        (roi_corners_.size() == 4)) {
      endProgram = true;
    }
    // clear
    if ((c == 'c') | (c == 'C')) {
      roi_corners_.clear();
    }
    // rotate
    if ((c == 'r') | (c == 'R')) {
      roi_corners_.push_back(roi_corners_[0]);
      roi_corners_.erase(roi_corners_.begin());
    }
    // inverse
    if ((c == 'i') | (c == 'I')) {
      swap(roi_corners_[0], roi_corners_[1]);
      swap(roi_corners_[2], roi_corners_[3]);
    }
    // save result
    if (((c == 's') | (c == 'S')) & validation_needed_ &
        (roi_corners_.size() == 4)) {
      // remove the black background
      cv::Mat warped_image_bgra;
#if (CV_VERSION_MAJOR >= 4)
      cv::cvtColor(warped_image, warped_image_bgra, cv::COLOR_BGR2BGRA);
#else
      cv::cvtColor(warped_image, warped_image_bgra, CV_BGR2BGRA);
#endif
      // find all black pixel and set alpha value to zero:
      for (int y = 0; y < warped_image_bgra.rows; ++y) {
        for (int x = 0; x < warped_image_bgra.cols; ++x) {
          auto& pixel = warped_image_bgra.at<cv::Vec4b>(y, x);
          // if pixel is black
          if (pixel[0] == 0 && pixel[1] == 0 && pixel[2] == 0) {
            // set alpha to zero:
            pixel[3] = 0;
          }
        }
      }
      cv::imwrite("screen.jpg", image_);
      cv::imwrite("result.jpg", warped_image);
      cv::imwrite("result.png", warped_image_bgra);
    }
  }

  if (roi_corners_.size() < 4) {
    std::cerr << "No enough points to warp image" << std::endl;
  }

  // get coordinate of the original point
  cv::Point3f tmp =
      t.M.inv() * cv::Point3f(roi_corners_[0].x, roi_corners_[0].y, 1);
  tmp = tmp * (1.0 / tmp.z);
  t.orig.x = left * side_length - tmp.x;
  t.orig.y = top * side_length + tmp.y;

  return t;
}

std::vector<WarpTransform> Warper::calibrate(
    const std::vector<WarpConfig>& configs) {
  std::vector<WarpTransform> ts;
  for (const auto& config : configs) {
    ts.push_back(calibrate_single(config));
  }
  return ts;
}

std::vector<WarpTransform> Warper::calibrate(
    const std::vector<cv::Mat>& frames,
    const std::vector<WarpConfig>& configs) {
  assert(frames.size() == configs.size());

  std::vector<WarpTransform> ts;
  for (size_t i = 0; i < configs.size(); ++i) {
    ts.push_back(calibrate_single(frames[i], configs[i]));
  }
  return ts;
}

cv::Mat Warper::merge(const std::vector<cv::Mat>& frames,
                      const std::vector<WarpTransform>& transforms) {
  // initialize output matrix
  float final_left{}, final_top{}, final_right{}, final_bottom{};
  for (const auto& t : transforms) {
    final_left = std::fmin(final_left, t.orig.x);
    final_top = std::fmin(final_top, t.orig.y);
    final_right = std::fmax(final_right, t.orig.x + t.width);
    final_bottom = std::fmax(final_bottom, t.orig.y + t.height);
  }
  cv::Mat output(int(final_bottom - final_top) + 1,
                 int(final_right - final_left) + 1, CV_8UC4);

  // merge images
  for (size_t i = 0; i < transforms.size(); ++i) {
    cv::Mat warped_image;
    cv::warpPerspective(frames[i], warped_image, transforms[i].M,
                        cv::Size(transforms[i].width, transforms[i].height),
                        cv::WARP_INVERSE_MAP);

    // remove the black background
    cv::Mat warped_image_bgra;
#if (CV_VERSION_MAJOR >= 4)
    cv::cvtColor(warped_image, warped_image_bgra, cv::COLOR_BGR2BGRA);
#else
    cv::cvtColor(warped_image, warped_image_bgra, CV_BGR2BGRA);
#endif
    // find all black pixel and set alpha value to zero:
    for (int y = 0; y < warped_image_bgra.rows; ++y) {
      for (int x = 0; x < warped_image_bgra.cols; ++x) {
        auto& pixel = warped_image_bgra.at<cv::Vec4b>(y, x);
        // if pixel is black
        if (pixel[0] == 0 && pixel[1] == 0 && pixel[2] == 0) {
          // set alpha to zero:
          pixel[3] = 0;
        }
      }
    }

    // add in BGRA color space
    auto roi = cv::Rect(transforms[i].orig.x + (-final_left + final_right) / 2,
                        -transforms[i].orig.y + (-final_top + final_bottom) / 2,
                        transforms[i].width, transforms[i].height);
    cv::add(warped_image_bgra, output(roi), output(roi));
  }

  // convert back to BGR color space
  cv::cvtColor(output, output, cv::COLOR_BGRA2BGR);

  return output;
}

void Warper::onCalibrationMouse(int event, int x, int y, int, void* param) {
  auto* pThis = (Warper*)param;
  // Action when left button is pressed
  if (pThis->roi_corners_.size() == 4) {
    for (int i = 0; i < 4; ++i) {
      if ((event == cv::EVENT_LBUTTONDOWN) &
          ((std::abs(pThis->roi_corners_[i].x - x) < 10)) &
          (std::abs(pThis->roi_corners_[i].y - y) < 10)) {
        pThis->selected_corner_index_ = i;
        pThis->is_dragging_ = true;
      }
    }
  } else if (event == cv::EVENT_LBUTTONDOWN) {
    auto pt = pThis->adjust_point(x, y);
    pThis->roi_corners_.push_back(pt);
    pThis->validation_needed_ = true;
  }

  // Action when left button is released
  if (event == cv::EVENT_LBUTTONUP) {
    pThis->is_dragging_ = false;
  }

  // Action when left button is pressed and mouse has moved over the window
  if ((event == cv::EVENT_MOUSEMOVE) && pThis->is_dragging_) {
    auto pt = pThis->adjust_point(x, y);
    pThis->roi_corners_[pThis->selected_corner_index_].x = pt.x;
    pThis->roi_corners_[pThis->selected_corner_index_].y = pt.y;
    pThis->validation_needed_ = true;
  }
}

void Warper::help(char** argv) {
  // print a welcome message, and the OpenCV version
  std::cout << "\nThis is a demo program shows how perspective transformation "
               "applied on images and how to merge them into single image, \n"
               "Using OpenCV version "
            << CV_VERSION << std::endl;

  std::cout << "\nUsage:\n"
            << argv[0] << " [config_name -- Default input.csv]\n"
            << std::endl;

  std::cout << "\nHot keys: \n"
               "\tESC, q - quit the program\n"
               "\tr - change order of points to rotate transformation\n"
               "\tc - delete selected points\n"
               "\ti - change order of points to inverse transformation \n"
               "\ts - save current transformation \n"
               "\nUse your mouse to select a point and move it to see "
               "transformation changes"
            << std::endl;

  std::cout << "\nConfig example: \n"
               "\turl,left,top,right,bottom\n"
               "\t../assets/warship/sim_1/cam1.png,2.0,1.5,4.0,0.5\n"
               "\t../assets/warship/sim_1/cam2.png,0.0,6.0,3.0,4.0\n"
               "\t../assets/warship/sim_1/cam3.png,-4.5,-0.5,-2.5,-1.5\n"
               "\t../assets/warship/sim_1/cam4.png,-2.0,-4.0,2,0,-6.0\n"
            << std::endl;
}

cv::Point2f Warper::adjust_point(const int& x, const int& y) {
  if (use_corner_detection_) {
    cv::Rect roi(std::max(0, x - roi_range_), std::max(0, y - roi_range_),
                 roi_range_, roi_range_);
    cv::Mat roiImg = original_image_(roi);
    cv::Mat gray;
    cv::cvtColor(roiImg, gray, cv::COLOR_BGR2GRAY);
    // A place to put the returned corners.
    // each element is an (x, y) coord of the corner
    corners_.clear();
    corners_.reserve(max_corners_);
    // Use GFTT to find Harris corners
    cv::goodFeaturesToTrack(gray, corners_, max_corners_, quality_level_,
                            min_distance_, cv::Mat(), block_size_,
                            user_harris_detector_, k_);
    if (!corners_.empty()) {
      return cv::Point2f{(float)(std::max(0, x - roi_range_) + corners_[0].x),
                         (float)(std::max(0, y - roi_range_) + corners_[0].y)};
    }
  }
  return cv::Point2f{float(x), float(y)};
}
