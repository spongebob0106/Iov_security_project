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
#include "veins/modules/application/traci/lof/lof.h"
#include <map>
#include <vector>
#include <yaml-cpp/yaml.h>
#include <omnetpp.h>
#include <fstream>
#include <iostream>

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


class VEINS_API TraCIDemo11p : public DemoBaseApplLayer {
public:
    void initialize(int stage) override;
    ~TraCIDemo11p()
    {
        outfile.close();
    }

protected:
    simtime_t lastDroveAt;
    bool sentMessage;
    int currentSubscribedServiceId;
    TrustAuthority& ta = TrustAuthority::getInstance();
    IDManage* id_manage = nullptr;
    CryptoManage* crypto_manage = nullptr;
    int64_t car_id;
    std::map<int64_t, std::vector<double>> beasons_map;
    double rcv_speed_avg;
    double rcv_flow_avg;
    double flow_own;
    double density_own;
    std::vector<Point> points;
    int radir;
    int neighbors_number;
    float lof_threshold;
    std::ofstream outfile;

protected:
    void onWSM(BaseFrame1609_4* wsm) override;
    void onWSA(DemoServiceAdvertisment* wsa) override;
    void onBSM(DemoSafetyMessage* bsm) override;

    void handleSelfMsg(cMessage* msg) override;
    void handlePositionUpdate(cObject* obj) override;
    
    double getCurrentDensity(int K, double r);
};

} // namespace veins
