# Logs

```shell
$ ./test_videos -i input_list.txt -c calib_list.txt
calibration list:
../assets/video/extracted/194_00001.jpg
../assets/video/extracted/195_00001.jpg
../assets/video/extracted/196_00001.jpg
../assets/video/extracted/197_00001.jpg
../assets/video/extracted/198_00001.jpg
../assets/video/extracted/199_00001.jpg
../assets/video/extracted/202_00001.jpg
../assets/video/extracted/192_00001.jpg
../assets/video/extracted/193_00001.jpg
input list:
../assets/video/194.mp4
../assets/video/195.mp4
../assets/video/196.mp4
../assets/video/197.mp4
../assets/video/198.mp4
../assets/video/199.mp4
../assets/video/202.mp4
../assets/video/192.mp4
../assets/video/193.mp4
------- Calibrating -------
finding features_...
Features in image #1: 1509
Features in image #6: 1509
Features in image #3: 1520
Features in image #9: 1523
Features in image #7: 1500
Features in image #4: 1527
Features in image #5: 1530
Features in image #8: 1530
Features in image #2: 1497
Finding features_, time: 0.0465524 sec
Pairwise matching
Pairwise matching, time: 0.520208 sec
Estimating
Estimating, time: 4.19139 sec
Warping images (auxiliary)...
Warping images, time: 0.00403689 sec
Compensating exposure...
Compensating exposure, time: 0.00187239 sec
Finding seams...
Finding seams, time: 0.0980209 sec
Compositing...
Multi-band blender, number of bands: 5
Compositing, time: 0.0707863 sec
Finished, total time: 4.933 sec
------- Processing -------
Clear state, time: 3.7108e-05 sec
Multi-band blender, number of bands: 5
Prepare blender, time: 0.00421497 sec
Blending, time: 0.107021 sec
Finished, total time: 0.111337 sec
```
