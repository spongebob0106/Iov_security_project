#ifndef BASEPOINT_H
#define BASEPOINT_H
#include <iostream>
#include "veins/base/utils/Coord.h"

namespace veins
{
class BasePoint 
{
public:
    BasePoint() noexcept
    {
    }

    BasePoint(int64_t id_, Coord senderPos_, double speed_, double senderCalDensity_, double senderFlow_): id(id_), senderPos(senderPos_), 
        speed(speed_), senderCalDensity(senderCalDensity_), senderFlow(senderFlow_)
    {}
    
    int64_t id;
    Coord senderPos;
    float speed;
    float senderCalDensity;
    float senderFlow;
};
}
#endif