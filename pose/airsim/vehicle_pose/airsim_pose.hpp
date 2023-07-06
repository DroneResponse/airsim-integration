#include "vehicles/multirotor/api/MultirotorRpcLibClient.hpp"

#include "vehicle_pose.hpp"

#ifndef AIRSIM_POSE_H
#define AIRSIM_POSE_H

namespace SimulatorInterface {
    
    class AirSimPose : public VehiclePose {
        public:
            /** Uses the AirSim simulation client
             * @param sim_client the airsim simulation client
            */
            AirSimPose(void *sim_client);
            ~AirSimPose();
            /** Spawns a new vehicle with the provided unique id
             * @param vehicle_id unique vehicle id within a simulation instance
             * @param pose initial pose when spawned
            */
            void spawn_vehicle(std::string vehicle_id, PoseTransfer::Pose pose);
            /** updates the vehicle pose via the AirSim client for a given unique vehicle id
             * @param pose pose to be set
             * @param vehicle_id unique id of a vehicle within the simulation
            */
            void set_vehicle_pose(PoseTransfer::Pose pose, std::string vehicle_id);
        private:
            void *sim_client;
            msr::airlib::MultirotorRpcLibClient *airsim_client;
    };
}


#endif
