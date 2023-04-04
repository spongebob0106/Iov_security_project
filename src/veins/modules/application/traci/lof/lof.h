#ifndef LOF_H
#define LOF_H

#include <stdarg.h>
#include <exception>
#include <cmath>
#include "veins/base/utils/Coord.h"
namespace veins {
class Point
{
    Point() noexcept
    : id(0)
    , senderCalDensity(0.0)
    , senderFlow(0.0)
    {
    }

    Point(int64_t id_, Coord senderPos_, Coord senderSpeed_, double senderCalDensity_, double senderFlow_): id(id_), senderPos(senderPos_), 
        senderSpeed(senderSpeed_), senderCalDensity(senderCalDensity_), senderFlow(senderFlow_)
    {
    }
public:
    int64_t id;
    Coord senderPos;
    Coord senderSpeed;
    double senderCalDensity;
    double senderFlow;
};


// 求欧几里得距离
double EuclideanDistance(Point& a, Point& b) {
    double sum = 0.0;
    sum += a.senderPos.sqrdist(b.senderPos);
    sum += a.senderSpeed.sqrdist(b.senderSpeed);
    sum += pow(a.senderCalDensity - b.senderCalDensity, 2);
    sum += pow(a.senderFlow - b.senderFlow, 2);
    return sqrt(sum);
}

// 计算数据点 i 和数据点 j 之间的可达距离
double ReachabilityDistance(std::vector<Point>& data, int i, int j, int k) {
    double maxDistance = 0.0;
    for (int l = 0; l < data.size(); l++) {
        if (l != i && l != j) {
            double distanceIL = EuclideanDistance(data[i], data[l]);
            double distanceJL = EuclideanDistance(data[j], data[l]);
            double distanceIJ = EuclideanDistance(data[i], data[j]);
            double distance = std::max(distanceIL, distanceJL);
            if (distance > maxDistance) {
                maxDistance = distance;
            }
        }
    }

    if (maxDistance == 0.0) {
        return 0.0;
    } else {
        return 1.0 / (maxDistance / k);
    }
}

// 计算数据点 i 的局部可达密度
double LocalReachabilityDensity(std::vector<Point>& data, int i, int k) {
    double sum = 0.0;
    for (int j = 0; j < data.size(); j++) {
        if (i != j) {
            double reachabilityDistance = ReachabilityDistance(data, i, j, k);
            sum += reachabilityDistance;
        }
    }
    return 1.0 / (sum / (data.size() - 1));
}

// 计算数据点 i 的 LOF 值
double LOF(std::vector<Point>& data, int i, int k) {
    double sum = 0.0;
    double lrdi = LocalReachabilityDensity(data, i, k);
    for (int j = 0; j < data.size(); j++) {
        if (i != j) {
            double lrdj = LocalReachabilityDensity(data, j, k);
            double reachabilityDistance = ReachabilityDistance(data, i, j, k);
            sum += lrdj / lrdi * reachabilityDistance;
        }
    }
    return sum / (data.size() - 1);
}
}
#endif