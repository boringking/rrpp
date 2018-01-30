#ifndef _LIBCGI_H_
#define _LIBCGI_H_
#include <stdio.h>
#include <unistd.h>
#include <wait.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <syslog.h>
#include <ctype.h>
#include "list.h"
#define CLISH_TIME_OUT "300"
#define BASE_FILE_PATH "/var/clish/"
#define XML_FILE_PATH  "/home/user/clish/"
#define SIZE_ARGV 0xFF
#define MAX_STR 1024
#define SIZE_RECVBUFFER		0x800//2K Bytes
#define CHECK_PID_BUF_SIZE	0x400//1K Bytes
#define FAIL 0
#define PASS 1
#define HTTPUNKNOWN 0
#define HTTPGET     1
#define HTTPPOST    2
#define OK			0
#define ERROR		-1

#define CLISH_ADMIN	"admin"
//#define VIRTUAL_TEST

#ifdef __cplusplus
extern "C" {
#endif
/************************************************************
 * CGI linked-list structure
************************************************************/
typedef struct cgiSubData{
	char *name;
	char *value;
	struct cgiSubData *next;
} m_cgiSubData, *p_cgiSubData;

typedef struct cgiData{
	unsigned short operation;	//'R':read, 'W':write
	char *name;
	unsigned short entry_num;
	char *value;
	p_cgiSubData entry;
	struct list_head list;
} m_cgiData, *p_cgiData;

typedef struct cgiRoot{
	char *filename;		//cgi name
	struct list_head list;
} m_cgiRoot, *p_cgiRoot;

/************************************************************
 * Name:	cgiParse
 * Description: Parse HTTP Get/Post string to CGI linked-list tree.
 * Parameters:	filename:	string of cgi name
 * Return value:
 *				success:	pointer of cgiRoot structure
 *				failed:		NULL
 * **********************************************************/
p_cgiRoot cgiParse(const char *filename);

/************************************************************
 * Name:	cgiFindData
 * Description: Search target data in cgi structure
 * Parameters:	root:		pointer of target cgi structure
 *				name:		pointer of name of target data
 *				entry_num:	entry number of target data
 * Return value:success:	pointer of target data
 *				failed:		NULL('cause target isn't existed)
 * **********************************************************/
p_cgiData cgiFindData(p_cgiRoot root, char *name, unsigned short entry_num);

/************************************************************
 * Name:	cgiConfigData
 * Description: Modify target data in cgi structure 
 * Parameters:	root:		pointer of target cgi structure
 *				name:		pointer of name of target data
 *				entry_num:	entry number of target data
 *				value:		new value of target data
 * Return value:success:	0
 *				failed:		-1('cause target data isn't existed)
 * **********************************************************/
int cgiConfigData(p_cgiRoot root, char *name, unsigned short entry_num, char *value);

/************************************************************
 * Name:	cgiFindSubData
 * Description: Search target sub. data in sub. data list
 * Parameters:	entry:	pointer of linked-list of first sub. data
 *				name:	pointer of name of target sub. data
 * Return value:success:	pointer of target sub. data
 *				failed:		NULL('cause target isn't existed)
 * **********************************************************/
p_cgiSubData cgiFindSubData(p_cgiSubData entry, char *name);

/************************************************************
 * Name:	cgiConfigSubData
 * Description: Modify target sub. data 
 * Parameters:	p_data:		pointer of target data
 *				p_sub:		pointer of target sub. data
 *				value:		pointer of new value of target sub. data
 * **********************************************************/
void cgiConfigSubData(p_cgiData p_data, p_cgiSubData p_sub, char *subvalue);

/************************************************************
 * Name:	cgiList2String
 * Description: transfer all data of cgi structrute to string 
 * Parameters:	root:		pointer of target cgi structure
 * Return value:success:	pointer of output string	
 *				failed:		NULL
 * **********************************************************/
char* cgiList2String(p_cgiRoot root);

/************************************************************
 * Name:	cgiFreeList
 * Description: Free allocated memory of target cgi structure 
 * Parameters:	root:		pointer of target cgi structure
 * Return value:success:	0
 *				failed:		-1
 * **********************************************************/
int cgiFreeList(p_cgiRoot root);

/************************************************************
 * Clish lib functions
************************************************************/
int isClishOK(char *pszCookie, const char *szCgiName);
int request_clish(char* pszCookie,char* command,int recvbuf_size,char* recvbuf);
int close_clish(char *pszCookie);

//szClishName: clish command; szCgiName: write to syslog
//int cgi_process(char szClishName[], char szCgiName[]);

#ifdef __cplusplus
}
#endif

#endif

