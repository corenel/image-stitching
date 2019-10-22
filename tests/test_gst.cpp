#include <tclap/CmdLine.h>
#include <string>
#include "stream_provider.hpp"
#include "stream_writer.hpp"

int main(int argc, char* argv[]) {
  // Parse arguments
  std::string path_to_input_file;
  std::string path_to_output_file;
  bool flag_no_display = false;
  bool flag_no_saving = false;
  bool flag_sync_frame = false;
  try {
    TCLAP::CmdLine cmd("Test for receiving and sending streams", ' ', "1.0");

    TCLAP::ValueArg<std::string> arg_input("i", "input", "Path to input video",
                                           true, "input.mp4", "string");
    TCLAP::ValueArg<std::string> arg_output(
        "o", "output", "Path to output video", false, "output.mp4", "string");
    TCLAP::SwitchArg arg_no_display("d", "no_display", "No display",
                                    flag_no_display);
    TCLAP::SwitchArg arg_no_saving("w", "no_saving", "No saving",
                                   flag_no_saving);
    TCLAP::SwitchArg arg_sync_frame("s", "sync_frame", "Sync frame",
                                    flag_sync_frame);

    cmd.add(arg_input);
    cmd.add(arg_output);
    cmd.add(arg_no_display);
    cmd.add(arg_no_saving);
    cmd.add(arg_sync_frame);

    // Parse the argv array.
    cmd.parse(argc, argv);

    // Get the value parsed by each arg.
    path_to_input_file = arg_input.getValue();
    path_to_output_file = arg_output.getValue();
    flag_no_display = arg_no_display.getValue();
    flag_no_saving = arg_no_saving.getValue();
    flag_sync_frame = arg_sync_frame.getValue();
  } catch (TCLAP::ArgException& e) {
    std::cerr << "Parse error: " << e.error() << " for arg " << e.argId()
              << std::endl;
  }

  // Recv
  int count = 0;
  cv::Mat frame;
  StreamProvider provider(path_to_input_file, "gst-nvidia", "h264", "720p", 25,
                          false, flag_sync_frame);
  StreamWriter writer(path_to_output_file, "gst-nvidia", "h264", "720p", 25);
  LOGLN("Start grabbing\n"
        << "Press any key to terminate");
  while (provider.isOpened()) {
    // wait for a new frame from camera and store it into 'frame'
    provider.read(frame);
    // check if we succeeded
    if (frame.empty()) {
      ERRLN("ERROR! blank frame grabbed");
      break;
    }
    // write to output stream
    if (!flag_no_saving) {
      if (frame.size() != writer.size()) {
        writer.open(frame.size());
      }
      if (writer.isOpened()) {
        writer.write(frame);
      }
    }
    // show live and wait for a key with timeout long enough to show images
    if (!flag_no_display) {
      cv::imshow("Live", frame);
      if (cv::waitKey(10) >= 0) break;
    } else {
      count += 1;
    }
  }

  provider.close();
  writer.close();

  return 0;
}