#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <unistd.h>
#include "main.h"
#include "vlan.h"
#include "major_node.h"
#include "transport_node.h"
#include "rrpp_regs.h"

/*
因为该程序可能被cgi程序调用，所以
标准输入/标准输出/标准错误可能不是指向串口
该函数将标准输入/标准输出/标准错误指向串口
*/
void reset_tty(void){
	int fd = open("/dev/console" , O_RDWR);
	if(fd<0)
		return;
	dup2(fd , STDOUT_FILENO);
	dup2(fd , STDIN_FILENO);
	dup2(fd , STDERR_FILENO);
	if(fd>STDERR_FILENO)
		close(fd);

	/* set stdout as line-buffer, stderr as no-buffer */
	if(setvbuf(stdout , malloc(1024) ,_IOLBF ,1024) || setvbuf(stderr ,NULL ,_IONBF ,0)){
		perror("setvbuf");
		exit(EXIT_FAILURE);
	}
}

/* 不管大小写的strcmp */
int non_case_strcmp(const char * __s1,const char * __s2){
	int i;
	char s1[128] , s2[128];
	strcpy(s1, __s1);
	strcpy(s2, __s2);
	for(i=0;i<strlen(s1);i++)
		s1[i] = tolower(s1[i]);
	for(i=0;i<strlen(s2);i++)
		s2[i] = tolower(s2[i]);
	return strcmp(s1,s2);
}

/* 启动主节点 */
int major_main(int argc , char ** argv){
	uint16_t did=0xffff , rid=0xffff;
	uint16_t vid[2] ={0xffff,0xffff};
	uint16_t main_portno=0xffff,second_portno=0xffff;

	int opt;
	/*
	  获取参数
	  例如 
 	  rrpp --major -d0 -r0 -v10 -m4 -s5
	  表示domain id =0
	      ring id = 0
	      vid = 10
	      mainport = 4
	      secondport=5
	*/
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
	// 构造主节点对象(初始化数据，创建线程等等)
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
	// 与主节点类似，参考major_main里的注释
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
	// 构造传输节点对象(初始化数据，创建线程等等)
	if(transport_node_constructor(&transport,did,rid,vid,main_portno,second_portno)<0){
		fprintf(stderr , "transport_node_constructor: failed\n");
		exit(EXIT_FAILURE);
	}
	for(;;){
		sleep(10000);
	}
	return 0;
}

/*
使进程成为精灵进程，解除对终端的占用
*/
int boot_daemon (int mode) {
	pid_t pid = fork ();

	if (pid == -1){
		perror("fork");
		return -1;
	}

	if (pid)
		exit (EXIT_SUCCESS);

	if (2!=mode){
		setsid ();

		int fd = open("/dev/null", O_RDWR, 0);
		if (fd != -1) {
			dup2 (fd, STDIN_FILENO);
			dup2 (fd, STDOUT_FILENO);
			dup2 (fd, STDERR_FILENO);
			if (fd > STDERR_FILENO)
				close (fd);
		}

		if( (fd = open("/home/rrpp/log" , O_WRONLY|O_APPEND|O_CREAT , 0664))!=-1 ){
			dup2 (fd, STDERR_FILENO);
			if (fd > STDERR_FILENO)
				close (fd);
			fprintf(stderr , "----------rrpp-log----------\n");
			return 0;
		}
	}
	return -1;
}

/*
usage of the program:
主节点:
rrpp --major <-d domain_id> <-r ring_id> <-v vid1[,vid2]> <-m mainport_number> <-s secondport_number>
传输节点:
rrpp --transport <-d domain_id> <-r ring_id> <-v vid1[,vid2]> <-m mainport_number> <-s secondport_number>
*/
int main(int argc, char ** argv){
	/* restore fd-0,1,2 to console if called by cgi */
	reset_tty();
	//printf("finish reset tty\n");
	if( argc < 2){
		prompt(argv);
		exit(EXIT_FAILURE);
	}
	int new_argc = argc-1;
	char ** new_argv = argv+1;
	/* boot as daemon */
	if( !strcmp("--daemon" , argv[1]) )
		boot_daemon(0);
	else if( !strcmp("--daemon2" , argv[1]) )
		boot_daemon(2);
	else
		goto NO_DAEMON;
	new_argc--;
	new_argv++;
NO_DAEMON:
	if( !non_case_strcmp(argv[argc-new_argc] , "--major") ){
		return major_main(new_argc,new_argv);
	}
	if( !non_case_strcmp(argv[argc-new_argc] , "--transport") ){
		return transport_main(new_argc,new_argv);
	}
	/* no match for argv[1] */
	prompt(argv);
	return EXIT_FAILURE;
}

