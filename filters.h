#ifndef FILTERS_H
#define FILTERS_H

#include <opencv2/opencv.hpp>

void gaussianFilter(cv::Mat &frame);
void sharpenFilter(cv::Mat &frame);
void sobelFilter(cv::Mat &frame);

#endif