
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include "processing.hpp"

using namespace std;
using namespace cv;

int main(int argc, char *argv[]) {
  VideoCapture *capdev;

  // open the video device
  capdev = new VideoCapture(1);
  if (!capdev->isOpened()) {
    printf("Unable to open video device\n");
    return (-1);
  }

  // get some properties of the image
  Size refS((int)capdev->get(CAP_PROP_FRAME_WIDTH),
            (int)capdev->get(CAP_PROP_FRAME_HEIGHT));
  printf("Expected size: %d %d\n", refS.width, refS.height);

  namedWindow("Video", 1);
  namedWindow("Threshold", 1);
  namedWindow("Cleaned Image", 1);

  Mat frame;

  for (;;) {
    *capdev >> frame;  // get a new frame from the camera, treat as a stream
    if (frame.empty()) {
      printf("frame is empty\n");
      break;
    }

    // see if there is a waiting keystroke
    char key = waitKey(10);

    resize(frame, frame, Size(500, 375));
    imshow("Video", frame);

    // show threshold video
    Mat thresholdFrame(frame.size(), CV_8U);
    thresholding(frame, thresholdFrame);
    imshow("Threshold", thresholdFrame);

    // show cleaned video
    Mat cleanedFrame(frame.size(), CV_8U);
    clean(thresholdFrame, cleanedFrame);
    imshow("Cleaned Image", cleanedFrame);

    // show the regions
    Mat segments(frame.size(), frame.type());
    Mat labels, stats, centroids;
    int main_object =
        getRegions(cleanedFrame, segments, labels, stats, centroids);
    imshow("Segments", segments);

    // get the moments of the image
    Mat isolated(frame.size(), CV_8U);
    double moments[8];
    getMoments(segments, isolated, labels, stats, centroids, main_object,
               moments);
    putText(isolated, "Press N and then type object name in console",
            Point(20, 20), FONT_HERSHEY_DUPLEX, 0.6, Scalar(255, 255, 255));

    // end program when "q" is pressed
    if (key == 'q' || getWindowProperty("Video", WND_PROP_VISIBLE) < 1) {
      break;
    } else if (key == 'n') {
      cout << "Please enter the name of the object: ";
      char name[256];
      cin >> name;
      cout << "You have written the features for object: " << name << endl;
      writeToCSV(name, moments);
    }

    vector<char *> names;
    vector<vector<float>> features;
    if (readFromCSV(names, features) == 0) {
      int match = compareMoments(moments, features);
      char object[256] = "Detected object: ";
      strcat(object, names[match]);
      putText(isolated, object, Point(20, 350), FONT_HERSHEY_DUPLEX, 0.6,
              Scalar(255, 255, 255));
      imshow("Isolated object", isolated);
    }
  }

  delete capdev;
  destroyAllWindows();
  return (0);
}
