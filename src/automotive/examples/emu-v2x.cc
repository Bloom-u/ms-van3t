/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2012 University of Washington, 2012 INRIA 
 *
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
 */
#include <fstream>
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/fd-net-device-module.h"
#include "ns3/sumo_xml_parser.h"
#include "ns3/mobility-module.h"
#include "ns3/traci-module.h"
#include "ns3/obuEmu-helper.h"
#include "ns3/emu-fd-net-device-helper.h"
#include "ns3/obuEmu.h"
#include "ns3/packet-socket-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("emu-v2x");

int
main (int argc, char *argv[])
{
  // Physical interface parameters
  std::string deviceName ("eth1");
  std::string encapMode ("Dix");

  // Simulation parameters
  std::string sumo_folder = "src/automotive/examples/sumo_files_v2v_map/";
  std::string mob_trace = "cars.rou.xml";
  std::string sumo_config ="src/automotive/examples/sumo_files_v2v_map/map.sumo.cfg";

  bool verbose = true;
  bool sumo_gui = true;
  double sumo_updates = 0.01;
  bool send_cam = true;
  bool print_summary = false;

  double emuTime = 100;

  uint16_t numberOfNodes = 0;
  uint32_t nodeCounter = 0;

  CommandLine cmd;
  xmlDocPtr rou_xml_file;

  /* Cmd Line option for vehicular application */
  cmd.AddValue ("sumo-gui", "Use SUMO gui or not", sumo_gui);
  cmd.AddValue ("sumo-updates", "SUMO granularity", sumo_updates);
  cmd.AddValue ("send-cam", "Enable car to send cam", send_cam);
  cmd.AddValue ("sumo-folder","Position of sumo config files",sumo_folder);
  cmd.AddValue ("mob-trace", "Name of the mobility trace file", mob_trace);
  cmd.AddValue ("sumo-config", "Location and name of SUMO configuration file", sumo_config);
  cmd.AddValue ("summary", "Print a summary for each vehicle at the end of the simulation", print_summary);
  cmd.AddValue ("verbose", "Enable verbose printing on stdout", verbose);
  cmd.AddValue ("interface", "Name of the physical interface to send V2X messages to", deviceName);
  cmd.AddValue ("sim-time", "Total duration of the emulation [s]", emuTime);

  cmd.Parse (argc, argv);

  /* If verbose is true, enable some additional logging */
  if (verbose)
    {
      LogComponentEnable ("emu-v2x", LOG_LEVEL_INFO);
      LogComponentEnable ("CABasicService", LOG_LEVEL_INFO);
      LogComponentEnable ("DENBasicService", LOG_LEVEL_INFO);
    }

  /* Using the real-time scheduler is mandatory when emulating vehicles */
  /* WARNING: you must be sure that the computer is capable enough to simulate
   * the desired number of vehicles, otherwise the simulation will slow down
   * and the real-time constraint will not be respected anymore (causing, for
   * instance, the vehicles to look as if they were moving more slowly and
   * sending CAMs with a lower frequency) */
  GlobalValue::Bind ("SimulatorImplementationType",
                     StringValue ("ns3::RealtimeSimulatorImpl"));

  GlobalValue::Bind ("ChecksumEnabled", BooleanValue (true));

  /* Read the SUMO mobility information and map from the .rou XML file */
  NS_LOG_INFO("Reading the .rou file...");
  std::string path = sumo_folder + mob_trace;

  /* Load the .rou.xml document */
  xmlInitParser();
  rou_xml_file = xmlParseFile(path.c_str ());
  if (rou_xml_file == NULL)
    {
      NS_FATAL_ERROR("Error: unable to parse the specified XML file: "<<path);
    }
  numberOfNodes = XML_rou_count_vehicles(rou_xml_file);

  xmlFreeDoc(rou_xml_file);
  xmlCleanupParser();

  if(numberOfNodes==0)
    {
      NS_FATAL_ERROR("Fatal error: cannot gather the number of vehicles from the specified XML file: "<<path<<". Please check if it is a correct SUMO file.");
    }
  NS_LOG_INFO("The .rou file has been read: " << numberOfNodes << " vehicles will be present in the simulation.");

  /* Set the emulation total time (in seconds) */
  NS_LOG_INFO("Simulation will last " << emuTime << " seconds");
  ns3::Time simulationTime (ns3::Seconds(emuTime));

  /* Create containers for OBUs */
  NodeContainer obuNodes;
  obuNodes.Create(numberOfNodes);

  /* Setup Mobility and position node pool */
  MobilityHelper mobility;
  mobility.Install (obuNodes);

  /* Setup Traci and start SUMO */
  Ptr<TraciClient> sumoClient = CreateObject<TraciClient> ();
  sumoClient->SetAttribute ("SumoConfigPath", StringValue (sumo_config));
  sumoClient->SetAttribute ("SumoBinaryPath", StringValue (""));    // use system installation of sumo
  sumoClient->SetAttribute ("SynchInterval", TimeValue (Seconds (sumo_updates)));
  sumoClient->SetAttribute ("StartTime", TimeValue (Seconds (0.0)));
  sumoClient->SetAttribute ("SumoGUI", (BooleanValue) sumo_gui);
  sumoClient->SetAttribute ("SumoPort", UintegerValue (3400));
  sumoClient->SetAttribute ("PenetrationRate", DoubleValue (1.0));
  sumoClient->SetAttribute ("SumoLogFile", BooleanValue (false));
  sumoClient->SetAttribute ("SumoStepLog", BooleanValue (false));
  sumoClient->SetAttribute ("SumoSeed", IntegerValue (10));
  sumoClient->SetAttribute ("SumoAdditionalCmdOptions", StringValue ("--verbose true"));
  sumoClient->SetAttribute ("SumoWaitForSocket", TimeValue (Seconds (1.0)));
  sumoClient->SetAttribute ("SumoAdditionalCmdOptions", StringValue ("--collision.action warn --collision.check-junctions --error-log=sumo-errors-or-collisions.xml"));

  /* Create the OBU application (obuEmu, see also model/obuEmu.cc/.h) */
  obuEmuHelper emuHelper;
  emuHelper.SetAttribute ("SendCam", (BooleanValue) send_cam);
  emuHelper.SetAttribute ("Client", (PointerValue) sumoClient); // pass TraciClient object for accessing sumo in application

  /* Create the FdNetDevice to send packets over a physical interface */
  EmuFdNetDeviceHelper emuDev;
  emuDev.SetDeviceName (deviceName);
  emuDev.SetAttribute ("EncapsulationMode", StringValue (encapMode));

  /* Give packet socket powers to nodes (otherwise, if the app tries to create a PacketSocket, CreateSocket will end up with a segmentation fault */
  PacketSocketHelper packetSocket;
  packetSocket.Install (obuNodes);

  /* Callback function for node creation */
  std::function<Ptr<Node> ()> setupNewEmuNode = [&] () -> Ptr<Node>
    {
      if (nodeCounter >= obuNodes.GetN())
        NS_FATAL_ERROR("Node Pool empty!: " << nodeCounter << " nodes created.");

      std::cout<<"Creating node: "<<nodeCounter<<std::endl;

      /* don't create and install the protocol stack of the node at simulation time -> take from "node pool" */
      Ptr<Node> includedNode = obuNodes.Get(nodeCounter);
      ++nodeCounter; //increment counter for next node

      /* Install FdNetDevice and set MAC address (using 00:00:00:00:00:01, 00:00:00:00:00:02, and so on, for each vehicle) */
      NetDeviceContainer fdnetContainer;
      std::ostringstream veh_mac;
      veh_mac << "00:00:00:00:00:" << std::setfill('0') << std::setw(2) << nodeCounter;
      fdnetContainer = emuDev.Install(includedNode);
      Ptr<FdNetDevice> dev = fdnetContainer.Get (0)->GetObject<FdNetDevice> ();
      dev->SetAddress (Mac48Address (veh_mac.str().c_str ()));

      std::cout<<"MAC of node "<<nodeCounter-1<<": "<<veh_mac.str()<<std::endl;

      /* Install Application */
      ApplicationContainer obuEmuApp = emuHelper.Install (includedNode);
      obuEmuApp.Start (Seconds (0.0));
      obuEmuApp.Stop (simulationTime - Simulator::Now () - Seconds (0.1));

      std::cout<<"New node: "<<nodeCounter-1<<std::endl;

      return includedNode;
    };

  /* Callback function for node shutdown */
  std::function<void (Ptr<Node>)> shutdownEmuNode = [] (Ptr<Node> exNode)
    {
      /* stop all applications */
      Ptr<obuEmu> obuEmuApp_ = exNode->GetApplication(0)->GetObject<obuEmu>();
      if(obuEmuApp_)
        obuEmuApp_->StopApplicationNow ();

       /* set position outside communication range */
      Ptr<ConstantPositionMobilityModel> mob = exNode->GetObject<ConstantPositionMobilityModel>();
      mob->SetPosition(Vector(-1000.0+(rand()%25),320.0+(rand()%25),250.0));// rand() for visualization purposes

      /* NOTE: further actions could be required for a safe shut down! */
    };

  /* Start traci client with the given function pointers */
  sumoClient->SumoSetup (setupNewEmuNode, shutdownEmuNode);

  /* Start Emulation */
  Simulator::Stop (simulationTime);

  Simulator::Run ();
  Simulator::Destroy ();
}