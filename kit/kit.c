#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "kit.h"
#include "ip1829lib.h"
#include "ip1829.h"

void maketimeout(struct timespec * tsp , int milliseconds){
	struct timeval now;

	/* get the current time */
	gettimeofday(&now , NULL);
	// add time
	now.tv_usec += milliseconds*1000;
	if( now.tv_usec >= 1000000 ){
		now.tv_sec += now.tv_usec/1000000;
		now.tv_usec %= 1000000;
	}
	
	tsp->tv_sec = now.tv_sec;
	tsp->tv_nsec = now.tv_usec * 1000;
	/* add the offset to get timeout value */
	//tsp->tv_sec += seconds;
}

// 执行shell命令
int exec_command(const char * command){
#if   0
	/* writing pipe is faster a lot than writing terminal */
	FILE * fp = popen(command , "r");
	if(!fp)
		return -1;
	pclose(fp);
	return 0;
#else
	return system(command);
#endif
}

// 调用icplus提供的库函数
int request_command(const char * command){
	char szCookiedata[40]="admin";
	char szRecv[0x400];
	char szBuffer[256];
	strcpy(szBuffer , command);
	//printf("cmd = %s\n" , szBuffer);
/*
	int i = 1, index = 0;
	for( ; i<argc ;++i){
		//strcpy(szBuffer+index , argv[i]);
		index += sprintf(szBuffer+index , "%s " , argv[i]);
	}
	szBuffer[index-1] = 0;
	printf("command = \"%s\"\n" , szBuffer);
*/
	if( !request_clish(szCookiedata,szBuffer,sizeof szRecv,szRecv))
	{
		return -1;
		//printf("FAIL\n");
	}
	return 0;
}

/*
int enable_stp_ports(const char * port , bool set){
	char cmd[128];
	sprintf(cmd , "cli_stp %s stp ports %s" , set?"enable":"disable" ,port);
	return exec_command(cmd);
}
*/

// 关闭stp
int kill_stp(void){
	char cmd[256];
	/* enable/disbale stp */
	printf("disable stp...\n");
	/* global stp setting */
	// 命令: cli_stp disable stp
	sprintf(cmd , CLI_PATH"cli_stp disable stp");
	if(exec_command(cmd)<0)
		goto CMD_FAILED;
	
	/* disable ports stp function */
	// 命令: cli_stp disable stp ports 1-28
	sprintf(cmd , CLI_PATH"cli_stp disable stp ports 1-28");
	if(exec_command(cmd)<0)
		goto CMD_FAILED;

	// 执行这个上面两条命令才能起作用
	sprintf(cmd , CLI_PATH"cli_stp restart stp");
	if(exec_command(cmd)<0)
		goto CMD_FAILED;

	// 使所有端口进入forward状态，详见手册
	/*----- forward all ports -----*/
	/* Each port of 
	 * IP1829A can be set in one of the four spanning tree states individually
	 * by programming page 0x01 
	 * register 0x57 to 0x5A for STP and page 0x01 register 
	 * 0x57 to 0x96 for MSTP */
	for(uint8_t reg=0x57;reg<=0x96;++reg){
		if(singleRegSetting(ID_COMMON_MISC_SET_REG,0x01,reg,0xffff)<0){
			fprintf(stderr , "write page 1 reg %#x failed\n" , reg);
			return -1;
		}
	}
	return 0;

CMD_FAILED:
	fprintf(stderr , "%s: failed\n",cmd);
	return -1;
}


// 使能mac学习功能
int enable_mac_learning(int port , bool set){
	uint16_t regval;
	if( port>=1 && port<=16 ){
		if(singleRegGetting(ID_COMMON_MISC_GET_REG,0x01,0x03,&regval)<0)
			return -1;
		if(set)
			regval |= 1ul<<(port-1);
		else
			regval &= ~(1ul<<(port-1));
		if(singleRegSetting(ID_COMMON_MISC_SET_REG,0x01,0x03,regval)<0)
			return -1;
	}
	else if( port>=17 && port<=28 ){
		if(singleRegGetting(ID_COMMON_MISC_GET_REG,0x01,0x04,&regval)<0)
			return -1;
		if(set)
			regval |= 1ul<<(port-17);
		else
			regval &= ~(1ul<<(port-17));
		if(singleRegSetting(ID_COMMON_MISC_SET_REG,0x01,0x04,regval)<0)
			return -1;
	}
	return 0;
}

// 刷新(清除)mac表
int refresh_fdb(void * arg){
	//printf("CLEAR MAC TABLE...\n");
	return request_command("clear mac_table ports all");
}

void hexdump(const void * p , int len){
	printf("hexdump:\n");
	for(int i=0;i<len;++i){
		printf("%02x " , ((const uint8_t*)p)[i]);
		if( i && !((i+1)%16) )
			printf("\n");
	}
	printf("\n");
}

/*
时间测试
用法:
	struct timeval time;
	time_test(__func__, &time, 0);
	要测试时间的代码
	time_test(__func__, &time,1); //打印结果,消耗多长时间
*/
void time_test(const char * runname,struct timeval * time ,int step){
	if( 0==step ){
		/* begin */
		/* get the current time */
		gettimeofday(time , NULL);
	}
	else{
		/* end */
		struct timeval now , diff;
		gettimeofday(&now , NULL);
		if( now.tv_usec < time->tv_usec){
			diff.tv_usec = now.tv_usec+1000000-time->tv_usec;
			diff.tv_sec  = now.tv_sec-time->tv_sec-1;
		}
		else{
			diff.tv_usec = now.tv_usec-time->tv_usec;
			diff.tv_sec  = now.tv_sec-time->tv_sec;
		}
#if  1
		if( diff.tv_sec ){
			printf("[%s] %d sec %d ms\n",runname,diff.tv_sec,diff.tv_usec/1000);
		}
		else{
			printf("[%s] %d ms\n",runname,diff.tv_usec/1000);
		}
#endif
	}
}

