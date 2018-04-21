/*
 * Credential checking using IMAP
 *
 * David Phillips <david <at> geektech.com>
 *
 * Public domain
 *
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// error definitions
#include "vadmin_auth.h"

 char *checkcreds_extra_configvar = "";

 static int socket_connect(const char *host, int port)
 {
 struct in_addr addr;
 struct hostent *hent;
 struct sockaddr_in saddr;
 int s;
 int r;

 if (inet_aton(host, &addr) == 0)
 {
 hent = gethostbyname(host);
 if (hent == NULL)
 return -1;
 addr = *((struct in_addr *)hent->h_addr);
 }
 saddr.sin_family = AF_INET;
 saddr.sin_addr = addr;
 saddr.sin_port = htons(port);
 memset(&saddr.sin_zero, 0, 8);

 s = socket(AF_INET, SOCK_STREAM, 0);
 if (s == -1)
 return -1;

 if (connect(s, (struct sockaddr *)&saddr, sizeof(struct sockaddr)) == -1)
 {
 close(s);
 return -1;
 }

 return s;
 }

 int checkcredentials(imapserver, imapport, user, passwd, extra)
 char *imapserver;
 int imapport;
 char *user;
 char *passwd;
 char *extra;
 {

 char LOGOUT[] = "a2 LOGOUT\r\n";
 int s;
 char buf[4096];
 FILE *fp;

 s = socket_connect(imapserver, imapport);
 if (s == -1)
 return ERR_CANT_READ_IMAP_SERVER;

 fp = fdopen(s, "r+");

 if (fgets(buf, sizeof(buf), fp) == NULL)
 goto error;

 fprintf(fp, "a1 LOGIN %s %s\r\n", user, passwd);

 if (fgets(buf, sizeof(buf), fp) == NULL)
 goto error;

 fprintf(fp, "a2 LOGOUT\r\n");
 fclose(fp);

#ifdef DEBUG
 fprintf(stderr, "IMAP response: %s\n", buf);
#endif

 if (strstr(buf, "OK") == NULL)
 return ERR_BAD_CREDENTIALS;

 return ERR_OK;

 error:
 fclose(fp);
 return ERR_CANT_READ_IMAP_SERVER;
 }

