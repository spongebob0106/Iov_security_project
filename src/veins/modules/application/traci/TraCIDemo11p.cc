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

#include "veins/modules/application/traci/TraCIDemo11p.h"
#include "veins/modules/application/traci/TraCIDemo11pMessage_m.h"

using namespace veins;

Define_Module(veins::TraCIDemo11p);

void TraCIDemo11p::initialize(int stage)
{
    DemoBaseApplLayer::initialize(stage);
    if (stage == 0) {
        sentMessage = false;
        lastDroveAt = simTime();
        currentSubscribedServiceId = -1;
        rcv_speed_avg = 0;
        rcv_flow_avg = 0;
        flow_own = 0;
        density_own = 0;

        // sim paramters setup
        YAML::Node config = YAML::LoadFile("/home/veins/src/veins/src/veins/modules/application/traci/sim_paramters/simconfig.yaml");
        radir = config["radir"].as<int>();
        neighbors_number = config["neighbors_number"].as<int>();
        lof_threshold = config["lof_threshold"].as<float>();

        // output data
        std::string externalid =  mobility->getExternalId();
        int pos = externalid.find(".");
           if (pos != std::string::npos) {
               externalid.erase(pos, 1);
           }
        std::string file_name = "/home/veins/src/veins/src/veins/modules/application/traci/data/data_" + externalid + ".csv";
        outfile.open(file_name);
        outfile << "car_id,time,density_own,flow_own,speed_own,rcv_speed_avg" << std::endl;

        id_manage = ta.getIDManage();
        crypto_manage = ta.getCryptoManage();
        car_id = id_manage->GetID();
        // char s[150];
        // std::snprintf(s, 150, "notify-send normal \"Car ID: #%ld Initialize\"", car_id);
        // std::system(s);

        TraCIDemo11pMessage* wsm = new TraCIDemo11pMessage();
        populateWSM(wsm);
        // wsm->setDemoData(mobility->getRoadId().c_str());
        scheduleAt(simTime() + 1, wsm);
    }
}

void TraCIDemo11p::onWSA(DemoServiceAdvertisment* wsa)
{
    if (currentSubscribedServiceId == -1) {
        mac->changeServiceChannel(static_cast<Channel>(wsa->getTargetChannel()));
        currentSubscribedServiceId = wsa->getPsid();
        if (currentOfferedServiceId != wsa->getPsid()) {
            stopService();
            startService(static_cast<Channel>(wsa->getTargetChannel()), wsa->getPsid(), "Mirrored Traffic Service");
        }
    }
}

void TraCIDemo11p::onWSM(BaseFrame1609_4* frame)
{
    // TraCIDemo11pMessage* wsm = check_and_cast<TraCIDemo11pMessage*>(bsm);
    // if (mobility->getRoadId()[0] != ':') traciVehicle->changeRoute(wsm->getDemoData(), 9999);
    // if (!sentMessage) {
    //     sentMessage = true;
    //     // repeat the received traffic update once in 2 seconds plus some random delay
    //     wsm->setSenderAddress(car_id);
    //     wsm->setSerial(3);
    //     scheduleAt(simTime() + 2 + uniform(0.01, 0.2), wsm->dup());
    // }
}

void TraCIDemo11p::onBSM(DemoSafetyMessage* bsm)
{
    // char s[150];
    // std::snprintf(s, 150, "notify-send normal \"Car ID: #%ld receive ID :%ld \"", car_id, bsm->getCarid());
    // std::system(s);
    findHost()->getDisplayString().setTagArg("i", 1, "green");
    auto it = beasons_map.find(bsm->getCarid());
    if (it != beasons_map.end()) {
        it->second.assign({bsm->getSpeed(), bsm->getSenderCalDensity(), bsm->getSenderPos().distance(curPosition)});
    }
    else
    {
       beasons_map.emplace(std::make_pair(bsm->getCarid(), std::vector<double>{bsm->getSpeed(), 
       bsm->getSenderCalDensity(), bsm->getSenderPos().distance(curPosition)}));
    }
}


void TraCIDemo11p::handleSelfMsg(cMessage* msg)
{
    if (TraCIDemo11pMessage* wsm = dynamic_cast<TraCIDemo11pMessage*>(msg)) {
        DemoSafetyMessage* bsm = new DemoSafetyMessage();
        populateWSM(bsm);
        density_own = getCurrentDensity(radir, neighbors_number);
        bsm->setCarid(car_id);
        bsm->setSpeed(mobility->getSpeed());
        bsm->setSenderCalDensity(density_own);
        double rcv_speed_sum = 0;
        double rcv_flow_sum = 0;
        for (auto it = beasons_map.begin(); it != beasons_map.end(); it++) {
            std::vector<double>& vec = it->second;
            rcv_speed_sum += vec[0];
        }
        if (beasons_map.size() != 0)
        {
            rcv_speed_avg = rcv_speed_sum / beasons_map.size();
        }
        for (auto it = beasons_map.begin(); it != beasons_map.end(); it++) {
            std::vector<double>& vec = it->second;
            rcv_flow_sum += rcv_speed_avg * vec[1];
        }
        rcv_flow_avg = rcv_flow_sum / beasons_map.size();
        flow_own = rcv_speed_avg * density_own;
        bsm->setSenderFlow(rcv_flow_avg);
        sendDown(bsm);

        // record data
        simtime_t simTime_ = simTime();
        double seconds = simTime_.dbl();

        outfile << car_id << "," << seconds << "," << density_own << "," << flow_own << "," << mobility->getSpeed() << "," << rcv_speed_avg << std::endl;
        scheduleAt(simTime() + 1, wsm);
    }
    else {
        DemoBaseApplLayer::handleSelfMsg(msg);
    }
}

void TraCIDemo11p::handlePositionUpdate(cObject* obj)
{
    DemoBaseApplLayer::handlePositionUpdate(obj);
    findHost()->getDisplayString().setTagArg("i", 1, "red");
    DemoSafetyMessage* bsm = new DemoSafetyMessage();
    populateWSM(bsm);
    bsm->setCarid(car_id);
    bsm->setSpeed(mobility->getSpeed());
    bsm->setSenderCalDensity(density_own);
    bsm->setSenderFlow(rcv_flow_avg);
    sendDown(bsm);

    // // stopped for for at least 10s?
    // if (mobility->getSpeed() < 1) {
    //     if (simTime() - lastDroveAt >= 10 && sentMessage == false) {
    //         findHost()->getDisplayString().setTagArg("i", 1, "red");
    //         sentMessage = true;

    //         TraCIDemo11pMessage* wsm = new TraCIDemo11pMessage();
    //         populateWSM(wsm);
    //         wsm->setDemoData(mobility->getRoadId().c_str());

    //         // host is standing still due to crash
    //         if (dataOnSch) {
    //             startService(Channel::sch2, 42, "Traffic Information Service");
    //             // started service and server advertising, schedule message to self to send later
    //             scheduleAt(computeAsynchronousSendingTime(1, ChannelType::service), wsm);
    //         }
    //         else {
    //             // send right away on CCH, because channel switching is disabled
    //             sendDown(wsm);
    //         }
    //     }
    // }
    // else {
    //     lastDroveAt = simTime();
    // }
}

double TraCIDemo11p::getCurrentDensity(int K, double r)
{
    double Dk = 0;
    double rho = 0;
    std::vector<std::pair<double, int>> distances;
    static int i = 0;
    // 计算点P到其他点的距离
    for (auto it = beasons_map.begin(); it != beasons_map.end(); it++) {
        std::vector<double>& vec = it->second;

        double distance = vec[2];
        if (distance <= r) {
            distances.push_back(std::make_pair(distance, i));
        }
        i++;
    }

    // 如果K值大于邻居数，邻居数取邻居总数
    K = std::min(K, (int)distances.size());

    // 如果邻居数为0，密度为0
    if (K == 0) {
        return 0;
    }

    // 计算平均距离Dk
    for (int i = 0; i < K; i++) {
        Dk += distances[i].first;
    }
    Dk /= K;

    // 计算密度估计值
    rho = (K / (M_PI * Dk * Dk)) * 1000;
    i = 0;
    return rho;
}
