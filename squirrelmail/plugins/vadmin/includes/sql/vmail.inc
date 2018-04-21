<?php

/**
 * vmail.inc
 * ---------
 * Replacement for VMailMgr's vmail.inc PHP backend/API
 * for use with SQL backends.
 *
 * Licensed under GNU GPL v2. Please see COPYING for full terms.
 *
 * $Id: vmail.inc,v 1.19 2009/02/24 09:15:25 pdontthink Exp $
 *
 * @author Paul Lesniewski ($Author: pdontthink $)
 * @version $Date: 2009/02/24 09:15:25 $
 *
 */



vadmin_get_pear_db();



// VADMIN_USER_DATA_FIELD_COUNT corresponds to the number of
// columns being queried when looking up users
// TODO: move this if needed elsewhere
//
global $custom_fields;
$custom_fields = vadmin_getvar('BACKEND', 'custom_user_account_fields');
if (!empty($custom_fields))
   $custom_fields = explode(',', $custom_fields);
else
   $custom_fields = array();
@array_walk($custom_fields, 'vadmin_trim_array');
define('VADMIN_USER_DATA_FIELD_COUNT', 12 + count($custom_fields));



function vadmin_zvalue($val){
    if ($val == false){
        $val = '-';
    } elseif ($val == '-'){
        $val = false;
    }
    return $val;
}



/**
  * Include Pear DB class
  *
  */
function vadmin_get_pear_db()
{

   $me = 'vadmin_get_pear_db';

   $db_file = 'DB.php';
   spew("$me: Loading Pear SQL database library $db_file");


   // mask include errors if not in debug mode
   //
   $debug = vadmin_debug_level();
   if ($debug > 1)
      $if_statement = 'return !include_once(\'' . $db_file . '\');';
   else
      $if_statement = 'return !@include_once(\'' . $db_file . '\');';
   
   if (eval($if_statement))
   {
      spew("$me: Could not find Pear DB library");
      $error = sprintf(_("Could not find Pear DB library"));
      vadmin_system_error($error);
   }

}



/**
  * Get a database connection
  * 
  * If a connection has already been opened, return that,
  * otherwise, open a new connection.
  * 
  * @param string $dsn The DSN to be used; if empty, 
  *                    value from Vadmin config is used.
  *                    (optional; default = use one from config)
  *
  * @return object The database connection handle.
  *
  */
function vadmin_get_database_connection($dsn='')
{

   global $vadmin_db_connection;
   $me = 'vadmin_get_database_connection';


   // make a new connection if needed; exit if failure
   //
   if (empty($vadmin_db_connection))
   {

      if (empty($dsn))
         $dsn = vadmin_getvar('BACKEND', 'db_dsn');

      spew("$me: Connecting to database at $dsn");
      $vadmin_db_connection = DB::connect($dsn);
      if (DB::isError($vadmin_db_connection))
      {
         spew("$me: Could not make database connection at $dsn!");
         $error = sprintf(_("Could not connect to database!"));
         vadmin_system_error($error);
      }
      $vadmin_db_connection->setFetchMode(DB_FETCHMODE_ORDERED);

   }


   // return connection
   //
   return $vadmin_db_connection;

}



/**
  * Proxy function for most backend commands
  *
  * Implemented commands are:
  * 
  * LISTDOMAIN
  *
  *   returns a list of all users in this domain
  *
  *   arguments:
  *     0 : command
  *     1 : domain
  *     2 : domain password
  *     3 : number of users per page
  *     4 : page number
  *     5 : sort column
  *     6 : search terms
  *     7 : active lookup flag
  *
  * LOOKUP
  *
  *   does the same as LISTDOMAIN, but just gets info
  *   on one user
  *
  *   arguments:
  *     0 : command
  *     1 : domain
  *     2 : userid
  *     3 : domain password
  *
  * CHATTR
  *
  *   changes a given user attribute
  *
  *   attributes are keyed by number:
  *     1 : password
  *     2 : dest (mailbox)
  *     3 : hard quota
  *     4 : soft quota
  *     5 : message size limit
  *     6 : message count limit
  *     7 : account expiry
  *     8 : mailbox flags
  *     9 : personal (personal/real name information)
  *    10 : forwards (non-vmailmgr; added for SQL backend)
  *    <or the name of a custom field if it is being changed, 
  *     with spaces changed to uderscores>
  *
  *   arguments:
  *     0 : command
  *     1 : domain
  *     2 : userid
  *     3 : domain password
  *     4 : attribute number
  *     5 : new attribute value
  *     (in the case of attribute 10, argument
  *     number 5 will be an array of new values)
  *
  * @param array $command An array describing the command to be
  *                       run, whose elements vary depending upon
  *                       the command being run.  The first element
  *                       of this array, however, is always the
  *                       command name (as a string).  See above
  *                       for a listing of the available commands
  *                       and their needed parameters, which make up
  *                       the subsequent values of this array.
  *
  * @return array(int, mixed) The return value is an array consisting
  *                           of two values: the first is a return code,
  *                           which is zero when no error occurred, or
  *                           non-zero when an error occurred.  When an
  *                           error has occurred, the second array element
  *                           will contain the error message.  When no
  *                           error has occurred, the second array element
  *                           will contain another array, whose structure
  *                           varies depending upon the command being 
  *                           called; see the actual called functions for
  *                           details.
  *
  */
function vm_daemon_raw($command)
{

   switch ($command[0])
   {

      case 'listdomain': 

         // third through seventh parameters are for pagination,
         // sorting, searches and active lookup flag, which are 
         // our own additions to the protocol, so accomodate 
         // times when they may be missing
         //
         if (!isset($command[3])) $command[3] = 0;
         if (!isset($command[4])) $command[4] = 0;
         if (!isset($command[5])) $command[5] = '';
         if (!isset($command[6])) $command[6] = 0;
         return getAllUsers($command[1], $command[2], $command[3], $command[4], $command[5], $command[6], $command[7]);

      case 'lookup': return getUser($command[1], $command[2], $command[3]);

      case 'chattr': return changeAttribute($command[1], $command[2], 
                                            $command[3], $command[4], $command[5]);

   }

}



/** 
  * Lookup is a proxy for calling the vm_daemon_raw function
  * with the 'lookup' command.  That is, it retrieves information
  * for just a single user.
  *
  * @param string $domain The vadmin domain.
  * @param string $user The user to be queried.
  * @param string $domainPassword The domain password.
  *
  * @return array An array of user information which contains 
  *               these fields (in this order):
  *                 string  account/alias name
  *                 string  password
  *                 string  mailbox location
  *                 array   forwarding addresses
  *                 string  personal information
  *                 string  hard quota
  *                 string  soft quota
  *                 string  message size limit
  *                 string  message count limit
  *                 string  account creation date/time
  *                 string  account expiry date/time
  *                 string  account flags
  *                 boolean alias? (TRUE if this is an
  *                         alias and not a real account)
  *                 mixed   an array of user webmail stats or 
  *                         possibly empty
  *                 mixed   custom first field value, if any
  *                 mixed   custom second field value, if any
  *                 mixed   custom third field value, if any
  *                 mixed   custom fourth field value, if any
  *                 etcetera
  *
  */
function lookup($domain, $user, $domainPassword)
{
   $tmp = vm_daemon_raw(array("lookup", $domain, $user, $domainPassword));
   if ($tmp[0])
      vadmin_system_error(sprintf(_("Cannot find user \"%s\""), $user));
   return $tmp[1];
}



/** 
  * Deletes a user account.
  *
  * @param string $domain The vadmin domain.
  * @param string $domainPassword The domain password.
  * @param string $user The user to be deleted.
  *
  * @return array A two-element array: the first being
  *               a return code (where zero is non-error), 
  *               the second being a message, which is usually
  *               only important if there was an error.
  *
  */
function vdeluser($domain, $domainPassword, $user)
{

   $me = 'vdeluser';
   $returnCode = 0;
   $errorMessage = '';
   

   // if this "user" is an alias, call vdelalias
   //
   $userInfo = getUser($domain, $user, $domainPassword);
   if (is_alias($userInfo[1])) 
      return vdelalias($domain, $domainPassword, $user);


   $AUTHCODE = vadmin_getvar('SESSION', 'VADMIN_AUTHCODE');
   $LVL = vadmin_getvar('VADMIN', 'LVL');
   
   
   // validate password
   //
   if (!vadmin_domain_passwd_validate($domain, $domainPassword))
   {

      spew("$me: the password did NOT verify");
   
      // See if we need to zero out the password.
      //
      if ($AUTHCODE == 'LOWLY')
      {
         spew("$me: Stored password did not verify! Removing it");
         vadmin_put_domain_passwd($domain, FALSE);
         $msg = _("The stored password file was invalid and has been removed. Please contact your systems administrator.");
         vadmin_system_error($msg);
      }
   
      // This is a CROSS or ELVIS with auth=user. This still
      // means that the stored password is incorrect, but we
      // can redirect them to the askdompass instead of just
      // booting them off. 
      //
      spew("$me: Stored password did not verify! Asking again.");
      vadmin_put_domain_passwd($domain, FALSE);
      vadmin_redirect($LVL, 'login', 'askdompass', $domain);
      
   }


   // get database connection
   //
   $db = vadmin_get_database_connection();


   // run each deletion query
   //
   spew("$me: Trying to delete account '$user' in domain '$domain'");
   $queries = vadmin_getvar('BACKEND', 'account_deletion_queries');
   $queries = explode(',', $queries);
   @array_walk($queries, 'vadmin_trim_array');
   foreach ($queries as $query)
   {

      if (empty($query)) continue;


      // insert domain and username into the query
      //
      $sql = vadmin_getvar('BACKEND', $query);
      $sql = str_replace(array('$1', '$2'), array($user, $domain), $sql);


      // go!
      //
      spew("$me: Running query: $sql");
      $result = $db->query($sql);


      // check for database errors
      //
      if (DB::isError($result))
      {
         $msg = $result->getMessage();
         spew("$me: Database error: $msg");
         $error = sprintf(_("Could not remove user account!"));
         vadmin_system_error($error);
      }

   }


   // now run system management proxy
   //
   $flags = vadmin_getvar('CONFIG', 'system_manager_proxy.account_deletion_flags');
   $sanitize = vadmin_getvar('CONFIG', 'system_manager_proxy.sanitize_flag_arguments');
   if (!empty($flags))
   {
      $username = vadmin_getvar('SQMAIL', 'username');
      $flags = str_replace(array('$1', '$2', '$u', '$p'), 
                           ($sanitize == 'yes' ?
                              array(escapeshellarg($user), 
                                    escapeshellarg($domain), 
                                    escapeshellarg($username), 
                                    escapeshellarg(vadmin_get_current_pwd())) 
                            : array($user, 
                                    $domain, 
                                    $username, 
                                    vadmin_get_current_pwd())), 
                           $flags);

      $returnCode = vadmin_system_manager_proxy($domain, $domainPassword, $flags);
      if ($returnCode[0] != 0) 
      {
         $errorMessage = _("Backend proxy error");
         if (!empty($returnCode[1]))
         {
            spew("$me: Backend proxy error: " . $returnCode[1]);
            // nah, let's not do this so no security issues are 
            // exposed in an error message from the backend...
            // $errorMessage = sprintf(_("Backend proxy error: %s"), $returnCode[1]);
         }
      }
   }


   // remove user from cache of user list
   //
   $users = vadmin_getvar('SESSION', 'vadmin_domain_lookup');
   if (!empty($users))
   {
      foreach ($users as $i => $u)
         if ($u[0] == $user)
         {
            unset($users[$i]);
            break;
         }
      vadmin_putvar('SESSION', 'vadmin_domain_lookup', $users);
   }
//   // clear cache of user list
//   //
//   vadmin_putvar('SESSION', 'vadmin_domain_lookup', '');


   return array($returnCode[0], $errorMessage);

}



/**
  * Deletes a system alias.
  *
  * @param string $domain The vadmin domain.
  * @param string $domainPassword The domain password.
  * @param string $alias The alias to be deleted.
  *
  * @return array A two-element array: the first being
  *               a return code (where zero is non-error),
  *               the second being a message, which is usually
  *               only important if there was an error.
  *
  */
function vdelalias($domain, $domainPassword, $alias)
{

   $me = 'vdelalias';
   $returnCode = 0;
   $errorMessage = '';


   $AUTHCODE = vadmin_getvar('SESSION', 'VADMIN_AUTHCODE');
   $LVL = vadmin_getvar('VADMIN', 'LVL');


   // validate password
   //
   if (!vadmin_domain_passwd_validate($domain, $domainPassword))
   {

      spew("$me: the password did NOT verify");

      // See if we need to zero out the password.
      //
      if ($AUTHCODE == 'LOWLY')
      {
         spew("$me: Stored password did not verify! Removing it");
         vadmin_put_domain_passwd($domain, FALSE);
         $msg = _("The stored password file was invalid and has been removed. Please contact your systems administrator.");
         vadmin_system_error($msg);
      }

      // This is a CROSS or ELVIS with auth=user. This still
      // means that the stored password is incorrect, but we
      // can redirect them to the askdompass instead of just
      // booting them off.
      //
      spew("$me: Stored password did not verify! Asking again.");
      vadmin_put_domain_passwd($domain, FALSE);
      vadmin_redirect($LVL, 'login', 'askdompass', $domain);

   }


   // get database connection
   //
   $db = vadmin_get_database_connection();


   // insert domain and username into the query
   //
   spew("$me: Trying to delete alias '$alias' in domain '$domain'");
   $delim = vadmin_getvar('BACKEND', 'delimiters');
   $delim = substr($delim, 0, 1);
   $sql = vadmin_getvar('BACKEND', 'delete_alias_query');
   $sql = str_replace('$1', str_esc("$alias$delim$domain"), $sql);


   // go!
   //
   spew("$me: Running query: $sql");
   $result = $db->query($sql);


   // check for database errors
   //
   if (DB::isError($result))
   {
      $msg = $result->getMessage();
      spew("$me: Database error: $msg");
      $error = sprintf(_("Could not remove alias!"));
      vadmin_system_error($error);
   }


   // now run system management proxy
   //
   $flags = vadmin_getvar('CONFIG', 'system_manager_proxy.alias_deletion_flags');
   $sanitize = vadmin_getvar('CONFIG', 'system_manager_proxy.sanitize_flag_arguments');
   if (!empty($flags))
   {
      $username = vadmin_getvar('SQMAIL', 'username');
      $flags = str_replace(array('$1', '$u', '$p'), 
                           ($sanitize == 'yes' ?
                              array(escapeshellarg("$alias$delim$domain"), 
                                    escapeshellarg($username), 
                                    escapeshellarg(vadmin_get_current_pwd())) 
                            : array("$alias$delim$domain",
                                    $username, 
                                    vadmin_get_current_pwd())), 
                           $flags);

      $returnCode = vadmin_system_manager_proxy($domain, $domainPassword, $flags);
      if ($returnCode[0] != 0) 
      {
         $errorMessage = _("Backend proxy error");
         if (!empty($returnCode[1]))
         {
            spew("$me: Backend proxy error: " . $returnCode[1]);
            // nah, let's not do this so no security issues are 
            // exposed in an error message from the backend...
            // $errorMessage = sprintf(_("Backend proxy error: %s"), $returnCode[1]);
         }
      }
   }


   // remove alias from cache of user list
   //
   $users = vadmin_getvar('SESSION', 'vadmin_domain_lookup');
   if (!empty($users))
   {
      foreach ($users as $i => $u)
         if ($u[0] == $alias)
         {
            unset($users[$i]);
            break;
         }
      vadmin_putvar('SESSION', 'vadmin_domain_lookup', $users);
   }
//   // clear cache of user list
//   //
//   vadmin_putvar('SESSION', 'vadmin_domain_lookup', '');
      
      
   return array($returnCode[0], $errorMessage);

}



/** 
  * Adds an alias.
  *
  * @param string $domain The vadmin domain.
  * @param string $domainPassword The domain password.
  * @param string $alias The alias name to be added.
  * @param string $password NOT USED
  * @param array $forwards A list of forwarding addresses for 
  *                        the new alias, if any.
  *
  * @return array A two-element array: the first being
  *               a return code (where zero is non-error), 
  *               the second being a message, which is usually
  *               only important if there was an error.
  *
  */
function vaddalias($domain, $domainPassword, $alias, $password='', $forwards)
{

   $me = 'vaddalias';
   $returnCode = 0;
   $errorMessage = '';
   

   if (empty($domain))         return array(1, _("Empty domain"));
   if (empty($domainPassword)) return array(1, _("Empty domain password"));

   //if (empty($alias))          return array(1, _("Please specify alias name"));
   $catchall_alias = vadmin_getvar('BACKEND', 'catchall_alias');
   if (empty($alias) && !empty($catchall_alias)) return array(1, _("Please specify alias name"));

   if (empty($forwards))       return array(1, _("Please specify forward destination(s)"));


   $aliastitle = $alias;
   if ($alias == $catchall_alias) $aliastitle = _("catchall");


   $AUTHCODE = vadmin_getvar('SESSION', 'VADMIN_AUTHCODE');
   $LVL = vadmin_getvar('VADMIN', 'LVL');
   
   
// we already have $domain passed in above, right?
//   $domain = vadmin_getvar('VADMIN',  'VADMIN_DOMAIN');
   $username = vadmin_getvar('SQMAIL', 'username');
   $designation = vadmin_get_user_designation($domain, $username);
   if ($designation == 'ELVIS'){
       $level = 'CROSS';
   } else {
       $level = 'LOWLY';
   }


   // validate password
   //
   if (!vadmin_domain_passwd_validate($domain, $domainPassword))
   {

      spew("$me: the password did NOT verify");
   
      // See if we need to zero out the password.
      //
      if ($AUTHCODE == 'LOWLY')
      {
         spew("$me: Stored password did not verify! Removing it");
         vadmin_put_domain_passwd($domain, FALSE);
         $msg = _("The stored password file was invalid and has been removed. Please contact your systems administrator.");
         vadmin_system_error($msg);
      }
   
      // This is a CROSS or ELVIS with auth=user. This still
      // means that the stored password is incorrect, but we
      // can redirect them to the askdompass instead of just
      // booting them off. 
      //
      spew("$me: Stored password did not verify! Asking again.");
      vadmin_put_domain_passwd($domain, FALSE);
      vadmin_redirect($LVL, 'login', 'askdompass', $domain);
      
   }


   // compact all forwards into single CSV string if needed
   //
   $csv = vadmin_getvar('BACKEND', 'forwardsCSV');
   if (strtolower($csv) == "yes")
      $forwards = array(implode(',', $forwards));


   // make sure alias name is available
   //
   $userInfo = getUser($domain, $alias, $domainPassword);
   if ($userInfo[0] === 0) 
      return array(80, sprintf(_("Alias name \"%s\" is already taken."), $aliastitle));


   // get database connection
   //
   $db = vadmin_get_database_connection();


   // run insert query for each new forward
   //
   $delim = vadmin_getvar('BACKEND', 'delimiters');
   $delim = substr($delim, 0, 1);
   spew("$me: Trying to add alias '$alias' in domain '$domain'");
   foreach ($forwards as $forward)
   {

      if (empty($forward)) continue;


      // insert domain and username into the query
      //
      $sql = vadmin_getvar('BACKEND', 'insert_alias_query');
      if (!empty($sql))
      {

         $sql = str_replace(array('$1', '$2'), 
                            array(str_esc("$alias$delim$domain"), 
                                  str_esc($forward)), 
                            $sql);


         // go!
         //
         spew("$me: Running query: $sql");
         $result = $db->query($sql);


         // check for database errors
         //
         if (DB::isError($result))
         {
            $msg = $result->getMessage();
            spew("$me: Database error: $msg");
            $error = sprintf(_("Could not create alias!"));
            vadmin_system_error($error);
         }

      }

   }


   // now run system management proxy
   //
   $flags = vadmin_getvar('CONFIG', 'system_manager_proxy.alias_creation_flags');
   $sanitize = vadmin_getvar('CONFIG', 'system_manager_proxy.sanitize_flag_arguments');
   if (!empty($flags))
   {
      $flags = str_replace(array('$1', '$2', '$u', '$p'), 
                           ($sanitize == 'yes' ?
                              array(escapeshellarg("$user$delim$domain"), 
                                    escapeshellarg(implode(',', $forwards)), 
                                    escapeshellarg($username), 
                                    escapeshellarg(vadmin_get_current_pwd())) 
                            : array("$user$delim$domain",
                                    implode(',', $forwards), 
                                    $username, 
                                    vadmin_get_current_pwd())), 
                           $flags);

      $returnCode = vadmin_system_manager_proxy($domain, $domainPassword, $flags);
      if ($returnCode[0] != 0) 
      {
         $errorMessage = _("Backend proxy error");
         if (!empty($returnCode[1]))
         {
            spew("$me: Backend proxy error: " . $returnCode[1]);
            // nah, let's not do this so no security issues are 
            // exposed in an error message from the backend...
            // $errorMessage = sprintf(_("Backend proxy error: %s"), $returnCode[1]);
         }
         return array($returnCode[0], 
                      sprintf(_("(%d) Could not create alias."), 
                              $returnCode[0]));
      }

   }


   // add alias to cache of user list
   //
   $users = vadmin_getvar('SESSION', 'vadmin_domain_lookup');
   if (!empty($users))
   {
      $newAliasInfo = getUser($domain, $alias, $domainPassword, TRUE, TRUE);
      if ($newAliasInfo[0])
      {
         spew("$me: Fatal error: could not retrieve alias that was just created");
         $error = sprintf(_("Could not retrieve alias that was just created!"));
         vadmin_system_error($error);
      }
      $users[] = $newAliasInfo[1];
// not needed:        usort($users, 'sortUserArray');
      vadmin_putvar('SESSION', 'vadmin_domain_lookup', $users);
   }
//   // clear cache of user list
//   //
//   vadmin_putvar('SESSION', 'vadmin_domain_lookup', '');


   return array($returnCode[0], $errorMessage);

}



/** 
  * Adds a user account.
  *
  * @param string $domain The vadmin domain.
  * @param string $domainPassword The domain password.
  * @param string $user The user to be added.
  * @param string $password The new user's password.
  * @param array $forwards A list of forwarding addresses for 
  *                        the new account, if any.
  *
  * @return array A two-element array: the first being
  *               a return code (where zero is non-error), 
  *               the second being a message, which is usually
  *               only important if there was an error.
  *
  */
function vadduser($domain, $domainPassword, $user, $password, $forwards)
{

   $me = 'vadduser';
   $returnCode = 0;
   $errorMessage = '';
   

   if (empty($domain))         return array(1, _("Empty domain"));
   if (empty($domainPassword)) return array(1, _("Empty domain password"));
   if (empty($user))           return array(1, _("Please specify username"));


   $AUTHCODE = vadmin_getvar('SESSION', 'VADMIN_AUTHCODE');
   $LVL = vadmin_getvar('VADMIN', 'LVL');
   
   
// we already have $domain passed in above, right?
//   $domain = vadmin_getvar('VADMIN',  'VADMIN_DOMAIN');
   $username = vadmin_getvar('SQMAIL', 'username');
   $designation = vadmin_get_user_designation($domain, $username);
   if ($designation == 'ELVIS'){
       $level = 'CROSS';
   } else {
       $level = 'LOWLY';
   }


   // validate password
   //
   if (!vadmin_domain_passwd_validate($domain, $domainPassword))
   {

      spew("$me: the password did NOT verify");
   
      // See if we need to zero out the password.
      //
      if ($AUTHCODE == 'LOWLY')
      {
         spew("$me: Stored password did not verify! Removing it");
         vadmin_put_domain_passwd($domain, FALSE);
         $msg = _("The stored password file was invalid and has been removed. Please contact your systems administrator.");
         vadmin_system_error($msg);
      }
   
      // This is a CROSS or ELVIS with auth=user. This still
      // means that the stored password is incorrect, but we
      // can redirect them to the askdompass instead of just
      // booting them off. 
      //
      spew("$me: Stored password did not verify! Asking again.");
      vadmin_put_domain_passwd($domain, FALSE);
      vadmin_redirect($LVL, 'login', 'askdompass', $domain);
      
   }


   // make sure username is available
   //
   $userInfo = getUser($domain, $user, $domainPassword);
   if ($userInfo[0] === 0)
      return array(9, sprintf(_("User name \"%s\" is already taken."), $user));


/* Not used currently; this is done for us by accounts module (AFTER the account is created)

   // use max values as the default limits for quotas
   //
   $defaultHardQuota = vadmin_zvalue(vadmin_smart_limit($domain, $level, 'hardquota'));
   $defaultMessageCountLimit = vadmin_zvalue(vadmin_smart_limit($domain, $level, 'count'));


   // construct Courier-style quota
   //
   $courierQuota = '';
   if (!empty($defaultHardQuota) && $defaultHardQuota != '-')
      $courierQuota = $defaultHardQuota . 'S';
   else
      $courierQuota = '0S';
   if (!empty($defaultMessageCountLimit) && $defaultMessageCountLimit != '-')
      $courierQuota .= $defaultMessageCountLimit . 'C';
   else
      $courierQuota .= '0C';
*/
$defaultHardQuota = $defaultMessageCountLimit = $courierQuota = '';


   // get password correctly formatted and encrypted as needed
   //
   $newPassword = constructPassword($password);


   // get database connection
   //
   $db = vadmin_get_database_connection();


   // run the query to add the user...
   //
   spew("$me: Trying to add account '$user' in domain '$domain'");
   $sql = vadmin_getvar('BACKEND', 'insert_user_query');
   if (!empty($sql))
   {

      // replace password last because in some cases it can contain some
      // character sequences such as $5 that should not be replaced
      //
      $sql = str_replace(array('$1', '$2', '$d', '$6', '$9', '$a', '$3'), 
                         array(str_esc($user), 
                               str_esc($domain), 
                               str_esc(date('Y-m-d H:i:s')), 
                               str_esc($defaultHardQuota), 
                               str_esc($defaultMessageCountLimit), 
                               str_esc($courierQuota), 
                               $newPassword),  // already escaped in constructPassword()
                         $sql);


      // go!
      //
      spew("$me: Running query: $sql");
      $result = $db->query($sql);


      // check for database errors
      //
      if (DB::isError($result))
      {
         $msg = $result->getMessage();
         spew("$me: Database error: $msg");
         $error = sprintf(_("Could not create user account!"));
         vadmin_system_error($error);
      }

   }


   // now run system management proxy
   //
   $flags = vadmin_getvar('CONFIG', 'system_manager_proxy.account_creation_flags');
   $sanitize = vadmin_getvar('CONFIG', 'system_manager_proxy.sanitize_flag_arguments');
   if (!empty($flags))
   {
      $flags = str_replace(array('$1', '$2', '$3', '$6', '$9', '$a', '$u', '$p'), 
                           ($sanitize == 'yes' ?
                              array(escapeshellarg($user), 
                                    escapeshellarg($domain), 
                                    escapeshellarg($password), 
                                    escapeshellarg($defaultHardQuota),
                                    escapeshellarg($defaultMessageCountLimit), 
                                    escapeshellarg($courierQuota), 
                                    escapeshellarg($username), 
                                    escapeshellarg(vadmin_get_current_pwd())) 
                            : array($user, 
                                    $domain, 
                                    $password, 
                                    $defaultHardQuota,
                                    $defaultMessageCountLimit, 
                                    $courierQuota, 
                                    $username, 
                                    vadmin_get_current_pwd())), 
                           $flags);

      $returnCode = vadmin_system_manager_proxy($domain, $domainPassword, $flags);
      if ($returnCode[0] != 0) 
      {
         $errorMessage = _("Backend proxy error");
         if (!empty($returnCode[1]))
         {
            spew("$me: Backend proxy error: " . $returnCode[1]);
            // nah, let's not do this so no security issues are 
            // exposed in an error message from the backend...
            // $errorMessage = sprintf(_("Backend proxy error: %s"), $returnCode[1]);
         }
         return array($returnCode[0], 
                      sprintf(_("(%d) Could not create user."), 
                              $returnCode[0]));
      }

   }


   // add user to cache of user list
   //
   $users = vadmin_getvar('SESSION', 'vadmin_domain_lookup');
   if (!empty($users))
   {
      $newUserInfo = getUser($domain, $user, $domainPassword, TRUE, TRUE);
      if ($newUserInfo[0])
      {
         spew("$me: Fatal error: could not retrieve user that was just created");
         $error = sprintf(_("Could not retrieve user that was just created!"));
         vadmin_system_error($error);
      }
      $users[] = $newUserInfo[1];
// not needed:        usort($users, 'sortUserArray');
      vadmin_putvar('SESSION', 'vadmin_domain_lookup', $users);
   }
//   // clear cache of user list
//   //
//   vadmin_putvar('SESSION', 'vadmin_domain_lookup', '');


   // now add any forwards
   //
   if (!empty($forwards))
      vchforward($domain, $domainPassword, $user, $forwards);


   return array($returnCode[0], $errorMessage);

}



/**
  *
//TODO - complete this function's documentation:  this function in our implementation is not needed, as the "line" should already be in the format needed (see function getUser()), so we just return it as is
  *
  *
  */
function listdomain_parse_userdata($line, $username)
{
   return $line;
}



/**
  * Get detailed account information for one user.
  *
  * Queries out a list of all user attributes for the given user
  * in the given domain. 
  *
  * @param string $domain The vadmin domain.
  * @param string $user The user to be queried.
  * @param string $domainPassword The domain password.
  * @param boolean $obscurePassword When TRUE, passwords
  *                                 are replaced with "Set"
  *                                 unless they are blank.
  *                                 When FALSE, they are 
  *                                 returned as is.
  *                                 (optional; default = TRUE)
  * @param boolean $rawDump When TRUE, no formatting or interpretation
  *                         of data fields is done before returning
  *                         except to follow the $obscurePassword parameter.
  *                         Mostly, this just refers to the transformation
  *                         of ===N/A=== fields and reformatting quota 
  *                         figures.  (optional; default = FALSE)
  *
  * @return array A two-element array: the first being
  *               a return code (where zero is non-error), 
  *               the second being an array of user 
  *               information which contains these fields 
  *               (in this order):
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
  *                 mixed an array of user webmail stats or 
  *                       possibly empty
  *                 mixed custom first field value, if any
  *                 mixed custom second field value, if any
  *                 mixed custom third field value, if any
  *                 mixed custom fourth field value, if any
  *                 etcetera
  *               Or if an error occurred, the second
  *               array element of the return array may
  *               contain an error message.
  *
  */
function getUser($domain, $user, $domainPassword, $obscurePassword=TRUE,
                 $rawDump=FALSE)
{

   $me = 'getUser';
   $returnCode = 0;


   $AUTHCODE = vadmin_getvar('SESSION', 'VADMIN_AUTHCODE');
   $LVL = vadmin_getvar('VADMIN', 'LVL');


   // validate password
   //
   if (!vadmin_domain_passwd_validate($domain, $domainPassword))
   {

      spew("$me: the password did NOT verify");

      // See if we need to zero out the password.
      //
      if ($AUTHCODE == 'LOWLY')
      {
         spew("$me: Stored password did not verify! Removing it");
         vadmin_put_domain_passwd($domain, FALSE);
         $msg = _("The stored password file was invalid and has been removed. Please contact your systems administrator.");
         vadmin_system_error($msg);
      }

      // This is a CROSS or ELVIS with auth=user. This still
      // means that the stored password is incorrect, but we
      // can redirect them to the askdompass instead of just
      // booting them off.
      //
      spew("$me: Stored password did not verify! Asking again.");
      vadmin_put_domain_passwd($domain, FALSE);
      vadmin_redirect($LVL, 'login', 'askdompass', $domain);

   }


   // get database connection
   //
   $db = vadmin_get_database_connection();


   // insert domain and username into the query
   //
   $sql = vadmin_getvar('BACKEND', 'find_user_account_query');
   $sql = str_replace(array('$1', '$2'), 
                      array(str_esc($user), str_esc($domain)), 
                      $sql);


   // query for account info
   //
   spew("$me: Trying to get account information for user '$user' in domain '$domain'");
   spew("$me: Running query: $sql");
   $userInfo = $db->getAll($sql);


   // check for database errors
   //
   if (DB::isError($userInfo))
   {
      $msg = $userInfo->getMessage();
      spew("$me: Database error: $msg");
      $error = sprintf(_("Could not query user info!"));
      vadmin_system_error($error);
   }


   // check for unexpected results (should always be the same number of
   // fields and should be no more than one returned rows)
   //
   if ((sizeof($userInfo) > 0 && sizeof($userInfo[0]) != VADMIN_USER_DATA_FIELD_COUNT) 
    || sizeof($userInfo) > 1 || sizeof($userInfo) < 0)
   {
      spew("$me: Uhm... Something's funny with your user account query.  Check the config file.");
      $error = sprintf(_("Bad user account query!"));
      vadmin_system_error($error);
   }


   $oldIndex12 = NULL;


   // if no user was found, try looking it up as an alias
   //
   if (empty($userInfo))
   {

      // insert domain and username into the query
      //
      $sql = vadmin_getvar('BACKEND', 'find_alias_query');
      $sql = str_replace(array('$1', '$2'), 
                         array(str_esc($user), str_esc($domain)), 
                         $sql);


      // query for account info
      //
      spew("$me: Trying to get alias information for user '$user' in domain '$domain'");
      spew("$me: Running query: $sql");
      $userInfo = $db->getAll($sql);


      // check for database errors
      //
      if (DB::isError($userInfo))
      {
         $msg = $userInfo->getMessage();
         spew("$me: Database error: $msg");
         $error = sprintf(_("Could not query alias info!"));
         vadmin_system_error($error);
      }


      // check for unexpected results (should always be the same number of
      // fields and should be no more than one returned rows)
      //
      if ((sizeof($userInfo) > 0 && sizeof($userInfo[0]) != VADMIN_USER_DATA_FIELD_COUNT)
       || sizeof($userInfo) > 1 || sizeof($userInfo) < 0)
      {
         spew("$me: Uhm... Something's funny with your alias query.  Check the config file.");
         $error = sprintf(_("Bad alias query!"));
         vadmin_system_error($error);
      }


      // if no alias was found either, return error
      //
      if (empty($userInfo))
         return array(1, _("User not found")); //TODO: better error message necessary?
      else
      {
         if (isset($userInfo[0][12]))
            $oldIndex12 = $userInfo[0][12]; // in case a custom field was there
         $userInfo[0][12] = TRUE;
      }

   }
   else
   {
      if (isset($userInfo[0][12]))
         $oldIndex12 = $userInfo[0][12]; // in case a custom field was there
      $userInfo[0][12] = FALSE;
   }


   $userInfo = $userInfo[0];


   // Two fields, index 12 and 13 are for special values
   // that will be added later (actually, index 12 is already
   // done here).  For now, we need to push other values down 
   // if any custom fields were queried into those index slots
   //
   if (sizeof($userInfo) > 12)
   {

      $extra_fields = array_splice($userInfo, 12);
      $userInfo = array_merge($userInfo, array($extra_fields[0], ''), $extra_fields);
      $userInfo[14] = $oldIndex12;

   }


   // make sure username doesn't contain domain
   //
   $delimiters = vadmin_getvar('BACKEND', 'delimiters');
   for ($i = 0; $i < strlen($delimiters); $i++)
   {
      if (($delimPos = strpos($userInfo[0], substr($delimiters, $i, 1))) !== FALSE)
      {
         $userInfo[0] = substr($userInfo[0], 0, $delimPos);
         break;
      }
   }


//TODO: this forwards code is duplicated in the getAllUsers fxn; we should abstract it
   // now we need to look up forwarding information for the user...


   // insert domain and username into query
   //
   $sql = vadmin_getvar('BACKEND', 'find_user_forwards_query');
   $sql = str_replace(array('$1', '$2'), 
                      array(str_esc($userInfo[0]), str_esc($domain)), 
                      $sql);


   // query for forwards
   //
   spew("$me: Trying to get forwarding information for user '" . $userInfo[0] . "' at '$domain'");
   spew("$me: Running query: $sql");
   $forwards = $db->getAll($sql);


   // check for database errors
   //
   if (DB::isError($forwards))
   {
      $msg = $forwards->getMessage();
      spew("$me: Database error: $msg");
      $error = sprintf(_("Could not query forwarding information!"));
      vadmin_system_error($error);
   }


   // insert forwards into user info array: first, pull into a CSV
   // string to unify databases where they are stored in CSV format
   // with databases where they are stored individually
   //
   $userInfo[3] = '';
   foreach ($forwards as $forward)
      $userInfo[3] .= $forward[0] . ', ';
   if (sizeof($forwards))
   {

      // remove trailing comma and put (back) into array form
      //
      $userInfo[3] = substr($userInfo[3], 0, strlen($userInfo[3]) - 2);
      $userInfo[3] = explode(',', $userInfo[3]);

      // remove whitespace
      //
      foreach ($userInfo[3] as $j => $fwd)
         $userInfo[3][$j] = trim($fwd);

   }
   else
      $userInfo[3] = array();


   if (!$rawDump)
   {

      // parse out Courier-style quota info for hard quota
      // and message count fields if necessary
      //
      if (!empty($userInfo[5]) && $userInfo[5] !== '===N/A===')
      {
         if (preg_match('/(\d+)S/', $userInfo[5], $matches))
            $userInfo[5] = $matches[1];
         else
            $userInfo[5] = '-';
      }
      if (!empty($userInfo[8]) && $userInfo[8] !== '===N/A===')
      {
         if (preg_match('/(\d+)C/', $userInfo[8], $matches))
            $userInfo[8] = $matches[1];
         else
            $userInfo[8] = '-';
      }


      // set N/A fields to just hyphen to match vmailmgr syntax
      //
      if ($userInfo[5] === '===N/A===' || $userInfo[5] == 0) $userInfo[5] = '-';
      if ($userInfo[6] === '===N/A===') $userInfo[6] = '-';
      if ($userInfo[7] === '===N/A===') $userInfo[7] = '-';
      if ($userInfo[8] === '===N/A===' || $userInfo[8] == 0) $userInfo[8] = '-';
      if ($userInfo[10] === '===N/A===') $userInfo[10] = '-';

   }


   // hide password
   //
   if ($obscurePassword && !empty($userInfo[1]))
      $userInfo[1] = 'Set';


   return array($returnCode, $userInfo);

}



/**
  * Get quick list of account names in a given domain
  *
  * @param string $domain The vadmin domain.
  * @param string $password The domain password.
  * @param string $searchTerms If given, specifies a pattern upon which
  *                            users should be searhced instead of
  *                            returning all users, returns only those
  *                            that match the pattern (optional; default = '')
  * @param boolean $includeAliases If TRUE, alias names are included in 
  *                                returned list, if FALSE, they are not
  *                                included; only actual user account names
  *                                are returned.  (optional; default = TRUE)
  * @param boolean $convertToLowerCase If TRUE, all usernames returned
  *                                    are converted to lowercase (optional;
  *                                    default = FALSE)
  *
  * @return array A two-element array: the first being
  *               a return code (where zero is non-error),
  *               the second being an array of user
  *               information arrays; each user
  *               information array contains these fields (in
  *               this order):
  *                 string account/alias name
  *               Or if an error occurred, the second
  *               array element of the return array may
  *               contain an error message.
  *
  */
function getQuickAccountList($domain, $password, $searchTerms='', 
                             $includeAliases=TRUE, $convertToLowerCase=FALSE)
{

   $me = 'getQuickAccountList';
   $returnCode = 0;


   $AUTHCODE    = vadmin_getvar('SESSION', 'VADMIN_AUTHCODE');
   $LVL = vadmin_getvar('VADMIN', 'LVL');


   // validate password
   //
   if (!vadmin_domain_passwd_validate($domain, $password))
   {

      spew("$me: the password did NOT verify");

      // See if we need to zero out the password.
      //
      if ($AUTHCODE == 'LOWLY')
      {                        
         spew("$me: Stored password did not verify! Removing it");
         vadmin_put_domain_passwd($domain, FALSE);
         $msg = _("The stored password file was invalid and has been removed. Please contact your systems administrator.");
         vadmin_system_error($msg);
      }                      
  
      // This is a CROSS or ELVIS with auth=user. This still
      // means that the stored password is incorrect, but we
      // can redirect them to the askdompass instead of just
      // booting them off.
      //          
      spew("$me: Stored password did not verify! Asking again.");
      vadmin_put_domain_passwd($domain, FALSE);
      vadmin_redirect($LVL, 'login', 'askdompass', $domain);
  
   }                
  
  
   // for now, not saving in session
   //
/*
   // if we have list stored in session, don't need to query database
   //
   spew("$me: Looking for user list in session.");
   $users = vadmin_getvar('SESSION', 'vadmin_quick_domain_lookup');
   $oldSearchTerms = vadmin_getvar('SESSION', 'vadmin_quick_domain_lookup_search_terms');
   $oldIncdludeAliases = vadmin_getvar('SESSION', 'vadmin_quick_domain_lookup_include_aliases');

   if (empty($users) || $oldSearchTerms != $searchTerms || $oldIncdludeAliases != $includeAliases)
   {
*/
   
      // get database connection
      //
//      spew("$me: Not found.  Querying from database.");
      $db = vadmin_get_database_connection();


      // figure out search clause if needed
      //
      if (!empty($searchTerms))
      {

         // strip off domain portion of username, if user
         // mistakenly added it
         //
         $delim = vadmin_getvar('BACKEND', 'delimiters');
         $delim = substr($delim, 0, 1);
         if (strpos($searchTerms, $delim) !== FALSE)
            $search = substr($searchTerms, 0, strpos($searchTerms, $delim));
         else
            $search = $searchTerms;

         // change * to correct database wildcard
         //
         $database_wildcard = vadmin_getvar('BACKEND', 'database_wildcard');
         $search = str_replace('*', $database_wildcard, $search);

         // construct the full search clause
         //
         $searchClause = vadmin_getvar('BACKEND', 'all_accounts_search_clause');
         $searchClause = str_replace('$1', str_esc($search), $searchClause);

      }
      else
         $searchClause = '';


      // insert domain (and search clause) into the query
      //
      $sql = vadmin_getvar('BACKEND', 'vadmin_get_all_usernames_for_domain_query');
      $sql = str_replace(array('$1', '$2'), 
                         array(str_esc($domain), $searchClause), 
                         $sql);


      // query for accounts
      //
      spew("$me: Trying to get user list for domain '$domain'");
      spew("$me: Running query: $sql");
      $users = $db->getAll($sql);


      // check for database errors
      //
      if (DB::isError($users))
      {
         $msg = $users->getMessage();
         spew("$me: Database error: $msg");
         $error = sprintf(_("Could not query user list!"));
         vadmin_system_error($error);
      }


      // check for unexpected results (should always be the same number of
      // fields for each user) (can only test this if we get at least one
      // user in the list)
      //
      if (sizeof($users) > 0 && sizeof($users[0]) != 1)
      {
         spew("$me: Uhm... Something's funny with your user list query.  Check the config file.");
         $error = sprintf(_("Bad user list query!"));
         vadmin_system_error($error);
      }


      // now get aliases...


      if ($includeAliases)
      {

         // insert domain into query
         //
         $sql = vadmin_getvar('BACKEND', 'find_all_aliases_query');
         $sql = str_replace('$1', str_esc($domain), $sql);


         // query for aliases
         //
         spew("$me: Trying to get aliases for '$domain'");
         spew("$me: Running query: $sql");
         $aliases = $db->getAll($sql);

   
         // check for database errors
         //
         if (DB::isError($aliases))
         {
            $msg = $aliases->getMessage();
            spew("$me: Database error: $msg");
            $error = sprintf(_("Could not query aliases!"));
            vadmin_system_error($error);
         }


         // remove any aliases that are duplicates of full user
         // accounts already queried above
         //
         $aliasList = array();
         foreach ($aliases as $aliasInfo)
         {

            // remove entries of users already queried above
            // (these are probably just forwards)
            //
            $foundMatch = FALSE;
            foreach ($users as $userInfo)
               if ($userInfo[0] == $aliasInfo[0])
                  $foundMatch = TRUE;

            if (!$foundMatch)
               $aliasList[] = $aliasInfo;

         }


         // merge aliases and user accounts
         //
         $users = array_merge($users, $aliasList);


      }


      // now we need to make sure username doesn't contain domain
      //
      $delimiters = vadmin_getvar('BACKEND', 'delimiters');
      foreach ($users as $index => $userInfo)
      {

         for ($i = 0; $i < strlen($delimiters); $i++)
         {
            if (($delimPos = strpos($userInfo[0], substr($delimiters, $i, 1))) !== FALSE)
            {
               $users[$index][0] = substr($userInfo[0], 0, $delimPos);
               $userInfo[0] = $users[$index][0];
               break;
            }
         }

      }


   // for now, not saving in session
   //
/*
      // store in session for better performance
      //
      vadmin_putvar('SESSION', 'vadmin_quick_domain_lookup', $users);
      vadmin_putvar('SESSION', 'vadmin_quick_domain_lookup_search_terms', $searchTerms);
      vadmin_putvar('SESSION', 'vadmin_quick_domain_lookup_include_aliases', $includeAliases);


   } // end lookup in database (not from session)
*/



   // if search terms are being used, pick only
   // those that match
   //
   // note that this is still needed even though
   // the query for user accounts above already
   // has a search clause, because the query above
   // for aliases does not, as well as the fact
   // that other functions herein (add user/alias,
   // change user, delete user/alias) will possibly
   // add users to the cached user list that do
   // NOT fit the search terms
   //
   if (!empty($searchTerms))
   {

      // build regexp
      //
      $searchTerms = str_replace('*', '.*?', $searchTerms);

      $newUsers = array();
      foreach ($users as $user)
      {

         if (preg_match("/^$searchTerms\$/", $user[0]))
            $newUsers[] = $user;

      }

      $users = $newUsers;

   }


   // flatten array (could probably just use $db->getCol(0) above,
   // but hopefully PHP isn't too slow here...  but while we're 
   // at it, convert to lower case if needed too
   //
   $newUsers = array();
   foreach ($users as $user)
      if ($convertToLowerCase)
         $newUsers[] = strtolower($user[0]);
      else
         $newUsers[] = $user[0];


   return $newUsers;

}



/**
  * Get full account list for a given domain.
  *
  * Queries out a list of users and aliases and their 
  * account attributes for any one domain.  Users are
  * returned sorted by username.
  *
  * NOTE that if pagination is being used, a global variable
  * called $LISTDOMAIN_TOTAL_USERS will be set to the number
  * of overall users and aliases in the database so the 
  * caller can calculate the total number of pages to display
  * to the user.  Also, an extra (optional) global array called
  * $LISTDOMAIN_FIRST_USER_PER_PAGE will be made available,
  * containing a list of the usernames that are first on
  * every page; the array starts with key number zero (for page
  * number one).
  *
  * NOTE that query results are cached in session to save
  * subsequent lookups, since this can be an expensive
  * query on large systems.  Other functions that change
  * user attributes or add/delete users should invalidate
  * this cache by removing the session variable named
  * 'vadmin_domain_lookup':
  *     vadmin_putvar('SESSION', 'vadmin_domain_lookup', '');
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
  *                        (optional; default first page if there is pagination)
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
  * @return array A two-element array: the first being
  *               a return code (where zero is non-error), 
  *               the second being an array of user 
  *               information arrays; each user
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
  *                 mixed an array of user webmail stats or 
  *                       possibly empty
  *                 mixed custom first field value, if any
  *                 mixed custom second field value, if any
  *                 mixed custom third field value, if any
  *                 mixed custom fourth field value, if any
  *                 etcetera
  *               Or if an error occurred, the second
  *               array element of the return array may
  *               contain an error message.
  *
  */
function getAllUsers($domain, $password, $limit=0, $pageNumber=0, $sortCol='', $searchTerms='', $activeLookup=0)
{

   $me = 'getAllUsers';
   $returnCode = 0;


   $AUTHCODE    = vadmin_getvar('SESSION', 'VADMIN_AUTHCODE');
   $LVL = vadmin_getvar('VADMIN', 'LVL');


   // validate password
   //
   if (!vadmin_domain_passwd_validate($domain, $password))
   {

      spew("$me: the password did NOT verify");

      // See if we need to zero out the password.
      // 
      if ($AUTHCODE == 'LOWLY')
      {
         spew("$me: Stored password did not verify! Removing it");
         vadmin_put_domain_passwd($domain, FALSE);
         $msg = _("The stored password file was invalid and has been removed. Please contact your systems administrator.");
         vadmin_system_error($msg);
      }

      // This is a CROSS or ELVIS with auth=user. This still
      // means that the stored password is incorrect, but we
      // can redirect them to the askdompass instead of just
      // booting them off.
      //
      spew("$me: Stored password did not verify! Asking again.");
      vadmin_put_domain_passwd($domain, FALSE);
      vadmin_redirect($LVL, 'login', 'askdompass', $domain);

   }


   // if we have list stored in session, don't need to query database
   //
   spew("$me: Looking for user list in session.");
   $users = vadmin_getvar('SESSION', 'vadmin_domain_lookup');
   $oldSearchTerms = vadmin_getvar('SESSION', 'vadmin_domain_lookup_search_terms');

   if (empty($users) || $oldSearchTerms != $searchTerms)
   {

      // get database connection
      //
      spew("$me: Not found.  Querying from database.");
      $db = vadmin_get_database_connection();


      // figure out search clause if needed
      //
      if (!empty($searchTerms))
      {

         // strip off domain portion of username, if user
         // mistakenly added it
         //
         $delim = vadmin_getvar('BACKEND', 'delimiters');
         $delim = substr($delim, 0, 1);
         if (strpos($searchTerms, $delim) !== FALSE) 
            $search = substr($searchTerms, 0, strpos($searchTerms, $delim));
         else
            $search = $searchTerms;

         // change * to correct database wildcard
         //
         $database_wildcard = vadmin_getvar('BACKEND', 'database_wildcard');
         $search = str_replace('*', $database_wildcard, $search);

         // construct the full search clause
         //
         $searchClause = vadmin_getvar('BACKEND', 'all_accounts_search_clause');
         $searchClause = str_replace('$1', str_esc($search), $searchClause);

      }
      else
         $searchClause = '';


      // insert domain (and search clause) into the query
      //
      $sql = vadmin_getvar('BACKEND', 'list_all_accounts_query');
      $sql = str_replace(array('$1', '$2'), 
                         array(str_esc($domain), $searchClause), 
                         $sql);


      // query for accounts
      //
      spew("$me: Trying to get account list for domain '$domain'");
      spew("$me: Running query: $sql");
      $users = $db->getAll($sql);


      // check for database errors
      //
      if (DB::isError($users))
      {
         $msg = $users->getMessage();
         spew("$me: Database error: $msg");
         $error = sprintf(_("Could not query account list!"));
         vadmin_system_error($error);
      }


      // check for unexpected results (should always be the same number of 
      // fields for each user) (can only test this if we get at least one
      // user in the list)
      //
      if (sizeof($users) > 0 && sizeof($users[0]) != VADMIN_USER_DATA_FIELD_COUNT)
      {
         spew("$me: Uhm... Something's funny with your account list query.  Check the config file.");
         $error = sprintf(_("Bad account list query!"));
         vadmin_system_error($error);
      }


      // Two fields, index 12 and 13 are for special values
      // that will be added later.  For now, we need to push
      // other values down if any custom fields were queried
      // into those index slots
      //
      if (sizeof($users) > 0 && sizeof($users[0]) > 12)
      {

         foreach ($users as $i => $user)
         {
            $extra_fields = array_splice($user, 12);
            $users[$i] = array_merge($user, array('', ''), $extra_fields);
         }

      }


      // now get aliases...
  
   
      // insert domain into query
      //
      $sql = vadmin_getvar('BACKEND', 'find_all_aliases_query');
      $sql = str_replace('$1', str_esc($domain), $sql);
   
   
      // query for aliases 
      //
      spew("$me: Trying to get aliases for '$domain'");
      spew("$me: Running query: $sql");
      $aliases = $db->getAll($sql);


      // check for database errors
      //
      if (DB::isError($aliases))
      {
         $msg = $aliases->getMessage();
         spew("$me: Database error: $msg");
         $error = sprintf(_("Could not query aliases!"));
         vadmin_system_error($error);
      }


      // remove any aliases that are duplicates of full user
      // accounts already queried above 
      //
      $delim = vadmin_getvar('BACKEND', 'delimiters');
      $delim = substr($delim, 0, 1);
      $aliasList = array();
      foreach ($aliases as $aliasInfo)
      {

         $aliasInfo[12] = TRUE;


         // make sure to compare apples with apples:
         // remove "@domain" if found
         //
         if (strpos($aliasInfo[0], "$delim$domain") !== FALSE) 
           $aliasComp = substr($aliasInfo[0], 0, strpos($aliasInfo[0], "$delim$domain"));
         else
           $aliasComp = $aliasInfo[0];


         // remove entries of users already queried above
         // (these are probably just forwards)
         //
         $foundMatch = FALSE;
         foreach ($users as $userInfo)
         {

            // make sure to compare apples with apples:
            // remove "@domain" if found
            //
            if (strpos($userInfo[0], "$delim$domain") !== FALSE) 
               $userComp = substr($userInfo[0], 0, strpos($userInfo[0], "$delim$domain"));
            else
               $userComp = $userInfo[0];


            // old way, without removing "@domain" before comparing: 
            // if ($userInfo[0] == $aliasInfo[0])
            if ($userComp == $aliasComp)
            {
               $foundMatch = TRUE;
               break;
            }

         }
   
         if (!$foundMatch)
            $aliasList[] = $aliasInfo;

      }


      // merge aliases and user accounts
      //
      $users = array_merge($users, $aliasList);


      // now we need to look up forwarding information for each user
      // and make sure username doesn't contain domain
      //
      $origSql = vadmin_getvar('BACKEND', 'find_user_forwards_query');
      $delimiters = vadmin_getvar('BACKEND', 'delimiters');
      foreach ($users as $index => $userInfo)
      {

         if (empty($users[$index][12])) $users[$index][12] = FALSE;


         // take care of username
         //
         for ($i = 0; $i < strlen($delimiters); $i++)
         {
            if (($delimPos = strpos($userInfo[0], substr($delimiters, $i, 1))) !== FALSE)
            {
               $users[$index][0] = substr($userInfo[0], 0, $delimPos);
               $userInfo[0] = $users[$index][0];
               break;
            }
         }


         // now get forwarding info...


         // insert domain and username into query
         //
         $sql = str_replace(array('$1', '$2'), 
                            array(str_esc($userInfo[0]), str_esc($domain)), 
                            $origSql);


         // query for forwards
         //
         spew("$me: Trying to get forwarding information for user '" . $userInfo[0] . "' at '$domain'");
         spew("$me: Running query: $sql");
         $forwards = $db->getAll($sql);


         // check for database errors
         //
         if (DB::isError($forwards))
         {
            $msg = $forwards->getMessage();
            spew("$me: Database error: $msg");
            $error = sprintf(_("Could not query forwarding information!"));
            vadmin_system_error($error);
         }


         // insert forwards into user info array: first, pull into a CSV
         // string to unify databases where they are stored in CSV format
         // with databases where they are stored individually
         //
         $users[$index][3] = '';
         foreach ($forwards as $forward)
            $users[$index][3] .= $forward[0] . ', ';
         if (sizeof($forwards))
         {

            // remove trailing comma and put (back) into array form
            //
            $users[$index][3] = substr($users[$index][3], 0, strlen($users[$index][3]) - 2);
            $users[$index][3] = explode(',', $users[$index][3]);

            // remove whitespace
            //
            foreach ($users[$index][3] as $j => $fwd)
               $users[$index][3][$j] = trim($fwd);

         }
         else
            $users[$index][3] = array();

      }


      // hide password for everyone
      //
      foreach ($users as $index => $user)
         if (!empty($user[1]))
            $users[$index][1] = 'Set';


      // store in session for better performance
      //
      vadmin_putvar('SESSION', 'vadmin_domain_lookup', $users);
      vadmin_putvar('SESSION', 'vadmin_domain_lookup_search_terms', $searchTerms);


   } // end lookup in database (not from session)



   // if active or inactive users only should be returned,
   // strip out anything else here
   //
   if ($activeLookup != 0)
   {

      $newUsers = array();
      foreach ($users as $user)
      {

         if (!isset($user[13]['LAST_LOGIN'])) 
            $user[13] = vadmin_get_user_webmail_stats($user[0], $domain);

         if ($activeLookup == -1 
          && $user[13]['NEVER_LOGGED_IN'] && !is_alias($user))
            $newUsers[] = $user;

         else if ($activeLookup == 1 && !$user[13]['NEVER_LOGGED_IN'])
            $newUsers[] = $user;

      }

      $users = $newUsers;

   }



   // if search terms are being used, pick only 
   // those that match 
   //
   // note that this is still needed even though
   // the query for user accounts above already
   // has a search clause, because the query above 
   // for aliases does not, as well as the fact
   // that other functions herein (add user/alias, 
   // change user, delete user/alias) will possibly
   // add users to the cached user list that do
   // NOT fit the search terms
   //
   if (!empty($searchTerms))
   {

      // build regexp
      //
      $searchTerms = str_replace('*', '.*?', $searchTerms);

      $newUsers = array();
      foreach ($users as $user)
      {

         if (preg_match("/^$searchTerms\$/i", $user[0]))
            $newUsers[] = $user;

      }

      $users = $newUsers;

   }



   // sort as needed
   //
   if ($sortCol == 'login') {
      global $sortdomain, $sortcatchall_alias, $sortusername,
             $sortdesig, $sort_add_delim;
      $sortdomain = $domain;
      $sortcatchall_alias = vadmin_getvar('BACKEND', 'catchall_alias');
      $sortusername = vadmin_getvar('SQMAIL', 'username');
      $sort_add_delim = vadmin_IMAP_usernames_have_domain($domain);
      $sortdesig = vadmin_get_user_designation($domain, $sortusername);
      usort($users, 'sortUserArrayByLoginTime');
   }
   else
      usort($users, 'sortUserArrayByName');



   // unfortunately, we have to paginate manually...
   //
   if ($limit)
   {

      $fancy_pagination = vadmin_getvar('BACKEND', 'fancy_pagination');

      global $LISTDOMAIN_TOTAL_USERS;
      $LISTDOMAIN_TOTAL_USERS = count($users);

      if ($fancy_pagination == 'yes')
      {
         global $LISTDOMAIN_FIRST_USER_PER_PAGE;
         $LISTDOMAIN_FIRST_USER_PER_PAGE = array();
      }

      if (!$pageNumber) $pageNumber = 1;
      $offset = ($pageNumber - 1) * $limit;

      $newUsers = array();
      $i = -1;
      foreach ($users as $user)
      {
         if ($fancy_pagination == 'yes' && (($i + 1) % $limit == 0)) 
            if ($sortCol == 'login') 
               $LISTDOMAIN_FIRST_USER_PER_PAGE[] = $user[13]['LAST_LOGIN_SHORT'] . ' (' . $user[0] . ')';
            else
               $LISTDOMAIN_FIRST_USER_PER_PAGE[] = $user[0];

         if (++$i < $offset) continue;
         if ($i >= $offset + $limit)
         {
            if ($fancy_pagination == 'yes') 
               continue;
            else
               break;
         }

         $newUsers[] = $user;
      }

      $users = $newUsers;

   }


   // last-second changes...
   //
   foreach ($users as $i => $user)
   {

      // parse out Courier-style quota info for hard quota
      // and message count fields if necessary
      //
      if (!empty($user[5]) && $user[5] !== '===N/A===')
      {
         if (preg_match('/(\d+)S/', $user[5], $matches))
            $users[$i][5] = $matches[1];
         else
            $users[$i][5] = '-';
      }
      if (!empty($user[8]) && $user[8] !== '===N/A===')
      {
         if (preg_match('/(\d+)C/', $user[8], $matches))
            $users[$i][8] = $matches[1];
         else
            $users[$i][8] = '-';
      }


      // need to get the N/A fields into vmailmgr syntax
      //
      if ($user[5] === '===N/A===' || $user[5] == 0) $users[$i][5] = '-';
      if ($user[6] === '===N/A===') $users[$i][6] = '-';
      if ($user[7] === '===N/A===') $users[$i][7] = '-';
      if ($user[8] === '===N/A===' || $user[8] == 0) $users[$i][8] = '-';
      if ($user[10] === '===N/A===') $users[$i][10] = '-';
      
   }


   // return users array info
   //
   return array($returnCode, $users);

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
function getRealAccountForAlias($domain, $alias, $domainPassword)
{

   $me = 'getRealAccountForAlias';
   $returnCode = 0;


   $AUTHCODE = vadmin_getvar('SESSION', 'VADMIN_AUTHCODE');
   $LVL = vadmin_getvar('VADMIN', 'LVL');


   // validate password
   //
   if (!vadmin_domain_passwd_validate($domain, $domainPassword))
   {

      spew("$me: the password did NOT verify");

      // See if we need to zero out the password.
      //
      if ($AUTHCODE == 'LOWLY')
      {
         spew("$me: Stored password did not verify! Removing it");
         vadmin_put_domain_passwd($domain, FALSE);
         $msg = _("The stored password file was invalid and has been removed. Please contact your systems administrator.");
         vadmin_system_error($msg);
      }

      // This is a CROSS or ELVIS with auth=user. This still
      // means that the stored password is incorrect, but we
      // can redirect them to the askdompass instead of just
      // booting them off.
      //
      spew("$me: Stored password did not verify! Asking again.");
      vadmin_put_domain_passwd($domain, FALSE);
      vadmin_redirect($LVL, 'login', 'askdompass', $domain);

   }


   // get database connection
   //
   $db = vadmin_get_database_connection();


   // insert domain and username into the query
   //
   $sql = vadmin_getvar('BACKEND', 'find_real_account_for_alias_query');
   $sql = str_replace(array('$1', '$2'), 
                      array(str_esc($alias), str_esc($domain)), 
                      $sql);


   // query for real account
   //
   spew("$me: Trying to get real account information for alias '" . $alias . "' at '$domain'");
   spew("$me: Running query: $sql");
   $realAccounts = $db->getAll($sql);


   // check for database errors
   //
   if (DB::isError($realAccounts))
   {
      $msg = $realAccounts->getMessage();
      spew("$me: Database error: $msg");
      $error = sprintf(_("Could not query real account information!"));
      vadmin_system_error($error);
   }


   // first, pull into a CSV string to unify databases where 
   // multiple results are stored in CSV format with databases 
   // where they are stored individually
   //
   $accounts = '';
   foreach ($realAccounts as $account)
      $accounts .= $account[0] . ', ';
   if (sizeof($realAccounts))
   {

      // remove trailing comma and put (back) into array form
      //
      $accounts = substr($accounts, 0, strlen($accounts) - 2);
      $accounts = explode(',', $accounts);

      // remove whitespace
      //
      foreach ($accounts as $j => $acct)
         $accounts[$j] = trim($acct);

   }
   else
      return array(1, _("Alias not found")); //TODO: better error message necessary?


   return array($returnCode, $accounts);

}



/**
  * Change an attribute for user account
  *
  * Changes one attribute at a time (yeah... inefficient...)
  * according to a numeric attribute key.
  *
  * @param string $domain The vadmin domain.
  * @param string $user The account name having its password changed.
  * @param string $domainPassword The domain password.
  * @param int $attr The numeric key of the attribute to be changed.  
  *                  This parameter should correspond to the following
  *                  list:
  *     1 : password
  *     2 : dest (mailbox)
  *     3 : hard quota
  *     4 : soft quota
  *     5 : message size limit
  *     6 : message count limit
  *     7 : account expiry
  *     8 : mailbox flags
  *     9 : personal (personal/real name information)
  *    10 : forwards (non-vmailmgr; added for SQL backend)
  *    <or the name of a custom field if it is being changed, 
  *     with spaces changed to uderscores>
  *
  * @param string $value The new value of the attribute.
  *
  * @return array A two-element array: the first being
  *               a return code (where zero is non-error),
  *               the second being a message, which is usually
  *               only important if there was an error.
  *
  */
function changeAttribute($domain, $user, $domainPassword, $attr, $value)
{
  
   $me = 'changeAttribute'; 
   $returnCode = 0;         
   $errorMessage = '';
   global $custom_fields;
  
  
   $AUTHCODE = vadmin_getvar('SESSION', 'VADMIN_AUTHCODE');
   $LVL = vadmin_getvar('VADMIN', 'LVL');
   
   
   // validate password
   //
   if (!vadmin_domain_passwd_validate($domain, $domainPassword))
   {

      spew("$me: the password did NOT verify");

      // See if we need to zero out the password.
      // 
      if ($AUTHCODE == 'LOWLY')
      {
         spew("$me: Stored password did not verify! Removing it");
         vadmin_put_domain_passwd($domain, FALSE);
         $msg = _("The stored password file was invalid and has been removed. Please contact your systems administrator.");
         vadmin_system_error($msg);
      }

      // This is a CROSS or ELVIS with auth=user. This still
      // means that the stored password is incorrect, but we
      // can redirect them to the askdompass instead of just
      // booting them off.
      //
      spew("$me: Stored password did not verify! Asking again.");
      vadmin_put_domain_passwd($domain, FALSE);
      vadmin_redirect($LVL, 'login', 'askdompass', $domain);
  
   }
  
  
   // get all user attributes 
   //
   $userInfo = getUser($domain, $user, $domainPassword, FALSE);
   if ($userInfo[0]) 
      vadmin_system_error(sprintf(_("Cannot find user \"%s\""), $user));
   $userInfo = $userInfo[1];


   // now, replace the one that needs updating
   //
   $checkCustomFields = FALSE;
   switch ($attr)
   {

      // password
      //
      case 1: 

         $userInfo[1] = constructPassword($value);
         break;


      // dest (mailbox)
      //
      case 2: 

         $userInfo[2] = $value;

         break;


      // hard quota
      //
      case 3: 

         $userInfo[5] = $value;

         break;


      // soft quota
      //
      case 4: 

         $userInfo[6] = $value;

         break;


      // message size limit
      //
      case 5: 

         $userInfo[7] = $value;

         break;


      // message count limit
      //
      case 6: 

         $userInfo[8] = $value;

         break;


      // account expiry
      //
      case 7: 

         $userInfo[10] = $value;

         break;


      // mailbox flags
      //
      case 8: 

         $userInfo[11] = $value;

         break;


      // personal info/real name
      //
      case 9: 

         $userInfo[4] = $value;

         break;


      // aliases
      //
      case 10: 

         // not yet, still need this: $userInfo[3] = $value;

         break;


      default:
         $checkCustomFields = TRUE;

   }


   // check custom fields
   //
   $foundIt = TRUE;
   if ($checkCustomFields)
   {
      $foundIt = FALSE;
      $i = 13;
      foreach ($custom_fields as $field)
      {
         $i++;
         if ($attr == str_replace(' ', '_', $field))
         {
            $foundIt = TRUE;
            $userInfo[$i] = $value;
            break;
         }
      }
   }


   // at long last, if we don't understand what attribute 
   // is being changed, error and bail
   //
   if (!$foundIt)
      vadmin_system_error(sprintf(_("Invalid chattr type: %d"), $attr));


   // forwards are changed differently
   //
   if ($attr == 10)
   {

      changeUserForwards($domain, $user, $domainPassword, $userInfo[3], $value);

   }


   // all other standard attributes
   //
   else
   {

      // put quotes around password as is when it isn't being changed
      //
      if ($attr != 1) $userInfo[1] = "'" . str_esc($userInfo[1]) . "'";


      // construct Courier-style quota 
      //
      $courierQuota = '';
      if (!empty($userInfo[5]) && $userInfo[5] != '-')
         $courierQuota = $userInfo[5] . 'S';
      else
         $courierQuota = '0S';
      if (!empty($userInfo[8]) && $userInfo[8] != '-')
         $courierQuota .= $userInfo[8] . 'C';
      else
         $courierQuota .= '0C';


      // get database connection
      //
      $db = vadmin_get_database_connection();
  
  
      // insert all parameters into query
      //
      $sql = vadmin_getvar('BACKEND', 'update_user_attributes_query');
      if (!empty($sql))
      {

         // replace password last because in some cases it can contain some
         // character sequences such as $5 that should not be replaced
         //
         $sql = str_replace(array('$1', '$2', '$d', '$4', '$5', '$6', '$7',
                                  '$8', '$9', '$a', '$b', '$c', '$3'), 
                            array(str_esc($user),
                                  str_esc($domain), 
                                  str_esc(date('Y-m-d H:i:s')), 
                                  str_esc($userInfo[2]), 
                                  str_esc($userInfo[4]), 
                                  str_esc($userInfo[5]), 
                                  str_esc($userInfo[6]), 
                                  str_esc($userInfo[7]), 
                                  str_esc($userInfo[8]), 
                                  str_esc($courierQuota), 
                                  str_esc($userInfo[10]), 
                                  str_esc($userInfo[11]), 
                                  $userInfo[1]),  // already escaped in constructPassword()
                            $sql);

         // substitute custom fields
         //
         $i = 97;  // ord('a')
         $j = 14;
         foreach ($custom_fields as $field)
         {
            $sql = str_replace('$_' . chr($i++), str_esc($userInfo[$j++]), $sql);
         }
      

         // send update to database
         //
         spew("$me: Trying to update account information for user '$user' in domain '$domain'");
         spew("$me: Running query: $sql");
         $result = $db->query($sql);


         // check for database errors
         //
         if (DB::isError($result))
         {
            $msg = $result->getMessage();
            spew("$me: Database error: $msg");
            $error = sprintf(_("Could not update user info!"));
            vadmin_system_error($error);
         }

      }


      // now run system management proxy
      //
      $flags = vadmin_getvar('CONFIG', 'system_manager_proxy.account_change_flags');
      $sanitize = vadmin_getvar('CONFIG', 'system_manager_proxy.sanitize_flag_arguments');
      if (!empty($flags))
      {
         $username = vadmin_getvar('SQMAIL', 'username');
         $flags = str_replace(array('$1', '$2', '$3', '$4', '$u', '$p'), 
                              ($sanitize == 'yes' ?
                                 array(escapeshellarg($user), 
                                       escapeshellarg($domain), 
                                       escapeshellarg($attr), 
                                       escapeshellarg($value),
                                       escapeshellarg($username), 
                                       escapeshellarg(vadmin_get_current_pwd())) 
                               : array($user, 
                                       $domain, 
                                       $attr, 
                                       $value,
                                       $username, 
                                       vadmin_get_current_pwd())), 
                              $flags);

         $returnCode = vadmin_system_manager_proxy($domain, $domainPassword, $flags);
         if ($returnCode[0] != 0) 
         {
            $errorMessage = _("Backend proxy error");
            if (!empty($returnCode[1]))
            {
               spew("$me: Backend proxy error: " . $returnCode[1]);
               // nah, let's not do this so no security issues are 
               // exposed in an error message from the backend...
               // $errorMessage = sprintf(_("Backend proxy error: %s"), $returnCode[1]);
            }
         }
      }


   }


   // replace user info in cache of user list
   //
   $users = vadmin_getvar('SESSION', 'vadmin_domain_lookup');
   if (!empty($users))
   {
      $newUserInfo = getUser($domain, $user, $domainPassword, TRUE, TRUE);
      if ($newUserInfo[0])
      {
         spew("$me: Fatal error: could not retrieve user that was just updated");
         $error = sprintf(_("Could not retrieve user that was just updated!"));
         vadmin_system_error($error);
      }
      foreach ($users as $i => $u)
         if ($u[0] == $newUserInfo[1][0])
         {
            $users[$i] = $newUserInfo[1];
            break;
         }
      vadmin_putvar('SESSION', 'vadmin_domain_lookup', $users);
   }
//   // clear cache of user list
//   //
//   vadmin_putvar('SESSION', 'vadmin_domain_lookup', '');


   return array($returnCode[0], $errorMessage);

}



/**
  * Change account forwards
  *
  * @param string $domain The vadmin domain.
  * @param string $user The account name having its forwards changed.
  * @param string $domainPassword The domain password.
  * @param array $oldForwards A list of the old forwards.
  * @param array $newForwards A list of the new forwards.
  *
  * @return array A two-element array: the first being
  *               a return code (where zero is non-error),
  *               the second being a message, which is usually
  *               only important if there was an error.
  *
  */
function changeUserForwards($domain, $user, $domainPassword, $oldForwards, $newForwards)
{

   if (empty($domain))         return array(1, _("Empty domain"));
   if (empty($domainPassword)) return array(1, _("Empty domain password"));

   $catchall_alias = vadmin_getvar('BACKEND', 'catchall_alias');
   if (empty($user) && !empty($catchall_alias)) return array(1, _("Empty username"));

      
   $me = 'changeUserForwards';
   $returnCode = 0;         
   $errorMessage = '';
         
         
   $AUTHCODE = vadmin_getvar('SESSION', 'VADMIN_AUTHCODE');
   $LVL = vadmin_getvar('VADMIN', 'LVL');
         
         
   // validate password
   //       
   if (!vadmin_domain_passwd_validate($domain, $domainPassword))
   {        

      spew("$me: the password did NOT verify");
            
      // See if we need to zero out the password.
      //    
      if ($AUTHCODE == 'LOWLY')
      {     
         spew("$me: Stored password did not verify! Removing it");
         vadmin_put_domain_passwd($domain, FALSE);
         $msg = _("The stored password file was invalid and has been removed. Please contact your systems administrator.");
         vadmin_system_error($msg);
      }        
            
      // This is a CROSS or ELVIS with auth=user. This still
      // means that the stored password is incorrect, but we
      // can redirect them to the askdompass instead of just
      // booting them off.
      //       
      spew("$me: Stored password did not verify! Asking again.");
      vadmin_put_domain_passwd($domain, FALSE);
      vadmin_redirect($LVL, 'login', 'askdompass', $domain);

   }


   // compact all forwards into single CSV string if needed
   //
   $csv = vadmin_getvar('BACKEND', 'forwardsCSV');
   if (strtolower($csv) == "yes")
   {
      $oldForwards = array(implode(',', $oldForwards));
      $newForwards = array(implode(',', $newForwards));
   }


   // get database connection
   //
   $db = vadmin_get_database_connection();


   // remove all old forwards
   // (currently, there is no need to use the old forward list)
   //
   $sql = vadmin_getvar('BACKEND', 'delete_all_user_forwards_query');
   $sql = str_replace(array('$1', '$2'), 
                      array(str_esc($user), str_esc($domain)), 
                      $sql);


   // go!
   //
   spew("$me: Trying to delete all forwards for account '$user' in domain '$domain'");
   spew("$me: Running query: $sql");
   $result = $db->query($sql);


   // check for database errors
   //
   if (DB::isError($result))
   {
      $msg = $result->getMessage();
      spew("$me: Database error: $msg");
      $error = sprintf(_("Could not remove user forwards!"));
      vadmin_system_error($error);
   }


   // run insert query for each new forward
   //
   $delim = vadmin_getvar('BACKEND', 'delimiters');
   $delim = substr($delim, 0, 1);
   spew("$me: Trying to add new forwards for account '$user' in domain '$domain'");
   foreach ($newForwards as $forward)
   {

      if (empty($forward)) continue;


      // insert domain and username into the query
      //
      $sql = vadmin_getvar('BACKEND', 'insert_forward_query');
      $sql = str_replace(array('$1', '$2'), 
                         array(str_esc("$user$delim$domain"), str_esc($forward)), 
                         $sql);


      // go!
      //
      spew("$me: Running query: $sql");
      $result = $db->query($sql);


      // check for database errors
      //
      if (DB::isError($result))
      {
         $msg = $result->getMessage();
         spew("$me: Database error: $msg");
         $error = sprintf(_("Could not add user forward!"));
         vadmin_system_error($error);
      }

   }


   // now run system management proxy
   //
   $flags = vadmin_getvar('CONFIG', 'system_manager_proxy.account_change_flags');
   $sanitize = vadmin_getvar('CONFIG', 'system_manager_proxy.sanitize_flag_arguments');
   if (!empty($flags))
   {
      $username = vadmin_getvar('SQMAIL', 'username');
      $flags = str_replace(array('$1', '$2', '$3', '$4', '$u', '$p'), 
                           ($sanitize == 'yes' ?
                              array(escapeshellarg($user), 
                                    escapeshellarg($domain), 
                                    '10', 
                                    escapeshellarg(implode(',', $newForwards)), 
                                    escapeshellarg($username), 
                                    escapeshellarg(vadmin_get_current_pwd())) 
                            : array($user, 
                                    $domain, 
                                    10,
                                    implode(',', $newForwards), 
                                    $username, 
                                    vadmin_get_current_pwd())), 
                           $flags);

      $returnCode = vadmin_system_manager_proxy($domain, $domainPassword, $flags);
      if ($returnCode[0] != 0) 
      {
         $errorMessage = _("Backend proxy error");
         if (!empty($returnCode[1]))
         {
            spew("$me: Backend proxy error: " . $returnCode[1]);
            // nah, let's not do this so no security issues are 
            // exposed in an error message from the backend...
            // $errorMessage = sprintf(_("Backend proxy error: %s"), $returnCode[1]);
         }
      }
   }


   return array($returnCode[0], $errorMessage);

}



/**
  * Change account forwards
  *
  * @param string $domain The vadmin domain.
  * @param string $domainPassword The domain password.
  * @param string $user The account name having its forwards changed.
  * @param array $newforwards A list of the new forwards.
  *
  * @return array A two-element array: the first being
  *               a return code (where zero is non-error),
  *               the second being a message, which is usually
  *               only important if there was an error.
  *
  */
function vchforward($domain, $domainPassword, $user, $newforwards)
{

   if (empty($domain))         return array(1, _("Empty domain"));
   if (empty($domainPassword)) return array(1, _("Empty domain password"));

   $catchall_alias = vadmin_getvar('BACKEND', 'catchall_alias');
   if (empty($user) && !empty($catchall_alias)) return array(1, _("Empty username"));

   return vm_daemon_raw(array('chattr', $domain, $user,
                              $domainPassword, '10', $newforwards));

}



/**
  * Change account password
  *
  * @param string $domain The vadmin domain.
  * @param string $domainPassword The domain password.
  * @param string $user The account name having its password changed.
  * @param string $newpass The value of the new password.
  *
  * @return array A two-element array: the first being
  *               a return code (where zero is non-error), 
  *               the second being a message, which is usually
  *               only important if there was an error.
  *
  */
function vchpass($domain, $domainPassword, $user, $newpass)
{

   if (empty($domain))         return array(1, _("Empty domain"));
   if (empty($domainPassword)) return array(1, _("Empty domain password"));
   if (empty($user))           return array(1, _("Empty username"));
   if (empty($newpass))        return array(1, _("Empty new password"));

   return vm_daemon_raw(array('chattr', $domain, $user, 
                              $domainPassword, '1', $newpass));

}



/**
  * Change an account attribute
  *
  * @param string $domain The vadmin domain.
  * @param string $domainPassword The domain password.
  * @param string $user The account name having its password changed.
  * @param string $attr The name of the attribute to be changed.  This
  *                     parameter should always correspond to the 
  *                     contants:
  *                       PASS                 password
  *                       DEST                 mailbox
  *                       HARDQUOTA            hard quota
  *                       SOFTQUOTA            soft quota
  *                       MSGSIZE              message size limit
  *                       MSGCOUNT             message count limit
  *                       EXPIRY               account expiration date
  *                       MAILBOX_ENABLED      account active flag
  *                       PERSONAL             personal information
  *                       FORWARDS             account forwards (non-vmailmgr; 
  *                                                              added for SQL 
  *                                                              backend)
  *                       <or the name of a custom field if it is being
  *                        changed, with spaces changed to uderscores>
  *
  * @param string $value The new value of the attribute.
  *
  * @return array A two-element array: the first being
  *               a return code (where zero is non-error),
  *               the second being a message, which is usually
  *               only important if there was an error.
  *
  */
function vchattr($domain, $domainPassword, $user, $attr, $value)
{

   if (empty($domain))         return array(1, _("Empty domain"));
   if (empty($domainPassword)) return array(1, _("Empty domain password"));
   if (empty($attr))           return array(1, _("No attribute specified"));

   $catchall_alias = vadmin_getvar('BACKEND', 'catchall_alias');
   if (empty($user) && !empty($catchall_alias)) return array(1, _("Empty username"));

   $attributeList = array(
      'PASS' =>            '1',
      'DEST' =>            '2',
      'HARDQUOTA' =>       '3',
      'SOFTQUOTA' =>       '4',
      'MSGSIZE' =>         '5',
      'MSGCOUNT' =>        '6',
      'EXPIRY' =>          '7',
      'MAILBOX_ENABLED' => '8',
      'PERSONAL' =>        '9',
      'FORWARDS' =>        '10',  // non-vmailmgr; added for SQL implementation
   );

   if (isset($attributeList[$attr]))
      $attributeKey = $attributeList[$attr];
   else
      $attributeKey = $attr;

   return vm_daemon_raw(array('chattr', $domain, $user, $domainPassword,
                              $attributeKey, $value));

}



