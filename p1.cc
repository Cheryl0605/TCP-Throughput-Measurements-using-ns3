#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-layout-module.h"
#include "ns3/applications-module.h"

using namespace ns3;

int main (int argc, char *argv[])
{
	
  	SeedManager::SetSeed(1);

  	uint32_t nSpokes = 8;
  	double endTime = 60.0;
  	std::string protocol = "TcpHybla";

	CommandLine cmd;
	cmd.AddValue("nSpokes", "Number of spokes to place in each star", nSpokes);
	cmd.AddValue("Protocol", "TCP Protocol to use: TcpNewReno, TcpHybla, TcpHighSpeed, TcpHtcp, TcpVegas, TcpScalable, TcpVeno, TcpBic, TcpYeah, TcpIllinois, TcpWestwood, TcpWestwoodPlus", protocol);
	cmd.Parse(argc, argv);
	
  	if(protocol == "TcpNewReno") {
    		Config::SetDefault("ns3::TcpL4Protocol::SocketType", TypeIdValue(TcpNewReno::GetTypeId()));
  	}
	else if(protocol == "TcpHybla") {
	    	Config::SetDefault("ns3::TcpL4Protocol::SocketType", TypeIdValue(TcpHybla::GetTypeId()));
	}
	else if(protocol == "TcpHighSpeed") {
	    	Config::SetDefault("ns3::TcpL4Protocol::SocketType", TypeIdValue(TcpHighSpeed::GetTypeId()));
	}
	else if (protocol =="TcpVegas")
    	{
      		Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue(TcpVegas::GetTypeId()));
  	}
  	else if (protocol =="TcpScalable")
    	{
      		Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue(TcpScalable::GetTypeId ()));
    	}
  	else if (protocol =="TcpHtcp")
    	{
      		Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue(TcpHtcp::GetTypeId ()));
    	}
  	else if (protocol =="TcpVeno")
    	{
      		Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue(TcpVeno::GetTypeId ()));
    	}
  	else if (protocol =="TcpBic")
    	{
      		Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue(TcpBic::GetTypeId ()));
    	}
  	else if (protocol =="TcpYeah")
    	{
      		Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue(TcpYeah::GetTypeId ()));
    	}
  	else if (protocol =="TcpIllinois")
    	{
      		Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue(TcpIllinois::GetTypeId ()));
	}
  	else if (protocol =="TcpWestwood")
    	{ 
      		Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue(TcpWestwood::GetTypeId ()));
      		Config::SetDefault ("ns3::TcpWestwood::FilterType", EnumValue(TcpWestwood::TUSTIN));
   	}
	else if (protocol =="TcpWestwoodPlus")
	{
      		Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue(TcpWestwood::GetTypeId ()));
      		Config::SetDefault ("ns3::TcpWestwood::ProtocolType", EnumValue(TcpWestwood::WESTWOODPLUS));
      		Config::SetDefault ("ns3::TcpWestwood::FilterType", EnumValue(TcpWestwood::TUSTIN));
    	}
	else
    	{
		std::cout << "Invalid Protocol" << std::endl;
      		exit(1);
    	}

	PointToPointHelper pointToPoint;
	pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
	pointToPoint.SetChannelAttribute("Delay", StringValue("10ms"));
	  
	PointToPointStarHelper star1(nSpokes, pointToPoint);
	PointToPointStarHelper star2(nSpokes, pointToPoint);  

	InternetStackHelper internet;
	star1.InstallStack(internet);
	star2.InstallStack(internet);

	Ipv4AddressHelper address1("10.1.1.0", "255.255.255.0");
	Ipv4AddressHelper address2("10.3.3.0", "255.255.255.0");
	Ipv4AddressHelper address3;
	address3.SetBase ("10.7.7.0", "255.255.255.0");

	star1.AssignIpv4Addresses(address1);
	star2.AssignIpv4Addresses(address2);

	PointToPointHelper pointToPointHubs;
	pointToPointHubs.SetDeviceAttribute("DataRate", StringValue("1Mbps"));
	pointToPointHubs.SetChannelAttribute("Delay", StringValue("20ms"));

	NetDeviceContainer p2pHubs;
	  
	p2pHubs = pointToPointHubs.Install(star1.GetHub(),star2.GetHub());
	  
	address3.Assign(p2pHubs);

	Address sinkLocalAddress(InetSocketAddress(Ipv4Address::GetAny(), 5000));
	PacketSinkHelper sinkHelper ("ns3::TcpSocketFactory", sinkLocalAddress);
	ApplicationContainer sinkApps;
	for(uint32_t i = 0; i < nSpokes; ++i) {
		sinkApps.Add(sinkHelper.Install(star1.GetSpokeNode(i)));
	}  

	sinkApps.Start(Seconds(1.0));

	BulkSendHelper bulkSender ("ns3::TcpSocketFactory", Address());

	ApplicationContainer sourceApps;
	for(uint32_t i = 0; i < nSpokes; ++i) 
	{
		AddressValue remoteAddress(InetSocketAddress(star1.GetSpokeIpv4Address(i), 5000));
		bulkSender.SetAttribute("Remote", remoteAddress);
		sourceApps.Add(bulkSender.Install(star2.GetSpokeNode(i)));
	}

	sourceApps.Start(Seconds(2.0));

	Ipv4GlobalRoutingHelper::PopulateRoutingTables();

	Simulator::Stop(Seconds(60.0));

	Simulator::Run ();

	uint64_t totalRx = 0;

	for(uint32_t i = 0; i < nSpokes; ++i) {

		Ptr<PacketSink> sink = DynamicCast<PacketSink>(sinkApps.Get(i));
	  	uint32_t bytesReceived = sink->GetTotalRx();
	  	totalRx += bytesReceived;
	  	std::cout << "Sink " << i << "\tTotalRx: " << bytesReceived * 1e-6 * 8 << "Mb";
	  	std::cout << "\tTroughput: " << (bytesReceived * 1e-6 * 8) / endTime << "Mbps" << std::endl;
	}

	std::cout << std::endl;
	std::cout << "Totals\tTotalRx: " << totalRx * 1e-6 * 8 << "Mb";
	std::cout << "\tThroughput: " << (totalRx * 1e-6 * 8) / endTime << "Mbps" << std::endl;

	Simulator::Destroy ();  
}
