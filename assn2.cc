#include <fstream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"


using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("assn2");

int 
main (int argc, char *argv[])
{
	LogComponentEnable("assn2",LOG_LEVEL_ALL);
	LogComponentEnable("UdpEchoClientApplication",LOG_LEVEL_INFO);


  // Create nodes
  Ptr<Node> nSrc1 = CreateObject<Node> ();
  Ptr<Node> nSrc2 = CreateObject<Node> ();
  Ptr<Node> nRtr = CreateObject<Node> ();
  Ptr<Node> nDst = CreateObject<Node> ();

  NodeContainer nodes = NodeContainer (nSrc1, nSrc2, nRtr, nDst);

  NodeContainer nSrc1nRtr = NodeContainer(nSrc1, nRtr);
  NodeContainer nSrc2nRtr = NodeContainer(nSrc2, nRtr);
  NodeContainer nRtrnDst  = NodeContainer(nRtr, nDst);

  InternetStackHelper stack;
  stack.Install (nodes);

  // Create P2P channels
  PointToPointHelper p2p;
  p2p.SetDeviceAttribute ("DataRate", StringValue ("1Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("5ms"));
  p2p.SetQueue("ns3::DropTailQueue","MaxSize",StringValue("1500B"));

  NetDeviceContainer dSrc1dRtr = p2p.Install (nSrc1nRtr);
  NetDeviceContainer dSrc2dRtr = p2p.Install (nSrc2nRtr);
  NetDeviceContainer dRtrdDst  = p2p.Install (nRtrnDst);

  // Add IP addresses
  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer iSrc1iRtr = ipv4.Assign (dSrc1dRtr);
  ipv4.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer iSrc2iRtr = ipv4.Assign (dSrc2dRtr);
  ipv4.SetBase ("10.1.3.0", "255.255.255.0");
  Ipv4InterfaceContainer iRtriDst = ipv4.Assign (dRtrdDst);

  // Set up the routing tables
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  // Implement TCP & UDP sinks to the destinations
  uint16_t sinkPortUdp = 9090;
  Address sinkAddressUdp (InetSocketAddress (iRtriDst.GetAddress (1), sinkPortUdp));

//==========================================================================================
/* ToDo: Install packet sinks to the destinations
	  Hint: Need to install packet sinks for both TCP and UDP traffic */
	UdpEchoServerHelper echoServer(9);
  ApplicationContainer serverApps(echoServer.Install(nDst));
	PacketSinkHelper packetSinkHelperUdp ("ns3::UdpSocketFactory",InetSocketAddress(Ipv4Address::GetAny(),sinkPortUdp));
	ApplicationContainer sinkAppUdp = packetSinkHelperUdp.Install(nDst);
//==========================================================================================

  serverApps.Start (Seconds (0.));
  serverApps.Stop (Seconds (31.));
  sinkAppUdp.Start (Seconds (0.));
  sinkAppUdp.Stop (Seconds (31.));

  // Implement TCP application
  UdpEchoClientHelper echoClient(iRtriDst.GetAddress(1), 9);
  echoClient.SetAttribute("MaxPackets",UintegerValue(1000000));
  echoClient.SetAttribute("Interval",TimeValue(Seconds(0.01)));
  echoClient.SetAttribute("PacketSize",UintegerValue(1024));

  ApplicationContainer clientApps;
  clientApps.Add(echoClient.Install(nSrc1));
  clientApps.Start (Seconds (1.));
  clientApps.Stop (Seconds (30.));


//==========================================================================================
/* ToDo: Implement UDP application
	 Hint: Refer to the TCP app implementation procedure above 
	 Warning: UDP app turns on and off every 1s and use variable "udpRate" for DataRate */
	OnOffHelper onoffUdp("ns3::UdpSocketFactory",sinkAddressUdp);
	onoffUdp.SetAttribute("OnTime",StringValue("ns3::ConstantRandomVariable[Constant=1]"));
  onoffUdp.SetAttribute("OffTime",StringValue("ns3::ConstantRandomVariable[Constant=1]"));
	onoffUdp.SetAttribute("PacketSize",UintegerValue(512));
  onoffUdp.SetAttribute("DataRate",DataRateValue(10000000));
	ApplicationContainer sourceAppUdp = onoffUdp.Install(nSrc2);
	sourceAppUdp.Start (Seconds (1.));
	sourceAppUdp.Stop (Seconds (30.));
//==========================================================================================

  Simulator::Stop (Seconds (30));
  Simulator::Run ();
  Simulator::Destroy ();



}
