#include <string>

#include "pose.hpp"

#ifndef VEHICLE_POSE_H
#define VEHICLE_POSE_H

namespace SimulatorInterface {
    class VehiclePose {
        public:
            /** Pass a pointer to the relevant simulation client. Simulator specific client details are
             * to be implemented within the public functions of this class
             * @param sim_client pointer to a simulation client
            */
            VehiclePose(void *sim_client) {};
            virtual ~VehiclePose() {};
            /** Spawns a new vehicle with the provided unique id
             * @param vehicle_id unique vehicle id within a simulation instance
            */
            virtual void spawn_vehicle(std::string vehicle_id) {};
            /** updates the vehicle pose via a simulator client for a given unique vehicle id
             * @param pose pose to be set
             * @param vehicle_id unique id of a vehicle within the simulation
            */
            virtual void set_vehicle_pose(PoseTransfer::Pose pose, std::string vehicle_id) {};
        private:
            void *sim_client;
    };
}

#endif