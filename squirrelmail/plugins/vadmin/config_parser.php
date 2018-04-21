<?php
/**
 * config_parser.php
 * ------------------
 * This is a file containing a simple config parser function. It MUST
 * be in the same directory as all other main vadmin files, as it is
 * required to be able to parse the configuration and find where
 * everything else is located.
 *
 * Licensed under GNU GPL v2. Please see COPYING for full terms.
 *
 * $Id: config_parser.php,v 1.9 2008/01/31 20:43:36 pdontthink Exp $
 *
 * @author  Konstantin Riabitsev ($Author: pdontthink $)
 * @version $Date: 2008/01/31 20:43:36 $
 */

/**
 * Returns the path to the Vadmin configuration directory
 *
 * @return string Path to the Vadmin configuration directory
 */
function vadmin_get_conf_location()
{


// =======================================================================
   /**
    * Change this value if you have moved the vadmin/conf
    * directory elsewhere.  For example,
    * $configuration_path = '/etc/vadmin/';
    * 
    * If you left it in the vadmin plugin directory, just
    * leave this setting BLANK.
    */
   $configuration_path = '';
// =======================================================================


   // change variable name so perl regexp doesn't munge code below
   //
   $conf_path = $configuration_path;

   if (empty($conf_path))
      if (defined('SM_PATH'))
         $conf_path = SM_PATH . 'plugins/vadmin/conf/';
      else
         if (is_dir('../plugins/vadmin/conf/'))
            $conf_path = '../plugins/vadmin/conf/';
         else if (is_dir('../../plugins/vadmin/conf/'))
            $conf_path = '../../plugins/vadmin/conf/';
   if ($conf_path{strlen($conf_path) - 1} !== '/')
      $conf_path .= '/';
   return $conf_path;

}

/**
 * Indicates whether or not Vadmin should use cached configuration
 * variables.  This should ALWAYS be TRUE unless you are debugging
 * your system and need to make on-the-fly configuration changes
 * without needing to log out and back in to SquirrelMail to see
 * if they work.
 *
 * I originally was going to just make this a simple global variable,
 * but chose not to so no one else can try to exploit it.
 *
 * @return boolean TRUE if Vadmin should use its cached configuration
 *                 values; FALSE otherwise.
 */
function use_cached_vadmin_vars()
{


// =======================================================================
   return TRUE;
// =======================================================================


}

/**
 * Returns a list of the known supported Vadmin backends
 *
 * Should rather define this in a better place(??), but 
 * for now this is good enough and used in just one or 
 * two places
 *
 * @return array An array of strings, each string being 
 *               the exact name of one of the supported
 *               backends.
 */
function vadmin_get_valid_backends()
{

// =======================================================================
   $vadmin_known_backends = array(
                                  'vmailmgr',
                                  'sql',
                                 );
// =======================================================================

   return $vadmin_known_backends;

}

/**
 * This function is a simple win.ini-style config parser.
 *
 * @param  $config  a config string in a win.ini style config file.
 * @return          an array with various array members reflecting
 *                  the config file.
 */
function vadmin_parse_config_file($config){
    $config = preg_replace("/[#;].*/m", '', $config);
    $config = preg_replace("/^<\?php.*/m", '', $config);
    $config = preg_replace("/\n+/s", "\n", $config);
    $config = preg_replace("/\r/s", '', $config);
    $config = preg_replace("/\\\\s*\n/s", '', $config);
    $config = preg_replace("/^\s*/m", '', $config);
    $lines = explode("\n", $config);
    array_pop($lines);
    $config_ary = array();
    foreach ($lines as $confline){
        if (preg_match("/^\[/", $confline)){
            $blockname = preg_replace("/[\[\]]/s", '', $confline);
        } else {
            $bunch = explode('=', $confline);
            $varname = rtrim(array_shift($bunch));
            if (sizeof($bunch) > 0){
                if (sizeof($bunch) > 1){
                    $varvalue = ltrim(join("=", $bunch));
                } else {
                    $varvalue = ltrim($bunch[0]);
                }
            } else {
                $varvalue = true;
            }
            if (isset($blockname)){
                $config_ary{$blockname}{$varname} = $varvalue;
            } else {
                $config_ary{$varname} = $varvalue;
            }
        }
    }
    return $config_ary;
}

/**
 * This function reads in the vadmin config file.
 * 
 * @param  $filename  the file to read in.
 * @return            string with the contents of the file.
 */
function vadmin_load_config_file($filename){
    if (!file_exists($filename)){
        echo "FATAL ERROR: vadmin config file '$filename' does not exist.";
        exit;
    }
    $fd = fopen($filename, "r");
    if ($fd == false){
        echo "FATAL ERROR: vadmin config file '$filename' could not be read.";
        exit;
    }
    $contents = fread($fd, filesize($filename));
    fclose($fd);
    return $contents;
}

/**
 * This function loads the configuration. First it checks the session
 * to see if we have already parsed the config file before. If so, 
 * it then returns it. If not, then it loads the file and parses it,
 * then puts it into the session so we don't have to do it again for this
 * user.
 *
 * @param  $filename  where vadmin config file is.
 * @return            an array with values from that config file.
 */
function vadmin_parse_config($filename){

    if (use_cached_vadmin_vars() && isset($_SESSION{'VADMIN_CONFIG'}{$filename})){
        $VADMIN_CONFIG = $_SESSION{'VADMIN_CONFIG'}{$filename};
    } else {
        /**
         * No, it's not in session. Load it from file.
         */
        $config = vadmin_load_config_file($filename);
        $VADMIN_CONFIG = vadmin_parse_config_file($config);

        /**
         * Stick it into session.
         */
        $_SESSION{'VADMIN_CONFIG'}{$filename} = $VADMIN_CONFIG;
    }
    return $VADMIN_CONFIG;
}


