#ifndef LOF_H
#define LOF_H

#include <stdarg.h>
#include <exception>
#include <cmath>
#include "veins/base/utils/Coord.h"
namespace veins {

//The 'k' in k-nearest-neighbors
static int K = 5;

//The number of points to compute reachability density
static int MINPTS = 10;

class Point
{
public:
    Point() noexcept
    {
    }

    Point(int64_t id_, Coord senderPos_, double speed_, double senderCalDensity_, double senderFlow_): id(id_), senderPos(senderPos_), 
        speed(speed_), senderCalDensity(senderCalDensity_), senderFlow(senderFlow_)
    {
    }
    int64_t id;
    Coord senderPos;
    float speed;
    float senderCalDensity;
    float senderFlow;
    // lof
    float kDistance;
    float lrd;
    float lof;
    std::vector<int> minPtsNeighbors;
};


/*
 * Compute euclidean distance between two points
 */
double computeDistance(Point& a, Point& b) {
    double sum = 0.0;
    // sum += a.senderPos.sqrdist(b.senderPos);
    sum += pow(a.speed - b.speed, 2);;
    sum += pow(a.senderCalDensity - b.senderCalDensity, 2) * 10;
    sum += pow(a.senderFlow - b.senderFlow, 2);
    return sqrt(sum);
}

/*
 * Compute the reachability density for every point by averaging the reach distance over MINPTS neighbors
 */
void computeReachDensity(std::vector<Point>& data)
{
	for (int i = 0; i < data.size(); i++)
	{
		Point p1 = data[i];
		std::vector<int> neighbors = p1.minPtsNeighbors;
		float summedReachDist = 0;

		//Iterate over the MINPTS neighbors to ith point
		for (int j = 0; j < neighbors.size(); j++)
		{
			Point p2 = data[j];

			float kDist      = p2.kDistance;
			float euclidDist = computeDistance(p1, p2);
			float reachDist  = std::max(kDist, euclidDist);

			summedReachDist += reachDist;
		}

		data[i].lrd = (float) neighbors.size() / summedReachDist;
	}
}

/*
 * Compute the LOF score for each point
 */
void computeLOF(std::vector<Point>& data)
{
	for (int i = 0; i < data.size(); i++)
	{
		Point p1 = data[i];
		std::vector<int> neighbors = p1.minPtsNeighbors;
		float lrd1 = p1.lrd;
		float summedLRDRatio = 0;

		for (int j = 0; j < neighbors.size(); j++)
		{
			Point p2 = data[j];
			float lrd2 = p2.lrd;
			summedLRDRatio += (lrd2 / lrd1);
		}

		data[i].lof = (float) summedLRDRatio / neighbors.size();
	}
}

/*
 * Find the k-nearest neighbors to each point
 * Use brute force at first (small number of points)
 * Eventually implement something better...
 */
void findNearestNeighbors(std::vector<Point>& data)
{
	std::vector<Point> neighbors;

	//Iterate over points
	//Use brute force to compute the distance to every other point
	for (int i = 0; i < data.size(); i++)
	{
		Point p1 = data[i];

		//Vector with distance from current point to every other point in the array
		std::vector<float> distances;
		//Vector with the indices of the points arranged in increasing distance to the current point
		std::vector<int> indices;

		for (int j = 0; j < data.size(); j++)
		{
			if (i == j) continue;

			Point p2 = data[j];

			float dist = computeDistance(p1, p2);

			//Insert first two elements in order
			if (distances.size() == 0)
			{
				distances.push_back(dist);
				indices.push_back(j);
			}
			else if (distances.size() == 1)
			{
				if (dist > distances[0])
				{
					distances.push_back(dist);
					indices.push_back(j);
				}
				else
				{
					distances.insert(distances.begin(), dist);
					indices.insert(indices.begin(), j);
				}
			}
            else
            {
                //Insert distance into array such that it is always ordered
                if (dist < distances[0])
                {
                    distances.insert(distances.begin(), dist);
                    indices.insert(indices.begin(), j);
                }
                else if (dist > distances[distances.size() - 1])
                {
                    distances.insert(distances.begin() + (distances.size()), dist);
                    indices.insert(indices.begin() + (indices.size()), j);
                }
                else
                {
                    for (int i = 0; i < distances.size() - 1; i++)
                    {
                        if (dist > distances[i] && dist < distances[i + 1])
                        {
                            distances.insert(distances.begin() + (i + 1), dist);
                            indices.insert(indices.begin() + (i + 1), j);
                        }
                    }
                }
            }
		}

		//Having computed all distances, take the max distance to the k-nearest neighbors as the k-distance for the point
        if ((distances.size() < (K - 1)) && (distances.size() != 0))
        {
		    data[i].kDistance = distances[distances.size() - 1];
        }
        else if (distances.size() == 0)
        {
        }
        else
        {
            data[i].kDistance = distances[K - 1];
        }
    
        data[i].minPtsNeighbors.clear();
		//Store the indices of the closest MINPTS neighbors in the data vector
        if ((indices.size() < MINPTS) && (indices.size() != 0)) {
            for (int k = 0; k < indices.size(); k++) {
                data[i].minPtsNeighbors.push_back(indices[k]);
            }
        }
        else if (indices.size() == 0)
        {
        }
        else {
            for (int k = 0; k < MINPTS; k++) {
                data[i].minPtsNeighbors.push_back(indices[k]);
            }
        }
    }
}

void lof(std::vector<Point>& data, int k, int minpts)
{
    K = k;
    MINPTS = minpts;
	findNearestNeighbors(data);
	computeReachDensity(data);
	computeLOF(data); 
}
}
#endif