#include "warper.hpp"

Warper::Warper(const std::string& filename) {
  original_image = cv::imread(filename);
}

Warper::~Warper() { cv::setMouseCallback(windowTitle, NULL, 0); }

void Warper::calibrate() {
  cv::namedWindow(windowTitle, cv::WINDOW_NORMAL);
  cv::namedWindow("Warped Image", cv::WINDOW_NORMAL);
  cv::moveWindow("Warped Image", 20, 20);
  cv::moveWindow(windowTitle, 330, 20);

  cv::setMouseCallback(windowTitle, onMouse, this);

  bool endProgram = false;
  while (!endProgram) {
    if (validation_needed & (roi_corners.size() < 4)) {
      validation_needed = false;
      image = original_image.clone();

      for (size_t i = 0; i < roi_corners.size(); ++i) {
        circle(image, roi_corners[i], 5, cv::Scalar(0, 255, 0), 3);

        if (i > 0) {
          line(image, roi_corners[i - 1], roi_corners[(i)],
               cv::Scalar(0, 0, 255), 2);
          circle(image, roi_corners[i], 5, cv::Scalar(0, 255, 0), 3);
          putText(image, labels[i].c_str(), roi_corners[i],
                  cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(255, 0, 0), 2);
        }
      }
      imshow(windowTitle, image);
    }

    cv::Mat warped_image;
    if (validation_needed & (roi_corners.size() == 4)) {
      image = original_image.clone();
      for (int i = 0; i < 4; ++i) {
        line(image, roi_corners[i], roi_corners[(i + 1) % 4],
             cv::Scalar(0, 0, 255), 2);
        circle(image, roi_corners[i], 5, cv::Scalar(0, 255, 0), 3);
        putText(image, labels[i].c_str(), roi_corners[i],
                cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(255, 0, 0), 2);
      }

      imshow(windowTitle, image);

      auto side_length = 200.0f;
      dst_corners[0].x = 0;
      dst_corners[0].y = 0;
      dst_corners[1].x = side_length;
      dst_corners[1].y = 0;
      dst_corners[2].x = side_length;
      dst_corners[2].y = 1.5 * side_length;
      dst_corners[3].x = 0;
      dst_corners[3].y = 1.5 * side_length;

      // calculate transformation
      cv::Matx33f M = cv::getPerspectiveTransform(dst_corners, roi_corners);

      // calculate warped position of all corners
      cv::Point3f a = M.inv() * cv::Point3f(0, 0, 1);
      a = a * (1.0 / a.z);

      cv::Point3f b = M.inv() * cv::Point3f(0, image.rows, 1);
      b = b * (1.0 / b.z);

      cv::Point3f c = M.inv() * cv::Point3f(image.cols, image.rows, 1);
      c = c * (1.0 / c.z);

      cv::Point3f d = M.inv() * cv::Point3f(image.cols, 0, 1);
      d = d * (1.0 / d.z);

      // to make sure all corners are in the image
      // every position must be > (0, 0)
      float x =
          std::ceil(std::abs(std::min(std::min(a.x, b.x), std::min(c.x, d.x))));
      float y =
          std::ceil(std::abs(std::min(std::min(a.y, b.y), std::min(c.y, d.y))));

      // and also < (width, height)
      float width = std::ceil(std::abs(
                        std::max(std::max(a.x, b.x), std::max(c.x, d.x)))) +
                    x;
      float height = std::ceil(std::abs(
                         std::max(std::max(a.y, b.y), std::max(c.y, d.y)))) +
                     y;

      // adjust target points accordingly
      for (int i = 0; i < 4; i++) {
        dst_corners[i] += cv::Point2f(x, y);
      }

      // recalculate transformation
      M = cv::getPerspectiveTransform(dst_corners, roi_corners);

      // get result
      cv::warpPerspective(original_image, warped_image, M,
                          cv::Size(width, height), cv::WARP_INVERSE_MAP);

      cv::imshow("Warped Image", warped_image);
    }

    char c = (char)cv::waitKey(10);

    // quit
    if ((c == 'q') | (c == 'Q') | (c == 27)) {
      endProgram = true;
    }
    // clear
    if ((c == 'c') | (c == 'C')) {
      roi_corners.clear();
    }
    // rotate
    if ((c == 'r') | (c == 'R')) {
      roi_corners.push_back(roi_corners[0]);
      roi_corners.erase(roi_corners.begin());
    }
    // inverse
    if ((c == 'i') | (c == 'I')) {
      swap(roi_corners[0], roi_corners[1]);
      swap(roi_corners[2], roi_corners[3]);
    }
    // save result
    if ((c == 's') | (c == 'S')) {
      // remove the black background
      cv::Mat warped_image_bgra;
#if (CV_VERSION_MAJOR >= 4)
      cv::cvtColor(warped_image, warped_image_bgra, cv::COLOR_BGR2BGRA);
#else
      cv::cvtColor(warped_image, warped_image_bgra, CV_BGR2BGRA);
#endif
      // find all white pixel and set alpha value to zero:
      for (int y = 0; y < warped_image_bgra.rows; ++y)
        for (int x = 0; x < warped_image_bgra.cols; ++x) {
          auto& pixel = warped_image_bgra.at<cv::Vec4b>(y, x);
          // if pixel is white
          if (pixel[0] == 0 && pixel[1] == 0 && pixel[2] == 0) {
            // set alpha to zero:
            pixel[3] = 0;
          }
        }
      cv::imwrite("result.jpg", warped_image);
      cv::imwrite("result.png", warped_image_bgra);
    }
  }
}

void Warper::onMouse(int event, int x, int y, int, void* param) {
  auto* pThis = (Warper*)param;
  // Action when left button is pressed
  if (pThis->roi_corners.size() == 4) {
    for (int i = 0; i < 4; ++i) {
      if ((event == cv::EVENT_LBUTTONDOWN) &
          ((std::abs(pThis->roi_corners[i].x - x) < 10)) &
          (std::abs(pThis->roi_corners[i].y - y) < 10)) {
        pThis->selected_corner_index = i;
        pThis->dragging = true;
      }
    }
  } else if (event == cv::EVENT_LBUTTONDOWN) {
    if (true) {
      int roi_range = 50;
      cv::Rect roi(std::max(0, x - roi_range), std::max(0, y - roi_range),
                   roi_range, roi_range);
      cv::Mat roiImg = pThis->original_image(roi);
      cv::Mat gray;
      cv::cvtColor(roiImg, gray, cv::COLOR_BGR2GRAY);
      // GFTT Settings
      int maxCorners = 1;
      double qualityLevel = 0.05;
      double minDistance = 2.0;
      int blockSize = 3;
      bool userHarrisDetector = true;
      double k = 0.04;
      // A place to put the returned corners.
      // each element is an (x, y) coord of the corner
      std::vector<cv::Point2f> corners;
      corners.reserve(maxCorners);
      // Use GFTT to find Harris corners
      goodFeaturesToTrack(gray, corners, maxCorners, qualityLevel, minDistance,
                          cv::Mat(), blockSize, userHarrisDetector, k);
      if (!corners.empty()) {
        pThis->roi_corners.emplace_back(
            (float)(std::max(0, x - roi_range) + corners[0].x),
            (float)(std::max(0, y - roi_range) + corners[0].y));

      } else {
        pThis->roi_corners.emplace_back((float)x, (float)y);
      }
    } else {
      pThis->roi_corners.emplace_back((float)x, (float)y);
    }
    pThis->validation_needed = true;
  }

  // Action when left button is released
  if (event == cv::EVENT_LBUTTONUP) {
    pThis->dragging = false;
  }

  // Action when left button is pressed and mouse has moved over the window
  if ((event == cv::EVENT_MOUSEMOVE) && pThis->dragging) {
    pThis->roi_corners[pThis->selected_corner_index].x = (float)x;
    pThis->roi_corners[pThis->selected_corner_index].y = (float)y;
    pThis->validation_needed = true;
  }
}

void Warper::help(char** argv) {
  // print a welcome message, and the OpenCV version
  std::cout << "\nThis is a demo program shows how perspective transformation "
               "applied on an image, \n"
               "Using OpenCV version "
            << CV_VERSION << std::endl;

  std::cout << "\nUsage:\n"
            << argv[0] << " [image_name -- Default right.jpg]\n"
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
}
