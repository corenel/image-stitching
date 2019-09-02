#pragma once

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include "opencv2/opencv.hpp"

/*
 * It will iterate through all the lines in file and
 * put them in given vector
 */
bool get_file_content(const std::string& filename,
                      std::vector<std::string>& results) {
  // Open the File
  std::ifstream in(filename.c_str());

  // Check if object is valid
  if (!in) {
    std::cerr << "Cannot open the File : " << filename << std::endl;
    return false;
  }

  std::string str;
  // Read the next line from File untill it reaches the end.
  while (std::getline(in, str)) {
    // Line contains string of length > 0 then save it in vector
    if (!str.empty()) {
      str.erase(std::remove(str.begin(), str.end(), '\n'), str.end());
      results.push_back(str);
    }
  }

  // Close The File
  in.close();
  return true;
}

bool load_images(const std::vector<std::string>& filenames,
                 std::vector<cv::Mat>& results) {
  for (size_t i = 0; i < filenames.size(); ++i) {
    results[i] = cv::imread(cv::samples::findFile(filenames[i]));
    if (results[i].empty()) {
      std::cerr << "Can't open image " << filenames[i] << std::endl;
      return false;
    }
  }
  return true;
}