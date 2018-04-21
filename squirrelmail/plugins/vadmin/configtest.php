<?php
/**
 * configtest.php
 * --------------
 * Configuration test checks for the vadmin plugin. This file 
 * is accessed by running SquirrelMail's configuration test
 * script, found at src/configtest.php
 *
 * Licensed under GNU GPL v2. Please see COPYING for full terms.
 *
 * $Id: configtest.php,v 1.5 2008/12/19 20:03:33 pdontthink Exp $
 *
 * @author Paul Lesniewski ($Author: pdontthink $)
 * @version $Date: 2008/12/19 20:03:33 $
 */


/**
 * This function checks that Vadmin is correctly installed
 * and configured and that the current server environment
 * is also suited for the correct functioning of Vadmin.
 *
 * @return boolean TRUE if there were any configuration or
 *                 other such errors, FALSE if there are
 *                 no problems and the plugin is ready to
 *                 rock.
 */
function vadmin_check_configuration_do()
{

   include_once(SM_PATH . 'plugins/vadmin/config_parser.php');


   // check PHP version
   //
   if(!check_php_version(4,1,0)) 
   {
      do_err('Vadmin plugin requires PHP 4.1.0 or above', FALSE);
      return TRUE;
   }


   // make sure Apache environment variables are as we expect
   //
   if (!sqGetGlobalVar('CRYPTO_HASH_LINE', $CRYPTO_HASH_LINE, SQ_SERVER))
   {
      do_err('Vadmin plugin\'s apache.conf is misconfigured: CRYPTO_HASH_LINE is not found', FALSE);
      return TRUE;
   }
   if (!sqGetGlobalVar('MCRYPT_ALGO', $MCRYPT_ALGO, SQ_SERVER))
   {
      do_err('Vadmin plugin\'s apache.conf is misconfigured: MCRYPT_ALGO is not found', FALSE);
      return TRUE;
   }


   // if not using our built-in encryption routine, test that
   // mycrypt is there and the chosen algorithm is available
   //
   if ($MCRYPT_ALGO != 'rc4_builtin' && !function_exists('mcrypt_generic'))
   {
      do_err('Vadmin plugin is misconfigured: MCRYPT_ALGO specifies an mcrypt algorithm, but mcrypt support is not present in PHP', FALSE);
      return TRUE;
   }


   // The default hashline is not acceptable
   //
   if ($CRYPTO_HASH_LINE == 'LLAMA')
   {
      do_err('Vadmin plugin\'s apache.conf is misconfigured: CRYPTO_HASH_LINE has not been changed from its default value', FALSE);
      return TRUE;
   }


   // check PHP extensions
   //
   $required_php_extensions = array('dba');
   $diff = array_diff($required_php_extensions, get_loaded_extensions());
   if(count($diff)) 
   {
      do_err('Vadmin plugin requires PHP extension "dba"', FALSE);
      return TRUE;
   }


   // make sure main configuration file is present and readable
   //
   $config_path = vadmin_get_conf_location();
   $config_file = 'vadmin.conf.php';
   if (!file_exists($config_path . $config_file))
   {
      do_err('Vadmin plugin main configuration file ' . $config_file . ' is missing', FALSE);
      return TRUE;
   }
   if (!is_readable($config_path . $config_file))
   {
      do_err('Vadmin plugin main configuration file ' . $config_file . ' is not readable', FALSE);
      return TRUE;
   }


   // load main config settings so we can determine which backend to test for
   //
   $vadmin_config = vadmin_parse_config($config_path . $config_file);
   $GLOBALS{'VADMIN_CONFIG'} = $vadmin_config;



   // validate backend
   //
   $vadmin_known_backends = vadmin_get_valid_backends();
   if (!in_array($vadmin_config{'backend'}{'type'}, $vadmin_known_backends))
   {
      do_err('Vadmin plugin configuration is incomplete: unknown backend "' . $vadmin_config{'backend'}{'type'} . '"', FALSE);
      return TRUE;
   }


   // make sure backend configuration file is present and readable
   //
   $backend_config_file = $vadmin_config{'backend'}{'type'} . '.conf.php';
   if (!file_exists($config_path . $backend_config_file))
   {
      do_err('Vadmin plugin backend configuration file ' . $backend_config_file . ' is missing', FALSE);
      return TRUE;
   }
   if (!is_readable($config_path . $backend_config_file))
   {
      do_err('Vadmin plugin backend configuration file ' . $backend_config_file . ' is not readable', FALSE);
      return TRUE;
   }


   // load backend config settings, in case we have anything else to check...
   //
   // NOTE that the include below will exit with its own error message if the
   // backend is not set up correctly; although it's not in the style of the
   // main SM configtest (and we could hack something into the vadmin system
   // error function to change that), for now, this is really just fine
   //
   include_once(SM_PATH . 'plugins/vadmin/includes/vadmin_functions.inc');
   $vadmin_backend_config = vadmin_parse_config($config_path . $backend_config_file);
   $GLOBALS{'VADMIN_BACKEND_CONFIG'} = $vadmin_backend_config;


   // make sure PHP dba module has the needed handler for the chosen flavor
   // (can't do this until now because vadmin_getvar() isn't available until
   // vadmin_functions.inc has been included)
   //
   $flavor = vadmin_getvar('CONFIG', 'storage.flavor');
   if (!in_array($flavor, dba_handlers()))
   {
      do_err('Vadmin plugin requires dba handler "' . $flavor . '"', FALSE);
      return TRUE;
   }


   // validations for sql backend...
   //
   if ($vadmin_config{'backend'}{'type'} == 'sql')
   {

      // mask include errors if not in debug mode
      //
      $db_file = 'DB.php';
      $debug = vadmin_debug_level();
      if ($debug > 1)
         $if_statement = 'return !include_once(\'' . $db_file . '\');';
      else
         $if_statement = 'return !@include_once(\'' . $db_file . '\');';


      // validate that Pear DB is there...
      //
      if (eval($if_statement))
      {
         do_err('Vadmin plugin sql backend requires Pear DB package, but it does not appear to be installed.  Increase Vadmin debug level to at least 2 (and enable PHP error logging and/or display) to see the PHP error associated with this issue.', FALSE);
         return TRUE;
      }


      // make sure the DSN is correct
      //
      $dsn = vadmin_getvar('BACKEND', 'db_dsn');
      $vadmin_db_connection = DB::connect($dsn);
      if (DB::isError($vadmin_db_connection))
      {
         do_err('Vadmin plugin sql backend database DSN appears to be incorrect.  Unable to connect to database', FALSE);
         return TRUE;
      }
      $vadmin_db_connection->disconnect();

   }


   // check if Cracklib is configured and if so, if Cracklib
   // support is available and the given dictionary is valid
   //
   $cracklib_dict = vadmin_getvar('CONFIG', 'password.cracklib_dict');
   if (!empty($cracklib_dict))
   {

      // Cracklib support built in?
      //
      if (!function_exists('crack_opendict') || !function_exists('crack_check'))
      {
         do_err('Vadmin plugin is configured to use Cracklib password strength testing, but Cracklib support does not appear to be built into this PHP build.  Test your PHP settings and see http://www.php.net/manual/ref.crack.php', FALSE);
         return TRUE;
      }

      // Valid dictionary?
      //
      if (!crack_opendict($cracklib_dict))
      {
         do_err('Vadmin plugin is configured to use Cracklib password strength testing, but the specified dictionary could not be opened.  See the password.cracklib_dict setting in the main Vadmin configuration file', FALSE);
         return TRUE;
      }

   }


   // only need to do this pre-1.5.2, as 1.5.2 will make this
   // check for us automatically
   //
   if (!check_sm_version(1, 5, 2))
   {

      // try to find Compatibility, and then that it is v2.0.14+
      //
      if (function_exists('check_plugin_version')
       && check_plugin_version('compatibility', 2, 0, 14, TRUE))
         return FALSE;


      // something went wrong
      //
      do_err('Vadmin plugin requires the Compatibility plugin version 2.0.14+', FALSE);
      return TRUE;

   }


   return FALSE;

}


