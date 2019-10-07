import cv2
import argparse

if __name__ == '__main__':
    ap = argparse.ArgumentParser()
    ap.add_argument("input", help="path to the image file")
    args = ap.parse_args()
    image_input = cv2.imread(args.input)
    corners = cv2.findChessboardCorners(image_input, (2, 2))
    print(corners)
