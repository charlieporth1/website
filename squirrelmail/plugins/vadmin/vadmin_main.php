<?php
/**
 * vadmin_main.php
 * ----------------
 * Main wrapper for the vadmin interface. This file is accessed
 * by admins and takes care of all authorizations.
 *
 * Licensed under GNU GPL v2. Please see COPYING for full terms.
 *
 * $Id: vadmin_main.php,v 1.22 2009/03/17 07:48:19 pdontthink Exp $
 *
 * @author Konstantin Riabitsev ($Author: pdontthink $)
 * @version $Date: 2009/03/17 07:48:19 $
 */


/**
 * Load some necessary stuff provided by SquirrelMail.
 */
if (file_exists('../../include/init.php'))
   include_once('../../include/init.php');
else if (file_exists('../../include/validate.php'))
{
   define('SM_PATH', '../../');
   include_once(SM_PATH . 'include/validate.php');
} 

/**
 * Make sure plugin is activated!
 */
global $plugins;
if (!in_array('vadmin', $plugins))
   exit;

/**
 * Load the config parser 
 */
include_once(SM_PATH . 'plugins/vadmin/config_parser.php');

/**
 * Parse the main and backend config files.
 */
$config_path = vadmin_get_conf_location();
$config_file = 'vadmin.conf.php';

$vadmin_config = vadmin_parse_config($config_path . $config_file);
$GLOBALS{'VADMIN_CONFIG'} = $vadmin_config;

// validate backend
$vadmin_known_backends = vadmin_get_valid_backends();
if (!in_array($vadmin_config{'backend'}{'type'}, $vadmin_known_backends))
{
   echo 'FATAL ERROR: vadmin backend (' 
      . $vadmin_config{'backend'}{'type'} 
      . ') has not been configured.';
   exit;
}

$vadmin_backend_config = vadmin_parse_config($config_path 
                         . $vadmin_config{'backend'}{'type'} . '.conf.php');
$GLOBALS{'VADMIN_BACKEND_CONFIG'} = $vadmin_backend_config;

/**
 * Load main functions.
 */
$include_dir = $vadmin_config{'paths'}{'includes'};
include_once($include_dir . '/vadmin_functions.inc');
$me = 'vadmin_main.php';
spew("$me: initializing...");
spew("$me: QUERY_STRING is " . vadmin_getvar('SERVER', 'QUERY_STRING'));

/**
 * Change to Vadmin text domain
 */
sq_change_text_domain('vadmin');

/**
 * See what setting is storage.type set to and load the needed
 * file. This is the only place where error checking is implemented
 * since many different storage types are possible.
 */
$storage_type = vadmin_getvar('CONFIG', 'storage.type');
spew("$me: storage.type is '$storage_type'");
$storage_functions_file = sprintf('%s/%s_functions.inc', 
                                  $include_dir, $storage_type);
spew("$me: attempting to load '$storage_functions_file'");
if (file_exists($storage_functions_file) 
    && is_readable($storage_functions_file)){
    include_once($storage_functions_file);
} else {
    spew("$me: FATAL: could not load the storage functions file!");
    vadmin_system_error(sprintf(
        _("Could not load function file for storage type %s"), $storage_type));
}

/**
 * Load vmail.inc.
 */
$vmail_inc = vadmin_getvar('BACKEND', 'vmail_path');
include_once($vmail_inc);

/**
 * MOD and LVL params are always in GET.
 */
$MOD = vadmin_getvar('GET', 'MOD');
$LVL = vadmin_getvar('GET', 'LVL');

if ($LVL == false){
    /**
     * Hrmph. Someone got here with no LVL. Set it to "user" and assume
     * it's an honest mistake.
     */
    spew("$me: WARNING: got a request with no LVL! Setting to 'user'");
    $LVL = 'user';
}
vadmin_putvar('VADMIN', 'LVL', $LVL);

switch ($LVL){
 case 'admin':
     /**
      * Kill prefs from session
      */
     vadmin_putvar('SESSION', 'prefs', null);
     $AUTHCODE = vadmin_auth();
     spew("$me: AUTHCODE: '$AUTHCODE'");
     if ($AUTHCODE == false || $AUTHCODE == 'NONER'){
         /**
          * This, theoretically, shouldn't happen, as the "Admin" link is
          * not displayed to the mere mortals. The very fact that the
          * person got here with an 'admin' LVL should indicate that they
          * are an admin. However, if the $AUTHCODE is not set, or set to
          * "NONER", then they are trying to be curious. :)
          */
         vadmin_security_breach();
     }
     
     if ($AUTHCODE == 'LOWLY' || $AUTHCODE == 'CROSS' || $AUTHCODE == 'ELVIS'){
         /** 
          * This person hasn't logged in yet.
          * Check whether https is enforced and if so, tell them to
          * log in via https. This is only enforced for ELVISES and
          * CROSS(-Admins).
          */
         $force_https = vadmin_getvar('CONFIG', 'auth.force_https');
         if ((strtolower($force_https) == 'yes')
             && ($AUTHCODE == 'CROSS' || $AUTHCODE == 'ELVIS')){
             spew("$me: HTTPS enforced. Checking.");
             global $is_secure_connection;
             if (!$is_secure_connection) {
                 $MOD = 'login';
                 $ACT = 'needhttps';
                 spew("$me: You will need to switch to https, buddy.");
                 spew("$me: Setting MOD and ACT appropriately'");
             }
         }
         
         /**
          * The default module is "login", since that should be the very
          * thing a person clicking on the "Admin" link should see.
          */
         if ($MOD == false){
             spew("$me: MOD is not set, setting it to 'login'");
             spew("$me: setting ACT to 'main'");
             $MOD = 'login';
             $ACT = 'main';
         } else {
             /**
              * If a person that is not yet logged in is trying to get to
              * a module that is not starting with "login_", that means they
              * are trying to pass $MOD as part of a QUERY_STRING, which 
              * in turn means that they are trying to do something nasty.
              * Vote them off the island.
              */
             if ($MOD != 'login'){
                 spew("$me: This person is not logged in, yet tries to...");
                 spew("$me: ...set MOD to something other than 'login'");
                 vadmin_security_breach();
             }
         }
     } else {
         /**
          * The person is already logged in.
          * Get and decrypt the domain they are authorized to admin
          * during this session. trim() is because for some reason the 
          * string terminates with a "null". I almost smashed my laptop 
          * before I found this one out. Don't know why it is there --
          * most likely a MCRYPT-related bug...
          */
         $VADMIN_DOMAIN = trim(vadmin_crypto($AUTHCODE, 'decrypt'));
         spew("$me: VADMIN_DOMAIN is '$VADMIN_DOMAIN'");
         vadmin_putvar('VADMIN', 'VADMIN_DOMAIN', $VADMIN_DOMAIN);
         
         /**
          * If $MOD is not set and the person is fully logged in, then
          * direct them to the main menu.
          */
         if ($MOD == false){
             list($start_mod, $start_act) = vadmin_get_startup_location();
             $MOD = $start_mod;
             $ACT = $start_act;
             spew("$me: redirecting to main menu");
         }
     }
     break;
 case 'user':
     $ACT = vadmin_getvar('POST', 'ACT');
     if (!$ACT) $ACT = vadmin_getvar('GET', 'ACT');
     if (!$MOD && !$ACT){
         vadmin_security_breach();
     }
     break;
}

/** 
 * See if someone is attempting to be nasty by trying to get out of the
 * modules directory, although it probably wouldn't do them any good,
 * since every module has to end with .mod. Still, they deserve
 * to be warned. ;)
 */
if (preg_match('/\W/s', $LVL) || preg_match('/\W/s', $MOD)){
    spew("$me: odd symbols found in the passed parameters!");
    vadmin_security_breach();
}

/**
 * So, I've been asked -- why am I writing my php software in modules?
 * Why don't I just stick it all on one page and do a switch? Well, there
 * are several reasons for that. One -- I think it's cleaner that way. 
 * Two, I believe it's easier to find what you are looking for.
 * But most importantly -- this is faster, since each time PHP only has
 * to parse through a very small amount of code. If it all was in one file,
 * PHP would have to parse 20kb worth of code just to execute 500 bytes
 * worth of it each time a page loads.
 */

$mod_dir = vadmin_getvar('CONFIG', 'paths.modules');

// Allow mod_dir override by other plugins...
// We could ask for the dir as a GET parameter, 
// but don't want to open this up to get hacked, 
// so use a hook instead.  Plugins hooking in here
// should return a string containing a comma-
// separated list of any module directories they 
// want to add.
//
$null = NULL;
$other_mod_dirs = concat_hook_function('vadmin_module_directories', $null);
if (!empty($other_mod_dirs)) {
    $other_mod_dirs = explode(',', $other_mod_dirs);
    @array_walk($other_mod_dirs, 'vadmin_trim_array');
} else {
    $other_mod_dirs = array();
}

$module = sprintf('%s/%s/%s.mod', $mod_dir, $LVL, $MOD);
if (!isset($ACT) || $ACT == false){
    $ACT = vadmin_getvar('GET', 'ACT');
    spew("$me: got ACT from GET: '$ACT'");
}
vadmin_putvar('VADMIN', 'ACT', $ACT);

/**
 * Check if this module exists first in vadmin's own modules 
 * directory.  If it does not, check secondary module directories
 * and otherwise throw an error.
 */
if (!file_exists($module)){
    $found_module = FALSE;
    foreach ($other_mod_dirs as $other_mod_dir){
        if (empty($other_mod_dir)) continue;
        $module = sprintf('%s/%s/%s.mod', $other_mod_dir, $LVL, $MOD);
        if (file_exists($module)){
            $found_module = TRUE;
            break;
        }
    }
    if (!$found_module){
        $message = sprintf(_("No such module '%s' for level '%s'"), $MOD, $LVL);
        spew("$me: $message");
        vadmin_system_error($message);
    }
}

/**
 * Load the necessary module. Include_once so squirrelmail doesn't
 * automatically "fix" it.
 */
spew("$me: invoking module '$module'");
$oldme = $me;
include_once($module);
$me = $oldme;
spew("$me: processing complete. Exiting.");
