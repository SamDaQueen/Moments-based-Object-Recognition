/**
 * @file processing.hpp
 * @author Samreen
 * @brief
 * @version 0.1
 * @date 2022-03-10
 *
 * @copyright Copyright (c) 2022
 *
 */
#include <cstdio>
#include <cstring>
#include <iostream>
#include <map>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

using namespace cv;
using namespace std;

/**
 * @brief thresholds the image and produces a binary image
 *
 * @param src input image
 * @param dst output image
 * @return int 0 if operation was succesful
 */
int thresholding(Mat &src, Mat &dst);

/**
 * @brief cleans up the binary image using morphological operators
 *
 * @param src input image
 * @param dst output image
 * @return int 0 if operation was succesful
 */

int clean(Mat &src, Mat &dst);

/**
 * @brief Get the regions in the image
 *
 * @param src input image
 * @return int the main object index
 */
int getRegions(Mat &src, Mat &dst, Mat &labels, Mat &stats, Mat &centroids);

int getMoments(Mat &src, Mat &dst, Mat &labels, Mat &stats, Mat &centroids,
               int main_object, double huMoments[]);

int writeToCSV(char name[], double moments[]);

int readFromCSV(vector<char *> &names, vector<vector<float>> &features);

int compareMoments(double moments[], vector<vector<float>> &features);