#include "ip1829fdat.h"

int (*func_of_common_smi[21])(void *cdata, int len) =
{
	&setPortAN,
	&getPortAN,
	&setPortSpeed,
	&getPortSpeed,
	&setPortDuplex,
	&getPortDuplex,
	&setPortPause,
	&getPortPause,
	&setPortAsymPause,
	&getPortAsymPause,
	&setPortLinkStatus,
	&getPortLinkStatus,
	&setPortBackpressure,
	&getPortBackpressure,
	&setPortPowerDown,
	&getPortPowerDown,
	&setPortForceLink,
	&getPortForceLink,
	&setPortUniDirection,
	&getPortUniDirection,
	&setMdioDivisor,
};

int (*func_of_common_cap[NUM_COMMON_CAP])(void *cdata, int len) =
{
	&setL2CapAct,
	&getL2CapAct,
	&setCapInBand,
	&getCapInBand,
	&setCapInBandRestrict,
	&getCapInBandRestrict,
	&setCapInBandRestrictCfg,
	&getCapInBandRestrictCfg,
	&setCapSwitchMac,
	&getCapSwitchMac,
	&setCapL3Act,
	&getCapL3Act,
	&setCapL3User,
	&getCapL3User,
	&setCapEtherUser,
	&getCapEtherUser,
	&setCapIpv6TcpUdpEnable,
	&getCapIpv6TcpUdpEnable,
	&setCapIpv6TcpUdpFlagEnable,
	&getCapIpv6TcpUdpFlagEnable,
	&setCapIpv6StopFinding,
	&getCapIpv6StopFinding,
	&setCapIpv6ToAllPortsHigh,
	&getCapIpv6ToAllPortsHigh,
	&setCapIpv6ToCpuHigh,
	&getCapIpv6ToCpuHigh,
	&setCapIpv6Act,
	&getCapIpv6Act,
	&setCapIpv6User,
	&getCapIpv6User,
	&setCapIcmpv6User,
	&getCapIcmpv6User,
};

int (*func_of_common_lut[NUM_COMMON_LUT])(void *cdata, int len) =
{
	&setSMACLearning,
	&getSMACLearning,
	&setSMACLrnCntCtrl,
	&getSMACLrnCntCtrl,
	&setSMACLrnThreshold,
	&getSMACLrnThreshold,
	&setLutPortFlush,
	&setLutAgingTime,
	&getLutAgingTime,
	&setLutAgingTimeEnable,
	&getLutAgingTimeEnable,
	&setLutLearningNullSA,
	&getLutLearningNullSA,
	&setLutHashingAlgorithm,
	&getLutHashingAlgorithm,
	&setLutBindingEnable,
	&getLutBindingEnable,
	&setLutLearnPktDropByVlanIgsChk,
	&getLutLearnPktDropByVlanIgsChk,
};

int (*func_of_common_sniffer[8])(void *cdata, int len) =
{
	&setSnifferSrc,
	&getSnifferSrc,
	&setSnifferDestGrp1,
	&getSnifferDestGrp1,
	&setSnifferDestGrp2,
	&getSnifferDestGrp2,
	&setS1Method,
	&getS1Method,
};

int (*func_of_common_storm[10])(void *cdata, int len) =
{
	&setStormFunc,
	&getStormFunc,
	&setStormThreshold,
	&getStormThreshold,
	&setStormCntrClrPeriod,
	&getStormCntrClrPeriod,
	&setStormBlockFrm2Cpu,
	&getStormBlockFrm2Cpu,
	&setStormDropInterrupt,
	&getStormDropInterrupt,
};

int (*func_of_common_eoc[5])(void *cdata, int len) =
{
	&setEocFunc,
	&getEocFunc,
	&getEocStatus,
	&setEocReleaseTime,
	&getEocReleaseTime,
};

int (*func_of_common_ld[9])(void *cdata, int len) =
{
	&setLdFunc,
	&getLdFunc,
	&setLdTimeUnit,
	&getLdTimeUnit,
	&setLdPktSendTimer,
	&getLdPktSendTimer,
	&setLdBlockReleaseTimer,
	&getLdBlockReleaseTimer,
	&getLdStatus,
};

int (*func_of_common_wol[12])(void *cdata, int len) =
{
	&setWolFunc,
	&getWolFunc,
	&setWolMode,
	&getWolMode,
	&setWolInterrupt,
	&getWolInterrupt,
	&setWolIPUnit,
	&getWolIPUnit,
	&setWolIPThreshold,
	&getWolIPThreshold,
	&setWolStatusInSlaveMode,
	&getWolStatus,
};

int (*func_of_common_stag[6])(void *cdata, int len) =
{
	&setCpuPortLink,
	&getCpuPortLink,
	&setSTagFunc,
	&getSTagFunc,
	&setSTagTypeLen,
	&getSTagTypeLen,
};

int (*func_of_common_ptp[NUM_COMMON_PTP])(void *cdata, int len) =
{
	&setPtpEnable,
	&getPtpEnable,
	&setPtpToCpu,
	&getPtpToCpu,
};

int (*func_of_common_misc[11])(void *cdata, int len) =
{
	&setJumboPktFunc,
	&getJumboPktFunc,
	&set8021xFunc,
	&get8021xFunc,
	&setReg,
	&getReg,
	&setCPUReg,
	&getCPUReg,
	&setSwitchRestart,
	&setSwitchReset,
	&setCpuIfSpeed,
};

int (*func_of_common_vlan[NUM_COMMON_VLAN])(void *cdata, int len) =
{
	&setVlanEgressFrame,		//0
	&getVlanEgressFrame,
	&setVlanTagging,
	&getVlanTagging,
	&setVlanType,
	&getVlanType,

	&setVlanGroup,

	&setVlanQinQPType,
	&getVlanQinQPType,
	&setVlanQinQPAddtag,
	&getVlanQinQPAddtag,	//10
	&setVlanQinQPRmvtag,
	&getVlanQinQPRmvtag,
	&setVlanQinQPRxdet,
	&getVlanQinQPRxdet,
	&setVlanQinQPKeep,		//15
	&getVlanQinQPKeep,
	&setVlanQinQPIndex,
	&getVlanQinQPIndex,
	&setVlanQinQIndex,
	&getVlanQinQIndex,		//20

	&setVlanPortAddtag,
	&getVlanPortAddtag,
	&setVlanPortRmvtag,
	&getVlanPortRmvtag,
	&setVlanPortForce,		//25
	&getVlanPortForce,
	&setVlanPortUplink,
	&getVlanPortUplink,
	&setVlanPortExclusive,
	&getVlanPortExclusive,	//30
	&setVlanPortEgress,
	&getVlanPortEgress,
	&setVlanPortIngressFrame,
	&getVlanPortIngressFrame,
	&setVlanPortIngressCheck,	//35
	&getVlanPortIngressCheck,
	&setVlanPortMember,
	&getVlanPortMember,
	&setVlanPortVid,
	&getVlanPortVid,		//40

	&setVlanProtocolMode,
	&getVlanProtocolMode,
	&setVlanProtocolVid,
	&getVlanProtocolVid,
	&setVlanProtocolType,		//45
	&getVlanProtocolType,

	&setVlanProtocolClear,
};

int (*func_of_common_stp[2])(void *cdata, int len) =
{
	&setMstpFunc,
	&getMstpFunc,
};

int (*func_of_common_lacp[6])(void *cdata, int len) =
{
	&setTrunkHashMthd,
	&getTrunkHashMthd,
	&setTrunkMbr,
	&getTrunkMbr,
	&setCpuNCareTrunkAndVlan,
	&getCpuNCareTrunkAndVlan,
};

int (*func_of_common_imp[8])(void *cdata, int len) =
{
	&setImpMode,
	&getImpMode,
	&setImpPassNullIP,
	&getImpPassNullIP,
	&setImpHash,
	&getImpHash,
	&setImpPort,
	&getImpPort,
};

int (*func_of_common_cos[NUM_COMMON_TCPUDP])(void *cdata, int len) =
{
	&setCosTcpUdpUserDefine,
	&getCosTcpUdpUserDefine,
	&setCosTcpUdpQueue,
	&getCosTcpUdpQueue,
	&setCosTcpUdpEnable,
	&getCosTcpUdpEnable,
	&setCosTcpEnable,
	&getCosTcpEnable,
	&setCosUdpEnable,
	&getCosUdpEnable,
	&setCosTcpFlagDropNull,
	&getCosTcpFlagDropNull,
	&setCosTcpFlagDropAllset,
	&getCosTcpFlagDropAllset,
	&setCosTcpFlag,
	&getCosTcpFlag,
	&setCosTcpFlagAct,
	&getCosTcpFlagAct,
	&setCosTcpFlagPort,
	&getCosTcpFlagPort,
};

int (*func_of_common_bandwidth[NUM_COMMON_BANDWIDTH])(void *cdata, int len) =
{
	&setBandwidthIngressRate,
	&getBandwidthIngressRate,
	&setBandwidthEgressRate,
	&getBandwidthEgressRate,
	&setBandwidthEgressPeriod,
	&getBandwidthEgressPeriod,
};
/* ---------------- functions of ip1829 ---------------------------*/
int (*func_of_ip1829_lut[NUM_1829_LUT])(void *cdata, int len) =
{
	&setLutLearningMode,
	&getLutLearningMode,
	&setLutUnknownSARule,
	&getLutUnknownSARule,
	&setLutEntry,
	&getLutEntry,
	&getLutValidEntry,
};

int (*func_of_ip1829_sniffer[12])(void *cdata, int len) =
{
	&setS1PktModify,
	&getS1PktModify,
	&setS1TM4CpuSTag,
	&getS1TM4CpuSTag,
	&setS1TM4Acl2Cpu,
	&getS1TM4Acl2Cpu,
	&setS1TM4Pkt2MPort,
	&getS1TM4Pkt2MPort,
	&setS2LTT4Grp1,
	&getS2LTT4Grp1,
	&setS2LTT4Grp2,
	&getS2LTT4Grp2,
};

int (*func_of_ip1829_storm[4])(void *cdata, int len) =
{
	&setMStormNBlockIpPkt,
	&getMStormNBlockIpPkt,
	&setMStormIgnr01005EXXXXXX,
	&getMStormIgnr01005EXXXXXX,
};

int (*func_of_ip1829_eoc[3])(void *cdata, int len) =
{
	&setEocBlockClr,
	&setEocClrBlockWhenRcvGood,
	&getEocClrBlockWhenRcvGood,
};

int (*func_of_ip1829_ld[12])(void *cdata, int len) =
{
	&setLdSMACB40,
	&getLdSMACB40,
	&setLdRerandom,
	&getLdRerandom,
	&setLdDMAC,
	&getLdDMAC,
	&setLdEtherType,
	&getLdEtherType,
	&setLdSubType,
	&getLdSubType,
	&setLdDeviceID,
	&getLdDeviceID,
};

int (*func_of_ip1829_wol[6])(void *cdata, int len) =
{
	&setWolWakeIfTxGetAnyPkt,
	&getWolWakeIfTxGetAnyPkt,
	&setWolWakeIfRxGetAnyPkt,
	&getWolWakeIfRxGetAnyPkt,
	&setWolWakeIfMatchAcl2Cpu,
	&getWolWakeIfMatchAcl2Cpu,
};

int (*func_of_ip1829_misc[15])(void *cdata, int len) =
{
	&setMACLoopBackFunc,
	&getMACLoopBackFunc,
	&setPausePktFunc,
	&getPausePktFunc,
	&setPausePktDest,
	&getPausePktDest,
	&setLocalTrafficFunc,
	&getLocalTrafficFunc,
	&setMACReset,
	&getMACReset,
	&setMACSelfTestFunc,
	&getMACSelfTestFunc,
	&setMACSelfTestPktNum,
	&getMACSelfTestPktNum,
	&getMACSelfTestResult,
};

int (*func_of_ip1829_stp[8])(void *cdata, int len) =
{
	&setBpduCapMode,
	&getBpduCapMode,
	&setBpduPortAct,
	&getBpduPortAct,
	&setStpPortState,
	&getStpPortState,
	&setStpAllPortsState,
	&getStpAllPortsState,
};

int (*func_of_ip1829_lacp[4])(void *cdata, int len) =
{
	&setTrunkHashMthdSeq,
	&getTrunkHashMthdSeq,
	&setTrunkGrpCombine,
	&getTrunkGrpCombine,
};

int (*func_of_ip1829_igmp[20])(void *cdata, int len) =
{
	&setIGMPSnooping,
	&getIGMPSnooping,
	&setIGMPMctByCPU,
	&getIGMPMctByCPU,
	&setIGMPGroupAgain,
	&getIGMPGroupAgain,
	&setIGMPRltByCPU,
	&getIGMPRltByCPU,
	&setIGMPPktForward,
	&getIGMPPktForward,
	&setIGMPRlt,
	&getIGMPRlt,
	&setIGMPHashMethod,
	&getIGMPHashMethod,
	&setIGMPMldRule,
	&getIGMPMldRule,
	&setIGMPMctTable,
	&getIGMPMctTable,
	&setIGMPSltTable,
	&getIGMPSltTable,
};

int (*func_of_ip1829_imp[2])(void *cdata, int len) =
{
	&setImpEntry,
	&getImpEntry,
};

int (*func_of_ip1829_vlan[NUM_IP1829_VLAN])(void *cdata, int len) =
{
	&setVlanEntryMember,
	&getVlanEntryMember,
	&setVlanEntryAddtag,
	&getVlanEntryAddtag,
	&setVlanEntryRmvtag,
	&getVlanEntryRmvtag,
	&setVlanEntryPriority,
	&getVlanEntryPriority,
	&setVlanEntryFid,
	&getVlanEntryFid,
	&setVlanEntryClear,
};

int (*func_of_ip1829_mib_counter[NUM_1829_MIB_COUNTER])(void *cdata, int len) =
{
	&setMibCounterEnable,
	&getMibCounterEnable,
	&getMibCounterAll,
	&getMibCounterByPort,
	&getMibCounterByItem,

};

int (*func_of_ip1829_qos[NUM_IP1829_QOS])(void *cdata, int len) =
{
  &setQOSAgingFunction,
  &getQOSAgingFunction,
  &setQOSAgingTime,
  &getQOSAgingTime,
  &setQOSFastAging,
  &getQOSFastAging,

  &setCOSACL,
  &getCOSACL,
  &setCOSIGMP,
  &getCOSIGMP,
  &setCOSIPAddress,
  &getCOSIPAddress,
  &setCOSMACAddress,
  &getCOSMACAddress,
  &setCOSVID,
  &getCOSVID,
  &setCOSTCPUDPPort,
  &getCOSTCPUDPPort,
  &setCOSDSCP,
  &getCOSDSCP,
  &setCOS8021P,
  &getCOS8021P,
  &setCOSPhsicalPort,
  &getCOSPhsicalPort,
  &setQOS8021PEdtion,
  &getQOS8021PEdtion,
  &setQOSDSCPBaseDSCP,
  &getQOSDSCPBaseDSCP,
  &setQOSDSCPBaseNoMatchAction,
  &getQOSDSCPBaseNoMatchAction,
  &setQOSmodeGroupMember,
  &getQOSmodeGroupMember,
  &setQOSGroupBEn,
  &getQOSGroupBEn,
  &setQOSMode,
  &getQOSMode,
  &setQOSMethod,
  &getQOSMethod,
  &setQOSWeight,
  &getQOSWeight,
  &setQOSMaxBandwidth,
  &getQOSMaxBandwidth,
  &setQOSUnit,
  &getQOSUnit,
  &setQOSRatioValue0Def,
  &getQOSRatioValue0Def,
  &setQOSSBMDBM,
  &getQOSSBMDBM,
  &setQOSDBMEn,
  &getQOSDBMEn,
  &setQOSEgressControl,
  &getQOSEgressControl,

  &setQOSRemap,
  &getQOSRemap,
};

int (*func_of_ip1829_acl[NUM_IP1829_ACL])(void *cdata, int len) =
{
	&setAclRule,
	&getAclRule,
	&aclCleanTable,
	&setAclFunctionEn,
	&getAclFunctionEn,
	&getAclUsedRules,
	&getAclUsedEntries,
	&setAclBW,
	&getAclBW,
	&setAclDscp,
	&getAclDscp,
	&setAclVlanGroup,
	&getAclVlanGroup,
	&setAclVidRemark,
	&getAclVidRemark,
};


