## Python bindings

1. Make sure `swig` has been installed
2. Clone the [`opencv-swig` repo](https://github.com/renatoGarcia/opencv-swig) and put it next to this project
3. Build with `-DBUILD_PYTHON_BINDINGS=ON`
4. Add the following files into your `PYTHONPATH` and `LD_LIBRARY_PATH` respectively:
   - `image_stitching.py`
   - `_image_stitching.so`
5. Just `import image_stitching` and enjoy!

> Some basic exmaples can be found in `tests/test_python_bindings.py`