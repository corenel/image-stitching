#include <tclap/CmdLine.h>
#include "stream_provider.hpp"

int main(int argc, char* argv[]) {
  // Parse arguments
  std::string path_to_input_file;
  try {
    TCLAP::CmdLine cmd("Command description message", ' ', "0.9");

    TCLAP::ValueArg<std::string> arg_input("i", "input",
                                           "Path to list of input video", true,
                                           "input.mp4", "string");

    cmd.add(arg_input);

    // Parse the argv array.
    cmd.parse(argc, argv);

    // Get the value parsed by each arg.
    path_to_input_file = arg_input.getValue();
  } catch (TCLAP::ArgException& e) {
    std::cerr << "Parse error: " << e.error() << " for arg " << e.argId()
              << std::endl;
  }

  // Recv
  StreamProvider provider(path_to_input_file);
  cv::Mat frame;
  if (provider.isOpened()) {
    provider.read(frame);
  }
  cv::imwrite("recv_0.jpg", frame);
  return 0;
}
