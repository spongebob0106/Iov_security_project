#include "veins/modules/application/traci/TrustAuthority.h"

// void TrustAuthority::initialize()
// {
//     snowflakeFactory snowflakefactory;
//     id_manage = snowflakefactory.createIDManage();
//     id_manage->initialize();
//     DVNCryptoFactory dvnflakefactory;
//     crypto_manage = dvnflakefactory.createCryptoManage();
//     crypto_manage->initialize();
// }

void TrustAuthority::registerAPI(int64_t id)
{
    auto it = registered_cars.find(id);
    if (it == registered_cars.end()) {
        registered_cars.emplace(std::make_pair(id, 100));
        // EV_INFO << "The car register success" << std::endl;
    }
    // else
    // {
    //     EV_ERROR << "The car is already registered" << std::endl;
    // }
}

void TrustAuthority::unregitsterAPI(int64_t id)
{
    auto it = registered_cars.find(id);
    registered_cars.erase(it);
}

void TrustAuthority::reportNodeStatus(int64_t id, bool is_malicious)
{
    auto it = registered_cars.find(id);
    if (it != registered_cars.end()) {
        if (it->second > 0 && is_malicious) {
            it->second--;
        }
    }
}
