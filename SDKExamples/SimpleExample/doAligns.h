//
// Created by tdev on 8/19/19.
//
#ifndef SISO_RT5_5_7_0_76321_LINUX_AMD64_DOALIGNS_H
#define SISO_RT5_5_7_0_76321_LINUX_AMD64_DOALIGNS_H
#include <opencv2/opencv.hpp>
#include <Eigen/SVD>
using Eigen::MatrixXd;
struct centroids {
public: std::vector<std::vector<double> > C;
};

void initAndCentroid(struct centroids&, const cv::Mat&);

double findAngle(const struct centroids&, int, int);
#endif //SISO_RT5_5_7_0_76321_LINUX_AMD64_DOALIGNS_H
