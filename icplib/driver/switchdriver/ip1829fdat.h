#ifndef IP1829FDAT_H
#define IP1829FDAT_H
#include "ip1829.h"

extern int (*func_of_common_smi[21])(void *cdata, int len);
extern int (*func_of_common_cap[NUM_COMMON_CAP])(void *cdata, int len);
extern int (*func_of_common_lut[NUM_COMMON_LUT])(void *cdata, int len);
extern int (*func_of_common_sniffer[8])(void *cdata, int len);
extern int (*func_of_common_storm[10])(void *cdata, int len);
extern int (*func_of_common_eoc[5])(void *cdata, int len);
extern int (*func_of_common_ld[9])(void *cdata, int len);
extern int (*func_of_common_wol[12])(void *cdata, int len);
extern int (*func_of_common_stag[6])(void *cdata, int len);
extern int (*func_of_common_ptp[NUM_COMMON_PTP])(void *cdata, int len);
extern int (*func_of_common_misc[11])(void *cdata, int len);
extern int (*func_of_common_vlan[NUM_COMMON_VLAN])(void *cdata, int len);
extern int (*func_of_common_stp[2])(void *cdata, int len);
extern int (*func_of_common_lacp[6])(void *cdata, int len);
extern int (*func_of_common_imp[8])(void *cdata, int len);
extern int (*func_of_common_cos[NUM_COMMON_TCPUDP])(void *cdata, int len);
extern int (*func_of_common_bandwidth[NUM_COMMON_BANDWIDTH])(void *cdata, int len);
extern int (*func_of_ip1829_sniffer[12])(void *cdata, int len);
extern int (*func_of_ip1829_storm[4])(void *cdata, int len);
extern int (*func_of_ip1829_eoc[3])(void *cdata, int len);
extern int (*func_of_ip1829_ld[12])(void *cdata, int len);
extern int (*func_of_ip1829_wol[6])(void *cdata, int len);
extern int (*func_of_ip1829_misc[15])(void *cdata, int len);
extern int (*func_of_ip1829_stp[8])(void *cdata, int len);
extern int (*func_of_ip1829_lacp[4])(void *cdata, int len);
extern int (*func_of_ip1829_lut[NUM_1829_LUT])(void *cdata, int len);
extern int (*func_of_ip1829_igmp[20])(void *cdata, int len);
extern int (*func_of_ip1829_imp[2])(void *cdata, int len);
extern int (*func_of_ip1829_vlan[NUM_IP1829_VLAN])(void *cdata, int len);
extern int (*func_of_ip1829_mib_counter[NUM_1829_MIB_COUNTER])(void *cdata, int len);
extern int (*func_of_ip1829_qos[NUM_IP1829_QOS])(void *cdata, int len);
extern int (*func_of_ip1829_acl[NUM_IP1829_ACL])(void *cdata, int len);

int setPortAN(void *cdata, int len);
int getPortAN(void *cdata, int len);
int setPortSpeed(void *cdata, int len);
int getPortSpeed(void *cdata, int len);
int setPortDuplex(void *cdata, int len);
int getPortDuplex(void *cdata, int len);
int setPortPause(void *cdata, int len);
int getPortPause(void *cdata, int len);
int setPortAsymPause(void *cdata, int len);
int getPortAsymPause(void *cdata, int len);
int setPortLinkStatus(void *cdata, int len);
int getPortLinkStatus(void *cdata, int len);
int setPortBackpressure(void *cdata, int len);
int getPortBackpressure(void *cdata, int len);
int setPortPowerDown(void *cdata, int len);
int getPortPowerDown(void *cdata, int len);
int setPortForceLink(void *cdata, int len);
int getPortForceLink(void *cdata, int len);
int setPortUniDirection(void *cdata, int len);
int getPortUniDirection(void *cdata, int len);
int setMdioDivisor(void *cdata, int len);

int setL2CapAct(void *cdata, int len);
int getL2CapAct(void *cdata, int len);
int setCapInBand(void *cdata, int len);
int getCapInBand(void *cdata, int len);
int setCapInBandRestrict(void *cdata, int len);
int getCapInBandRestrict(void *cdata, int len);
int setCapInBandRestrictCfg(void *cdata, int len);
int getCapInBandRestrictCfg(void *cdata, int len);
int setCapSwitchMac(void *cdata, int len);
int getCapSwitchMac(void *cdata, int len);
int setCapL3Act(void *cdata, int len);
int getCapL3Act(void *cdata, int len);
int setCapL3User(void *cdata, int len);
int getCapL3User(void *cdata, int len);
int setCapEtherUser(void *cdata, int len);
int getCapEtherUser(void *cdata, int len);
int setCapIpv6TcpUdpEnable(void *cdata, int len);
int getCapIpv6TcpUdpEnable(void *cdata, int len);
int setCapIpv6TcpUdpFlagEnable(void *cdata, int len);
int getCapIpv6TcpUdpFlagEnable(void *cdata, int len);
int setCapIpv6StopFinding(void *cdata, int len);
int getCapIpv6StopFinding(void *cdata, int len);
int setCapIpv6ToAllPortsHigh(void *cdata, int len);
int getCapIpv6ToAllPortsHigh(void *cdata, int len);
int setCapIpv6ToCpuHigh(void *cdata, int len);
int getCapIpv6ToCpuHigh(void *cdata, int len);
int setCapIpv6Act(void *cdata, int len);
int getCapIpv6Act(void *cdata, int len);
int setCapIpv6User(void *cdata, int len);
int getCapIpv6User(void *cdata, int len);
int setCapIcmpv6User(void *cdata, int len);
int getCapIcmpv6User(void *cdata, int len);

int setSMACLearning(void *cdata, int len);
int getSMACLearning(void *cdata, int len);
int setSMACLrnCntCtrl(void *cdata, int len);
int getSMACLrnCntCtrl(void *cdata, int len);
int setSMACLrnThreshold(void *cdata, int len);
int getSMACLrnThreshold(void *cdata, int len);
int setLutPortFlush(void *cdata, int len);
int setLutAgingTime(void *cdata, int len);
int getLutAgingTime(void *cdata, int len);
int setLutAgingTimeEnable(void *cdata, int len);
int getLutAgingTimeEnable(void *cdata, int len);
int setLutLearningNullSA(void *cdata, int len);
int getLutLearningNullSA(void *cdata, int len);
int setLutHashingAlgorithm(void *cdata, int len);
int getLutHashingAlgorithm(void *cdata, int len);
int setLutBindingEnable(void *cdata, int len);
int getLutBindingEnable(void *cdata, int len);
int setLutLearnPktDropByVlanIgsChk(void *cdata, int len);
int getLutLearnPktDropByVlanIgsChk(void *cdata, int len);

int setLutLearningMode(void *cdata, int len);
int getLutLearningMode(void *cdata, int len);
int setLutUnknownSARule(void *cdata, int len);
int getLutUnknownSARule(void *cdata, int len);
int setLutEntry(void *cdata, int len);
int getLutEntry(void *cdata, int len);
int getLutValidEntry(void *cdata, int len);

int setSnifferSrc(void *cdata, int len);
int getSnifferSrc(void *cdata, int len);
int setSnifferDestGrp1(void *cdata, int len);
int getSnifferDestGrp1(void *cdata, int len);
int setSnifferDestGrp2(void *cdata, int len);
int getSnifferDestGrp2(void *cdata, int len);
int setS1Method(void *cdata, int len);
int getS1Method(void *cdata, int len);

int setStormFunc(void *cdata, int len);
int getStormFunc(void *cdata, int len);
int setStormThreshold(void *cdata, int len);
int getStormThreshold(void *cdata, int len);
int setStormCntrClrPeriod(void *cdata, int len);
int getStormCntrClrPeriod(void *cdata, int len);
int setStormBlockFrm2Cpu(void *cdata, int len);
int getStormBlockFrm2Cpu(void *cdata, int len);
int setStormDropInterrupt(void *cdata, int len);
int getStormDropInterrupt(void *cdata, int len);

int setEocFunc(void *cdata, int len);
int getEocFunc(void *cdata, int len);
int getEocStatus(void *cdata, int len);
int setEocReleaseTime(void *cdata, int len);
int getEocReleaseTime(void *cdata, int len);

int setLdFunc(void *cdata, int len);
int getLdFunc(void *cdata, int len);
int setLdTimeUnit(void *cdata, int len);
int getLdTimeUnit(void *cdata, int len);
int setLdPktSendTimer(void *cdata, int len);
int getLdPktSendTimer(void *cdata, int len);
int setLdBlockReleaseTimer(void *cdata, int len);
int getLdBlockReleaseTimer(void *cdata, int len);
int getLdStatus(void *cdata, int len);

int setWolFunc(void *cdata, int len);
int getWolFunc(void *cdata, int len);
int setWolMode(void *cdata, int len);
int getWolMode(void *cdata, int len);
int setWolInterrupt(void *cdata, int len);
int getWolInterrupt(void *cdata, int len);
int setWolIPUnit(void *cdata, int len);
int getWolIPUnit(void *cdata, int len);
int setWolIPThreshold(void *cdata, int len);
int getWolIPThreshold(void *cdata, int len);
int setWolStatusInSlaveMode(void *cdata, int len);
int getWolStatus(void *cdata, int len);

int setCpuPortLink(void *cdata, int len);
int getCpuPortLink(void *cdata, int len);
int setSTagFunc(void *cdata, int len);
int getSTagFunc(void *cdata, int len);
int setSTagTypeLen(void *cdata, int len);
int getSTagTypeLen(void *cdata, int len);

int setCosTcpUdpUserDefine(void *cdata, int len);
int getCosTcpUdpUserDefine(void *cdata, int len);
int setCosTcpUdpQueue(void *cdata, int len);
int getCosTcpUdpQueue(void *cdata, int len);
int setCosTcpUdpEnable(void *cdata, int len);
int getCosTcpUdpEnable(void *cdata, int len);
int setCosTcpEnable(void *cdata, int len);
int getCosTcpEnable(void *cdata, int len);
int setCosUdpEnable(void *cdata, int len);
int getCosUdpEnable(void *cdata, int len);
int setCosTcpFlagDropNull(void *cdata, int len);
int getCosTcpFlagDropNull(void *cdata, int len);
int setCosTcpFlagDropAllset(void *cdata, int len);
int getCosTcpFlagDropAllset(void *cdata, int len);
int setCosTcpFlag(void *cdata, int len);
int getCosTcpFlag(void *cdata, int len);
int setCosTcpFlagAct(void *cdata, int len);
int getCosTcpFlagAct(void *cdata, int len);
int setCosTcpFlagPort(void *cdata, int len);
int getCosTcpFlagPort(void *cdata, int len);

int setBandwidthIngressRate(void *cdata, int len);
int getBandwidthIngressRate(void *cdata, int len);
int setBandwidthEgressRate(void *cdata, int len);
int getBandwidthEgressRate(void *cdata, int len);
int setBandwidthEgressPeriod(void *cdata, int len);
int getBandwidthEgressPeriod(void *cdata, int len);

int setPtpEnable(void *cdata, int len);
int getPtpEnable(void *cdata, int len);
int setPtpToCpu(void *cdata, int len);
int getPtpToCpu(void *cdata, int len);

int setJumboPktFunc(void *cdata, int len);
int getJumboPktFunc(void *cdata, int len);
int set8021xFunc(void *cdata, int len);
int get8021xFunc(void *cdata, int len);
int setReg(void *cdata, int len);
int getReg(void *cdata, int len);
int setCPUReg(void *cdata, int len);
int getCPUReg(void *cdata, int len);
int setSwitchRestart(void *cdata, int len);
int setSwitchReset(void *cdata, int len);
int setCpuIfSpeed(void *cdata, int len);

int setVlanEgressFrame(void *cdata, int len);
int getVlanEgressFrame(void *cdata, int len);
int setVlanTagging(void *cdata, int len);
int getVlanTagging(void *cdata, int len);
int setVlanType(void *cdata, int len);
int getVlanType(void *cdata, int len);
int setVlanGroup(void *cdata, int len);

int setVlanQinQPType(void *cdata, int len);
int getVlanQinQPType(void *cdata, int len);
int setVlanQinQPAddtag(void *cdata, int len);
int getVlanQinQPAddtag(void *cdata, int len);
int setVlanQinQPRmvtag(void *cdata, int len);
int getVlanQinQPRmvtag(void *cdata, int len);
int setVlanQinQPRxdet(void *cdata, int len);
int getVlanQinQPRxdet(void *cdata, int len);
int setVlanQinQPKeep(void *cdata, int len);
int getVlanQinQPKeep(void *cdata, int len);
int setVlanQinQPIndex(void *cdata, int len);
int getVlanQinQPIndex(void *cdata, int len);
int setVlanQinQIndex(void *cdata, int len);
int getVlanQinQIndex(void *cdata, int len);

int setVlanPortAddtag(void *cdata, int len);
int getVlanPortAddtag(void *cdata, int len);
int setVlanPortRmvtag(void *cdata, int len);
int getVlanPortRmvtag(void *cdata, int len);
int setVlanPortForce(void *cdata, int len);
int getVlanPortForce(void *cdata, int len);
int setVlanPortUplink(void *cdata, int len);
int getVlanPortUplink(void *cdata, int len);
int setVlanPortExclusive(void *cdata, int len);
int getVlanPortExclusive(void *cdata, int len);
int setVlanPortEgress(void *cdata, int len);
int getVlanPortEgress(void *cdata, int len);
int setVlanPortIngressFrame(void *cdata, int len);
int getVlanPortIngressFrame(void *cdata, int len);
int setVlanPortIngressCheck(void *cdata, int len);
int getVlanPortIngressCheck(void *cdata, int len);
int setVlanPortMember(void *cdata, int len);
int getVlanPortMember(void *cdata, int len);
int setVlanPortVid(void *cdata, int len);
int getVlanPortVid(void *cdata, int len);

int setVlanProtocolMode(void *cdata, int len);
int getVlanProtocolMode(void *cdata, int len);
int setVlanProtocolVid(void *cdata, int len);
int getVlanProtocolVid(void *cdata, int len);
int setVlanProtocolType(void *cdata, int len);
int getVlanProtocolType(void *cdata, int len);
int setVlanProtocolClear(void *cdata, int len);

int setVlanEntryMember(void *cdata, int len);
int getVlanEntryMember(void *cdata, int len);
int setVlanEntryAddtag(void *cdata, int len);
int getVlanEntryAddtag(void *cdata, int len);
int setVlanEntryRmvtag(void *cdata, int len);
int getVlanEntryRmvtag(void *cdata, int len);
int setVlanEntryPriority(void *cdata, int len);
int getVlanEntryPriority(void *cdata, int len);
int setVlanEntryFid(void *cdata, int len);
int getVlanEntryFid(void *cdata, int len);
int setVlanEntryClear(void *cdata, int len);

int setMstpFunc(void *cdata, int len);
int getMstpFunc(void *cdata, int len);

int setTrunkHashMthd(void *cdata, int len);
int getTrunkHashMthd(void *cdata, int len);
int setTrunkMbr(void *cdata, int len);
int getTrunkMbr(void *cdata, int len);
int setCpuNCareTrunkAndVlan(void *cdata, int len);
int getCpuNCareTrunkAndVlan(void *cdata, int len);

int setImpMode(void *cdata, int len);
int getImpMode(void *cdata, int len);
int setImpPassNullIP(void *cdata, int len);
int getImpPassNullIP(void *cdata, int len);
int setImpHash(void *cdata, int len);
int getImpHash(void *cdata, int len);
int setImpPort(void *cdata, int len);
int getImpPort(void *cdata, int len);

int setImpEntry(void *cdata, int len);
int getImpEntry(void *cdata, int len);

int setIGMPSnooping(void *cdata, int len);
int getIGMPSnooping(void *cdata, int len);
int setIGMPMctByCPU(void *cdata, int len);
int getIGMPMctByCPU(void *cdata, int len);
int setIGMPGroupAgain(void *cdata, int len);
int getIGMPGroupAgain(void *cdata, int len);
int setIGMPRltByCPU(void *cdata, int len);
int getIGMPRltByCPU(void *cdata, int len);
int setIGMPPktForward(void *cdata, int len);
int getIGMPPktForward(void *cdata, int len);
int setIGMPRlt(void *cdata, int len);
int getIGMPRlt(void *cdata, int len);
int setIGMPHashMethod(void *cdata, int len);
int getIGMPHashMethod(void *cdata, int len);
int setIGMPMldRule(void *cdata, int len);
int getIGMPMldRule(void *cdata, int len);
int setIGMPMctTable(void *cdata, int len);
int getIGMPMctTable(void *cdata, int len);
int setIGMPSltTable(void *cdata, int len);
int getIGMPSltTable(void *cdata, int len);


int setS1PktModify(void *cdata, int len);
int getS1PktModify(void *cdata, int len);
int setS1TM4CpuSTag(void *cdata, int len);
int getS1TM4CpuSTag(void *cdata, int len);
int setS1TM4Acl2Cpu(void *cdata, int len);
int getS1TM4Acl2Cpu(void *cdata, int len);
int setS1TM4Pkt2MPort(void *cdata, int len);
int getS1TM4Pkt2MPort(void *cdata, int len);
int setS2LTT4Grp1(void *cdata, int len);
int getS2LTT4Grp1(void *cdata, int len);
int setS2LTT4Grp2(void *cdata, int len);
int getS2LTT4Grp2(void *cdata, int len);

int setMStormNBlockIpPkt(void *cdata, int len);
int getMStormNBlockIpPkt(void *cdata, int len);
int setMStormIgnr01005EXXXXXX(void *cdata, int len);
int getMStormIgnr01005EXXXXXX(void *cdata, int len);

int setEocBlockClr(void *cdata, int len);
int setEocClrBlockWhenRcvGood(void *cdata, int len);
int getEocClrBlockWhenRcvGood(void *cdata, int len);

int setLdSMACB40(void *cdata, int len);
int getLdSMACB40(void *cdata, int len);
int setLdRerandom(void *cdata, int len);
int getLdRerandom(void *cdata, int len);
int setLdDMAC(void *cdata, int len);
int getLdDMAC(void *cdata, int len);
int setLdEtherType(void *cdata, int len);
int getLdEtherType(void *cdata, int len);
int setLdSubType(void *cdata, int len);
int getLdSubType(void *cdata, int len);
int setLdDeviceID(void *cdata, int len);
int getLdDeviceID(void *cdata, int len);

int setWolWakeIfTxGetAnyPkt(void *cdata, int len);
int getWolWakeIfTxGetAnyPkt(void *cdata, int len);
int setWolWakeIfRxGetAnyPkt(void *cdata, int len);
int getWolWakeIfRxGetAnyPkt(void *cdata, int len);
int setWolWakeIfMatchAcl2Cpu(void *cdata, int len);
int getWolWakeIfMatchAcl2Cpu(void *cdata, int len);

int setMACLoopBackFunc(void *cdata, int len);
int getMACLoopBackFunc(void *cdata, int len);
int setPausePktFunc(void *cdata, int len);
int getPausePktFunc(void *cdata, int len);
int setPausePktDest(void *cdata, int len);
int getPausePktDest(void *cdata, int len);
int setLocalTrafficFunc(void *cdata, int len);
int getLocalTrafficFunc(void *cdata, int len);
int setMACReset(void *cdata, int len);
int getMACReset(void *cdata, int len);
int setMACSelfTestFunc(void *cdata, int len);
int getMACSelfTestFunc(void *cdata, int len);
int setMACSelfTestPktNum(void *cdata, int len);
int getMACSelfTestPktNum(void *cdata, int len);
int getMACSelfTestResult(void *cdata, int len);

int setBpduCapMode(void *cdata, int len);
int getBpduCapMode(void *cdata, int len);
int setBpduPortAct(void *cdata, int len);
int getBpduPortAct(void *cdata, int len);
int setStpPortState(void *cdata, int len);
int getStpPortState(void *cdata, int len);
int setStpAllPortsState(void *cdata, int len);
int getStpAllPortsState(void *cdata, int len);

int setTrunkHashMthdSeq(void *cdata, int len);
int getTrunkHashMthdSeq(void *cdata, int len);
int setTrunkGrpCombine(void *cdata, int len);
int getTrunkGrpCombine(void *cdata, int len);

int setMibCounterEnable(void *cdata, int len);
int getMibCounterEnable(void *cdata, int len);
int getMibCounterAll(void *cdata, int len);
int getMibCounterByPort(void *cdata, int len);
int getMibCounterByItem(void *cdata, int len);

int setQOSAgingFunction(void *cdata, int len);
int getQOSAgingFunction(void *cdata, int len);
int setQOSAgingTime(void *cdata, int len);
int getQOSAgingTime(void *cdata, int len);
int setQOSFastAging(void *cdata, int len);
int getQOSFastAging(void *cdata, int len); 
  
int setCOSACL(void *cdata, int len);
int getCOSACL(void *cdata, int len);
int setCOSIGMP(void *cdata, int len);
int getCOSIGMP(void *cdata, int len);
int setCOSIPAddress(void *cdata, int len);
int getCOSIPAddress(void *cdata, int len);
int setCOSMACAddress(void *cdata, int len);
int getCOSMACAddress(void *cdata, int len);
int setCOSVID(void *cdata, int len);
int getCOSVID(void *cdata, int len);
int setCOSTCPUDPPort(void *cdata, int len);
int getCOSTCPUDPPort(void *cdata, int len);
int setCOSDSCP(void *cdata, int len);
int getCOSDSCP(void *cdata, int len);
int setCOS8021P(void *cdata, int len);
int getCOS8021P(void *cdata, int len);
int setCOSPhsicalPort(void *cdata, int len);
int getCOSPhsicalPort(void *cdata, int len);
int setQOS8021PEdtion(void *cdata, int len);
int getQOS8021PEdtion(void *cdata, int len);
int setQOSDSCPBaseDSCP(void *cdata, int len);
int getQOSDSCPBaseDSCP(void *cdata, int len);
int setQOSDSCPBaseNoMatchAction(void *cdata, int len);
int getQOSDSCPBaseNoMatchAction(void *cdata, int len);
int setQOSmodeGroupMember(void *cdata, int len);
int getQOSmodeGroupMember(void *cdata, int len);
int setQOSGroupBEn(void *cdata, int len);
int getQOSGroupBEn(void *cdata, int len);
int setQOSMode(void *cdata, int len);
int getQOSMode(void *cdata, int len);
int setQOSMethod(void *cdata, int len);
int getQOSMethod(void *cdata, int len);
int setQOSWeight(void *cdata, int len);
int getQOSWeight(void *cdata, int len);
int setQOSMaxBandwidth(void *cdata, int len);
int getQOSMaxBandwidth(void *cdata, int len);
int setQOSUnit(void *cdata, int len);
int getQOSUnit(void *cdata, int len);
int setQOSRatioValue0Def(void *cdata, int len);
int getQOSRatioValue0Def(void *cdata, int len);
int setQOSSBMDBM(void *cdata, int len);
int getQOSSBMDBM(void *cdata, int len);
int setQOSDBMEn(void *cdata, int len);
int getQOSDBMEn(void *cdata, int len);
int setQOSEgressControl(void *cdata, int len);
int getQOSEgressControl(void *cdata, int len);
  
int setQOSRemap(void *cdata, int len);
int getQOSRemap(void *cdata, int len);

void acl_init(void);
int setAclRule(void *cdata, int len);
int getAclRule(void *cdata, int len);
int aclCleanTable(void *cdata, int len);
int setAclFunctionEn(void *cdata, int len);
int getAclFunctionEn(void *cdata, int len);
int getAclUsedRules(void *cdata, int len);
int getAclUsedEntries(void *cdata, int len);
int setAclBW(void *cdata, int len);
int getAclBW(void *cdata, int len);
int setAclDscp(void *cdata, int len);
int getAclDscp(void *cdata, int len);
int setAclVlanGroup(void *cdata, int len);
int getAclVlanGroup(void *cdata, int len);
int setAclVidRemark(void *cdata, int len);
int getAclVidRemark(void *cdata, int len);
void ic_mdio_set_divisor(unsigned long div);

/*	ipv:	4:	IPv4
 *			6:	IPv6
 *	addr:	IP	address
 *	method:	1:	Direct
 *			0:	CRC
 * */
unsigned char tb_calc_index(unsigned char ipv, void *addr, unsigned char method);
#endif		/* IP1829FDAT_H */
