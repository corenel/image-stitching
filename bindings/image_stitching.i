%module image_stitching

%include <opencv.i>
%include <std_vector.i>
%cv_instantiate_all_defaults
%template(StringVector) std::vector<std::string>;
%template(MatVector) std::vector<cv::Mat>;

%{
#include "stitcher.hpp"
#include "stream_provider.hpp"
#include "stream_writer.hpp"
%}

%include "stitcher.hpp"
%include "stream_provider.hpp"
%include "stream_writer.hpp"
