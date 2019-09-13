#include <tclap/CmdLine.h>
#include "stream_provider.hpp"
#include "stream_writer.hpp"

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
  cv::Mat frame;
  cv::Size sz(2856, 394);
  StreamWriter writer("rtmp://192.168.6.3/live/test", "gst-nvidia", "h264",
                      "720p", 25, sz);

  //  StreamProvider provider(path_to_input_file, "gst-nvidia", "h264", "720p",
  //  25); while (provider.isOpened()) {
  //    fake_reader->read(frame);
  //    if (writer.isOpened()) {
  //      writer.write(frame);
  //    }
  //    cv::imwrite("recv_0.jpg", frame);
  //  }

  auto fake_reader = createSynthSource(sz, 25);
  while (fake_reader->isOpened()) {
    fake_reader->read(frame);
    if (writer.isOpened()) {
      writer.write(frame);
    }
  }
  return 0;
}
