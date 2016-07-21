/*
 *  Copyright (C) 1997-2015 JDE Developers Team
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see http://www.gnu.org/licenses/.
 *  Authors :
 *       Victor Arribas Raigadas <v.arribas.urjc@gmai.com>
 */


#include "turtlebot/turtlebotsensors.hh"


using namespace turtlebot;
using namespace gazebo::physics;
using namespace gazebo::sensors;

TurtlebotSensors::TurtlebotSensors(){
    ONDEBUG_INFO(std::cout << _log_prefix << "TurtlebotSensors::TurtlebotSensors()" << std::endl;)
}

TurtlebotSensors::~TurtlebotSensors(){
    ONDEBUG_INFO(std::cout << _log_prefix << "TurtlebotSensors::~TurtlebotSensors()" << std::endl;)
}

void
TurtlebotSensors::Load(ModelPtr model){
    this->model = model;
    this->base_link_id = model->GetChildLink("rack")->GetId();

    SensorManager *sm = SensorManager::Instance();

    for (SensorPtr s: sm->GetSensors()){
//        if (s->GetParentId() != base_link_id) continue;
        std::string name = s->GetName();
        if (name.find("laser") != std::string::npos)
            laser = boost::static_pointer_cast<RaySensor>(s);
        if (name.find("left") != std::string::npos)
            cam[CAM_LEFT] = boost::static_pointer_cast<CameraSensor>(s);
        if (name.find("right") != std::string::npos)
            cam[CAM_RIGHT] = boost::static_pointer_cast<CameraSensor>(s);
    }

    //Pose3d
}


void
TurtlebotSensors::Init(){
    for (int id=0; id<NUM_CAMS; id++){
        if (cam[id]){
            sub_cam[id] = cam[id]->ConnectUpdated(
                boost::bind(&TurtlebotSensors::_on_cam, this, id));
        }else
            std::cerr << _log_prefix << "\t cam["<<id<<"] was not connected (NULL pointer)" << std::endl;
    }

    if (laser){
        sub_laser = laser->ConnectUpdated(
            boost::bind(&TurtlebotSensors::_on_laser, this));
    }else
        std::cerr << _log_prefix << "\t laser was not connected (NULL pointer)" << std::endl;

    //pose3d
    boost::bind(&TurtlebotSensors::_on_pose, this);
}

void
TurtlebotSensors::debugInfo(){
    std::cout << _log_prefix << "Sensors of " << model->GetName() << std::endl;
    boost::format fmt(_log_prefix+"\t%1% (id: %2%)\n");

    std::cout << fmt % cam[CAM_LEFT]->GetName() % cam[CAM_LEFT]->GetId();
    std::cout << fmt % cam[CAM_RIGHT]->GetName() % cam[CAM_RIGHT]->GetId();
    std::cout << fmt % laser->GetName() % laser->GetId();
}

void
TurtlebotSensors::_on_cam(int id){
    //// Assumption: Camera (raw) data is constant, so after Camera boostrap
    /// Camera.data will be a constant pointer.
    /// Therefore, we can use the cv::Mat constructor to simply wrap this
    /// data and avoid copy overload.
    /// Warning: thread safe is not supplied, but one can never ensure it
    /// if CameraSensor thread is overriding data and there are no option
    /// to intercept this update.
    /// Warning 2: char* is NULL until CameraSensor.OnUpdate(), so it is not
    /// possible to bootstrap at Load neither Init step.

    std::cout << "ON CAM" << std::endl;
    if (img[id].empty()){
        std::cout << "ON CAM1" << std::endl;

        const unsigned char *data = cam[id]->GetImageData();
        std::cout << data << std::endl;
        if (data == 0)
            return;
        std::cout << "ON CAM2" << std::endl;

        std::cout <<  _log_prefix << "\tbootstrap cam["<<id<<"]" << std::endl;
        uint32_t h = cam[id]->GetImageHeight();
        uint32_t w = cam[id]->GetImageWidth();
        img[id] = cv::Mat(h, w, CV_8UC3, (uint8_t*)data);
        std::cout << "ON CAM3" << std::endl;


        cam[id]->DisconnectUpdated(sub_cam[id]); // close connection
        std::cout << "ON CAM4" << std::endl;

    }
    std::cout << "END ON CAM" << std::endl;
}


void
TurtlebotSensors::_on_laser(){
    /*assert(laser->GetRangeCount() > 0);
    std::vector<double> ranges(laser->GetRangeCount());
    laser->GetRanges(ranges);
    std::sort(ranges.begin(), ranges.end());
    //altitude = ranges[0];
    int c = std::ceil(ranges.size()*0.20); // smooth value by take 20% of minor values
    ranges.resize(c);*/

    std::cout << "ON LASER" << std::endl;
    //laser values
    MultiRayShapePtr laserV = this->laser->GetLaserShape();


    laserValues.resize(laserV->GetSampleCount ());
    for (int i = 0; i< laserV->GetSampleCount (); i++){
        laserValues[i] = laserV->GetRange(i);
    }
    std::cout << "END ON LASER" << std::endl;

}

void
TurtlebotSensors::_on_pose(){
std::cout << "ON POSE" << std::endl;
    pose = model->GetWorldPose();
    std::cout << "END ON POSE" << std::endl;

}
