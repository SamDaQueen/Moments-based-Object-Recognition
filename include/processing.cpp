/**
 * @file processing.cpp
 * @author Samreen
 * @brief
 * @version 0.1
 * @date 2022-03-10
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "processing.hpp"

using namespace cv;
using namespace std;

int thresholding(Mat &src, Mat &dst) {
  Mat hsv(src.size(), src.type());
  cvtColor(src, hsv, COLOR_BGR2HSV);

  // imshow("HSV", hsv);

  // make highly saturated areas darker
  for (unsigned int i = 0; i < hsv.rows; i++) {
    for (unsigned int j = 0; j < hsv.cols; j++) {
      Vec3b value = hsv.at<Vec3b>(i, j);
      if (value[1] > 90) {
        src.at<Vec3b>(i, j) = src.at<Vec3b>(i, j) / 20;
      }
    }
  }

  Mat gray(src.size(), CV_8U);
  cvtColor(src, gray, COLOR_BGR2GRAY);  // grayscale
  Mat blurred(src.size(), CV_8U);
  GaussianBlur(gray, blurred, Size(7, 7), 0);  // blurring the grayscale image
  // Otsu thresholding
  threshold(blurred, dst, 0, 255, THRESH_OTSU);
  bitwise_not(dst, dst);
  // imshow("gray", dst);

  return 0;
}

int shrink(Mat &src, Mat &dst) {
  for (unsigned int i = 1; i < src.rows - 1; i++) {
    for (unsigned int j = 1; j < src.cols - 1; j++) {
      unsigned char value = src.at<unsigned char>(i, j);
      if (value == 0) {
        dst.at<unsigned char>(i - 1, j - 1) = 0;
        dst.at<unsigned char>(i - 1, j) = 0;
        dst.at<unsigned char>(i - 1, j + 1) = 0;
        dst.at<unsigned char>(i, j - 1) = 0;
        dst.at<unsigned char>(i, j) = 0;
        dst.at<unsigned char>(i, j + 1) = 0;
        dst.at<unsigned char>(i + 1, j - 1) = 0;
        dst.at<unsigned char>(i + 1, j) = 0;
        dst.at<unsigned char>(i + 1, j - +1) = 0;
      } else {
        dst.at<unsigned char>(i, j) = 255;
      }
    }
  }
  return 0;
}

int grow(Mat &src, Mat &dst) {
  for (unsigned int i = 1; i < src.rows - 1; i++) {
    for (unsigned int j = 1; j < src.cols - 1; j++) {
      unsigned char value = src.at<unsigned char>(i, j);
      if (value == 255) {
        dst.at<unsigned char>(i - 1, j - 1) = 255;
        dst.at<unsigned char>(i - 1, j) = 255;
        dst.at<unsigned char>(i - 1, j + 1) = 255;
        dst.at<unsigned char>(i, j - 1) = 255;
        dst.at<unsigned char>(i, j) = 255;
        dst.at<unsigned char>(i, j + 1) = 255;
        dst.at<unsigned char>(i + 1, j - 1) = 255;
        dst.at<unsigned char>(i + 1, j) = 255;
        dst.at<unsigned char>(i + 1, j - +1) = 255;
      } else {
        dst.at<unsigned char>(i, j) = 0;
      }
    }
  }
  return 0;
}

int clean(Mat &src, Mat &dst) {
  for (unsigned int i = 0; i < 5; i++) {
    Mat grown(src.size(), src.type());
    grow(src, grown);
    src = grown;
  }

  for (unsigned int i = 0; i < 5; i++) {
    Mat shrunk(src.size(), src.type());
    shrink(src, shrunk);
    src = shrunk;
  }

  dst = src;

  return 0;
}

int getRegions(Mat &src, Mat &dst, Mat &labels, Mat &stats, Mat &centroids) {
  int i, regions = connectedComponentsWithStats(src, labels, stats, centroids);

  // cout << "regions:" << regions << endl;
  // cout << "labels " << labels.size() << endl;
  // cout << "stats " << stats.size() << endl;
  // cout << "centroids " << centroids.size() << endl;

  // define region colors in RGB
  int colors[regions][3] = {{0, 0, 0},     {255, 0, 0},   {255, 255, 255},
                            {255, 255, 0}, {255, 0, 255}, {0, 128, 128},
                            {0, 255, 255}, {128, 0, 0},   {0, 128, 0}};

  for (unsigned int i = 0; i < labels.rows; i++) {
    for (unsigned int j = 0; j < labels.cols; j++) {
      unsigned int value = labels.at<unsigned int>(i, j);
      dst.at<Vec3b>(i, j)[0] = colors[value][2];
      dst.at<Vec3b>(i, j)[1] = colors[value][1];
      dst.at<Vec3b>(i, j)[2] = colors[value][0];
    }
  }

  // cout << stats << endl;
  // cout << centroids;

  int main_object = 1;
  float dist = INFINITY;
  // get distance of centroids from the center of the image
  for (unsigned int i = 1; i < regions; i++) {
    float currentDist = sqrt((centroids.at<double>(i, 0) - 250) *
                                 (centroids.at<double>(i, 0) - 250) +
                             (centroids.at<double>(i, 1) - 187.5) *
                                 (centroids.at<double>(i, 1) - 187.5));

    // cout << i << " " << currentDist << endl;
    if (currentDist < dist) {
      dist = currentDist;
      main_object = i;
    }
  }
  // cout << main_object << endl;

  // cout << stats;
  Rect rect(stats.at<int>(main_object, 0), stats.at<int>(main_object, 1),
            stats.at<int>(main_object, 2), stats.at<int>(main_object, 3));

  rectangle(dst, rect, Scalar(0, 255, 0), 2);

  return main_object;
}

int getMoments(Mat &src, Mat &dst, Mat &labels, Mat &stats, Mat &centroids,
               int main_object, double huMoments[]) {
  vector<Vec2i> points;
  for (unsigned int i = 0; i < labels.rows; i++) {
    for (unsigned int j = 0; j < labels.cols; j++) {
      unsigned int value = labels.at<unsigned int>(i, j);
      if (value == main_object) {
        points.push_back(Vec2i(i, j));
        dst.at<unsigned char>(i, j) = 255;
      } else {
        dst.at<unsigned char>(i, j) = 0;
      }
    }
  }

  circle(dst,
         Point(centroids.at<double>(main_object, 0),
               centroids.at<double>(main_object, 1)),
         3, Scalar(255, 255, 255), 3);
  circle(dst,
         Point(centroids.at<double>(main_object, 0),
               centroids.at<double>(main_object, 1)),
         1, Scalar(0, 0, 0), 3);

  putText(dst, "Centroid",
          Point(centroids.at<double>(main_object, 0) + 10,
                centroids.at<double>(main_object, 1) + 10),
          FONT_HERSHEY_DUPLEX, 0.8, Scalar(128, 128, 128));

  Moments imageMoments = moments(points);
  HuMoments(imageMoments, huMoments);

  // reduce magnitude of huMoments
  for (int i = 0; i < 6; i++) {
    huMoments[i] = -1 * copysign(1.0, huMoments[i]) * log10(abs(huMoments[i]));
  }

  huMoments[6] = double(
      abs(stats.at<int>(main_object, 0) - stats.at<int>(main_object, 2)));
  huMoments[7] = double(
      abs(stats.at<int>(main_object, 1) - stats.at<int>(main_object, 3)));

  return 0;
}

int writeToCSV(char name[], double moments[]) {
  char filename[256] = "./data/features.csv";
  char buffer[256];
  FILE *fp;

  fp = fopen(filename, "a");
  if (!fp) {
    printf("Unable to open output file %s\n", filename);
    exit(-1);
  }

  strcpy(buffer, name);
  fwrite(buffer, sizeof(char), strlen(buffer), fp);
  for (int i = 0; i < 8; i++) {
    char tmp[256];
    sprintf(tmp, ",%.4f", moments[i]);
    fwrite(tmp, sizeof(char), strlen(tmp), fp);
  }

  std::fwrite("\n", sizeof(char), 1, fp);  // EOL

  fclose(fp);

  return 0;
}

/*
  Utility function for reading one float value from a CSV file

  The value is stored in the v parameter

  The function returns true if it reaches the end of a line or the file
 */
int getfloat(FILE *fp, float *v) {
  char s[256];
  int p = 0;
  int eol = 0;

  for (;;) {
    char ch = fgetc(fp);
    if (ch == ',') {
      break;
    } else if (ch == '\n' || ch == EOF) {
      eol = 1;
      break;
    }

    s[p] = ch;
    p++;
  }
  s[p] = '\0';  // terminator
  *v = atof(s);

  return (eol);  // return true if eol
}

/*
  reads a string from a CSV file. the 0-terminated string is returned in the
  char array os.

  The function returns false if it is successfully read. It returns true if it
  reaches the end of the line or the file.
 */
int getstring(FILE *fp, char os[]) {
  int p = 0;
  int eol = 0;

  for (;;) {
    char ch = fgetc(fp);
    if (ch == ',') {
      break;
    } else if (ch == '\n' || ch == EOF) {
      eol = 1;
      break;
    }
    // printf("%c", ch ); // uncomment for debugging
    os[p] = ch;
    p++;
  }
  // printf("\n"); // uncomment for debugging
  os[p] = '\0';

  return (eol);  // return true if eol
}

int readFromCSV(vector<char *> &names, vector<vector<float>> &features) {
  FILE *fp;
  float fval;
  char img_file[256];
  char filename[256] = "./data/features.csv";

  fp = fopen(filename, "r");
  if (!fp) {
    printf("Unable to open feature file\n");
    return (-1);
  }

  for (;;) {
    std::vector<float> dvec;

    // read the filename
    if (getstring(fp, img_file)) {
      break;
    }

    // read the whole feature file into memory
    for (;;) {
      // get next feature
      float eol = getfloat(fp, &fval);
      dvec.push_back(fval);
      if (eol) break;
    }
    // printf("read %lu features\n", dvec.size() );

    features.push_back(dvec);

    char *fname = new char[strlen(img_file) + 1];
    strcpy(fname, img_file);
    names.push_back(fname);
  }
  fclose(fp);

  if (names.size() == 0) {
    return -1;
  }

  return 0;
}

int compareMoments(double moments[], vector<vector<float>> &features) {
  int match = 0;
  float mean[features.size()];
  float stddev[features.size()];

  for (unsigned int i = 0; i < features.size(); i++) {
    for (unsigned int j = 0; j < features[0].size(); j++) {
      mean[i] += features[i][j];
    }
    mean[i] /= features[0].size();
  }

  for (unsigned int i = 0; i < features.size(); i++) {
    for (unsigned int j = 0; j < features[0].size(); j++) {
      stddev[i] += (features[i][j] - mean[i]) * (features[i][j] - mean[i]);
    }
    stddev[i] /= features[0].size();
    stddev[i] = sqrt(stddev[i]);
  }

  float minDistance = INFINITY;
  for (unsigned int i = 1; i < features.size(); i++) {
    float distance = 0;
    for (unsigned int j = 0; j < features[0].size(); j++) {
      distance += abs(moments[i] - features[i][j]);
    }
    distance /= stddev[i];
    if (distance < minDistance) {
      minDistance = distance;
      match = i;
    }
  }

  return match;
}
