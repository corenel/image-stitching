import cv2
import argparse


def callback(x):
    pass


def grab_contours(cnts):
    # if the length the contours tuple returned by cv2.findContours
    # is '2' then we are using either OpenCV v2.4, v4-beta, or
    # v4-official
    if len(cnts) == 2:
        cnts = cnts[0]

    # if the length of the contours tuple is '3' then we are using
    # either OpenCV v3, v4-pre, or v4-alpha
    elif len(cnts) == 3:
        cnts = cnts[1]

    # otherwise OpenCV has changed their cv2.findContours return
    # signature yet again and I have no idea WTH is going on
    else:
        raise Exception(
            ("Contours tuple must have length 2 or 3, "
             "otherwise OpenCV changed their cv2.findContours return "
             "signature yet again. Refer to OpenCV's documentation "
             "in that case"))

    # return the actual contours array
    return cnts


if __name__ == '__main__':
    ap = argparse.ArgumentParser()
    ap.add_argument("input", help="path to the image file")
    args = ap.parse_args()
    image = cv2.imread(args.input)

    ilowH = 0
    ihighH = 255
    ilowS = 0
    ihighS = 255
    ilowV = 0
    ihighV = 255
    cv2.namedWindow('Mask')
    cv2.createTrackbar('lowH', 'Mask', ilowH, 179, callback)
    cv2.createTrackbar('highH', 'Mask', ihighH, 179, callback)
    cv2.createTrackbar('lowS', 'Mask', ilowS, 255, callback)
    cv2.createTrackbar('highS', 'Mask', ihighS, 255, callback)
    cv2.createTrackbar('lowV', 'Mask', ilowV, 255, callback)
    cv2.createTrackbar('highV', 'Mask', ihighV, 255, callback)

    while True:
        # get trackbar positions
        ilowH = cv2.getTrackbarPos('lowH', 'Mask')
        ihighH = cv2.getTrackbarPos('highH', 'Mask')
        ilowS = cv2.getTrackbarPos('lowS', 'Mask')
        ihighS = cv2.getTrackbarPos('highS', 'Mask')
        ilowV = cv2.getTrackbarPos('lowV', 'Mask')
        ihighV = cv2.getTrackbarPos('highV', 'Mask')

        print([ilowH, ilowS, ilowV], [ihighH, ihighS, ihighV])
        image_hsv = cv2.cvtColor(image, cv2.COLOR_BGR2HSV)
        image_mask = cv2.inRange(image_hsv, (ilowH, ilowS, ilowV),
                                 (ihighH, ihighS, ihighV))
        image_mask = cv2.erode(image_mask, None, iterations=2)
        image_mask = cv2.dilate(image_mask, None, iterations=2)
        result = cv2.bitwise_and(image, image, mask=image_mask)

        cnts = cv2.findContours(image_mask.copy(), cv2.RETR_TREE,
                                cv2.CHAIN_APPROX_SIMPLE)
        cnts = grab_contours(cnts)
        cnts = sorted(cnts, key=cv2.contourArea, reverse=True)
        if len(cnts) > 0:
            c = cnts[0]
            peri = cv2.arcLength(c, True)
            approx = cv2.approxPolyDP(c, 0.015 * peri, True)
            print(approx)
            cv2.drawContours(result, [approx], -1, (0, 255, 0), 3)

        cv2.imshow('Mask', result)
        key = cv2.waitKey(0)
        if chr(key) == 'q':
            break
