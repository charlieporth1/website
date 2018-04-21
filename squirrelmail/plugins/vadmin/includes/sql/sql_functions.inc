<?php
/**
 * sql_functions.inc
 * ----------------------
 * All SQL backend functions are in this file.
 *
 * Licensed under GNU GPL v2. Please see COPYING for full terms.
 *
 * $Id: sql_functions.inc,v 1.6 2008/12/30 09:59:51 pdontthink Exp $
 *
 * @author Paul Lesniewski ($Author: pdontthink $)
 * @version $Date: 2008/12/30 09:59:51 $
 *
 */



/**
  * Get full account list for a given domain.
  *
  * Queries out a list of users and aliases and their
  * account attributes for any one domain.
  *
  * @param string $domain The vadmin domain.
  * @param string $password The domain password.
  * @param int $limit If given, the number of results to limit the
  *                   query to for pagination. (optional; default = 0;
  *                   no pagination) 
  * @param int $pageNumber If given, the page number of result sets to
  *                        limit the query to for pagination (note this
  *                        is NOT the offset
  *                        (offset = ($pageNumber - 1) * $limit)).
  *                        (optional; default = 0; no pagination)
  * @param string $sortCol If given, indicates which data element
  *                        should be sorted upon. (optional; default='', 
  *                        sort by name)
  * @param string $searchTerms If given, specifies a pattern upon which
  *                            users should be searhced instead of
  *                            returning all users, returns only those
  *                            that match the pattern (optional; default = '')
  * @param int $activeLookup If -1, only inactive users will be returned
  *                          (accounts that have never used webmail, no
  *                          aliases), if 1, only active accounts will be
  *                          returned (users that have used webmail at least
  *                          once, no aliases), if 0, all accounts and aliases
  *                          are returned.  (optional; default = 0)
  *      
  * @return array An array of user information arrays; each user
  *               information array contains these fields (in
  *               this order):
  *                 string account/alias name
  *                 string password
  *                 string mailbox location
  *                 array forwarding addresses
  *                 string personal information
  *                 string hard quota
  *                 string soft quota
  *                 string message size limit
  *                 string message count limit
  *                 string account creation date/time
  *                 string account expiry date/time
  *                 string account flags
  *                 boolean alias? (TRUE if this is an 
  *                         alias and not a real account)
  *
  */
function listdomain($domain, $password, $limit=0, $pageNumber=0, $sortCol='', $searchTerms='', $activeLookup=0)
{ 
   $ret = vm_daemon_raw(array('listdomain', $domain, $password, $limit, $pageNumber, $sortCol, $searchTerms, $activeLookup));
   return $ret[1];
} 



/**
  * Get list of all domains on the system
  * 
  * @return array A list of all the domain names
  *               served on this system
  *
  */
function vadmin_list_virtual_domains()
{

   $me = 'vadmin_list_virtual_domains';


   // get database connection
   //
   $db = vadmin_get_database_connection();


   // query domain list
   //
   $sql = vadmin_getvar('BACKEND', 'domain_list_query');
   spew("$me: Running query: $sql");
   $domains = $db->getAll($sql);


   // check for database errors
   //
   if (DB::isError($domains))
   {
      $msg = $domains->getMessage();
      spew("$me: Database error: $msg");
      $error = sprintf(_("Could not query domain list!"));
      vadmin_system_error($error);
   }


   // check for unexpected number of results
   //
   if (sizeof($domains) < 1)
   {
      spew("$me: Uhm... No domains seem to be defined.");
      $error = sprintf(_("No virtual domains defined in %s!"), $vfile);
      vadmin_system_error($error);
   }


   // re-org like we work for Intel
   //
   $domain_array = array();
   foreach ($domains as $domain)
   {
      spew("$me: found " . $domain[0]);
      array_push($domain_array, $domain[0]);
   }


   return $domain_array;

}



/**
  * Validate domain password.
  *
  * Tries to validate the supplied domain/password pair by
  * connecting to the database and using the query as given
  * in vadmin configuration file.
  *
  * @param string $domain The vadmin domain.
  * @param string $password The password for this domain.
  *
  * @return boolean TRUE if successful, or FALSE if not.
  *
  */
function vadmin_domain_passwd_validate($domain, $password)
{

   $me = 'vadmin_domain_passwd_validate';


   // get database connection
   //
   $db = vadmin_get_database_connection();


   // insert domain and password into the query
   //
   $sql = vadmin_getvar('BACKEND', 'domain_password_query');
   $sql = str_replace(array('$1', '$2'), 
                      array(str_esc($domain), 
                            str_esc($password)), 
                      $sql);


   // query domain password
   //
   spew("$me: Trying to validate password for domain '$domain'");
   spew("$me: Running query: $sql");
   $result = $db->getAll($sql);


   // check for database errors
   //
   if (DB::isError($result))
   {
      $msg = $result->getMessage();
      spew("$me: Database error: $msg");
      $error = sprintf(_("Could not query domain password!"));
      vadmin_system_error($error);
   }


   // check for unexpected results
   //
   if (sizeof($result) != 1 || sizeof($result[0]) != 1 
    || !is_numeric($result[0][0]) || $result[0][0] < 0 || $result[0][0] > 1)
   {
      spew("$me: Uhm... Something's funny with your domain password query.  Check the config file.");
      $error = sprintf(_("Bad domain password query!"));
      vadmin_system_error($error);
   }


   // did not validate. Return the error message.
   //
   if ($result[0][0] == 0)
   {
      spew("$me: password did not validate!");
      spew("$me: password/domain mismatch");
      return FALSE;
   } 


   // validation successful. Return TRUE.
   //
   else 
   {
      spew("$me: password validated successfully");
      return TRUE;
   }

}



/**
  * Construct Password
  *  
  * Returns correctly encrypted (if needed depending on config
  * settings for your backend) password, ready for insertion into
  * backend, including any quotations needed for SQL statements.
  *
  * @param $plainPassword string The raw password value to be 
  *                              encrypted (or other)
  *
  * @return string The re-formatted and/or encrypted password.
  *
  */
function constructPassword($plainPassword)
{

   // hyphen means blank/erased password
   //
   if ($plainPassword == '-')
      $newPwd = "''";

   else
   {

      // encrypt password as needed
      //
      $passwordEncryption = vadmin_getvar('BACKEND', 'password_encryption');


      switch ($passwordEncryption)
      {

         // md5plain
         //
         case 'md5plain':
            $newPwd = "'" . str_esc(md5($plainPassword)) . "'";
            break;

         // md5crypt
         //
         case 'md5crypt':
            $salt = vadmin_generate_random_password(FALSE, 8, FALSE);
            $newPwd = '';

            // use native PHP crypt if system crypt supports md5crypt
            //
            if (function_exists('crypt')
             && CRYPT_MD5 == 1 && CRYPT_SALT_LENGTH == 12)
               $newPwd = "'" . str_esc(crypt($plainPassword, '$1$' . $salt . '$')) . "'";

            // or use built-in library
            //
            if (strpos($newPwd, '$1$') !== 1)
            {
               include_once(SM_PATH . 'plugins/vadmin/includes/md5crypt.php');
               $newPwd = "'" . str_esc(md5crypt_unix($plainPassword, $salt)) . "'";
               //$newPwd = "'" . str_esc(md5crypt_unix($plainPassword, '$1$' . $salt . '$')) . "'";
            }

            // we can also use MySQL ENCRYPT(), which also just uses system crypt
            //
            //$newPwd = "encrypt('" . str_esc($plainPassword) . "', '$1$" . $salt . "$')";

            break;
   
         // sasl crypt
         //
         case 'saslcrypt':
            $newPwd = "password('" . str_esc($plainPassword) . "')";
            break;
   
         // unix crypt
         //
         case 'unixcrypt':
            $salt = vadmin_generate_random_password(FALSE, 2, FALSE);
            $newPwd = '';

            // use native PHP crypt if available
            //
            if (function_exists('crypt'))
               $newPwd = "'" . str_esc(crypt($plainPassword, $salt)) . "'";

            // or use MySQL's ENCRYPT function
            //
            if (empty($newPwd))
            {
               $newPwd = "encrypt('" . str_esc($plainPassword) . "', '" . $salt . "')";
            }

            break;
   
         // vadmin crypt
         //
         case 'vadmincrypt':
            $newPwd = "'" . str_esc(vadmin_crypto($plainPassword, 'encrypt')) . "'";
            break;
   
         // plaintext - YIKES
         //
         case 'plaintext':
         default:
            $newPwd = "'" . str_esc($plainPassword) . "'";

      }

   }

   return $newPwd;

}



/**
  * Get real account for an alias.
  *
  * Queries the account that accepts mail for the given
  * alias name.
  *
  * @param string $domain The vadmin domain.
  * @param string $alias The alias to be queried.
  * @param string $domainPassword The domain password.
  *
  * @return array A two-element array: the first being
  *               a return code (where zero is non-error),
  *               the second being an array of user
  *               account names for the alias.  Those 
  *               account names are not guaranteed to be
  *               in the same domain as is being administered
  *               so it is the caller's responsibility to
  *               use them as needed.
  *               Or if an error occurred, the second
  *               array element of the return array may
  *               contain an error message.
  *
  */
function get_real_account_for_alias($domain, $alias, $domainPassword)
{

   return getRealAccountForAlias($domain, $alias, $domainPassword);

}



/**
  * Determines if the given data represents
  * an alias or not
  *
  * @param array Typical account/alias data array as returned
  *              from a lookup() call
  *
  * @return boolean TRUE if the data represents an alias,
  *                 FALSE if not.
  *
  */
function is_alias($user_data)
{
   return !empty($user_data[12]);
}



