#include <stdbool.h>
//
//Parameters:
//	name - useraccound
//	passwd - password
//Description:
//	If name is already exist, set the passwd to the new password
//	else create a new account by name, and set it's password by passwd
//Return value: TRUE or FLASE
//
bool uacUserAdd(char *name, char *passwd);

//
//Parameter:
//	name - useraccount
//Description:
//	Delete useraccount by name
//Return value: TRUE or FALSE
//
bool uacUserDel(char *name);

//
//Parameter:
//	name - useraccount to get password
//	passwd - password buffer
//	passwd_len - input buffer len
//Description:
//
//Return value: passwd length
//
bool uacChkPasswd(char *name, char *passwd);

