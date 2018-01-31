#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include "kit.h"
#include "vlan.h"
#include "vlan_raw.h"


int fill_vlan_frame(void * dst , const void * src , int len , uint16_t vid){
	memcpy(dst , src , 12);
	struct vlan_field * p_tag = (struct vlan_field *)((char*)dst+12);
	memset(p_tag , 0 , sizeof *p_tag);
	/* fill vlan tag */
	p_tag->tpid = htons(0x8100);
	p_tag->vid  = vid;
	memcpy(dst+12+sizeof(struct vlan_field) , src+12 , len-12);
	return len+sizeof(struct vlan_field);
}



#if   0

int vlan_delete_entry(const char * name){
	char command[256];
	sprintf(command , "cli_vlan delete vlan name %s" ,name);
	return exec_command(command);
}

int vlan_add_entry(const char * name , int vid){
	char command[256];
	sprintf(command , "cli_vlan create vlan name %s vid %d" ,name,vid);
	//printf("cmd = %s\n" , command);
	return exec_command(command);
}

int vlan_delete_port_from_entry(const char * name , int port){
	char command[256];
	sprintf(command , "cli_vlan config vlan name %s delete %d" ,name, port);
	return exec_command(command);
}


int vlan_add_port_into_entry(const char * name , int port){
	char command[256];
	sprintf(command , "cli_vlan config vlan name %s add tag 0 untag 0 mem %d forbidden 0 priority 0 gvrp disable" ,name, port);
	return exec_command(command);
}

#endif


int vlan_create_data_file(void){
	vlan_t buf[64] = {0};
	FILE * pfp = popen(CLI_PATH"cli_vlan show vlan name all" , "r");
	if( !pfp ){
		fprintf(stderr , "popen: %s\n" , strerror(errno));
		return -1;
	}
	char line[256];
	int name_cnter = 0 , vid_cnter = 0;
	while( fgets(line , sizeof line , pfp) ){
		if( strstr(line , "Name")==line ){
			sscanf(strstr(line,":")+1 , "%s",buf[name_cnter++].name);
		}
		else if( strstr(line,"VID")==line ){
			buf[vid_cnter++].vid = (uint16_t)atoi(strstr(line,":")+1);
		}
	}
	FILE * wfp = fopen(VLAN_DATA_PATH , "w");
	if( !wfp ){
		fprintf(stderr , "fopen: %s\n" , strerror(errno));
		return -1;
	}
	fwrite(buf , sizeof buf , 1 , wfp);
	pclose(pfp);
	fclose(wfp);
	return 0;
}


#if    0
/*
	cli_vlan show vlan name all
*/
const vlan_t ** vlan_read_all_entry_name(void){
//	static char name_group[64][256];
//	static const char * namev[64];
	static vlan_t vlan_group[64];
	static const vlan_t * vlanv[64];

//	time_test_begin();
	//FILE * fp = popen("cli_vlan show vlan name all | grep 'Name'" , "r");
	//FILE * fp = popen("cli_vlan show vlan name all" , "r");
	FILE * fp = fopen("/home/vlan.txt" , "r");
	if( !fp ){
		fprintf(stderr , "fopen: %s\n" , strerror(errno));
		return NULL;
	}

	char line[256];
	int cnter = 0 , vid_cnter = 0;
	for(;fgets(line, sizeof(line),fp);){
		if( !strncmp("Name",line,4) ){
			//printf(line);
			line[strlen(line)-1]=0;   // substitue '\n' as '\0'
			for(int find = 0 ; ;find++){
				if( ':' == line[find] ){
					strcpy(vlan_group[cnter].name, line+find+1);
					vlanv[cnter] = vlan_group+cnter;
					break;
				}
			}
			cnter++;
		}
		if( !strncmp("VID",line,3) ){
			//printf(line);
			line[strlen(line)-1]=0;   // substitue '\n' as '\0'
			for(int find = 0 ; ;find++){
				if( ':' == line[find] ){
					vlan_group[vid_cnter].vid = atoi(line+find+1);
					break;
				}
			}
			vid_cnter++;
		}
	}
	vlanv[cnter] = NULL;
	//pclose(fp);
	fclose(fp);
//	time_test_end();
	return vlanv;
}
#endif


int block_port(void * arg , int port , const vlan_t * exclude){
	time_test_begin();
	printf("BLOCK port %d...\n" , port);
#if   0
	//enable_mac_learning(port,false);
	const char ** namev = vlan_read_all_entry_name();
	for(int i=0;namev[i];i++){
		if( strcmp(exclude->name,namev[i]) ){
			struct timeval time2;
			//time_test("delete port",&time2,0);
			int ret=vlan_delete_port_from_entry(namev[i],port);
			//time_test("delete port",&time2,1);
			if(ret<0)
				return -1;
		}
	}
#else
	//const vlan_t ** vlanv = vlan_read_all_entry_name();
	vlan_t buf[64];
	FILE * fp = fopen(VLAN_DATA_PATH , "r");
	fread(buf , sizeof buf , 1 , fp);
	fclose(fp);
	for(int i=0;i<ARRAY_SIZE(buf);++i){
		if( *(buf[i].name) ){
			//printf("vlan[%d]=%s\n" , buf[i].vid, buf[i].name);
			if(raw_vlan_rm_ports(buf[i].vid, PBIT(port))<0)
				return -1;
		}
	}
#endif
	time_test_end();
	return 0;
}

int release_port(void * arg , int port , const vlan_t * exclude){
	time_test_begin();
	printf("RELEASE port %d...\n" , port);
#if  0
	//enable_mac_learning(port,true);
	const char ** namev = vlan_read_all_entry_name();
	for(int i=0;namev[i];i++){
		if(strcmp(exclude->name , namev[i])){
			struct timeval time2;
			//time_test("add port",&time2,0);
			int ret=vlan_add_port_into_entry(namev[i],port);
			//time_test("add port",&time2,1);
			if(ret<0)
				return -1;
		}
	}
#else
	vlan_t buf[64];
	FILE * fp = fopen(VLAN_DATA_PATH , "r");
	fread(buf , sizeof buf , 1 , fp);
	fclose(fp);
	for(int i=0;i<ARRAY_SIZE(buf);++i){
		if( *(buf[i].name) ){
			//printf("vlan[%d]=%s\n" , buf[i].vid, buf[i].name);
			if(raw_vlan_add_ports(buf[i].vid, PBIT(port))<0)
				return -1;
		}
	}
#endif
	time_test_end();
	return 0;
}

