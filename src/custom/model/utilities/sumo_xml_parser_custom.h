#ifndef SUMO_XML_PARSER_CUSTOM_H
#define SUMO_XML_PARSER_CUSTOM_H

#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <libxml/tree.h>
#include <string>

namespace ns3 {

  int XML_rou_count_vehicles(std::string path);
}

#endif
