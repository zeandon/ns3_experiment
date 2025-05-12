/*
* SPDX-License-Identifier: GPL-2.0-only
*/
#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/wifi-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("ScratchSimulator");

/* 该函数实现了周期性计算并输出网络应用的吞吐量 */
void ThroughputMonitor(Ptr<PacketSink> sink, double startTime, double interval)
{
    // 计算吞吐量
    static double lastTotalRx = 0;
    double currentTotalRx = sink->GetTotalRx();
    double throughput = (currentTotalRx - lastTotalRx) * 8 / (interval * 1e6); // Mbps
    lastTotalRx = currentTotalRx;
    // 日志输出
    NS_LOG_UNCOND(Simulator::Now().GetSeconds() << "s: Throughput = " << throughput << " Mbps");
    Simulator::Schedule(Seconds(interval), &ThroughputMonitor, sink, startTime, interval);
}

/* 该函数实现了实时计算并记录 WiFi 接收数据包时的信噪比（SNR）​​ */
void SnrCallback(Ptr<const Packet> packet, unsigned short channelFrequencyMhz, WifiTxVector txVector, MpduInfo mpduInfo, SignalNoiseDbm signalNoise, unsigned short channelWidth)
{
    // 计算 SNR
    double snr = signalNoise.signal - signalNoise.noise;
    // 日志输出
    NS_LOG_UNCOND(Simulator::Now().GetSeconds() << "s: SNR = " << snr << " dB");
}

/* 该函数实现了 WiFi 物理层接收数据时的信噪比（SNR）监控​​ */
void SnrMonitor(Ptr<NetDevice> device)
{
    // 通过动态类型转换获取 WifiNetDevice 对象（可以实现验证传入的device是否为Wi-Fi设备）
    Ptr<WifiNetDevice> wifiDevice = DynamicCast<WifiNetDevice>(device);
    if (wifiDevice)
    {
        // 从Wi-Fi设备中获取物理层对象
        Ptr<WifiPhy> phy = wifiDevice->GetPhy();
        // 连接到物理层的接收数据回调函数
        phy->TraceConnectWithoutContext("MonitorSnifferRx", MakeCallback(&SnrCallback));
    }
}

int main(int argc, char* argv[])
{
    // 设置基本参数
    double simuTime = 35.0;
    double velocity = 2.0;  // 速度，单位：米/秒
    double interval = 1.0;  // 吞吐量监控的时间间隔（秒）

    // 创建AP和STA节点
    NodeContainer wifiApNodes;
    wifiApNodes.Create(1);
    NodeContainer wifiStaNodes;
    wifiStaNodes.Create(1);

    // 配置WiFi信道和物理层
    YansWifiPhyHelper phy;
    // 利用YansWifiChannelHelper进行物理层信道设置
    YansWifiChannelHelper channel;
    // 选择 Friis 自由空间传播损耗模型进行仿真，并设置该模型依赖的Frequency参数
    channel.AddPropagationLoss("ns3::FriisPropagationLossModel",
                            "Frequency", DoubleValue(2412 * 1e6));
    // 选择双射线地面反射模型进行仿真，并设置该模型依赖的发射接收天线高度参数
    // channel.AddPropagationLoss(
    //     "ns3::TwoRayGroundPropagationLossModel",
    //     "HeightAboveZ", DoubleValue(1),          // 发射天线高度
    //     "HeightAboveZ", DoubleValue(2)           // 接收天线高度
    //     );
    // 选择对数距离路径损耗模型
    // channel.AddPropagationLoss(
    //     "ns3::LogDistancePropagationLossModel",
    //     "Exponent", DoubleValue(3.0),              // 路径损耗指数 n=3（典型城市环境）
    //     "ReferenceDistance", DoubleValue(1.0),     // 参考距离 d0=1m
    //     "ReferenceLoss", DoubleValue(46.7)         // PL0=46.7dB（2.4GHz Friis模型在d0=1m的损耗）
    // );
    // 设置传播时延模型为光速传播
    channel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
    // 在phyhelper里创建信道实例
    phy.SetChannel(channel.Create());
    // 设置物理层占用的{子信道，带宽，频段，主信道索引}
    phy.Set("ChannelSettings", StringValue("{0, 20, BAND_2_4GHZ, 0}"));

    // 配置WiFi MAC层和设备
    WifiHelper wifi;
    wifi.SetStandard(WIFI_STANDARD_80211ax);
    // 设置速率控制算法
    wifi.SetRemoteStationManager("ns3::IdealWifiManager");
    // 设置AP和STA的WiFi MAC类型
    WifiMacHelper wifiMac;
    Ssid ssid = Ssid("AP");
    wifiMac.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid));
    NetDeviceContainer staDevices = wifi.Install(phy, wifiMac, wifiStaNodes);
    wifiMac.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssid));
    NetDeviceContainer apDevices = wifi.Install(phy, wifiMac, wifiApNodes);

    // 设置位置与移动模型
    MobilityHelper mobility;
    Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator>();
    positionAlloc->Add(Vector(0.0, 0.0, 0.0)); // AP位置
    positionAlloc->Add(Vector(5.0, 0.0, 0.0)); // STA初始位置
    mobility.SetPositionAllocator(positionAlloc);
    // 设置AP和STA的移动模型（AP不动，STA匀速运动）
    mobility.SetMobilityModel("ns3::ConstantVelocityMobilityModel");
    mobility.Install(wifiApNodes);
    mobility.Install(wifiStaNodes);
    // 通过动态类型转换获取 STA 节点的 ConstantVelocityMobilityModel 对象，设置STA的速度
    Ptr<ConstantVelocityMobilityModel> mob =
        wifiStaNodes.Get(0)->GetObject<ConstantVelocityMobilityModel>();
    mob->SetVelocity(Vector(velocity, 0.0, 0.0));

    // 配置IP协议栈
    InternetStackHelper stack;
    stack.Install(wifiApNodes);
    stack.Install(wifiStaNodes);
    // 设置 STA 的 IP 地址
    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = address.Assign(NetDeviceContainer(apDevices, staDevices));

    // 配置应用程序
    uint16_t port = 9;
    Address sinkAddress(InetSocketAddress(interfaces.GetAddress(1), port));
    // 设置接收应用程序
    PacketSinkHelper sinkHelper("ns3::UdpSocketFactory", sinkAddress);
    ApplicationContainer sinkApp = sinkHelper.Install(wifiStaNodes.Get(0));
    sinkApp.Start(Seconds(0.5));        // 0.5s后开始接收数据
    sinkApp.Stop(Seconds(simuTime));    // 接收数据直到仿真结束
    // 设置发送应用程序
    OnOffHelper onOffHelper("ns3::UdpSocketFactory", sinkAddress);
    onOffHelper.SetConstantRate(DataRate("300Mb/s"), 1420);
    onOffHelper.SetAttribute("StartTime", TimeValue(Seconds(0.5)));
    onOffHelper.SetAttribute("StopTime", TimeValue(Seconds(simuTime)));
    ApplicationContainer sourceApp = onOffHelper.Install(wifiApNodes.Get(0));

    // 监控吞吐量
    Ptr<PacketSink> sink = DynamicCast<PacketSink>(sinkApp.Get(0));
    Simulator::Schedule(Seconds(interval), &ThroughputMonitor, sink, 0.5, interval);
    // 监控信噪比
    Ptr<WifiPhyStateHelper> phyStateHelper = CreateObject<WifiPhyStateHelper>();
    SnrMonitor(apDevices.Get(0));
    
    // 运行仿真
    Simulator::Stop(Seconds(simuTime));
    Simulator::Run();
    Simulator::Destroy();
    return 0;
}
