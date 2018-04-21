/**
 * vadmin_storage_gdbm.c
 * ---------------------
 * Access vadmin storage backend using GDBM lookups.
 *
 * Licensed under GNU GPL v2. Please see COPYING for full terms.
 *
 * $Id: vadmin_storage_gdbm.c,v 1.3 2008/12/04 09:57:39 pdontthink Exp $
 *
 * @author Paul Lesniewski ($Author: pdontthink $)
 * @version $Date: 2008/12/04 09:57:39 $
 *
 */

#include <gdbm.h>
#include <stdlib.h>
#include <errno.h>

/*
 * @return int Non-zero on error, zero otherwise.
 */
int get_vadmin_stored_value(char *database, char *key, char *value)
{

   datum db_key;
   datum db_value;

   db_key.dptr = key;
   db_key.dsize = strlen(key);

   GDBM_FILE dbf;

   #ifdef DEBUG
   fprintf(stderr, "Looking in %s for %s\n", database, key);
   #endif

   //dbf = gdbm_open(database, 0, 0, 0, NULL);
   dbf = gdbm_open(database, 512, GDBM_READER, 0, NULL);
   if (dbf)
   {
      db_value = gdbm_fetch(dbf, db_key);
      gdbm_close(dbf);
      if (db_value.dptr != NULL)
      {
         strncpy(value, db_value.dptr, db_value.dsize);
         value[db_value.dsize] = '\0';
         free(db_value.dptr);

         #ifdef DEBUG
         fprintf(stderr, "Found %s\n\n", value);
         #endif

         return 0;
      }
   }


   #ifdef DEBUG
   fprintf(stderr, "ERROR: %s (%d/%d)\n\n", gdbm_strerror(gdbm_errno), gdbm_errno, errno);
   #endif
   value[0] = '\0';
   return 1;

}

