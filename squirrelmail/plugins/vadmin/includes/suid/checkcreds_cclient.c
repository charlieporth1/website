/* Credential checking using UW c-client library
 *
 *
 */

#include <stdio.h>

// error definitions
#include "vadmin_auth.h"

// c-client library
#include MAIL_H

#ifndef MAILBOXFLAGS
#define MAILBOXFLAGS "/norsh"
#endif

static char *callback_user, *callback_passwd;
char *checkcreds_extra_configvar = "$cclient_mailboxflags";

int checkcredentials(imapserver, imapport, user, passwd, extra)
char *imapserver;
int imapport;
char *user;
char *passwd;
char *extra;
{
	char mailboxname[MAILTMPLEN];
	MAILSTREAM *stream;
	NETMBX mb;
	DRIVER *d;

        /* we expect these to be accessed before this function returns */
	callback_user = user;
	callback_passwd = passwd;

        /* initialize all of the c-client drivers */
        #include LINKAGE_C

	/* ensure the c-client library supports IMAP */
 	d = (DRIVER*) mail_parameters(NIL, GET_DRIVER, (void*)"imap");
	if (!d) { 
		return ERR_BAD_LIBRARY;
	}
 
 	/* reduce timeout/retries */
 	#ifdef IMAP_TIMEOUT
        mail_parameters(NIL, SET_OPENTIMEOUT, (void*)IMAP_TIMEOUT);
        mail_parameters(NIL, SET_READTIMEOUT, (void*)IMAP_TIMEOUT);
        mail_parameters(NIL, SET_WRITETIMEOUT, (void*)IMAP_TIMEOUT);
        mail_parameters(NIL, SET_CLOSETIMEOUT, (void*)IMAP_TIMEOUT);
        mail_parameters(NIL, SET_RSHTIMEOUT, (void*)IMAP_TIMEOUT);
        #endif
  	mail_parameters(NIL, SET_MAXLOGINTRIALS, (void*)1);

        /* extra may be null (if configvar was not set) */
        if (!extra) extra = "";

	snprintf(mailboxname, MAILTMPLEN, 
		"{%s:%d/service=imap/user=%s%s%s}INBOX",
		imapserver, imapport, user, MAILBOXFLAGS, extra);
		
	#ifdef DEBUG
		fprintf(stderr, "c-client mailboxname: %s\n", mailboxname);
	#endif

	stream = mail_open(NIL, mailboxname, NIL);

	if (stream != NIL) {
		mail_close(stream);
		return ERR_OK;
	} else {
		return ERR_BAD_CREDENTIALS;
	}
}

/* we must store user and password when this is called by the c-client library */
void mm_login (NETMBX *mb,char *user,char *pwd,long trial) {
	#ifdef DEBUG
		fprintf(stderr, "mm_login: {%s/%s/user=\"%s\"}\n",mb->host,mb->service,mb->user);
	   #if DEBUG > 1
		fprintf(stderr, "mm_login -> %s %s\n", callback_user, callback_passwd);
	   #else
		fprintf(stderr, "mm_login -> %s\n", callback_user);
	   #endif
	#endif

	strncpy(user, callback_user, MAILTMPLEN);
	strncpy(pwd, callback_passwd, MAILTMPLEN);
}
void mm_log (char *string,long errflg) {
	char *errflgname;
	switch ((short) errflg) {
		case NIL:	errflgname = "NIL"; break;
		case PARSE: 	errflgname = "PARSE"; break;
		case WARN:	errflgname = "WARN"; break;
		case ERROR:	errflgname = "ERROR"; break;
		default:	errflgname = "?"; break;
	}
	#ifdef DEBUG
		fprintf(stderr, "mm_log: %s: %s\n", errflgname, string);
	#endif
}
void mm_notify (MAILSTREAM *stream,char *string,long errflg) {
	mm_log(string, errflg);
}


/* c-client callbacks we don't need to implement*/
void mm_flags (MAILSTREAM *stream,unsigned long number){}
void mm_status (MAILSTREAM *stream,char *mailbox,MAILSTATUS *status){}
void mm_searched (MAILSTREAM *stream,unsigned long number){}
void mm_exists (MAILSTREAM *stream,unsigned long number){}
void mm_expunged (MAILSTREAM *stream,unsigned long number){}
void mm_list (MAILSTREAM *stream,int delimiter,char *name,long attributes) {}
void mm_lsub (MAILSTREAM *stream,int delimiter,char *name,long attributes) {}
void mm_dlog (char *string){}
void mm_critical (MAILSTREAM *stream) {}
void mm_nocritical (MAILSTREAM *stream) {}
long mm_diskerror (MAILSTREAM *stream,long errcode,long serious) {}
void mm_fatal (char *string) {}
