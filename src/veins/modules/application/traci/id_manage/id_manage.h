#ifndef ID_MANAGE_H
#define ID_MANAGE_H
#include <cstdint>
#include <chrono>
#include <string>

class IDManage
{
public:
    virtual void initialize() =0;
    virtual int64_t GetID() = 0;
    virtual ~IDManage() {}
};

class IDManageFactory
{
public:
    virtual IDManage* createIDManage() = 0;
};
#endif 