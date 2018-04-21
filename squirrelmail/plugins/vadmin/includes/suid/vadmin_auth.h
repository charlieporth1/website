
/* error definitions */
#define ERR_OK 0
#define ERR_NO_RC_FILE 1
#define ERR_NO_CMD 2
#define ERR_USAGE 3
#define ERR_NOT_SUID 4
#define ERR_INVALID_COMMAND 5
#define ERR_USER_IS_ROOT 6
#define ERR_COPY_CANT_OPEN_SRC 7
#define ERR_COPY_CANT_OPEN_DEST 8
#define ERR_NEED_CREDENTIALS 9
#define ERR_CANT_READ_IMAP_SERVER 10
#define ERR_BAD_UID_GID 11
#define ERR_BAD_CREDENTIALS 12
#define ERR_BAD_LIBRARY 13
#define ERR_CANT_FIND_VIRTUAL_DOMAIN 14
#define ERR_CANT_TRUST_FILE 15
#define ERR_BAD_RC_FILE_TYPE 16
#define ERR_NO_RC_FILE_PATH 17
#define ERR_CANT_FIND_USER_IN_DATABASE 18
#define ERR_MYSQL_CONFIG_FILE_INCOMPLETE 19
#define ERR_LDAP_CONFIG_FILE_INCOMPLETE 20
#define ERR_MCRYPT_CANT_OPEN_MODULE 21
#define ERR_MCRYPT_CANT_INIT_MODULE 22
#define ERR_NEED_INPUT 23
#define ERR_BAD_DOMAIN_PWD 24
#define ERR_NO_ADMIN_RIGHTS 25

static char *err_strings[] = {
/* non error (error index 0) */
"",

/* silent error -- rcexists false result */
"",

"You forgot to provide a command.\n",

/* placeholder - usage errors are expected to be printed directly */
"Usage error - consult vadmin_auth.c\n",  

"Unable to setuid/setgid. Binary not setuid root?\n",

"Invalid command specified.\n",

"The root user is not allowed here for security reasons\n",

"Can't open source file\n",

"Can't open destination file\n",

"Credentials not passed correctly\n",

"Can't read imap server from configfile\n",

"Can't find uid/gid for user\n",

"Bad credentials\n",

"Can't verify credentials, IMAP not supported by library\n",

"Can't look up virtual user\n",

"Can't trust config file -- make sure they are all owned by root\n",

"Bad RC file type\n",

"No RC file path found\n",

"User not found in database\n",

"MySQL configuration file is incomplete\n",

"LDAP configuration file is incomplete\n",

"Cannot open mcrypt module\n",

"Could not init mcrypt module\n",

"Invalid input\n",

"Bad domain password\n",

"User does not have admin access\n",

};
