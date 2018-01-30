#include <sys/ioctl.h>
#include <sys/errno.h>
#include "fcntl.h"
#include "unistd.h"
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "dbsfunc.h"

#define IP1829_DBS_TYPE '\x12'
/************************************************************
 * Name:	cfgCreateData
 * Description: Create new data into Data Base 
 * Parameters:	filename:	pointer of the file whitch need to add new data
 *				name:		pointer of name/entry of new data
 *				value:		pointer of value of new data
 *				entrytype:	if new data is an entry, set this argument to 1,
 *							else set it ot 0
 * Return value:success:	0
 *				failed:		!0
 * **********************************************************/
int cfgCreateData(char *filename, char *name, char *value, bool entrytype);
/************************************************************
 * Name:	cfgConfigData
 * Description: Modify target data in Data Base 
 * Parameters:	filename:	pointer of the file whitch need to config
 *				name:		pointer of name/entry of target data
 *				entry_num:	entry number of target data
 *				value:		new value of target data
 * Return value:success:	0
 *				failed:		!0
 * **********************************************************/
int cfgConfigData(char *filename, char *name, unsigned short entry_num, char *value);
/************************************************************
 * Name:	cfgConfigSubData
 * Description: Modify target sub. data in Data Base 
 * Parameters:	filename:	pointer of the file whitch need to config
 *				name:		pointer of name/entry of target data
 *				entry_num:	entry number of target data
 *				subname:	pointer of subname
 *				subvalue:   pointer of new data of target sub. data
 * Return value:success:	0
 *				failed:		!0
 * **********************************************************/
int cfgConfigSubData(char *filename, char *name, unsigned short entry_num, char *subname, char *subvalue);
/************************************************************
 * Name:	cfgFindData
 * Description: Find target data in Data Base 
 * Parameters:	filename:	pointer of the file whitch need to Find
 *				name:		pointer of name/entry of target data
 *				entry_num:	entry number of target data
 *				value:		OUTPUT pointer of find value of target data
 * Return value:success:	0
 *				failed:		!0
 * **********************************************************/
int cfgFindData(char *filename, char *name, unsigned short entry_num, char *value);
/************************************************************
 * Name:	cfgFindSubData
 * Description: Find target sub. data in Data Base 
 * Parameters:	filename:	pointer of the file whitch need to Find
 *				name:		pointer of name/entry of target data
 *				entry_num:	entry number of target data
 *				subname:	pointer of target sub. data name
 *				subvalue:	OUTPUT pointer of find value of target sub. data
 * Return value:success:	0
 *				failed:		!0
 * **********************************************************/
int cfgFindSubData(char *filename, char *name, unsigned short entry_num, char *subname, char *subvalue);
/************************************************************
 * Name:	cfgBuldRoot
 * Description: Create new file to Data Base 
 * Parameters:	filename:	pointer of the file whitch need to Create
 * Return value:success:	0
 *				failed:		!0
 * **********************************************************/
int cfgBuildRoot(char *filename);
/************************************************************
 * Name:	cfgDeleteData
 * Description: Delete target data in Data Base 
 * Parameters:	filename:	pointer of the file whitch need to delete
 *				name:		pointer of name/entry of target data
 *				entry_num:	entry number of target data
 * Return value:success:	0
 *				failed:		!0
 * **********************************************************/
int cfgDeleteData(char *filename, char *name, unsigned short entry_num);
/************************************************************
 * Name:	cfgDeleteSubData
 * Description: Delete target sub. data in Data Base 
 * Parameters:	filename:	pointer of the file whitch need to delete
 *				name:		pointer of name/entry of target data
 *				entry_num:	entry number of target data
 *				subname:	pointer of sub. name of target data
 * Return value:success:	0
 *				failed:		!0
 * **********************************************************/
int cfgDeleteSubData(char *filename, char *name, unsigned short entry_num, char *subname);
/************************************************************
 * Name:	cfgInitFile
 * Description: Initial exist file to data base
 * Parameters:	filename:	pointer of the file
 * Return value:success:	0
 *				failed:		!0
 * **********************************************************/
int cfgInitFile(char *filename);
/************************************************************
 * Name:	cfgSaveFile
 * Description: Save data base information to file
 * Return value:success:	0
 *				failed:		!0
 * **********************************************************/
int cfgSaveFile(char *filename);
/************************************************************
 * Name:	cfgFreeFile
 * Description: Free all dbs driver file memory
 * Return value:success:	0
 *				failed:		!0
 * **********************************************************/
int cfgFreeFile();
