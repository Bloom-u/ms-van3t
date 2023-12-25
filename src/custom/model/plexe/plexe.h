#ifndef PLEXE_H
#define PLEXE_H

#include <string>
#include <vector>
#include <map>
#include <tuple>

#include "ns3/sumo-TraCIAPI.h"
#include "ns3/utils.h"


namespace ns3{
    class Plexe
    {
    private:
        /* data */
        const int DEFAULT_LC = 0b011001010101;
        const int DEFAULT_NOTRACI_LC = 0b1010101010;
        const int FIX_LC = 0b1000000000;
        const int FIX_LC_AGGRESSIVE = 0b0000000000;

        const std::string CC_PAR_CACC_XI = "ccxi";
        const std::string CC_PAR_CACC_OMEGA_N = "ccon";
        const std::string CC_PAR_CACC_C1 = "ccc1";
        const std::string PAR_CACC_SPACING = "ccsp";
        const std::string PAR_CC_DESIRED_SPEED = "ccds";
        const std::string PAR_ACTIVE_CONTROLLER = "ccac";
        const std::string PAR_ACC_HEADWAY_TIME = "ccaht";
        const std::string PAR_USE_AUTO_FEEDING = "ccaf";
        const std::string PAR_ADD_MEMBER = "ccam";
        const std::string PAR_ENABLE_AUTO_LANE_CHANGE = "ccalc";
        
        std::map<std::string, std::tuple<int, bool, bool>> lane_changes;
        void _set_par(const std::string& vid, const std::string& par, const std::string& value);
        void _set_lane_change_mode(const std::string& vid, bool safe, bool fixed);
        void _change_lane(const std::string& vid, int current_lane, int direction, bool safe = true);
    public:
        TraCIAPI::VehicleScope vehicle;
        Plexe(TraCIAPI& traci);
        void step();
        void set_cc_desired_speed(const std::string& vid, double speed);
        void set_active_controller(const std::string& vid, int controller);
        void set_fixed_lane(const std::string& vid, int lane, bool safe = true);
        void set_path_cacc_parameters(const std::string& vid, double distance = -1, double xi = -1,
                                    double omega_n = -1, double c1 = -1);
        void enable_auto_feed(const std::string& vid, bool enable, const std::string& leader_id = "",
                            const std::string& front_id = "");
        void add_member(const std::string& vid, const std::string& member_id, int position);
        void enable_auto_lane_changing(const std::string& vid, bool enable);       
    }; 
}


#endif // PLEXE_H