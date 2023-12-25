/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 * Created by:
 *  Marco Malinverno, Politecnico di Torino (marco.malinverno1@gmail.com)
 *  Francesco Raviglione, Politecnico di Torino (francescorav.es483@gmail.com)
*/

#include "sumo_xml_parser_custom.h"

namespace ns3 {
int
XML_rou_count_vehicles (std::string path)
{
    xmlDocPtr doc;
    xmlInitParser ();
    doc = xmlReadFile (path.c_str (), NULL, 0);
    if (doc == NULL)
        {
        return -1;
        }

    int num_vehicles = 0;
    int flow_number = 0;
    // 创建 XPath 上下文
    xmlXPathContextPtr xpathCtx = xmlXPathNewContext(doc);
    if (xpathCtx == NULL) {
        printf("Error: unable to create new XPath context\n");
        xmlFreeDoc(doc);
        return -1;
    }

    // 执行 XPath 表达式
    xmlXPathObjectPtr xpathObj = xmlXPathEvalExpression((const xmlChar *)"//flow[@id='flow_platoon']/@number", xpathCtx);
    if (xpathObj == NULL) {
        printf("Error: unable to evaluate xpath expression\n");
        xmlXPathFreeContext(xpathCtx);
        xmlFreeDoc(doc);
        return -1;
    }

    // 输出结果
    if (xpathObj->nodesetval->nodeNr > 0) {
        xmlNodePtr node = xpathObj->nodesetval->nodeTab[0];
        flow_number = atoi((char *)xmlNodeGetContent(node));
        printf("Flow number: %s\n", xmlNodeGetContent(node));
    }

    // 清理
    xmlXPathFreeObject(xpathObj);
    xmlXPathFreeContext(xpathCtx);
    xmlFreeDoc(doc);
    xmlCleanupParser();
  
    // add background vehicles
    num_vehicles = num_vehicles + flow_number + 1;
    return num_vehicles;
}
} // namespace ns3
