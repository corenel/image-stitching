import unittest
import numpy as np
import image_stitching as stit


class TestImageStitching(unittest.TestCase):
    def test_stitching(self):
        # prepare input files
        paths_to_input_file = [
            '/home/yuthon/Workspace/image-stitching/assets/data/test2/test_cam_1.mp4',
            '/home/yuthon/Workspace/image-stitching/assets/data/test2/test_cam_2.mp4',
            '/home/yuthon/Workspace/image-stitching/assets/data/test2/test_cam_3.mp4',
            '/home/yuthon/Workspace/image-stitching/assets/data/test2/test_cam_4.mp4',
            '/home/yuthon/Workspace/image-stitching/assets/data/test2/test_cam_5.mp4',
            '/home/yuthon/Workspace/image-stitching/assets/data/test2/test_cam_6.mp4',
            '/home/yuthon/Workspace/image-stitching/assets/data/test2/test_cam_7.mp4',
            '/home/yuthon/Workspace/image-stitching/assets/data/test2/test_cam_8.mp4'
        ]
        path_vector = stit.StringVector()
        for path in paths_to_input_file:
            path_vector.push_back(path)

        # initialize
        provider = stit.MultipleStreamProvider(
            path_vector, 'gst-libav', 'h264', '720p', 25)
        stitcher = stit.Stitcher(len(paths_to_input_file), stit.Size(1280, 720))

        # process
        frame = stit.MatVector()
        result, result_mask = stit.Mat(), stit.Mat()
        if provider.isOpened():
            provider.read(frame)
            stitcher.calibrate(frame, result, result_mask)
            self.assertFalse(result.empty())

    def test_stream_provider(self):
        # path_to_input_file = 'rtsp://184.72.239.149/vod/mp4:BigBuckBunny_115k.mov'
        path_to_input_file = '/home/yuthon/Workspace/image-stitching/assets/data/test2/test_cam_1.mp4'
        provider = stit.StreamProvider(
            path_to_input_file, 'gst-libav', 'h264', '720p', 25)
        self.assertTrue(provider.isOpened())

        frame = stit.Mat()
        if provider.isOpened():
            provider.read(frame)
            self.assertFalse(frame.empty())

    def test_stream_writer(self):
        writer = stit.StreamWriter(
            "rtmp://192.168.6.3/live/test", "gst-nvidia", "h264",
            "720p", 25, stit.Size(1280, 720))
        self.assertTrue(writer.isOpened())

        frame = stit.Mat.from_array(
            np.zeros([100, 100, 3], dtype=np.uint8))
        if writer.isOpened():
            writer.write(frame)


if __name__ == '__main__':
    unittest.main()
