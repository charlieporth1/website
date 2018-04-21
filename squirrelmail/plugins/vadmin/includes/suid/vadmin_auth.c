/*  NAME:
 * 
 *	vadmin_auth -- authenticates and proxys administrative access to 
 *                     mail server administrative scripts
 *
 *  SYNOPSYS:
 *
 * 	vadmin_auth [passthruargs]
 *  
 *  DESCRIPTION:
 *	
 *	The vadmin_auth program allows an unpriviledged program (SquirrelMail)
 *      to connect to a system management backend that requires root 
 *      privileges. The backend called here is configurable in the Makefile
 *      and will be handed any and all arguments that are passed in here.
 *      Actions taken before gaining root and calling the backend application
 *      are:
 *              - Verify the domain password (from STDIN)
 *              - Check that the current user (from STDIN) is a legal 
 *                Vadmin administrator
 *              - Check that the user has a valid IMAP login (password
 *                also from STDIN)
 *      This program also uses the same Vadmin cryptography features that 
 *      are a part of the main Vadmin PHP code.
 *
 *  CREDENTIALS
 *
 *	Credentials are passed to standard input. Each input item should be 
 *      terminated with a newline (\n) character. No other characters should 
 *      be passed. These credential items are required:
 *
 *              - Vadmin internal encryption algorithm
 *              - Vadmin internal encryption hashline
 *              - Current Vadmin domain
 *              - Domain password
 *              - Username
 *              - User password
 *	
 *	The encryption credentials are used to unencrypt the domain password
 *      and domain administrator list, which are all read from whatever database
 *      backend Vadmin uses (this must be synchronized between the main Vadmin
 *      configuration file and this program's Makefile). 
 *
 *      The domain and domain password are then verified against Vadmin's stored 
 *      version thereof.  The username credential is also compared to the list 
 *      of Vadmin administrators.
 *	
 *	Finally, the username and password credentials are used to log in to an 
 *      IMAP server. The server host name and port is read from the SquirrelMail 
 *      configuration file.
 *	
 * 	IMAP authentication is performed using the c-client software from UW or
 *      or own simplified version thereof.
 *	See checkcreds_cclient.c or checkcreds_imap.c for more details.
 *	
 *  DIAGNOSTICS
 *
 *	Setting an environment variable DEBUG to 1 will 
 *	cause debugging output to be printed (to stderr).
 *
 *	vadmin_auth will exit with a non-zero status if an error occurs.
 *	vadmin_auth will write text to standard error in the case of
 *	some errors.  See vadmin_auth.h for details.
 *	
 */
 
#define VERSION "vadmin_auth 1.0, part of Vadmin plugin for SquirrelMail"



#define STR_MAX 1024
#define CODE_MAX 4096
#define MAX_ARGS 30



/* define this in Makefile */
#ifndef SQUIRRELMAILCONFIGFILE
#define SQUIRRELMAILCONFIGFILE "/etc/squirrelmail/config.php"
#endif
#ifndef VADMINCONFIG
#define VADMINCONFIG "../../plugins/vadmin/conf"
#endif
#ifndef VADMINCONFIGFILE
#define VADMINCONFIGFILE "vadmin.conf.php"
#endif
#ifndef PHPPATH
#define PHPPATH "/usr/bin/php"
#endif



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include "vadmin_auth.h"



/* externally defined functions and variables */
extern char *checkcreds_extra_configvar;
int  checkcredentials(char*, int, char*, char*, char*);



/* function prototypes */
void eperror(register char *s, int exitcode);



/* main entry point */
int main(int argc, char *argv[]){

   int err;
   char checkcreds_extra[STR_MAX] = "";
   char imap_server[STR_MAX];
   int imap_port;
   char elvises[STR_MAX];
   char storage_dir[STR_MAX];
   char db_suffix[STR_MAX];
   char algo[STR_MAX];
   char hashline[STR_MAX];
   char domain[STR_MAX];
   char domain_pwd[STR_MAX];
   char *domain_pass;
   char domain_pwd_from_user[STR_MAX];
   char password[STR_MAX];
   char user[STR_MAX];
   char *start;
   char *end;
   char db_path[STR_MAX];
   char key[STR_MAX];
   char value[STR_MAX];
   char lookup[STR_MAX];
   char lowlies[STR_MAX];
   char xadmins[STR_MAX];
   char xadminsfordomain[STR_MAX];



   /* get algo from STDIN */
   err = readvar(algo, STR_MAX);
   if (err) return inerror(err);

   /* get hashline from STDIN */
   err = readvar(hashline, STR_MAX);
   if (err) return inerror(err);

   /* get domain from STDIN */
   err = readvar(domain, STR_MAX);
   if (err) return inerror(err);

   /* get domain password from STDIN */
   err = readvar(domain_pwd_from_user, STR_MAX);
   if (err) return inerror(err);

   /* get username and password from STDIN */
   err = readcredentials(user, password, STR_MAX);
   if (err) return inerror(err);



   /* get IMAP server address and port from config file */
   err = readimapserver(imap_server, &imap_port, STR_MAX);
   if (err) return inerror(err);

   /* get elvises from config file */
   readconfigvar(VADMINCONFIG "/" VADMINCONFIGFILE, "elvis", elvises, STR_MAX, 1);

   /* get storage directory and suffix from config file */
   readconfigvar(VADMINCONFIG "/" VADMINCONFIGFILE, "dir", storage_dir, STR_MAX, 1);
   readconfigvar(VADMINCONFIG "/" VADMINCONFIGFILE, "suffix", db_suffix, STR_MAX, 1);



   /* Grab additional check creds vars as defined elsewhere */
   if (strlen(checkcreds_extra_configvar))
   {
      readconfigvar(VADMINCONFIG "/" VADMINCONFIGFILE, checkcreds_extra_configvar, checkcreds_extra, STR_MAX, 1);
   }



   /* Auth user against IMAP server */
   err = checkcredentials(imap_server, imap_port, user, password, checkcreds_extra);
   #ifdef DEBUG
   fprintf(stderr, "Check IMAP credentials result = %d\n\n", err);
   #endif



   /* If we didn't get IMAP auth, bail! */
   if (err) return err;



   /* Get domain password for this domain */

   /* construct database path */
   strncpy(db_path, storage_dir, strlen(storage_dir));
   db_path[strlen(storage_dir)] = '\0';
   strncat(db_path, "/", STR_MAX - strlen(db_path));
   strncat(db_path, domain, STR_MAX - strlen(db_path));
   strncat(db_path, db_suffix, STR_MAX - strlen(db_path));

   /* construct lookup key name */
   strncpy(lookup, domain, strlen(domain));
   lookup[strlen(domain)] = '\0';
   strncat(lookup, "passwd", STR_MAX - strlen(lookup));

   /* encrypt the lookup key */
   vadmin_crypto(algo, hashline, "encrypt", lookup, key);

   /* now retrieve from database */
   get_vadmin_stored_value(db_path, key, value);

   /* decrypt the lookup value */
   if (strcmp("", value) == 0)
   {
      domain_pwd[0] = '\0';
      domain_pass = domain_pwd;
   }
   else
   {
      vadmin_crypto(algo, hashline, "decrypt", value, domain_pwd);

      /* strip off PHP serialization crap from beginning */
      /* remove starting s:25:" and ending ";\0          */
      domain_pass = domain_pwd;
      /* domain_pass += strspn(domain_pass, "s:"); */
      domain_pass += 2;
      domain_pass += strspn(domain_pass, "1234567890"); 
      domain_pass += 2;
      domain_pass[strlen(domain_pass) - 2] = '\0';
   }

   #ifdef DEBUG
   fprintf(stderr, "FINISHED CALCULATING DOMAIN PASSWORD\n-------------------------------------\nDB path: %s\nLookup: %s\nEncrypted lookup: %s\nEncrypted result: %s\nDecrypted result: %s\n\n", db_path, lookup, key, value, domain_pass);
   #endif


   /* Get lowly admins for this domain */

   /* construct database path */
   strncpy(db_path, storage_dir, strlen(storage_dir));
   db_path[strlen(storage_dir)] = '\0';
   strncat(db_path, "/", STR_MAX - strlen(db_path));
   strncat(db_path, domain, STR_MAX - strlen(db_path));
   strncat(db_path, db_suffix, STR_MAX - strlen(db_path));

   /* construct lookup key name */
   strncpy(lookup, domain, strlen(domain));
   lookup[strlen(domain)] = '\0';
   strncat(lookup, "admins", STR_MAX - strlen(lookup));

   /* encrypt the lookup key */
   vadmin_crypto(algo, hashline, "encrypt", lookup, key);

   /* now retrieve from database */
   get_vadmin_stored_value(db_path, key, value);

   /* decrypt the lookup value */
   if (strcmp("", value) == 0)
   {
      lowlies[0] = '\0';
   }
   else
   {
      vadmin_crypto(algo, hashline, "decrypt", value, lowlies);
   }

   #ifdef DEBUG
   fprintf(stderr, "FINISHED CALCULATING LOWLIES\n-------------------------------------\nDB path: %s\nLookup: %s\nEncrypted lookup: %s\nEncrypted result: %s\nDecrypted result: %s\n\n", db_path, lookup, key, value, lowlies);
   #endif



   /* Get cross admins */

   /* construct database path */
   strncpy(db_path, storage_dir, strlen(storage_dir));
   db_path[strlen(storage_dir)] = '\0';
   strncat(db_path, "/master", STR_MAX - strlen(db_path));
   strncat(db_path, db_suffix, STR_MAX - strlen(db_path));

   /* construct lookup key name */
   strncpy(lookup, "master", strlen("master"));
   lookup[strlen("master")] = '\0';
   strncat(lookup, "cross-admins", STR_MAX - strlen(lookup));

   /* encrypt the lookup key */
   vadmin_crypto(algo, hashline, "encrypt", lookup, key);

   /* now retrieve from database */
   get_vadmin_stored_value(db_path, key, value);

   /* decrypt the lookup value */
   if (strcmp("", value) == 0)
   {
      xadmins[0] = '\0';
   }
   else
   {
      vadmin_crypto(algo, hashline, "decrypt", value, xadmins);
   }

   /* now pull out the admin string for just our domain */
   if ((start = strstr(xadmins, domain)) == NULL 
    || *(start - 1) != '"'
    || *(start + strlen(domain)) != '"')
   {
      #ifdef DEBUG
      fprintf(stderr, "Domain not found in cross-admin database.\n");
      #endif
      xadminsfordomain[0] = '\0';
   }
   else
   {
      if ((end = strchr(start, '}')) == NULL)
      {
         #ifdef DEBUG
         fprintf(stderr, "Badly formatted cross-admin string.\n");
         #endif
         xadminsfordomain[0] = '\0';
      }
      else
      {
         strncpy(xadminsfordomain, start, end - start + 1);
         xadminsfordomain[end - start + 1] = '\0';
         #ifdef DEBUG
         fprintf(stderr, "New cross-admin string: %s\n", xadminsfordomain);
         #endif
      }
   }
   
   #ifdef DEBUG
   fprintf(stderr, "FINISHED CALCULATING X-ADMINS\n--------------------------------------\nDB path: %s\nLookup: %s\nEncrypted lookup: %s\nEncrypted result: %s\nDecrypted result: %s\nParsed cross-admin string: %s\n\n", db_path, lookup, key, value, xadmins, xadminsfordomain);
   #endif
   


   /* Separate elvis names (comma/whitespace) */
   char *elvtok;
   int isElvis = 0;
   elvtok = strtok(elvises, ", \t\n");
   while (elvtok != NULL)
   {
      #ifdef DEBUG
      fprintf(stderr, "Checking elvis --::%s::--\n", elvtok);
      #endif
      if (strcmp(user, elvtok) == 0)
      {
         isElvis = 1;
         break;
      }
      elvtok = strtok(NULL, ", \t\n");
   }



   /* Check user access */



   /* Check if elvis */
   if (isElvis == 1)
   {
      #ifdef DEBUG
      fprintf(stderr, "Elvis is in the building!\n\n");
      #endif
   }



   /* Check if cross admin (only if not an elvis) */
   else if ((start = strstr(xadminsfordomain, user)) != NULL
       && *(start - 1) == '"'
       && *(start + strlen(user)) == '"')
   {
      #ifdef DEBUG
      fprintf(stderr, "We have a cross!\n\n");
      #endif
   }



   /* Check if lowly (only if not a cross admin) */
   else if ((start = strstr(lowlies, user)) != NULL
       && *(start - 1) == '"'
       && *(start + strlen(user)) == '"')
   {
      #ifdef DEBUG
      fprintf(stderr, "We have a LOWLY!\n\n");
      #endif
   }



   /* No admin rights, kick this user */
   else
   {
      #ifdef DEBUG
      fprintf(stderr, "NO admin priv!\n\n");
      #endif
      return inerror(ERR_NO_ADMIN_RIGHTS);
   }



   /* Check domain password */
   if (strcmp(domain_pass, domain_pwd_from_user) != 0)
   { 
      #ifdef DEBUG
      fprintf(stderr, "Bad domain password! (user gave \"%s\", correct password is \"%s\")\n", domain_pwd_from_user, domain_pass);
      #endif
      return inerror(ERR_BAD_DOMAIN_PWD);
   }


   /* Now change UID to root and pass command to shell */



   /* go root */
   #ifdef DEBUG
   fprintf(stderr, "My UID:GID is currently %d:%d\n", getuid(), getgid());
   #endif
   if((setgid(0)) < 0) eperror("setgid", ERR_NOT_SUID);
   if((setuid(0)) < 0) eperror("setuid", ERR_NOT_SUID);
   #ifdef DEBUG
   fprintf(stderr, "Now, after changing to root, my UID:GID is %d:%d\n", getuid(), getgid());
   #endif


   /* construct script call */
   char *arguments[MAX_ARGS];
   char command[STR_MAX];

   snprintf(command, (sizeof(command) - 1), MANAGER_SCRIPT);
   command[sizeof(command) - 1] = '\0';
   arguments[0] = command;

   char *argtok;
   char defaultargs[STR_MAX];
   char *an_arg;
   int i = 1;
   strncpy(defaultargs, MANAGER_OPTIONS, sizeof(defaultargs) - 1);
   defaultargs[sizeof(defaultargs) - 1] = '\0';
   argtok = strtok(defaultargs, " \t");
   while (argtok != NULL)
   {
      #ifdef DEBUG
      fprintf(stderr, "manager option: %s\n", argtok);
      #endif
      an_arg = malloc(strlen(argtok) + 1);
      strncpy(an_arg, argtok, strlen(argtok));
      an_arg[strlen(argtok)] = '\0';
      arguments[i] = an_arg;
      argtok = strtok(NULL, " \t");
      i++;
   }

   /* copy command line args too */
   int j;
   for (j = i; j - i + 1 < argc; j++)
   {

         arguments[j] = argv[j - i + 1]; 
/* Passwords with spaces in them seem to be not sent as
   one argument(?), but putting quotes around them seems
   to be sending the quotes as part of the string instead
   of delimiters.... bah, not worth the trouble of figuring
   out, since we just disallow spaces in passwords anyway */
      /* put quotes around arguments that contain spaces */
/*
Hrmph .... doesn't work
      if (strchr(argv[j - i + 1], ' ') == NULL)
      {
         arguments[j] = argv[j - i + 1]; 
      }
      else
      {
         arguments[j] = malloc(strlen(argv[j - i + 1]) + 4);
         sprintf(arguments[j], "\"%s\"", argv[j - i + 1]);
      }
*/
      /* put quotes around arguments that don't start with a dash */
/*
2nd choice, but also doesn't work
      if (argv[j - i + 1][0] == '-')
      {
         arguments[j] = argv[j - i + 1]; 
      }
      else
      {
         arguments[j] = malloc(strlen(argv[j - i + 1]) + 4);
         sprintf(arguments[j], "\"%s\"", argv[j - i + 1]);
      }
*/

   }
   arguments[j] = "\0";

   #ifdef DEBUG
   fprintf(stderr, "%s: %s--%s--%s--%s--%s--%s--%s--%s--%s\n", command, arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8]);
   #endif



   /* yikes - go call script */
   int returnValue;
   returnValue = execv(command, arguments);
   perror("didn't exec"); 
   _exit(99);

}



void eperror(s, exitcode)
register char *s;
int exitcode;
{
   char str[STR_MAX];

   snprintf(str,STR_MAX,"vadmin_auth: %s",s);
   perror(str);
   exit(exitcode);
}

int inerror(errnum)
int errnum;
{
	fprintf(stderr, "%s", err_strings[errnum]);
	return errnum;	
}



/** 
  * Read a variable from STDIN.
  *
  * Variable should be contained on one line and 
  * terminated with newline, and should not contain spaces.
  *
  * @param var OUT This buffer is filled with the variable
  *                value read from STDIN.
  * @param buflen IN The buffer size for the buffer.
  *
  * @return ERR_OK if the variable was read successfully, 
  *         otherwise, non-zero error code.
  *
  */
int readvar(char *var, int buflen)
{

   int n;
   if (fgets(var, buflen, stdin) == NULL) 
      return ERR_NEED_INPUT;

   n = strlen(var);
   if (n < 2)
      return ERR_NEED_INPUT;

   if (var[n-1] == '\n') 
      var[n-1] = '\0';    /* remove \n */

   return ERR_OK;

}



/** 
  * Read credentials from STDIN.
  * Username and passwd should each be on a separate 
  * line.  Username may not include spaces. 
  * FIXME: Password may not include spaces either, currently.
  *
  * @param user OUT The buffer is filled with username read 
  *                 from stdin.
  * @param passwd OUT The buffer is filled with password 
  *                   read from stdin.
  * @param buflen IN The buffer size for both buffers.
  *
  * @return ERR_OK if the credentials were read successfully, 
  *         otherwise, non-zero error code.
  *
  */
int readcredentials(char *user, char *passwd, int buflen)
{

   int n;
   if (fgets(user, buflen, stdin) == NULL) 
      return ERR_NEED_CREDENTIALS;

   n = strlen(user);
   if (n < 2)
      return ERR_NEED_CREDENTIALS;

   if (user[n-1] == '\n') 
      user[n-1] = '\0';    /* remove \n */

   if (fgets(passwd, buflen, stdin) == NULL) 
      return ERR_NEED_CREDENTIALS;

   n = strlen(passwd);
   if (n < 2)
      return ERR_NEED_CREDENTIALS;

   if (passwd[n-1] == '\n')
      passwd[n-1] = '\0';    /* remove \n */

   return ERR_OK;

}



/**
  * Reads IMAP server information from SQUIRRELMAILCONFIGFILE.
  *
  * Also reads the port. The routine is not fussy about 
  * finding a port specification in the file -- it will just 
  * set a useful default if there are issues.
  *
  * @param server OUT The server name read from config.php
  * @param port OUT The server post read from config.php
  * @param serverlen IN Buffer size of the server buffer
  *
  * @return ERR_OK if it found the necessary parameters, or
  *         an error code if not.
  *
  */
int readimapserver(char *server, int *port, int serverlen) 
{

   int foundserver;
   char port_str[STR_MAX];
	
   *port = 143; /* a useful default */
   if (readconfigvar(SQUIRRELMAILCONFIGFILE, "$imapPort", port_str, STR_MAX, 0)) 
   {
      sscanf(port_str, "%d", port);
   }
	
   foundserver = readconfigvar(SQUIRRELMAILCONFIGFILE, "$imapServerAddress", server, serverlen, 0);
   return foundserver ? ERR_OK : ERR_CANT_READ_IMAP_SERVER;

}



/**
  * Reads a setting from a config file.
  *
  * @param phpvar IN The name of the PHP variable 
  *                  to parse out
  * @param outbuf OUT The setting as read from the file
  * @param outbuflen IN Buffer size of the outbuf buffer
  * @param isVadminConf IN One if we are parsing a vadmin
  *                        config file (non-php syntax), 
  *                        zero otherwise.
  * FIXME: vadmin config files actually have "sections",
  *        where the vars we are looking for actually
  *        can reside in more than one section, identified
  *        by a line with [section_name] on it alone.  
  *        this function currently ignores that and assumes
  *        a unique varname over the whole file being searched
  *
  * @return 1 if it finds the parameter, 0 if not.
  *
  */
int readconfigvar(char *configfile, char *phpvar, char *outbuf, int outbuflen, int isVadminConf) 
{

   FILE *f;
   char line[STR_MAX];
   int err, found = 0;

   err = trusted_open(&f, configfile, "r");
   if (err) return err;
   while (fgets(line, STR_MAX, f)) 
   {
      if (isVadminConf) 
      {
         found = parsevadminconfigstring(phpvar, line, outbuf, outbuflen);
      }
      else
      {
         found = parsephpstring(phpvar, line, outbuf, outbuflen);
      }
      if (found) break;
   }

   #ifdef DEBUG
   fprintf(stderr, "config: %s %s -> %d\n", configfile, phpvar, found);
   #endif

   return found;

}



/**
  * Looks for a given varname in a line of PHP code. 
  *
  * If it is found, the code will try to parse out the 
  * number or string between quotes. This function is 
  * easily confused and when it returns 1 it means the 
  * variable was found, not necessarily that the string 
  * contents were successfully parsed.
  *
  * FIXME: I think variable names with numbers in them 
  * will break this function?!
  *
  * @return varname IN The PHP variable to search for. 
  *                    Should include the dollar sign
  *                    variable identified prefix ("$").
  * @return line IN A line of text.  May end with \n. 
  * @return dest OUT Where the string contents will be 
  *                  placed if there is a match.
  * @return destlen IN Size of the dest buffer.
  *
  * @return 1 if the varname was found, 0 if not.
  *
  */
int parsephpstring(char *varname, char *line, char *dest, int destlen)
{

   char *s;
   char *commentpos;
   int n;

   s = strstr(line, varname);       
   if (s == NULL) return 0;
	
   /* look for // comment marker, making sure the // can't */
   /* be treated as a comment by the  compiler             */
   commentpos = strstr(line, "/" "/");
   if (commentpos && commentpos < s) return 0;

   /* skip ahead to either ' or " or a number */
   s += strcspn(s, "\"'0123456789\0\n");
   if (strchr("0123456789", *s)) 
   {
      /* found number - now find semicolon, space or EOL */
      n = strcspn(s, "; \0\n");
   } 
   else 
   {
      /* found quote - now find next quote or EOL */
      /* this does not find matching quotes */
      s++;
      n = strcspn(s, "\"'\0\n");
   }
   if (n > destlen) n = destlen;
   strncpy(dest, s, n);
   dest[n] = '\0';

   return 1;

}



/**
  * Looks for a given varname in a line of a vadmin 
  * config file.
  *
  * If it is found, the code will try to parse out the 
  * number or string following an equal sign. 
  *
  * @return varname IN The variable to search for. 
  * @return line IN A line of text.  May end with \n. 
  * @return dest OUT Where the string contents will be 
  *                  placed if there is a match.
  * @return destlen IN Size of the dest buffer.
  *
  * @return 1 if the varname was found, 0 if not.
  *
  */
int parsevadminconfigstring(char *varname, char *line, char *dest, int destlen)
{

   char *s;
   char *n;
   char *commentpos;

   s = strstr(line, varname);       
   if (s == NULL || (s > line && *(s - 1) != ' ')) return 0;
	
   /* look for # or ; comment markers, making sure */
   /* they don't block use of this line            */
   commentpos = strstr(line, "#");
   if (commentpos && commentpos < s) return 0;
   commentpos = strstr(line, ";");
   if (commentpos && commentpos < s) return 0;

   /* skip ahead to equal sign */
   s = strchr(s, '=');
   s++;

   /* skip any whitespace */
   s += strspn(s, " \t");  /* no newline? i nah */

   if ((n = strstr(s, "\n")) == NULL)
   n = s + strlen(s);
   
   if (n - s > destlen) n = s + destlen;
   strncpy(dest, s, n - s);
   dest[n - s] = '\0';

   return 1;

}



/**
  * Open a file while verifying that it is 
  * sufficiently protected so that we can 
  * trust its contents.
  *
  * That means the file must be owned by root,
  * and either not group readable or be in 
  * root's group.
  *
  * The same test is applied to all the files, 
  * all the way up the tree.
  *
  */
int trusted_open(FILE** f, char *filepath, char *mode) 
{
   char curpath[STR_MAX];
   int err;

   strncpy(curpath, filepath, STR_MAX);
   err = trusted_check(filepath, curpath);
   if (err) return err;
   *f = fopen(filepath, mode);
   return ERR_OK;
}

int trusted_check(char *filepath, char *curpath) 
{

   struct stat buf;
   char *ix = NULL;

   do {

      if (stat(filepath, &buf))
         eperror(filepath, ERR_CANT_TRUST_FILE);
      if (buf.st_mode & S_IWOTH)
         return trusted_error("writable by others", curpath, filepath);
      if ((buf.st_mode & S_IWGRP) && (buf.st_gid != 0))
         return trusted_error("writable by non-root group", curpath, filepath);
      if ((buf.st_mode & S_IWUSR) && (buf.st_uid != 0))
         return trusted_error("writable by non-root user", curpath, filepath);

      /* find parent directory */
      ix = rindex(curpath, '/');
      if (!ix) return ERR_OK;
      *ix = '\0';

   } while (1);

}

int trusted_error(char *err, char *path, char *file) 
{
   #ifdef DEBUG
   fprintf(stderr, "trust error on %s while checking %s\n  %s\n", path, file, err);
   #endif
   return ERR_CANT_TRUST_FILE;
}



/**
  * Vadmin Crypto
  *
  * Encrypts or decrypts strings using a forked call to
  * PHP, where we call Vadmin's built-in crypto code.
  *
  * @param IN algo
  * @param IN hashline 
  * @param IN action Must be "encrypt" or "decrypt" only.
  * @param IN str Originating string.
  * @param OUT result Resultant string.
  *
  */
int vadmin_crypto(char *algo, char *hashline, char *action, char *str, char *result)
{

   int returnValue;
   char command[STR_MAX];
   char php_code[CODE_MAX];
   char *arguments[MAX_ARGS];

   pid_t pid;
   int pipehandle[2];

   if (pipe(pipehandle) == -1)    
   {
      perror("can't make pipe"); 
      exit(1);
   }
   if ((pid = fork()) == -1)
   {
      perror("can't fork"); 
      exit(1);
   }


   /* ------------------ CHILD ----------------- */
   /* Take write-end of pipe and call off to PHP */
   /*                                            */
   else if (pid == 0) 
   {     

      snprintf(command, (sizeof(command) - 1), PHPPATH);
      command[sizeof(command) - 1] = '\0';
      arguments[0] = command;
      arguments[1] = "-r";

      /* compacted versions of vadmin_rc4_crypt() and vadmin_crypto() */
      /* from includes/vadmin_functions.inc */
      snprintf(php_code, CODE_MAX, "function vadmin_rc4_crypt($input, $key) { $k_tmp = preg_split('//', $key, -1, PREG_SPLIT_NO_EMPTY); foreach($k_tmp as $char) { $k[] = ord($char); } unset($k_tmp); $message = preg_split('//', $input, -1, PREG_SPLIT_NO_EMPTY); $rep = count($k); for ($n=0;$n<$rep;$n++) { $s[] = $n; } $i = 0; $f = 0; for ($i = 0;$i<$rep;$i++) { $f = (($f + $s[$i] + $k[$i]) %% $rep); $tmp = $s[$i]; $s[$i] = $s[$f]; $s[$f] = $tmp; } $i = 0; $f = 0; foreach($message as $letter) { $i = (($i + 1) %% $rep); $f = (($f + $s[$i]) %% $rep); $tmp = $s[$i]; $s[$i] = $s[$f]; $s[$f] = $tmp; $t = $s[$i] + $s[$f]; $done = ($t^(ord($letter))); $i++; $f++; $enc_array[] = chr($done); } $coded = implode('', $enc_array); return $coded; } function vadmin_crypto($input, $mode) { $CRYPTO_HASH_LINE = '%s'; $MCRYPT_ALGO = '%s'; if (!$CRYPTO_HASH_LINE || !$MCRYPT_ALGO){ $message = ''; if (!$CRYPTO_HASH_LINE){ $message .= 'Could not find CRYPTO_HASH_LINE! '; } if (!$MCRYPT_ALGO){ $message .= 'Could not find MCRYPT_ALGO! '; } echo 'Cannot use crypt functions for the following reasons:' .  $message; exit(1); } $key = $CRYPTO_HASH_LINE; if ($MCRYPT_ALGO == 'rc4_builtin'){ switch($mode){ case 'encrypt': $endresult = base64_encode(vadmin_rc4_crypt($input, $key)); break; case 'decrypt': $endresult = vadmin_rc4_crypt(base64_decode($input), $key); break; } } else { if (!function_exists('mcrypt_generic')){ $message = 'An algorithm (' . $MCRYPT_ALGO . ') other than \"rc4_builtin\" specified, but mcrypt support not found.'; } $td = mcrypt_module_open($MCRYPT_ALGO, '', MCRYPT_MODE_ECB, ''); $iv = mcrypt_create_iv(mcrypt_enc_get_iv_size ($td), MCRYPT_RAND); @mcrypt_generic_init($td, $key, $iv); switch ($mode){ case 'encrypt': $endresult = base64_encode(mcrypt_generic($td, $input)); break; case 'decrypt': $endresult = mdecrypt_generic($td, base64_decode($input)); $endresult = rtrim($endresult); break; } mcrypt_generic_deinit($td); } return $endresult; } echo vadmin_crypto('%s', '%s'); \0", hashline, algo, str, action);

      arguments[2] = php_code;

      arguments[3] = '\0';
      close(pipehandle[0]);
      dup2(pipehandle[1], STDOUT_FILENO);
      close(pipehandle[1]);

      returnValue = execv(PHPPATH, arguments);
      perror("child didn't exec"); 
      _exit(99);

   }


   /* ---------------- PARENT ----------------- */
   /* Take read-end of pipe and read from child */
   /*                                           */
   else 
   {

      close(pipehandle[1]);
      dup2(pipehandle[0], STDIN_FILENO);
      close(pipehandle[0]);
      char *buffer = (char *) malloc(256 * 1024);
      memset(buffer, 0, 256*1024);
      int pos = 0;
      int count = 0;
      while ((count = read(STDIN_FILENO, buffer+pos, (256*1024)-pos)) > 0) 
      {
         #ifdef DEBUG
         fprintf(stderr, "%d bytes read from pipe to vadmin_crypto()\n", count);
         #endif

         pos += count;

         #ifdef DEBUG
         fprintf(stderr, "pos = %d\n", pos);
         #endif
      }

      #ifdef DEBUG
      fprintf(stderr, "%d bytes read in total from vadmin_crypto()\n", pos);	
      fprintf(stderr, "%s result: %s --> %s\n\n", action, str, buffer);
      #endif

      strncpy(result, buffer, pos);
      result[pos] = '\0';

   }

   return 0;

}

