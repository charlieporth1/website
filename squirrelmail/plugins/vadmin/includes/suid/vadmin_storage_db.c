/**
 * vadmin_storage_db.c
 * ---------------------
 * Access vadmin storage backend using Berkeley DB lookups.
 *
 * Licensed under GNU GPL v2. Please see COPYING for full terms.
 *
 * $Id: vadmin_storage_db.c,v 1.2 2008/12/04 09:57:38 pdontthink Exp $
 *
 * @author Paul Lesniewski ($Author: pdontthink $)
 * @version $Date: 2008/12/04 09:57:38 $
 *
 */

#include <db.h>
#include <string.h>
#include <errno.h>

#define STR_MAX 1024


/*
 * @return int Non-zero on error, zero otherwise.
 */
int get_vadmin_stored_value(char *database, char *key, char *value)
{

   DB *dbp;
   DBT lookup_key, lookup_result;
   int ret;

   #ifdef DEBUG
   fprintf(stderr, "Looking in %s for %s\n", database, key);
   #endif

   ret = db_create(&dbp, NULL, 0);
   if (ret == 0)
   {
      // DB_BTREE, DB_RECNO, DB_HASH
      ret = dbp->open(dbp, NULL, database, NULL, DB_BTREE, DB_RDONLY, 0);
      if (ret == 0)
      {
         memset(&lookup_key,  0, sizeof(DBT));
         memset(&lookup_result, 0, sizeof(DBT));

         lookup_key.data = key;
         lookup_key.size = strlen(key);

         lookup_result.data = value;
         lookup_result.ulen = STR_MAX;
         lookup_result.flags = DB_DBT_USERMEM;

         ret = dbp->get(dbp, NULL, &lookup_key, &lookup_result, 0);
         dbp->close(dbp, 0);

         if (ret == 0)
         {
            #ifdef DEBUG
            fprintf(stderr, "Found %s\n\n", value);
            #endif

            return 0;
         }
      }
   }

   #ifdef DEBUG
   fprintf(stderr, "ERROR: %s (%d)\n\n", db_strerror(ret), ret);
   #endif
   value[0] = '\0';
   return ret;

}

