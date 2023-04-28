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

void TraCIDemo11p::getSimparamters()
{

    YAML::Node config = YAML::LoadFile("/home/veins/src/veins/src/veins/modules/application/traci/sim_paramters/simconfig.yaml");
    radir = config["radir"].as<int>();
    neighbors_number = config["neighbors_number"].as<int>();
    lof_threshold = config["lof_threshold"].as<float>();
    k_distance = config["k_distance"].as<int>();
    k_nearest_neighors = config["k_nearest_neighors"].as<int>();
    minpts = config["minpts"].as<int>();
    with_defense_cars_rate = config["with_defense_cars_rate"].as<float>();
    sybil_params.fake_id = config["fake_id"].as<int>();
    sybil_params.fake_speed = config["fake_speed"].as<double>();
    sybil_params.fake_density = config["fake_density"].as<double>();
    sybil_params.fake_flow = config["fake_flow"].as<double>();
    sybil_params.fake_rate = config["fake_rate"].as<double>();
    sybil_params.attack_time = config["attack_time"].as<int>();
    is_open_debug = config["is_open_debug"].as<bool>();
}


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
        getSimparamters();
        // output data
        std::string externalid =  mobility->getExternalId();
        int pos = externalid.find(".");
           if (pos != std::string::npos) {
               externalid.erase(pos, 1);
           }
        std::string file_name = "/home/veins/src/veins/src/veins/modules/application/traci/data/csv/data_" + externalid + ".csv";
        std::string debug_file_name = "/home/veins/src/veins/src/veins/modules/application/traci/data/log/debug_" + externalid + ".log";
        outfile.open(file_name);
        debugfile.open(debug_file_name);
        outfile << "car_id,time,density_own,flow_own,speed_own,rcv_speed_avg" << std::endl;

        id_manage = ta.getIDManage();
        crypto_manage = ta.getCryptoManage();
        car_id = id_manage->GetID();
        ta.registerAPI(car_id);
        cars.push_back(std::make_pair(car_id, false));

        // char s[150];
        // std::snprintf(s, 150, "notify-send normal \"Car ID: #%ld Initialize\"", car_id);
        // std::system(s);

        TraCIDemo11pMessage* wsm = new TraCIDemo11pMessage();
        populateWSM(wsm);
        is_with_defence = bernoulli(with_defense_cars_rate);
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
    if (bsm->getCarid() == 0)
    {
        return;
    }
    last_points.assign(cur_points.begin(), cur_points.end());
    int i = 0;
    for (; i < cur_points.size(); i++)
    {
        if (cur_points[i].id == bsm->getCarid())
        {
            cur_points[i].senderPos = bsm->getSenderPos();
            cur_points[i].speed = bsm->getCarSpeed();  
            cur_points[i].senderCalDensity = bsm->getSenderCalDensity();
            cur_points[i].senderFlow = bsm->getSenderFlow();                  
            break;
        }
    }
    if (i == cur_points.size() || cur_points.size() == 0)
    {
        Point point;
        point.senderPos = bsm->getSenderPos();
        point.speed = bsm->getCarSpeed();    
        point.senderCalDensity = bsm->getSenderCalDensity();
        point.senderFlow = bsm->getSenderFlow(); 
        point.id = bsm->getCarid();
        cur_points.push_back(point);
    }
}

void TraCIDemo11p::handleSelfMsg(cMessage* msg)
{
    if (TraCIDemo11pMessage* wsm = dynamic_cast<TraCIDemo11pMessage*>(msg)) {
        DemoSafetyMessage* bsm = new DemoSafetyMessage();
        populateWSM(bsm);
        simtime_t simTime_ = simTime();
        int seconds = simTime_.dbl();
        // start sybil attack
        if (seconds == sybil_params.attack_time && attack_flag == false)
        {
            // Assign malicious cars
            int malicious_car_numbers = cars.size() * sybil_params.fake_rate;
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<int> dis(0, cars.size()-1);
            for (int i = 0; i < malicious_car_numbers; i++) {
                int randomIndex = dis(gen);
                cars[randomIndex].second = true;
                maliciouscarsfile << cars[randomIndex].first << std::endl;
            }
            attack_flag = true;
        }
        if (attack_flag == true && is_malicious == false)
        {
            for (int i = 0; i < cars.size(); i++)
            {
                if (cars[i].first == car_id)
                {
                    is_malicious = cars[i].second;
                }
            }
        }

        outfile << car_id << "," << seconds << "," << density_own << "," << flow_own << "," << mobility->getSpeed() << "," << rcv_speed_avg << std::endl;
        dataHandle(seconds);
        // package bsm message
        if (!is_malicious) {
            bsm->setCarid(car_id);
            bsm->setCarSpeed(curSpeed.length());
            bsm->setSenderCalDensity(density_own);
            bsm->setSenderFlow(rcv_flow_avg);
        }
        else {
            bsm->setCarid(car_id);
            bsm->setCarSpeed(sybil_params.fake_speed);
            Coord fake_speed_coord(sybil_params.fake_speed, sybil_params.fake_speed);
            bsm->setSenderSpeed(fake_speed_coord);
            bsm->setSenderCalDensity(sybil_params.fake_density);
            bsm->setSenderFlow(sybil_params.fake_flow);
        }
        sendDown(bsm);
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
    if (!is_malicious)
    {
        bsm->setCarid(car_id);
        bsm->setCarSpeed(curSpeed.length());
        bsm->setSenderCalDensity(density_own);
        bsm->setSenderFlow(rcv_flow_avg);
    }
    else
    {
        bsm->setCarid(car_id);
        bsm->setCarSpeed(sybil_params.fake_speed);
        Coord fake_speed_coord(sybil_params.fake_speed, sybil_params.fake_speed);
        bsm->setSenderSpeed(fake_speed_coord);
        bsm->setSenderCalDensity(sybil_params.fake_density);
        bsm->setSenderFlow(sybil_params.fake_flow);
    }
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

void TraCIDemo11p::dataHandle(int time)
{
    double rcv_speed_sum = 0;
    double rcv_flow_sum = 0;
    density_own = getCurrentDensity(neighbors_number, radir);
    if (is_with_defence && attack_flag == true) {
        if (is_open_debug) {
            debugfile << "new start car_id: " << car_id << " is_with_defence: " << is_with_defence << std::endl;
        }
        // calc lof
        lof(cur_points, k_nearest_neighors, minpts);
        int cnt = 0;
        for (int i = 0; i < cur_points.size(); i++) {
            if (is_open_debug) {
                // record debug data
                debugfile << "neighors Point: " << i << " Id: " << cur_points[i].id << " Time: " << time << " Speed: " << cur_points[i].speed
                          << " Density: " << cur_points[i].senderCalDensity << " Flow: " << cur_points[i].senderFlow
                          << " lrd: " << cur_points[i].lrd << " lof: " << cur_points[i].lof;
            }
            if (cur_points[i].lof > lof_threshold) {
                // abormal
                if (is_open_debug) {
                    debugfile << " abormal car" << std::endl;
                }
                ta.reportNodeStatus(cur_points[i].id, true);
            }
            else {
                // normal
                if (is_open_debug) {
                    debugfile << " normal car" << std::endl;
                }
                rcv_speed_sum += cur_points[i].speed;
                cnt++;
            }
        }
        if (cnt != 0) {
            rcv_speed_avg = rcv_speed_sum / cnt;
        }
        for (int i = 0; i < cur_points.size(); i++) {
            if (cur_points[i].lof < lof_threshold) {
                rcv_flow_sum += rcv_speed_avg * cur_points[i].senderCalDensity;
            }
        }
        rcv_flow_avg = rcv_flow_sum / cnt;
        flow_own = rcv_speed_avg * density_own;
    }
    else {
        if (is_open_debug) {
            debugfile << "new start car_id: " << car_id << std::endl;
        }
        for (int i = 0; i < cur_points.size(); i++) {
            if (is_open_debug) {
                // record debug data
                debugfile << "neighors Point: " << i << " Id: " << cur_points[i].id << " Time: " << time << " Speed: " << cur_points[i].speed
                          << " Density: " << cur_points[i].senderCalDensity << " Flow: " << cur_points[i].senderFlow << " distance: " << cur_points[i].senderPos.distance(curPosition) << std::endl;
            }
            rcv_speed_sum += cur_points[i].speed;
        }
        if (cur_points.size() != 0) {
            rcv_speed_avg = rcv_speed_sum / cur_points.size();
        }
        for (int i = 0; i < cur_points.size(); i++) {
            rcv_flow_sum += rcv_speed_avg * cur_points[i].senderCalDensity;
        }
        rcv_flow_sum = rcv_speed_avg / cur_points.size();
        flow_own = rcv_speed_avg * density_own;
    }
}

double TraCIDemo11p::getCurrentDensity(int K, double r)
{
    double Dk = 0;
    double rho = 0;
    std::vector<std::pair<double, int>> distances;
    int i = 0;
    // 计算点P到其他点的距离
    for (int j = 0; j < cur_points.size(); j++) {
        double distance = cur_points[j].senderPos.distance(curPosition);
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
    rho = (K / (M_PI * Dk)) * 10;
    // rho = Dk / 5000;
    debugfile << "DK " << Dk << " K: "  <<  K << " rho: " << rho << std::endl;
    return rho;
}