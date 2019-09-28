/**
@file warpPerspective_demo.cpp
@brief a demo program shows how perspective transformation applied on an image
@based on a sample code
http://study.marearts.com/2015/03/image-warping-using-opencv.html
@modified by Suleyman TURKMEN
*/

#include <iostream>
#include "opencv2/calib3d.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"

using namespace std;
using namespace cv;

static void help(char** argv) {
  // print a welcome message, and the OpenCV version
  cout << "\nThis is a demo program shows how perspective transformation "
          "applied on an image, \n"
          "Using OpenCV version "
       << CV_VERSION << endl;

  cout << "\nUsage:\n"
       << argv[0] << " [image_name -- Default right.jpg]\n"
       << endl;

  cout << "\nHot keys: \n"
          "\tESC, q - quit the program\n"
          "\tr - change order of points to rotate transformation\n"
          "\tc - delete selected points\n"
          "\ti - change order of points to inverse transformation \n"
          "\nUse your mouse to select a point and move it to see "
          "transformation changes"
       << endl;
}

static void onMouse(int event, int x, int y, int, void*);
Mat warping(Mat image, Size warped_image_size, vector<Point2f> srcPoints,
            vector<Point2f> dstPoints);

String windowTitle = "Perspective Transformation Demo";
String labels[4] = {"TL", "TR", "BR", "BL"};
vector<Point2f> roi_corners;
vector<Point2f> dst_corners(4);
int roiIndex = 0;
bool dragging;
int selected_corner_index = 0;
bool validation_needed = true;

int main(int argc, char** argv) {
  help(argv);
  CommandLineParser parser(argc, argv, "{@input| right.jpg |}");

  string filename = samples::findFile(parser.get<string>("@input"));
  Mat original_image = imread(filename);
  Mat image;

  roi_corners.emplace_back(586, 283);
  roi_corners.emplace_back(693, 283);
  roi_corners.emplace_back(699, 359);
  roi_corners.emplace_back(581, 359);

  namedWindow(windowTitle, WINDOW_NORMAL);
  namedWindow("Warped Image", WINDOW_NORMAL);
  moveWindow("Warped Image", 20, 20);
  moveWindow(windowTitle, 330, 20);

  setMouseCallback(windowTitle, onMouse, 0);

  bool endProgram = false;
  while (!endProgram) {
    if (validation_needed & (roi_corners.size() < 4)) {
      validation_needed = false;
      image = original_image.clone();

      for (size_t i = 0; i < roi_corners.size(); ++i) {
        circle(image, roi_corners[i], 5, Scalar(0, 255, 0), 3);

        if (i > 0) {
          line(image, roi_corners[i - 1], roi_corners[(i)], Scalar(0, 0, 255),
               2);
          circle(image, roi_corners[i], 5, Scalar(0, 255, 0), 3);
          putText(image, labels[i].c_str(), roi_corners[i],
                  FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 0, 0), 2);
        }
      }
      imshow(windowTitle, image);
    }

    Mat warped_image;
    if (validation_needed & (roi_corners.size() == 4)) {
      image = original_image.clone();
      for (int i = 0; i < 4; ++i) {
        line(image, roi_corners[i], roi_corners[(i + 1) % 4], Scalar(0, 0, 255),
             2);
        circle(image, roi_corners[i], 5, Scalar(0, 255, 0), 3);
        putText(image, labels[i].c_str(), roi_corners[i], FONT_HERSHEY_SIMPLEX,
                0.8, Scalar(255, 0, 0), 2);
      }

      imshow(windowTitle, image);

      dst_corners[0].x = 0;
      dst_corners[0].y = 0;
      dst_corners[1].x = (float)std::max(norm(roi_corners[0] - roi_corners[1]),
                                         norm(roi_corners[2] - roi_corners[3]));
      dst_corners[1].y = 0;
      dst_corners[2].x = (float)std::max(norm(roi_corners[0] - roi_corners[1]),
                                         norm(roi_corners[2] - roi_corners[3]));
      dst_corners[2].y = (float)std::max(norm(roi_corners[1] - roi_corners[2]),
                                         norm(roi_corners[3] - roi_corners[0]));
      dst_corners[3].x = 0;
      dst_corners[3].y = (float)std::max(norm(roi_corners[1] - roi_corners[2]),
                                         norm(roi_corners[3] - roi_corners[0]));
      //      // get homography
      //      Mat H = findHomography(roi_corners, dst_corners);
      //      Size warped_image_size =
      //          Size(cvRound(dst_corners[2].x), cvRound(dst_corners[2].y));
      //      // do perspective transformation
      //      warpPerspective(original_image, warped_image, H,
      //      warped_image_size);

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
      float x = ceil(abs(min(min(a.x, b.x), min(c.x, d.x))));
      float y = ceil(abs(min(min(a.y, b.y), min(c.y, d.y))));

      // and also < (width, height)
      float width = ceil(abs(max(max(a.x, b.x), max(c.x, d.x)))) + x;
      float height = ceil(abs(max(max(a.y, b.y), max(c.y, d.y)))) + y;

      // adjust target points accordingly
      for (int i = 0; i < 4; i++) {
        dst_corners[i] += cv::Point2f(x, y);
      }

      // recalculate transformation
      M = cv::getPerspectiveTransform(dst_corners, roi_corners);

      // get result
      cv::warpPerspective(original_image, warped_image, M,
                          cv::Size(width, height), cv::WARP_INVERSE_MAP);

      imshow("Warped Image", warped_image);
    }

    char c = (char)waitKey(10);

    if ((c == 'q') | (c == 'Q') | (c == 27)) {
      endProgram = true;
    }

    if ((c == 'c') | (c == 'C')) {
      roi_corners.clear();
    }

    if ((c == 'r') | (c == 'R')) {
      roi_corners.push_back(roi_corners[0]);
      roi_corners.erase(roi_corners.begin());
    }

    if ((c == 'i') | (c == 'I')) {
      swap(roi_corners[0], roi_corners[1]);
      swap(roi_corners[2], roi_corners[3]);
    }

    if ((c == 's') | (c == 'S')) {
      // remove the black background
      cv::Mat warped_image_bgra;
#if (CV_VERSION_MAJOR >= 4)
      cv::cvtColor(warped_image, warped_image_bgra, COLOR_BGR2BGRA);
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
      imwrite("result.jpg", warped_image);
      imwrite("result.png", warped_image_bgra);
    }
  }
  return 0;
}

static void onMouse(int event, int x, int y, int, void*) {
  // Action when left button is pressed
  if (roi_corners.size() == 4) {
    for (int i = 0; i < 4; ++i) {
      if ((event == EVENT_LBUTTONDOWN) & ((abs(roi_corners[i].x - x) < 10)) &
          (abs(roi_corners[i].y - y) < 10)) {
        selected_corner_index = i;
        dragging = true;
      }
    }
  } else if (event == EVENT_LBUTTONDOWN) {
    roi_corners.push_back(Point2f((float)x, (float)y));
    validation_needed = true;
  }

  // Action when left button is released
  if (event == EVENT_LBUTTONUP) {
    dragging = false;
  }

  // Action when left button is pressed and mouse has moved over the window
  if ((event == EVENT_MOUSEMOVE) && dragging) {
    roi_corners[selected_corner_index].x = (float)x;
    roi_corners[selected_corner_index].y = (float)y;
    validation_needed = true;
  }
}
