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
 *  Carlos Mateo Risma Carletti, Politecnico di Torino (carlosrisma@gmail.com)
*/
#include "stopWarningClient.h"

#include "ns3/CAM.h"
#include "ns3/DENM.h"
#include "ns3/vdpTraci.h"
#include "ns3/socket.h"
#include "ns3/network-module.h"



namespace ns3
{
  NS_LOG_COMPONENT_DEFINE("stopWarningClient");

  NS_OBJECT_ENSURE_REGISTERED(stopWarningClient);

  TypeId
  stopWarningClient::GetTypeId (void)
  {
    static TypeId tid =
        TypeId ("ns3::stopWarningClient")
        .SetParent<Application> ()
        .SetGroupName ("Applications")
        .AddConstructor<stopWarningClient> ()
        .AddAttribute ("PrintSummary",
            "To print summary at the end of simulation",
            BooleanValue(false),
            MakeBooleanAccessor (&stopWarningClient::m_print_summary),
            MakeBooleanChecker ())
        .AddAttribute ("RealTime",
            "To compute properly timestamps",
            BooleanValue(false),
            MakeBooleanAccessor (&stopWarningClient::m_real_time),
            MakeBooleanChecker ())
        .AddAttribute ("CSV",
            "CSV log name",
            StringValue (),
            MakeStringAccessor (&stopWarningClient::m_csv_name),
            MakeStringChecker ())
        .AddAttribute ("ServerAddr",
            "Ip Addr of the server",
            Ipv4AddressValue("10.0.0.1"),
            MakeIpv4AddressAccessor (&stopWarningClient::m_server_addr),
            MakeIpv4AddressChecker ())
        .AddAttribute ("Client",
            "TraCI client for SUMO",
            PointerValue (0),
            MakePointerAccessor (&stopWarningClient::m_client),
            MakePointerChecker<TraciClient> ())
        .AddAttribute ("PRRSupervisor",
            "PRR Supervisor to compute PRR according to 3GPP TR36.885 V14.0.0 page 70",
            PointerValue (0),
            MakePointerAccessor (&stopWarningClient::m_PRR_supervisor),
            MakePointerChecker<PRRSupervisor> ())
        .AddAttribute ("SendCAM",
            "To enable/disable the transmission of CAM messages",
            BooleanValue(true),
            MakeBooleanAccessor (&stopWarningClient::m_send_cam),
            MakeBooleanChecker ());
        return tid;
  }

  stopWarningClient::stopWarningClient ()
  {
    NS_LOG_FUNCTION(this);

    m_client = nullptr;
    m_print_summary = true;
    m_already_print = false;
    m_cam_sent = 0;
    m_denm_received = 0;
  }

  stopWarningClient::~stopWarningClient ()
  {
    //m_denService.cleanup();
    NS_LOG_FUNCTION(this);
  }

  void
  stopWarningClient::DoDispose (void)
  {
    NS_LOG_FUNCTION(this);
    Application::DoDispose ();
  }

  void
  stopWarningClient::StartApplication (void)
  {
    NS_LOG_FUNCTION(this);

    /*
     * This application works as client for the areaSpeedAdvisorServer80211p. It is intended to be installed over a vehicular OBU node,
     * and it is set to generate broadcast CAM messages on top of BTP and GeoNet.
     * As soon as a DENM is received, it reads the information inside the RoadWorks container
     * and sets the speed accordingly (see receiveDENM() function)
     */

    m_id = m_client->GetVehicleId (this->GetNode ());

    /* Create the socket for TX and RX */
    TypeId tid = TypeId::LookupByName ("ns3::PacketSocketFactory");

    /* Socket used to send CAMs and receive DENMs */
    m_socket = Socket::CreateSocket (GetNode (), tid);

    /* Bind the socket to local address */
    PacketSocketAddress local;
    local.SetSingleDevice (GetNode ()->GetDevice (0)->GetIfIndex ());
    local.SetPhysicalAddress (GetNode ()->GetDevice (0)->GetAddress ());
    local.SetProtocol (0x8947);
    if (m_socket->Bind (local) == -1)
    {
      NS_FATAL_ERROR ("Failed to bind client socket");
    }
    /* Create new BTP and GeoNet objects*/
    m_btp = CreateObject <btp>();
    m_geoNet = CreateObject <GeoNet>();

    if(m_PRR_supervisor!=nullptr)
    {
      m_geoNet->setPRRSupervisor(m_PRR_supervisor);
    }

    m_btp->setGeoNet(m_geoNet);

    /*background vehicle's id is Veh_bd0, platoon's is "flow_platoon"
     *according to vehicleId to decide whether to send CAMs or not
     */
    unsigned long fixed_stationid = std::stol(extractDigits(m_id));
    if (m_id.find("platoon") != std::string::npos) //not background vehicle
    {
      /* Set sockets, callback and station properties in DENBasicService */
      m_denService.setBTP(m_btp);
      m_denService.setStationProperties (fixed_stationid, StationType_passengerCar);
      m_denService.addDENRxCallback (std::bind(&stopWarningClient::receiveDENM,this,std::placeholders::_1,std::placeholders::_2));
      m_denService.setRealTime (m_real_time);
      m_denService.setSocketRx (m_socket);

      VDP* traci_vdp = new VDPTraCI(m_client,m_id);
      m_denService.setVDP(traci_vdp);
    } 
    // else if (m_id.find("Un") != std::string::npos)
    else if (m_id.find("ramp") != std::string::npos)
    {                                  //background vehicle
      /* Set the socket to broadcast */
      PacketSocketAddress remote;
      remote.SetSingleDevice (GetNode ()->GetDevice (0)->GetIfIndex ());
      remote.SetPhysicalAddress (GetNode ()->GetDevice (0)->GetBroadcast ());
      remote.SetProtocol (0x8947);

      m_socket->Connect(remote);

      /* Set BTP and GeoNet objects in DENBasicService and CABasicService */
      m_denService.setBTP(m_btp);
      m_caService.setBTP(m_btp);

      /* Set sockets, callback and station properties in DENBasicService */
      m_denService.setStationProperties (fixed_stationid, StationType_passengerCar);
      // m_denService.addDENRxCallback (std::bind(&stopWarningClient::receiveDENM,this,std::placeholders::_1,std::placeholders::_2));
      m_denService.setRealTime (m_real_time);
      m_denService.setSocketRx (m_socket);

      /* Set sockets, callback, station properties and TraCI VDP in CABasicService */
      m_caService.setSocketTx (m_socket);
      m_caService.setSocketRx (m_socket);
      m_caService.addCARxCallback (std::bind(&stopWarningClient::receiveCAM,this,std::placeholders::_1,std::placeholders::_2));
      m_caService.setStationProperties (fixed_stationid, StationType_passengerCar);
      m_caService.setRealTime (m_real_time);
      VDP* traci_vdp = new VDPTraCI(m_client,m_id);

      m_caService.setVDP(traci_vdp);
      m_denService.setVDP(traci_vdp);
      
      /* Schedule CAM dissemination */
      if(m_send_cam == true)
      {
        std::srand(Simulator::Now().GetNanoSeconds ());
        double desync = ((double)std::rand()/RAND_MAX);
        m_caService.startCamDissemination(desync);
      }
    }
    else {
      /* Set the socket to broadcast */
      PacketSocketAddress remote;
      remote.SetSingleDevice (GetNode ()->GetDevice (0)->GetIfIndex ());
      remote.SetPhysicalAddress (GetNode ()->GetDevice (0)->GetBroadcast ());
      remote.SetProtocol (0x8947);

      m_socket->Connect(remote);

      /* Set BTP and GeoNet objects in DENBasicService and CABasicService */
      m_denService.setBTP(m_btp);
      m_caService.setBTP(m_btp);

      /* Set sockets, callback and station properties in DENBasicService */
      m_denService.setStationProperties (fixed_stationid, StationType_passengerCar);
      // m_denService.addDENRxCallback (std::bind(&stopWarningClient::receiveDENM,this,std::placeholders::_1,std::placeholders::_2));

      /* Set sockets, callback, station properties and TraCI VDP in CABasicService */
      m_caService.setStationProperties (fixed_stationid, StationType_passengerCar);
      m_caService.setRealTime (m_real_time);
      VDP* traci_vdp = new VDPTraCI(m_client,m_id);

      m_caService.setVDP(traci_vdp);
      m_denService.setVDP(traci_vdp);
      
      /* Schedule CAM dissemination */
      if(m_send_cam == true)
      {
        std::srand(Simulator::Now().GetNanoSeconds ());
        double desync = ((double)std::rand()/RAND_MAX);
        m_caService.startCamDissemination(desync);
      }
    }
    /* Create CSV file, if requested */
    if (!m_csv_name.empty ())
    {
      m_csv_ofstream.open (m_csv_name+"-"+m_id+".csv",std::ofstream::trunc);
      m_csv_ofstream << "messageID,originatingStationID,sequence,referenceTime,detectionTime,stationID" << std::endl;
    }
  }

  void
  stopWarningClient::StopApplication ()
  {
    NS_LOG_FUNCTION(this);
    Simulator::Cancel(m_sendCamEvent);
    Simulator::Cancel(m_denmTimeout);

    uint64_t cam_sent;
    cam_sent = m_caService.terminateDissemination ();
    m_denService.cleanup();


    if (!m_csv_name.empty ())
      m_csv_ofstream.close ();

    if (m_print_summary && !m_already_print)
    {
      std::cout << "INFO-" << m_id
                << ",CAM-SENT:" << cam_sent
                << ",DENM-RECEIVED:" << m_denm_received
                << std::endl;
      m_already_print=true;
    }
  }

  void
  stopWarningClient::StopApplicationNow ()
  {
    NS_LOG_FUNCTION(this);
    StopApplication ();
  }

  void
  stopWarningClient::receiveDENM (denData denm, Address from)
  {
    Simulator::Cancel (m_denmTimeout);
    std::cout << m_id<< " received DENM from RSU" << std::endl;
    m_denm_received++;

    // Uncomment the following line to print a line to standard output for each DENM received by a vehicle
    //std::cout << "DENM received by " << m_id << std::endl;

    /*
     * Check the speed limit saved in the roadWorks container inside
     * the optional "A la carte" container
     * The division by 3.6 is used to convert the value stored in the DENM
     * from km/h to m/s, as required by SUMO
    */
    if(!denm.getDenmAlacarteData_asn_types ().getData ().roadWorks.getData ().speedLimit.isAvailable ())
    {
      NS_FATAL_ERROR("Error in stopWarningClient.cc. Received a NULL pointer for speedLimit.");
    }


    // double speedLimit = denm.getDenmAlacarteData_asn_types ().getData ().roadWorks.getData ().speedLimit.getData ();

    // m_client->TraCIAPI::vehicle.setMaxSpeed (m_id, speedLimit/3.6);

    // /* Change color for slow-moving vehicles to green (just for visualization purpose) */
    // libsumo::TraCIColor green;
    // green.r=50;green.g=205;green.b=50;green.a=255;
    // m_client->TraCIAPI::vehicle.setColor (m_id,green);
    // char Char_car = m_id[m_id.length() - 1]; // 获取最后一个字符
    // int car = Char_car - '0'; // 转换为整数
    // if (car == 0)
    // {
    //   std::vector<std::string> all_vehicle_id = m_client->TraCIAPI::vehicle.getIDList();
    //   std::vector<std::string> ramp_id;

    //   for (const auto& x : all_vehicle_id) 
    //   {
    //     if (x.find("ramp.") != std::string::npos) 
    //     {
    //       ramp_id.push_back(x);
    //     }
    //   }
      
      
    // // Converting ramp_id vector to a tuple (C++ does not have built-in tuple support)
    //   char platoon = m_id[m_id.length() - 3];
    //   int i = platoon - '0';
    //   std::string vid_0 = "platoon." + std::to_string(i) + "." + "0";
    //   std::string vid_1 = "platoon." + std::to_string(i) + "." + "1";
    //   std::string vid_2 = "platoon." + std::to_string(i) + "." + "2";
    //   if((800<m_client->TraCIAPI::vehicle.getPosition(vid_0).x)&&(m_client->TraCIAPI::vehicle.getPosition(vid_0).x<=1000))
    //   {
    //     std::vector<std::string> ramp_current_id;
    //     std::vector<double> dis_ramp_current_list;
    //     for (const auto& x : ramp_id) 
    //     {
    //       if (m_client->TraCIAPI::vehicle.getDistance(x) < 500) 
    //       {
    //         ramp_current_id.push_back(x);
    //       }
    //     }
    //     for (const auto& veh_id : ramp_current_id) 
    //     {
    //       double distance = m_client->TraCIAPI::vehicle.getDistance(veh_id);
    //       dis_ramp_current_list.push_back(distance);
    //     }
    //     if (dis_ramp_current_list.size() != 0 && m_client->TraCIAPI::vehicle.getPosition(vid_0).x < 1000) 
    //     {
    //       double max_distance = *std::max_element(dis_ramp_current_list.begin(), dis_ramp_current_list.end());
    //       auto max_index = std::distance(dis_ramp_current_list.begin(), std::find(dis_ramp_current_list.begin(), dis_ramp_current_list.end(), max_distance));
    //       if ((0 <= ((1000 - m_client->TraCIAPI::vehicle.getPosition(vid_2).x) / m_client->TraCIAPI::vehicle.getSpeed(vid_2))- ((500 - max_distance) / m_client->TraCIAPI::vehicle.getSpeed(ramp_current_id[max_index]))) && (((1000 - m_client->TraCIAPI::vehicle.getPosition(vid_2).x) / m_client->TraCIAPI::vehicle.getSpeed(vid_2))- ((500 - max_distance) / m_client->TraCIAPI::vehicle.getSpeed(ramp_current_id[max_index]))<= 1.5)) 
    //       {
    //         // m_client->plexe.vehicle.setSpeed(ramp_current_id[max_index], m_client->plexe.vehicle.getSpeed(ramp_current_id[max_index]) - 2);
    //         m_client->plexe.vehicle.changeLane(vid_0, 1, 0);
    //         m_client->plexe.vehicle.changeLane(vid_1, 1, 0);
    //         m_client->plexe.vehicle.changeLane(vid_2, 1, 0);
    //       } 
// 若匝道车辆比编队头车先通过合流点，比较相差时间与1.5s，若小于1.5s，不够安全，匝道车辆最高速行驶
    //       else if ((-1.5 <= (((1000 - m_client->TraCIAPI::vehicle.getPosition(vid_0).x) / m_client->TraCIAPI::vehicle.getSpeed(vid_0))- ((500 - max_distance) /m_client->TraCIAPI::vehicle.getSpeed(ramp_current_id[max_index]))))&& ((((1000 - m_client->TraCIAPI::vehicle.getPosition(vid_0).x) / m_client->TraCIAPI::vehicle.getSpeed(vid_0))- ((500 - max_distance) /m_client->TraCIAPI::vehicle.getSpeed(ramp_current_id[max_index])))<= 0)) 
    //       {
    //         m_client->TraCIAPI::vehicle.setSpeed(ramp_current_id[max_index], 25);
    //       }
    //     }
    //   }
    // }
    //   double X_veh_bd = m_client->TraCIAPI::vehicle.getPosition("Un1").x;
    //   double X_vid_0 = m_client->TraCIAPI::vehicle.getPosition(m_id).x;
    //   if ((X_veh_bd - X_vid_0 < 200) && (X_veh_bd - X_vid_0 > 0) && (m_client->TraCIAPI::vehicle.getSpeed("Un1") == 0))
    //   {
    //     m_client->plexe.vehicle.changeLane(vid_0, 1, 0);
    //     m_client->plexe.vehicle.changeLane(vid_1, 1, 0);
    //     m_client->plexe.vehicle.changeLane(vid_2, 1, 0);
    //   } 
    //   // if ((X_vid_0 -  X_veh_bd>100) && (X_vid_0 -  X_veh_bd <200) )
    //   // {
    //   //   m_client->plexe.vehicle.changeLane(vid_0, 0, 0);
    //   //   m_client->plexe.vehicle.changeLane(vid_1, 0, 0);
    //   //   m_client->plexe.vehicle.changeLane(vid_2, 0, 0);
    //   // }
    // }
         

    if (!m_csv_name.empty ())
    {
      m_csv_ofstream << denm.getDenmHeaderMessageID () << ","
                     << denm.getDenmActionID ().originatingStationID << ","
                     << denm.getDenmActionID ().sequenceNumber << ","
                     << denm.getDenmMgmtReferenceTime () << ","
                     << denm.getDenmMgmtDetectionTime () << ","
                     << denm.getDenmHeaderStationID () << std::endl;
    }

    /* Start the DENM timer. If after 1.5 seconds no other DENM is received, than go back to the normal speed */
    // m_denmTimeout = Simulator::Schedule(Seconds(1.5),&stopWarningClient::denmTimeout,this);
  }

  void
  stopWarningClient::receiveCAM (asn1cpp::Seq<CAM> cam, Address from)
  {
    /* Implement CAM strategy here */
    std::cout << m_id<< "CAM received from " << from << std::endl;
    (void) cam;
    (void) from;

   // Free the received CAM data structure
//   ASN_STRUCT_FREE(asn_DEF_CAM,cam);
  }

  long
  stopWarningClient::compute_timestampIts ()
  {
    /* To get millisec since  2004-01-01T00:00:00:000Z */
    auto time = std::chrono::system_clock::now(); // get the current time
    auto since_epoch = time.time_since_epoch(); // get the duration since epoch
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(since_epoch); // convert it in millisecond since epoch

    long elapsed_since_2004 = millis.count() - TIME_SHIFT; // in TIME_SHIFT we saved the millisec from epoch to 2004-01-01
    return elapsed_since_2004;
  }

  void
  stopWarningClient::denmTimeout ()
  {
   /* If vehicle hasn't received any denm for 1.5 second, change color
    * for fast-moving vehicles to orange, and increase their speed to 75km/h */
    libsumo::TraCIColor orange;
    orange.r=255;orange.g=99;orange.b=71;orange.a=255;
    m_client->TraCIAPI::vehicle.setColor (m_id,orange);
    double speedLimit = 75/3.6;
    m_client->TraCIAPI::vehicle.setMaxSpeed (m_id,speedLimit);
  }
}





