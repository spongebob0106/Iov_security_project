#ifndef _TRUSTAUTHORITY_H_
#define _TRUSTAUTHORITY_H_

#include "veins/modules/application/traci/id_manage/id_manage.h"
#include "veins/modules/application/traci/crypto_manage/crypto_manage.h"
#include "veins/modules/application/traci/id_manage/snowflake/snowflake.h"
#include "veins/modules/application/traci/crypto_manage/DVNCrypto/DVNCrypto.h"

class TrustAuthority
{
public:
    ~TrustAuthority() 
    {
        delete id_manage;
        // delete crypto_manage;
    }

    // void initialize();
    IDManage* getIDManage() { return id_manage;}
    CryptoManage* getCryptoManage() { return crypto_manage;}
    static TrustAuthority& getInstance()
    {
        static TrustAuthority instance;
        return instance;
    }
private:
    TrustAuthority() {
        snowflakeFactory snowflakefactory;
        id_manage = snowflakefactory.createIDManage();
        id_manage->initialize();
        DVNCryptoFactory dvnflakefactory;
        crypto_manage = dvnflakefactory.createCryptoManage();
        crypto_manage->initialize();
    }
    IDManage* id_manage = nullptr;
    CryptoManage* crypto_manage = nullptr;
};



#endif