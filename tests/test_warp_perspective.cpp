#include <iostream>
#include "warper.hpp"

static void help(char** argv) {
}

int main(int argc, char** argv) {
  cv::CommandLineParser parser(argc, argv, "{@input| right.jpg |}");

  std::string filename =
      cv::samples::findFile(parser.get<std::string>("@input"));
  Warper warper(filename);

  warper.help(argv);
  warper.calibrate();

  return 0;
}
