## Install NVDEC and NVENC as GStreamer plugins

### Environment

- Ubuntu 18.04
- NVIDIA driver 430.40
- NVIDIA Video Codec SDK 9.0.20

### Steps

1. Clone the `gst-plugins-bad` and check out to the same version as GStreamer

```shell
git clone git://anongit.freedesktop.org/gstreamer/gst-plugins-bad
cd gst-plugins-bad/
git checkout 1.14.0
```

2. Download [NVIDIA Video Codec SDK](https://developer.nvidia.com/nvidia-video-codec-sdk) and copy header files

```shell
cd /path/to/video/codec/sdk
cp /usr/local/cuda/include/cuda.h /path/to/gst-plugins-bad/sys/nvenc
cp include/nvEncodeAPI.h /path/to/gst-plugins-bad/sys/nvenc
cp include/cuviddec.h /path/to/gst-plugins-bad/sys/nvdec
cp include/nvcuvid.h /path/to/gst-plugins-bad/sys/nvdev
```

3. Build plugins

```shell
cd /path/to/gst-plugins-bad
NVENCODE_CFLAGS="-I/path/to/gst-plugins-bad/sys/nvenc" ./autogen.sh --with-cuda-prefix="/usr/local/cuda"
cd sys/nvenc
make
sudo cp .libs/libgstnvenc.so /usr/lib/x86_64-linux-gnu/gstreamer-1.0/
cd ../nvdec
make
sudo cp .libs/libgstnvdec.so /usr/lib/x86_64-linux-gnu/gstreamer-1.0/
```

4. Check installation

```shell
gst-insplect-1.0 | grep nvenc
gst-insplect-1.0 | grep nvdec
```

### References

- [DeepStream ContainerでH264ハードウエアエンコーダーを使う](https://weblog.hirozo.net/?p=541)

