## Image stitching

### Seam resolution

CPU

|                       | 0.1M pixels (s)  | 0.05M pixels (s)  | 0.01M pixels (s)  |
| --------------------- | ---------------- | ----------------- | ----------------- |
| Finding features      | 1.06047          | 1.07069           | 1.05935           |
| Pairwise matching     | 0.960332         | 0.956351          | 0.976539          |
| Estimating            | 0.35077          | 0.334906          | 0.334005          |
| Warping images        | 0.0951291        | 0.0495488         | 0.0114975         |
| Compensating exposure | 0.110796         | 0.036418          | 0.00516394        |
| Finding seam          | 8.76386          | 1.52497           | 0.128938          |
| Compositing           | 0.739655         | 0.752825          | 0.778532          |
| **Total**             | 12.1026          | 4.74757           | 3.31633           |

### Seam methods

CPU

|              | no (s)  | gc color (s)  | gc colorgrad (s)  | voronoi (s)  |
| ------------ | ------- | ------------- | ----------------- | ------------ |
| Finding seam | N/A     | 8.75229       | 14.0364           | 0.0313928    |
| **Total**    | 3.31363 | 12.1075       | 17.3814           | 3.35101      |

### CPU vs GPU

|                       | CPU (s)   | GPU w/ CPU seam (s) |
| --------------------- | --------- | -------------------- |
| Finding features      | 1.06047   | 0.313686             |
| Pairwise matching     | 0.960332  | 0.743778             |
| Estimating            | 0.35077   | 0.272862             |
| Warping images        | 0.0951291 | 0.0395567            |
| Compensating exposure | 0.110796  | 0.106621             |
| Finding seam          | 8.76386   | 8.56208              |
| Compositing           | 0.739655  | 0.169609             |
| **Total**             | 12.1026   | 10.23128             |

## References

- [High level stitching API (Stitcher class) - OpenCV](https://docs.opencv.org/master/d8/d19/tutorial_stitcher.html)
- [Images stitching - OpenCV](https://docs.opencv.org/4.1.1/d1/d46/group__stitching.html)
- [Fast Panorama Stitching](https://software.intel.com/en-us/articles/fast-panorama-stitching)
