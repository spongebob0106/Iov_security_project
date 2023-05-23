//
// Copyright (C) 2006-2011 Christoph Sommer <christoph.sommer@uibk.ac.at>
//
// Documentation for these modules is at http://veins.car2x.org/
//
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#pragma once

#include "veins/modules/application/ieee80211p/DemoBaseApplLayer.h"
#include "veins/modules/application/traci/TrustAuthority.h"
#include "veins/modules/application/traci/algorithm/lof/lof.h"
#include "veins/modules/application/traci/algorithm/isolationforest/isolation_forest.h"
#include <map>
#include <vector>
#include <yaml-cpp/yaml.h>
#include <omnetpp.h>
#include <fstream>
#include <iostream>
#include <random>
#include <algorithm>

#define IS_USE_LOF 0
#define IS_USE_ISOLATIONFOREST 1
namespace veins {

/**
 * @brief
 * A tutorial demo for TraCI. When the car is stopped for longer than 10 seconds
 * it will send a message out to other cars containing the blocked road id.
 * Receiving cars will then trigger a reroute via TraCI.
 * When channel switching between SCH and CCH is enabled on the MAC, the message is
 * instead send out on a service channel following a Service Advertisement
 * on the CCH.
 *
 * @author Christoph Sommer : initial DemoApp
 * @author David Eckhoff : rewriting, moving functionality to DemoBaseApplLayer, adding WSA
 *
 */
struct SybilAttackParams {
    int fake_id;
    float fake_speed;
    float fake_density;
    float fake_flow;
    float fake_rate;
    int attack_time;
};

class VEINS_API TraCIDemo11p : public DemoBaseApplLayer {
public:
    void getSimparamters();
    void initialize(int stage) override;
    ~TraCIDemo11p()
    {
        outfile.close();
        debugfile.close();
    }

protected:
    simtime_t lastDroveAt;
    bool sentMessage;
    int currentSubscribedServiceId;
    TrustAuthority& ta = TrustAuthority::getInstance();
    IDManage* id_manage = nullptr;
    CryptoManage* crypto_manage = nullptr;
    int64_t car_id;
    float rcv_speed_avg;
    float rcv_flow_avg;
    float flow_own;
    float density_own;
    // car parameters
    int radir;
    int neighbors_number;
    // lof parameters
    float lof_threshold;
    int k_distance;
    int k_nearest_neighors;
    int minpts;
    // isolation_forest
    uint32_t numtrees;
    uint32_t maxheight;
    uint32_t randomseed;
    float iforset_threshold;
    //sybli params
    SybilAttackParams sybil_params;
    static std::vector<std::pair<int64_t, bool>> cars;
    static bool attack_flag; 
    std::ofstream outfile;
    std::ofstream debugfile;
    static std::ofstream carstypefile;
    bool is_malicious = false;
    bool is_with_defence = false;
    bool is_open_debug = false;
    float with_defense_cars_rate;
    // algorithm
#if IS_USE_LOF
    lof::Point point;
    std::vector<lof::Point> last_points;
    std::vector<lof::Point> cur_points;
    std::vector<lof::Point> fix_points;
#elif IS_USE_ISOLATIONFOREST
    iforest::IsolationForest<float, 4> forest;
    iforest::Point point;
    std::vector<iforest::Point> last_points;
    std::vector<iforest::Point> cur_points;
    std::vector<iforest::Point> fix_points;
#else
    BasePoint point;
    std::vector<BasePoint> last_points;
    std::vector<BasePoint> cur_points;
    std::vector<BasePoint> fix_points;
#endif 
protected:
    void onWSM(BaseFrame1609_4* wsm) override;
    void onWSA(DemoServiceAdvertisment* wsa) override;
    void onBSM(DemoSafetyMessage* bsm) override;

    void handleSelfMsg(cMessage* msg) override;
    void handlePositionUpdate(cObject* obj) override;
    
    float getCurrentDensity(int K, float r);
    void dataHandle(int time);
};

// sttaic init
bool TraCIDemo11p::attack_flag = false;
std::vector<std::pair<int64_t, bool>> TraCIDemo11p::cars = {};
std::ofstream TraCIDemo11p::carstypefile("/home/veins/src/veins/src/veins/modules/application/traci/data/cars_type.log");

} // namespace veins
