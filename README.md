# image-stitching
A playground for testing image stitching algorithms.

## Usage

### Requirements

- OpenCV (tested on 3.4.6 and 4.1.0)
- OpenMP
- TCLAP (`libtclap-dev`)

### Executables

- `test_stitching_detailed_origianl`: Original version of detailed image stitching form OpenCV
- `test_images`: Test image stitching on given images
- `test_videos`: Test image stitching on given calibration images and input videos
  ```shell
  USAGE:
     ./test_videos  -i <string> -c <string> [-h]
  Where:
     -i <string>,  --input <string>
       (required)  Path to list of input videos (one path per line as format of plain text)
     -c <string>,  --calib <string>
       (required)  Path to list of calib images (one path per line as format of plain text)
  ```
 - `test_warp_perspective`: Convert images into top view and merge thme into single one. Details can be seen in [this document](docs/warp.md)

### Python bindings

See [this document](docs/bindings.md).
