#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "main.h"
#include "vlan.h"
#include "major_node.h"
#include "transport_node.h"
#include "rrpp_regs.h"


int non_case_strcmp(const char * __s1,const char * __s2){
	int i;
	char s1[128] , s2[128];
	strcpy(s1, __s1);
	strcpy(s2, __s2);
	for(i=0;i<strlen(s1);i++)
		s1[i] = tolower(s1[i])
	for(i=0;i<strlen(s2);i++)
		s2[i] = tolower(s2[i])
	return strcmp(s1,s2);
}


int major_main(int argc , char ** argv){
	uint16_t did=0xffff , rid=0xffff;
	uint16_t vid[2] ={0xffff,0xffff};
	uint16_t main_portno=0xffff,second_portno=0xffff;

	int opt;
	while( (opt=getopt(argc,argv,"d:r:v:m:s:"))!=-1 ){
		switch(opt){
			case 'd':  // domain id
				did = atoi(optarg);
				break;
			case 'r': // ring id
				rid = atoi(optarg);
				break;
			case 'v': // vid
				/*
				vlan[0].vid = atoi(optarg);
				sprintf(vlan[0].name , "rrpp_vlan%d",vlan[0].vid);
				if( strchr(optarg,',') ){
					vlan[1].vid = atoi(strchr(optarg,',')+1);
					sprintf(vlan[1].name , "rrpp_vlan%d",vlan[1].vid);
				}else{
					*(vlan[1].name) = 0;
					vlan[1].vid = 0;
				}*/
				vid[0] = atoi(optarg);
				if( strchr(optarg,',') )
					vid[1] = atoi(strchr(optarg,',')+1);
				else
					vid[1] = 0;
				break;
			case 'm': // main port
				main_portno = atoi(optarg);
				break;
			case 's': // secondary port
				second_portno = atoi(optarg);
				break;
			default: // unknown option
				major_prompt(argv);
				exit(EXIT_FAILURE);
		}
	}
	if(0xffff==did ||
	   0xffff==rid ||
	   0xffff==vid[0] ||
	   0xffff==vid[1] ||
	   0xffff==main_portno ||
	   0xffff==second_portno)
	{
		major_prompt(argv);
		exit(EXIT_FAILURE);
	}
	printf("did = %d\n",did);
	printf("rid = %d\n",rid);
	printf("vlan = (%d)+(%d)\n",vid[0],vid[1]);
	printf("main_portno = %d\n",main_portno);
	printf("second_portno = %d\n",second_portno);
	/* boot major node... */
	printf("====>boot MAJOR node...........\n");
	struct major_node major;
	if(major_node_constructor(&major,did,rid,vid,main_portno,second_portno)<0){
		fprintf(stderr , "major_node_constructor: failed\n");
		exit(EXIT_FAILURE);
	}
	for(;;){
		sleep(10000);
	}
	return 0;
}



int transport_main(int argc , char ** argv){
	uint16_t did=0xffff , rid=0xffff;
	uint16_t vid[2] ={0xffff,0xffff};
	uint16_t main_portno=0xffff,second_portno=0xffff;

	int opt;
	while( (opt=getopt(argc,argv,"d:r:v:m:s:"))!=-1 ){
		switch(opt){
			case 'd':  // domain id
				did = atoi(optarg);
				break;
			case 'r': // ring id
				rid = atoi(optarg);
				break;
			case 'v': // vid
				/*
				vlan[0].vid = atoi(optarg);
				sprintf(vlan[0].name , "rrpp_vlan%d",vlan[0].vid);
				if( strchr(optarg,',') ){
					vlan[1].vid = atoi(strchr(optarg,',')+1);
					sprintf(vlan[1].name , "rrpp_vlan%d",vlan[1].vid);
				}else{
					*(vlan[1].name) = 0;
					vlan[1].vid = 0;
				}*/
				vid[0] = atoi(optarg);
				if( strchr(optarg,',') )
					vid[1] = atoi(strchr(optarg,',')+1);
				else
					vid[1] = 0;
				break;
			case 'm': // main port
				main_portno = atoi(optarg);
				break;
			case 's': // secondary port
				second_portno = atoi(optarg);
				break;
			default: // unknown option
				transport_prompt(argv);
				exit(EXIT_FAILURE);
		}
	}
	if(0xffff==did ||
	   0xffff==rid ||
	   0xffff==vid[0] ||
	   0xffff==vid[1] ||
	   0xffff==main_portno ||
	   0xffff==second_portno)
	{
		transport_prompt(argv);
		exit(EXIT_FAILURE);
	}
	printf("did = %d\n",did);
	printf("rid = %d\n",rid);
	printf("vlan = (%d)+(%d)\n",vid[0],vid[1]);
	printf("main_portno = %d\n",main_portno);
	printf("second_portno = %d\n",second_portno);
	/* boot transport node... */
	printf("====>boot TRANSPORT node...........\n");
	struct transport_node transport;
	if(transport_node_constructor(&transport,did,rid,vid,main_portno,second_portno)<0){
		fprintf(stderr , "transport_node_constructor: failed\n");
		exit(EXIT_FAILURE);
	}
	for(;;){
		sleep(10000);
	}
	return 0;
}

// 使调用进程成为精灵进程
int boot_daemon (void) {
	// 创建子进程
	pid_t pid = fork ();

	// 若失败
	if (pid == -1){
		perror("fork");
		return -1;
	}

	// 若为父进程
	if (pid)
		// 退出，使子进程成为孤儿进程并被init进程收养
		exit (EXIT_SUCCESS);

	// 子进程创建新会话并成为新会话中唯一进程组的组长
	// 进程，进而与原会话、原进程组和控制终端脱离关系
	setsid ();

	// 打开空设备文件
	int fd = open("/dev/null", O_RDWR, 0);
	// 若成功
	if (fd != -1) {
		// 复制空设备文件描述符到标准输入
		dup2 (fd, STDIN_FILENO);
		// 复制空设备文件描述符到标准输出
		dup2 (fd, STDOUT_FILENO);
		// 复制空设备文件描述符到标准出错
		dup2 (fd, STDERR_FILENO);
		// 若空设备文件描述符大于标准出错
		if (fd > STDERR_FILENO)
			// 关闭空设备文件描述符
			close (fd);
	}
/*
	// 若配置器中的日志文件路径非空且打开(创建)日志文件成功
	if (! m_cfg.m_logFile.empty () &&
		(fd = open (m_cfg.m_logFile.c_str (),
		O_WRONLY | O_APPEND | O_CREAT, 0664)) != -1) { */

	if( (fd = open("/home/rrpp/log" , O_WRONLY|O_APPEND|O_CREAT , 0664))!=-1 ){
		// 复制日志文件描述符到标准输出
		dup2 (fd, STDERR_FILENO);
		// 若日志文件描述符大于标准出错
		if (fd > STDERR_FILENO)
			// 关闭日志文件描述符
			close (fd);
		fprintf(stderr , "----------rrpp-log----------\n");
		return 0;
	}
	return -1;
}

int main(int argc, char ** argv){
	if( argc < 2){
		prompt(argv);
		exit(EXIT_FAILURE);
	}
	int new_argc = argc-1;
	char ** new_argv = argv+1;
	/* boot as daemon */
	if( non_case_strcmp("--daemon" , argv[1]) )
		goto NO_DAEMON;
	boot_daemon();
	new_argc--;
	new_argv++;
NO_DAEMON:
	if( !non_case_strcmp(argv[argc-new_argc] , "--major") ){
		return major_main(new_argc,new_argv);
	}
	if( !non_case_strcmp(argv[argc-new_argc] , "--trans") ){
		return transport_main(new_argc,new_argv);
	}
	/* no match for argv[1] */
	prompt(argv);
	return EXIT_FAILURE;
}

