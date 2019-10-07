#include <iostream>
#include "csv.h"
#include "warper.hpp"

int main(int argc, char** argv) {
  cv::CommandLineParser parser(argc, argv, "{@input| input.csv |}");

  Warper warper;
  warper.help(argv);

  std::string filename =
      cv::samples::findFile(parser.get<std::string>("@input"));
  io::CSVReader<5> in(filename);
  in.read_header(io::ignore_extra_column, "url", "left", "top", "right",
                 "bottom");
  std::string url;
  float left, top, right, bottom;
  while (in.read_row(url, left, top, right, bottom)) {
    // do stuff with the data
    warper.calibrate_single(url, left, top, right, bottom);
  }

  return 0;
}
