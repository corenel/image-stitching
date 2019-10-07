#include <iostream>
#include "csv.h"
#include "warper.hpp"

int main(int argc, char** argv) {
  // parse arguments
  cv::CommandLineParser parser(argc, argv, "{@input| input.csv |}");
  std::string filename =
      cv::samples::findFile(parser.get<std::string>("@input"));
  io::CSVReader<5> in(filename);
  in.read_header(io::ignore_extra_column, "url", "left", "top", "right",
                 "bottom");
  std::string url;
  float left, top, right, bottom;
  std::vector<WarpConfig> configs;
  while (in.read_row(url, left, top, right, bottom)) {
    configs.emplace_back(url, left, top, right, bottom);
  }

  // calibrate
  Warper warper;
  warper.help(argv);
  auto transforms = warper.calibrate(configs);

  return 0;
}
