#ifndef PLEXE_UTILS_H
#define PLEXE_UTILS_H

#include <string>
#include <cctype>
#include <vector>
#include <sstream>

#include "plexe.h"

namespace ns3{
    void add_vehicles(Plexe& plexe, int platoonID, int nVehicles, int position);
    void add_platooning_vehicle(Plexe& plexe, std::string vid, int position, int lane,
                                double speed, int cacc_space, std::string vtype);
}

#endif // PLEXE_UTILS_H