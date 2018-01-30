#ifndef __DBSFUNC_H__
#define __DBSFUNC_H__
#include "ip1829.h"
//
//IOCTRL Struct and Command ID
//
#define DBS_CREATE_DATA		1
#define DBS_CONFIG_DATA		2
#define DBS_CONFIG_SUBDATA	3
#define DBS_FIND_DATA		4
#define DBS_FIND_SUBDATA	5
#define DBS_BUILD_ROOT		6
#define DBS_DELETE_DATA		7
#define DBS_DELETE_SUBDATA	8
#define DBS_INIT_FILE		9
#define DBS_SAVE_FILE		10
#define DBS_FREE_FILE		11


#define COMMAND_CREATE_DATA			MAKECMDID(_CMDID_USG_DBS, 0, 0, DBS_CREATE_DATA)
#define COMMAND_CONFIG_DATA			MAKECMDID(_CMDID_USG_DBS, 0, 0, DBS_CONFIG_DATA)
#define COMMAND_CONFIG_SUBDATA		MAKECMDID(_CMDID_USG_DBS, 0, 0, DBS_CONFIG_SUBDATA)
#define COMMAND_FIND_DATA			MAKECMDID(_CMDID_USG_DBS, 0, 0, DBS_FIND_DATA)
#define COMMAND_FIND_SUBDATA		MAKECMDID(_CMDID_USG_DBS, 0, 0, DBS_FIND_SUBDATA)
#define COMMAND_BUILD_ROOT			MAKECMDID(_CMDID_USG_DBS, 0, 0, DBS_BUILD_ROOT)
#define COMMAND_DELETE_DATA			MAKECMDID(_CMDID_USG_DBS, 0, 0, DBS_DELETE_DATA)
#define COMMAND_DELETE_SUBDATA		MAKECMDID(_CMDID_USG_DBS, 0, 0, DBS_DELETE_SUBDATA)
#define COMMAND_INIT_FILE			MAKECMDID(_CMDID_USG_DBS, 0, 0, DBS_INIT_FILE)
#define COMMAND_SAVE_FILE			MAKECMDID(_CMDID_USG_DBS, 0, 0, DBS_SAVE_FILE)
#define COMMAND_FREE_FILE			MAKECMDID(_CMDID_USG_DBS, 0, 0, DBS_FREE_FILE)

#define DBS_SUCCESS			0
#define DBS_ERROR_FILE		1
#define DBS_ERROR_NAME		2
#define DBS_ERROR_SUBNAME	3
#define DBS_ERROR_OPENFILE	4
#define DBS_ERROR_READFILE	5
#define DBS_ERROR_LENGTH	6
#define DBS_ERROR_IOCTRL	7

typedef struct _dbsInfo{
	unsigned long cmdid;
	unsigned char result;
	unsigned char file[50];
	unsigned long entry_num;
	unsigned char name[50];
	unsigned char value[1024];
	unsigned char subname[50];
	unsigned char subvalue[50];
}m_dbsInfo, *p_dbsInfo;

void createData(void *cdata, unsigned long len);
void configData(void *cdata, unsigned long len);
void configSubData(void *cdata, unsigned long len);
void getData(void *cdata, unsigned long len);
void getSubData(void *cdata, unsigned long len);
void buildRoot(void *cdata, unsigned long len);
void deleteData(void *cdata, unsigned long len);
void deleteSubData(void *cdata, unsigned long len);
void initFile(void *cdata, unsigned long len);
void saveFile(void *cdata, unsigned long len);
void freeFile(void *cdata, unsigned long len);
#endif
