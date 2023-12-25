#include "plexe.h"

namespace ns3{
    Plexe::Plexe(TraCIAPI& traci) : vehicle(traci){
        lane_changes = {};
    }

    void Plexe::_set_par(const std::string& vid, const std::string& par, const std::string& value) {
        /*
        Shorthand for the setParameter method
        :param vid: vehicle id
        :param par: parameter name
        :param value: numeric or string value for the parameter
        */
        vehicle.setParameter(vid, "carFollowModel." + par, value);
    }

    void Plexe::_set_lane_change_mode(const std::string& vid, bool safe, bool fixed) {
        if (!fixed) {
            vehicle.setLaneChangeMode(vid, DEFAULT_LC);
        } else {
            if (safe) {
                vehicle.setLaneChangeMode(vid, FIX_LC);
            } else {
                vehicle.setLaneChangeMode(vid, FIX_LC_AGGRESSIVE);
            }
        }
    }

    void Plexe::_change_lane(const std::string& vid, int current, int direction, bool safe) {
        if (safe) {
            _set_lane_change_mode(vid, safe, true);
            vehicle.changeLane(vid, current + direction, 0);
        } else {
            std::pair<int, int> result = vehicle.getLaneChangeState(vid, direction);
            int state1 = result.first;
            if (state1 && (LCA_OVERLAPPING == 0)){
                _set_lane_change_mode(vid, safe, true);
                vehicle.changeLane(vid, current + direction, 0);
                auto [lane, safe, wait] = lane_changes[vid];
                lane_changes[vid] = std::make_tuple(lane, safe, true);
            }
        }
    }

    void Plexe::step() {
        /*
        Perform a step in the simulation
        :param step: the step number
        */
        std::vector<std::string> satisfied = {};
        for (auto& kv : lane_changes) {
            std::string vid = kv.first;
            auto [lane, safe, wait] = kv.second;
            if (wait) {
                lane_changes[vid] = std::make_tuple(lane, safe, false);
                continue;
            }
            int current = vehicle.getLaneIndex(vid);
            int n_lanes = lane - current;
            int direction;
            if (n_lanes > 0) {
                direction = 1;
            } else if (n_lanes < 0) {
                direction = -1;
            } else {
                direction = 0;
            }
            if (direction == 0){
                satisfied.push_back(vid);
                _set_lane_change_mode(vid, safe, true);
            } else {
                _change_lane(vid, current, direction, safe);
            }
        }
        for (const std::string& vid : satisfied) {
            lane_changes.erase(vid);
        }
    }

    void Plexe::set_cc_desired_speed(const std::string& vid, double speed) {
        /*
        Set the desired speed for the CC controller
        :param vid: vehicle id
        :param speed: desired speed
        */
        _set_par(vid, PAR_CC_DESIRED_SPEED, std::to_string(speed));
    }

    void Plexe::set_active_controller(const std::string& vid, int controller) {
        /*
        Set the active controller for the vehicle
        :param vid: vehicle id
        :param controller: controller id
        */
        _set_par(vid, PAR_ACTIVE_CONTROLLER, std::to_string(controller));
    }

    void Plexe::set_fixed_lane(const std::string& vid, int lane, bool safe) {
        /*
        Set the fixed lane for the vehicle
        :param vid: vehicle id
        :param lane: fixed lane
        :param safe: if true, the vehicle will change lane only if it is safe to do so
        */
        lane_changes[vid] = std::make_tuple(lane, safe, false);
    }

    void Plexe::set_path_cacc_parameters(const std::string& vid, double distance, double xi, double omega_n, double c1) {
        /*
        Set the parameters for the path CACC controller
        :param vid: vehicle id
        :param distance: desired distance to the leader
        :param xi: xi parameter
        :param omega_n: omega_n parameter
        :param c1: c1 parameter
        */
        if (distance >= 0) {
            _set_par(vid, PAR_CACC_SPACING, std::to_string(distance));
        }
        if (xi >= 0) {
            _set_par(vid, CC_PAR_CACC_XI, std::to_string(xi));
        }
        if (omega_n >= 0) {
            _set_par(vid, CC_PAR_CACC_OMEGA_N, std::to_string(omega_n));
        }
        if (c1 >= 0) {
            _set_par(vid, CC_PAR_CACC_C1, std::to_string(c1));
        }
    }

    void Plexe::enable_auto_feed(const std::string& vid, bool enable, const std::string& leader_id, const std::string& front_id) {
        /*
        Enable or disable the auto feed feature
        :param vid: vehicle id
        :param enable: if true, the auto feed feature is enabled
        :param leader_id: the id of the leader vehicle
        :param front_id: the id of the front vehicle
        */
        if (enable) {
            _set_par(vid, PAR_USE_AUTO_FEEDING, pack(std::vector<std::string>{"1", leader_id, front_id}));
        } else {
            _set_par(vid, PAR_USE_AUTO_FEEDING, "0");
        }
    }

    void Plexe::add_member(const std::string& vid, const std::string& member_id, int position) {
        /*
        Add a member to the platoon
        :param vid: vehicle id
        :param member_id: the id of the member to add
        :param position: the position of the member in the platoon
        */
        _set_par(vid, PAR_ADD_MEMBER, pack(std::vector<std::string>{member_id, std::to_string(position)}));
    }

    void Plexe::enable_auto_lane_changing(const std::string& vid, bool enable) {
        /*
        Enable or disable the auto lane changing feature
        :param vid: vehicle id
        :param enable: if true, the auto lane changing feature is enabled
        */
        if (enable) {
            _set_par(vid, PAR_ENABLE_AUTO_LANE_CHANGE, "1");
        } else {
            _set_par(vid, PAR_ENABLE_AUTO_LANE_CHANGE, "0");
        }
    }
}