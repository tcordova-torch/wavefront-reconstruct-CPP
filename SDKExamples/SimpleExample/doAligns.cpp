//
// Created by tdev on 8/19/19.
//
#include <cmath>
#include <vector>
#include <numeric>
#include <opencv2/opencv.hpp>

struct centroids {
public:
    std::vector<std::vector<double> > C;
};

void initAndCentroid(struct centroids &mats, const cv::Mat& nImg) {
    cv::Mat nBinary;
    std::vector<std::vector<cv::Point> > nContours;
    cv::Mat nHierarchy = {};
    std::vector<std::vector<double> > nCentroids;

    cv::threshold(nImg, nBinary, 0, 255, cv::THRESH_BINARY+cv::THRESH_OTSU);
    cv::findContours(nBinary, nContours, nHierarchy, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);
    int cSize = nContours.size();
    std::vector<cv::Moments> M(cSize);
    for(int c = 0; c < cSize; c++) {
        M[c] = cv::moments(nContours[c]);
        double cX, cY;
        if (M[c].m00 != 0) {
            cX = (M[c].m10 / M[c].m00);
            cY = (M[c].m01 / M[c].m00);
        } else {
            cX = 0.0;
            cY = 0.0;
        }
        nCentroids.emplace_back();
        nCentroids[c].push_back(cX);
        nCentroids[c].push_back(cY);
    }

    for (int i = 0; i < (int)nCentroids.size(); i++) {
        if (nCentroids[i][0] == 0 && nCentroids[i][1] == 0) {
            nCentroids.erase(nCentroids.begin() + i);
        }
    }

}
double findAngle(const struct centroids &mats, int xStart, int matchLength) {
    double xVal = mats.C[xStart][0];
    double yVal = mats.C[xStart][1];
    int match = 0; int i = 0; int abs_tolX = 30; int abs_tolY = 10; double slopeM = 0;
    bool STOP = false;
    std::vector<std::vector<double> > ptsCoord;
    std::vector<double> angle;
    while (match < matchLength) {
        while (i < mats.C.size() && !STOP) {
            if (xVal - abs_tolX < mats.C[i][0] < xVal + abs_tolX && yVal + abs_tolY < mats.C[i][1] < yVal + abs_tolY) {
                xVal = mats.C[i][0];
                yVal = mats.C[i][0];
                ptsCoord.emplace_back();
                ptsCoord[i].push_back(xVal);
                ptsCoord[i].push_back(yVal);
                match++;
                STOP = true;
            } else {i++;}
        }
        if (i >= mats.C.size()) {break;}
        STOP = false;
        i = 0;
    }
    for (int j = 0; j < (int)ptsCoord.size() -1; j++) {
        if ((ptsCoord[j + 1][0] - ptsCoord[j][0]) == 0) { slopeM = 0;}
        else {
            slopeM = (ptsCoord[j + 1][ 1] - ptsCoord[j][ 1]) / (ptsCoord[j + 1][ 0] - ptsCoord[j][ 0]);
        }
        angle.push_back(-atan(slopeM) * (180 / M_PI));
    }
    return std::accumulate( angle.begin(), angle.end(), 0.0) / angle.size();
}