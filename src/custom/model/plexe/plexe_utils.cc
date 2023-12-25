#include "plexe_utils.h"

namespace ns3{
    void add_vehicles(Plexe& plexe, int platoonID, int nVehicles, int position) {
        int DISTANCE = 10;
        int LENGTH = 7;
        int SPEED = 25;

        int CACC = 2;
        std::string leaderID;
        for (int i = 0; i < nVehicles; i++) {
            std::string vid = "platoon." + std::to_string(platoonID) + "." + std::to_string(i);
            add_platooning_vehicle(plexe, vid, position - i * (DISTANCE + LENGTH), 
                                    0, SPEED, DISTANCE, "tru_platoon");
            plexe.set_fixed_lane(vid, 0, false);
            plexe.vehicle.setSpeedMode(vid, 0);
            
            // if (i == 0){
            //     leaderID = vid;
            //     plexe.set_active_controller(vid, 0);
            //     plexe.enable_auto_lane_changing(leaderID, true);
            // } else {
            //     plexe.set_active_controller(vid, CACC);
            //     plexe.enable_auto_feed(vid, true, leaderID, "platoon." + std::to_string(platoonID) + "." + std::to_string(i - 1));
            //     plexe.add_member(leaderID, vid, i);
            // }
        }
    }

    void add_platooning_vehicle(Plexe& plexe, std::string vid, int position, int lane,
                                double speed, int cacc_space, std::string vtype) {
        std::string routeID = "platoon_route";
        std::string departPos = std::to_string(position);
        std::string departSpeed = "25";
        std::string departLane = "0";
        libsumo::TraCIColor orange;
        orange.r = 255;
        orange.g = 165;
        orange.b = 0;
        orange.a = 255;

        plexe.vehicle.add(vid, routeID, vtype, "now", departLane, departPos, departSpeed);
        plexe.set_path_cacc_parameters(vid, cacc_space, 2, 1, 0.5);
        plexe.set_cc_desired_speed(vid, speed);
        plexe.vehicle.setColor(vid, orange);
    }
}