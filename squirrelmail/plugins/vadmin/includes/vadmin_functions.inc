<?php
/**
 * vadmin_functions.inc
 * ---------------------
 * All Vadmin-wide functions are in this file.
 * 
 * Licensed under GNU GPL v2. Please see COPYING for full terms.
 *
 * $Id: vadmin_functions.inc,v 1.89 2009/09/05 02:45:10 pdontthink Exp $
 *
 * @author Konstantin Riabitsev ($Author: pdontthink $)
 * @version $Date: 2009/09/05 02:45:10 $
 * 
 */

/**
 * Load the functions needed for the chosen backend.
 */
$me = 'vadmin_functions.inc';
$vadmin_backend = vadmin_getvar('CONFIG', 'backend.type');
$backend_file = vadmin_getvar('CONFIG', 'paths.includes') 
              . '/' . $vadmin_backend . '/' . $vadmin_backend . '_functions.inc';
spew("$me: Loading backend functions from $backend_file");
if (!file_exists($backend_file) || !is_readable($backend_file)){
    spew("$me: Could not find $backend_file!");
    $error = sprintf(_("Could not find backend %s!"), $vadmin_backend);
    vadmin_system_error($error);
}
include_once($backend_file);


/**
  * What debug level are we at?
  *
  * @return int The current debug level
  *
  */
function vadmin_debug_level() {
    static $debug_level;
    if (!isset($debug_level)){
        $debug_level = vadmin_getvar('CONFIG', 'debug.level');
    }
    return $debug_level;
}


/**
 * This is a debugging function.
 *
 * @param  $message  The message to put into the debug log.
 * @return           void
 */
function spew($message){
    static $debug_file;
    if (!isset($debug_file)){
        $debug_file  = vadmin_getvar('CONFIG', 'debug.output');
        $username    = vadmin_getvar('SQMAIL', 'username');
        $debug_file .= '.' . $username;
    }
    $debug_level = vadmin_debug_level();
    if ($debug_level > 0){
        $nowdate = date('H:i:s');
        error_log("$nowdate> $message\n", 3, $debug_file);
    }
}

/**
 * This function sets a cookie
 *
 * @param string  $sName     The name of the cookie.
 * @param string  $sValue    The value of the cookie.
 * @param int     $iExpire   The time the cookie expires. This is a 
 *                           Unix timestamp so is in number of seconds 
 *                           since the epoch.
 * @param string  $sPath     The path on the server in which the cookie 
 *                           will be available on.
 * @param string  $sDomain   The domain that the cookie is available.
 * @param boolean $bSecure   Indicates that the cookie should only be 
 *                           transmitted over a secure HTTPS connection.
 * @param boolean $bHttpOnly Disallow JS to access the cookie (IE6 only)
 *
 * @return void
 */
function vadmin_setcookie($sName, $sValue='deleted', $iExpire=0, $sPath='', 
                          $sDomain='', $bSecure=FALSE, $bHttpOnly=TRUE)
{
   if (function_exists('sqsetcookie'))
      sqsetcookie($sName, $sValue, $iExpire, $sPath, $sDomain, $bSecure, $bHttpOnly);
   else if (check_php_version(5, 2))
      setcookie($sName, $sValue, $iExpire, $sPath, $sDomain, $bSecure, $bHttpOnly);
   else 
      setcookie($sName, $sValue, $iExpire, $sPath, $sDomain, $bSecure);

}

/**
 * This function retrieves various php-provided variables in a sane
 * and safe way, offering a path of migration from php 4.0.x to 4.1 and
 * beyond, as well as allowing us to work with register_globals=off.
 *
 * @param  $vartype  The type of the variable to retireve.
 * @param  $varname  The name of the variable to retrieve.
 * @return           The value of the variable.
 */
function vadmin_getvar($vartype, $varname){
    $varvalue = false;
    switch ($vartype){
//FIXME: use SM superglobal code sqGetGlobalVar() and sqsession_register()
    case 'SERVER':
        if (isset($_SERVER{$varname})){
            $varvalue = $_SERVER{$varname};
        }
        break;
    case 'COOKIE':
        if (isset($_COOKIE{$varname})){
            $varvalue = $_COOKIE{$varname};
            $varvalue = vadmin_htmlspecialchars($varvalue);
        }
        break;
    case 'ENV':
        if (isset($_ENV{$varname})){
            $varvalue = $_ENV{$varname};
        }
        break;
    case 'POST':
        if (isset($_POST{$varname})){
            $varvalue = $_POST{$varname};
            $varvalue = vadmin_htmlspecialchars($varvalue);
        }
        break;
    case 'GET':
        if (isset($_GET{$varname})){
            $varvalue = $_GET{$varname};
            $varvalue = vadmin_htmlspecialchars($varvalue);
        }
        break;
    case 'FILES':
        if (isset($_FILES{$varname})){
            $varvalue = $_FILES{$varname};
        }
        break;
    case 'SESSION':
        if (isset($_SESSION{'VADMIN_SESSION_VARS'}) 
            && isset($_SERVER{'HTTP_HOST'})){
            $VADMIN_SESSION_VARS = $_SESSION{'VADMIN_SESSION_VARS'};
            $domain = $_SERVER{'HTTP_HOST'};
            if (isset($VADMIN_SESSION_VARS{$domain}{$varname})){
                $varvalue = $VADMIN_SESSION_VARS{$domain}{$varname};
            }
        }
        break;
    case 'SQMAIL_SESSION':
        if (isset($_SESSION{$varname})){
            $varvalue = $_SESSION{$varname};
        }
        break;
    case 'SQMAIL':
        if (isset($GLOBALS{$varname})){
            $varvalue = $GLOBALS{$varname};
        }
        break;
    case 'CONFIG':
        $parent_child = explode('.', $varname);
        if (isset($parent_child[1])){
            if (isset($GLOBALS{'VADMIN_CONFIG'}{$parent_child[0]}{$parent_child[1]})){
                $varvalue = $GLOBALS{'VADMIN_CONFIG'}{$parent_child[0]}{$parent_child[1]};
            }
        } else {
            if (isset($GLOBALS{'VADMIN_CONFIG'}{$parent_child[0]})){
                $varvalue = $GLOBALS{'VADMIN_CONFIG'}{$parent_child[0]};
            }
        }
        break;
    case 'BACKEND':
        $parent_child = explode('.', $varname);
        if (isset($parent_child[1])){
            if (isset($GLOBALS{'VADMIN_BACKEND_CONFIG'}{$parent_child[0]}{$parent_child[1]})){
                $varvalue = $GLOBALS{'VADMIN_BACKEND_CONFIG'}{$parent_child[0]}{$parent_child[1]};
            }
        } else {
            $backend_name = vadmin_getvar('CONFIG', 'backend.type');

            if (isset($GLOBALS{'VADMIN_BACKEND_CONFIG'}{$parent_child[0]})){
                $varvalue = $GLOBALS{'VADMIN_BACKEND_CONFIG'}{$parent_child[0]};

            // try prepending backend name as the actual "parent"
            } else if (isset($GLOBALS{'VADMIN_BACKEND_CONFIG'}{$backend_name}{$parent_child[0]})){
                $varvalue = $GLOBALS{'VADMIN_BACKEND_CONFIG'}{$backend_name}{$parent_child[0]};

            // for fun try to find setting in main config under backend parent
            } else if (isset($GLOBALS{'VADMIN_CONFIG'}{$backend_name}{$parent_child[0]})){
                $varvalue = $GLOBALS{'VADMIN_CONFIG'}{$backend_name}{$parent_child[0]};
            }
        }
        break;
    case 'VADMIN':
        if (isset($GLOBALS{'VADMIN_VARS'}{$varname})){
            $varvalue = $GLOBALS{'VADMIN_VARS'}{$varname};
        }
        break;
    }
    return $varvalue;
}

/**
 * This function sets certain variables that should be handled safely.
 *
 * @param  $vartype  The type of the variable to set.
 * @param  $varname  The name of the variable.
 * @param  $varvalue The value of the variable.
 * @return           void
 */
function vadmin_putvar($vartype, $varname, $varvalue){
    $me = 'vadmin_putvar';
    spew("$me: Received putvar request of type '$vartype'");
    spew("$me: varname: '$varname', varvalue: '$varvalue'");
    switch($vartype){
//FIXME: use SM superglobal code sqGetGlobalVar() and sqsession_register()
    case 'SESSION':
        $domain = $_SERVER{'HTTP_HOST'};
        if (isset($_SESSION{'VADMIN_SESSION_VARS'})){
            $VADMIN_SESSION_VARS = $_SESSION{'VADMIN_SESSION_VARS'};
        }
        $VADMIN_SESSION_VARS{$domain}{$varname} = $varvalue;
        $_SESSION{'VADMIN_SESSION_VARS'} = $VADMIN_SESSION_VARS;
        break;
    case 'SQMAIL':
        $GLOBALS{$varname} = $varvalue;
        break;
    case 'VADMIN':
        $GLOBALS{'VADMIN_VARS'}{$varname} = $varvalue;
        break;
    }
}

/**
  * Replacement for htmlspecialchars that works on arrays too
  *
  * @param mixed $value The variable to sanitize 
  *
  * @return mixed The sanitized version of the given $value
  *
  */
function vadmin_htmlspecialchars($value) {
    if (is_array($value)) {
        $return_value = array();
        foreach($value as $k => $v)
            $return_value[vadmin_htmlspecialchars($k)] = vadmin_htmlspecialchars($v);
        return $return_value;
    }
    return htmlspecialchars($value);
}

/**
  * Generate and return a random user password
  *
  * @param boolean $use_all_valid_chars When TRUE, the characters 
  *                                     used will come from the set 
  *                                     of valid password characters 
  *                                     defined in vadmin.conf.php for 
  *                                     the valid_characters setting 
  *                                     under the [password] section.  
  *                                     When FALSE, only the characters 
  *                                     A-Z, a-z, and 0-9 will be used 
  *                                     (OPTIONAL; default is FALSE).
  * @param int     $length  The desired password length (OPTIONAL;
  *                         default is zero, in which case the value
  *                         from the minimum_length setting under the
  *                         [password] section in vadmin.conf.php is
  *                         used for the generated password length.
  * @param boolean $use_human_readable_chars When $use_all_valid_chars is 
  *                                          set to FALSE and this is TRUE, 
  *                                          the characters used will be
  *                                          a subset of A-Z, a-z and 0-9,
  *                                          without easily confused characters
  *                                          such as O and 0, l and 1, etc.
  *                                          (OPTIONAL; default is TRUE)
  * @param string  $valid_chars  When $use_all_valid_chars and 
  *                              $use_human_readable_chars are both set to
  *                              FALSE, the actual characters to be used may 
  *                              be specified here as a string, instead 
  *                              (OPTIONAL; default is empty in which case, 
  *                              as stated above, A-Z, a-z, and 0-9 will be used).
  *
  * @return string The new random password
  *
  */
function vadmin_generate_random_password($use_all_valid_chars=FALSE,
                                         $length=0, $use_human_readable_chars=TRUE,
                                         $valid_chars='')
{

//FIXME: should this be maximum_length instead?
   if (empty($length))
      $length = vadmin_getvar('CONFIG', 'password.minimum_length');

   $valid_characters = '';
   if ($use_all_valid_chars)
      $valid_characters = vadmin_getvar('CONFIG', 'password.valid_characters');
   else if ($use_human_readable_chars)
      $valid_characters = '23456789abcdefghjkmnpqrstuvwxyzABCDEFGHIJKLMNPQRSTUVWXYZ';
   else if (!empty($valid_chars))
      $valid_characters = $valid_chars;

   if (empty($valid_characters)) $flags = 7;
   else $flags = 0;

   $password = GenerateRandomString($length, $valid_characters, $flags);

   // if passwords are expected to be alpha-numeric make sure they
   // are so (although this has the potential to add characters
   // that were not in $valid_chars (if it was used), in which case
   // we'll then NOT make this safety check (it also could do the same
   // thing if the Vadmin config file has a stripped down list of valid
   // password characters when using $use_all_valid_chars, but we'll
   // assume no one is going to do that))
   //
   if (empty($valid_chars)) {
      $alpha_numeric = vadmin_getvar('CONFIG', 'password.alpha_numeric');
      if ($alpha_numeric == 'yes'
       && !preg_match('/([0-9]+.*[a-zA-Z]+)|([a-zA-Z]+.*[0-9]+)/', $password)) {

         // generate two random positions in the password,
         // one for a random A-Z value and one for a 2-9 value
         //
         $first_pos = mt_rand(0, $length - 1);
         $second_pos = $first_pos;
         while ($first_pos == $second_pos) $second_pos = mt_rand(0, $length - 1);
         $alpha = 'abcdefghjkmnpqrstuvwxyzABCDEFGHIJKLMNPQRSTUVWXYZ';
         $digits = '23456789';
         $password[$first_pos] = $alpha{mt_rand(0, strlen($alpha) - 1)};
         $password[$second_pos] = $digits{mt_rand(0, strlen($digits) - 1)};
      }
   }

   return $password;

}

/**
  * Validate mail forwarding address
  *
  * Checks if a desired mail forwarding address is restricted or not.
  * See the [forwarding] settings in the main Vadmin configuration file.
  *
  * @param string  $address        The desired forwarding address
  * @param string  $dom            The current domain
  * @param string  $user_name      The user account for whose account
  *                                the check is being made.
  * @param boolean $admin_override When TRUE, ignore some
  *                                checks because a superuser
  *                                is overriding them (some
  *                                checks are still applicable,
  *                                such as the valid email format
  *                                check) (OPTIONAL; default FALSE)
  *
  * @return array First entry will be zero when the address
  *               passes all tests; otherwise it will be one
  *               of the following error numbers, and the
  *               second array entry will be an error message.
  *                 1 = Invalid email format
  *                 2 = Address is in restriction list
  *
  */
function validate_restricted_user_mail_forwarding($address, $dom, $user_name, $admin_override=FALSE)
{

   $me = 'validate_restricted_user_mail_forwarding';
   spew("$me: inspecting address \"$address\" for problems"
      . ($admin_override ? '(with elvis override).' : '.'));

   global $Email_RegExp_Match;
   include_once(SM_PATH . 'functions/url_parser.php');

   static $restriction_list, $local_addresses_ok, $account_only_regexp;
   if (empty($restriction_list))
   {
      $restriction_list = vadmin_getvar('CONFIG', 'forwarding.restriction_list');
      $restriction_list = explode(',', $restriction_list);
      @array_walk($restriction_list, 'vadmin_trim_array');
   }
   if (empty($local_addresses_ok))
   {
      $local_addresses_ok = vadmin_getvar('CONFIG', 'forwarding.local_addresses_ok');
   }
   if (empty($account_only_regexp))
   {
      $account_only_regexp = substr($Email_RegExp_Match, 0, strpos($Email_RegExp_Match, '@'));
   }


   // validate email format
   //
   if (!eregi('^' . $Email_RegExp_Match . '$', $address)
    && ($local_addresses_ok != 'yes' || !eregi('^' . $account_only_regexp . '$', $address)))
   {
      spew("$me: address \"$address\" isn't in valid email address format");
      return array(1, _("Address \"$address\" is not a valid email address"));
   }


   // check restriction list
   //
   if (!$admin_override && !empty($restriction_list))
   {
      foreach($restriction_list as $restricted_addr)
      {
         if (preg_match('/^' . str_replace(array('?', '*'), array('\w{1}', '.*?'),
                                           $restricted_addr) . '$/i', $address))
         {
            spew("$me: address \"$address\" was found in restriction list (\"$restricted_addr\")");
            return array(2, _("Address \"$address\" is restricted"));
         }
      }
   }


   // address looks good!
   //
   return array(0);

}

/**
  * Validate password
  *
  * @param string  $password       The password to validate
  * @param string  $dom            The current domain 
  * @param string  $user_name      When non-NULL, this is to
  *                                be used to compare that the
  *                                password is not the same as
  *                                the username (OPTIONAL; 
  *                                default NULL)
  * @param boolean $admin_override When TRUE, ignore some 
  *                                checks because a superuser 
  *                                is overriding them (some 
  *                                checks are still applicable, 
  *                                such as the valid characters 
  *                                check) (OPTIONAL; default FALSE)
  *
  * @return array First entry will be zero when the password
  *               passes all tests; otherwise it will be one
  *               of the following error numbers, and the 
  *               second array entry will be an error message.
  *                 1 = Invalid characters found
  *                 2 = Password is the same as the username
  *                 3 = Password is too short or too long
  *                 4 = Password does not contain both numerals
  *                     and alphabetic characters (0-9, a-z)
  *                 5 = Password failed Cracklib test
  *
  */
function vadmin_validate_password($password, $dom, $user_name=NULL, $admin_override=FALSE)
{

   $me = 'vadmin_validate_password';
   spew("$me: inspecting password \"$password\" for problems" 
      . ($admin_override ? '(with elvis override).' : '.'));

   static $password_chars;
   if (empty($password_chars))
      $password_chars = vadmin_getvar('CONFIG', 'password.valid_characters');


   // validate password characters
   //
   if (!empty($password_chars)
    && !vadmin_verify_character_input($password, $password_chars)) {
       spew("$me: password contains illegal characters");
       return array(1, sprintf(_("Password may only contain these characters: %s."), $password_chars));
   }


   // make sure not the same as username
   //
   if (!$admin_override && !is_null($user_name)) {

//FIXME: should we be using some delim value instead of hard-coded "@"?
       if (strpos($user_name, '@') === FALSE)
           $user_test = $user_name;
       else
//FIXME: should we be using some delim value instead of hard-coded "@"?
           $user_test = substr($user_name, 0, strpos($user_name, '@'));

       static $differ_from_username;
       if (empty($differ_from_username))
           $differ_from_username = vadmin_getvar('CONFIG', 'password.differ_from_username');

       if ($differ_from_username == 'yes'
        && (strtolower($user_test) == strtolower($password)
//FIXME: should we be using some delim value instead of hard-coded "@"?
         || strtolower("$user_test@$dom") == strtolower($password))) {
           spew("$me: password is the same as the username");
           return array(2, _("Password cannot be the same as the username."));
       }
   }


   // verify password length
   //
   if (!$admin_override) {

      static $minlen, $maxlen;
      if (empty($minlen))
          $minlen = vadmin_getvar('CONFIG', 'password.minimum_length');
      if (empty($maxlen))
          $maxlen = vadmin_getvar('CONFIG', 'password.maximum_length');

      $len = strlen($password);
      if ($minlen > 0 && $maxlen > 0 && ($len < $minlen || $len > $maxlen)) {
          spew("$me: vpassword is not an allowable length");
          return array(3, sprintf(_("Password must be between %d and %d characters in length."), $minlen, $maxlen));
      }
   }


   // verify alpha numeric
   //
   if (!$admin_override) {

       static $alpha_numeric;
       if (empty($alpha_numeric))
           $alpha_numeric = vadmin_getvar('CONFIG', 'password.alpha_numeric');

       if ($alpha_numeric == 'yes'
        && !preg_match('/([0-9]+.*[a-zA-Z]+)|([a-zA-Z]+.*[0-9]+)/', $password)) {
           spew("$me: password is not alpha-numeric");
           return array(4, _("Password must contain both letters and numerals."));
       }
   }


   // apply Cracklib tests
   //
   if (!$admin_override) {

       static $cracklib_dict;
       if (empty($cracklib_dict))
           $cracklib_dict = vadmin_getvar('CONFIG', 'password.cracklib_dict');

       if (!empty($cracklib_dict)) {

           // no Cracklib support built in?
           //
           if (!function_exists('crack_opendict') || !function_exists('crack_check'))
               spew("$me: Cracklib support is not available in this PHP build even though it has been configured in Vadmin.  Test your PHP settings and see http://www.php.net/manual/ref.crack.php");

           // we have Cracklib, so proceed
           //
           else {

               $crack_dict = crack_opendict($cracklib_dict);

               // dictionary valid?
               //
               if (!$crack_dict)
                   spew("$me: Could not open Cracklib dictionary at $cracklib_dict.");

               // run the crack check and bail if password fails
               //
               else if (!crack_check($crack_dict, $password)) {

                   $diagnostic = crack_getlastmessage();
                   crack_closedict($crack_dict);
                   spew("$me: password failed Cracklib strength test: " . $diagnostic);

                   // i18n-ize the Cracklib diagnostic messages
                   // These strings are culled from:
                   // http://www.php.net/manual/function.crack-getlastmessage.php
                   // as well as the actual source in the Cracklib trunk
                   // as of 2008/01/23:
                   // http://cracklib.svn.sourceforge.net/viewvc/cracklib/trunk/cracklib/lib/fascist.c?view=markup
                   // The syntax, punctuation and word usage in these messages
                   // are inconsistent, so if anyone sees that these are not
                   // correctly translated, please report the needed corrections
                   //
                   if (0) // this block just so these strings will be included in vadmin.pot
                   {
//TODO: possibly combine the English messages here (by passing the "getlastmessage()" through an internal translation first) into a smaller number of more accurate and appropriate strings (for our context at least), then the pot file has fewer similar strings
                      $ignore = _("you are not registered in the password file");
                      $ignore = _("it is based on your username");
                      $ignore = _("it is based upon your password entry");
                      $ignore = _("it is derived from your password entry");
                      $ignore = _("it's derived from your password entry");
                      $ignore = _("it is derivable from your password entry");
                      $ignore = _("it's derivable from your password entry");
                      $ignore = _("it's WAY too short");
                      $ignore = _("it is WAY too short");
                      $ignore = _("it is too short");
                      $ignore = _("it does not contain enough DIFFERENT characters");
                      $ignore = _("it is all whitespace");
                      $ignore = _("it is too simplistic/systematic");
                      $ignore = _("it looks like a National Insurance number.");
                      $ignore = _("it is based on a dictionary word");
                      $ignore = _("it is based on a (reversed) dictionary word");
                      $ignore = _("strong password");
                   }

                   return array(5, sprintf(_("Password failed strength test: %s."), _($diagnostic)));
               }

               // password passed Cracklib tests; now just close the dictionary
               //
               else crack_closedict($crack_dict);
           }

       }

   }


   // password looks good!
   //
   return array(0);

}

/**
  * Gets per-domain SquirrelMail setting
  *
  * Gets SquirrelMail variable value for the given domain.
  * This function is mainly useful if you have the Login
  * Manager (Vlogin) plugin installed and have the setting
  * overridden from the default SquirrelMail configuration
  * value.
  *
  * Warning: call this function with caution, as it will run the
  * Vlogin configuration override code, which can mangle request
  * results if not called at the end of a request after any globals
  * have already been used, etc.  Returned values are stored in
  * session, so if you look up the desired setting once in a 
  * strategic location, further calls here do not carry any such risk.
  *
  * @param $setting string The name of the setting to retrieve.
  * @param $domain  string The domain to retrieve the setting for.
  *                        If not given, we just get the default 
  *                        setting in the main SquirrelMail configuration 
  *                        (or Vlogin's global override, if any).
  *                        FIXME: actually, I think vlogin will
  *                        fall back to the *current* domain;
  *                        this may not be reliable if $domain
  *                        is empty in conjunction with vlogin.
  *
  * @return string The value for the desired setting.
  *
  */
function vadmin_get_per_domain_sm_setting($setting, $domain='') {

    $me = 'vadmin_get_per_domain_sm_setting';
    spew("$me: figure out $setting value for domain $domain.");

    $value = vadmin_getvar('SESSION', $setting . $domain);
    if (empty($value)) {

        spew("$me: Isn't in SESSION, getting from SM/Vlogin.");
        $value = vadmin_getvar('SQMAIL', $setting);
        spew("$me: SM config $setting: $value");

        // look in vlogin if present, it may have different override for the domain
        //
        global $plugins;
        if (in_array('vlogin', $plugins)) {
            spew("$me: Looking for value from Vlogin.");
            $current_real_domain = vadmin_getvar('SQMAIL', 'domain');
            $saved_globals = $GLOBALS;
            $saved_session = $_SESSION;
            include_once(SM_PATH . 'plugins/vlogin/functions.php');
            overrideSmConfig('', '', $domain, $domain, $domain);
            $vlogin_setting_override = vadmin_getvar('SQMAIL', $setting);

            // set config back to what it should be
            //
            $GLOBALS = $saved_globals;
            $_SESSION = $saved_session;
            //overrideSmConfig('', '', $current_real_domain, $current_real_domain, $current_real_domain);
            $value = $vlogin_setting_override;
        }
        vadmin_putvar('SESSION', $setting . $domain, $value);

    }

    spew("$me: Finally, drumroll please... $setting is: $value");
    return $value;
}

/**
  * Get SquirrelMail pref value for other user
  *
  * Just like calling getPref() against a user other than the one
  * who is currently logged in.  Avoids polluting the current user's
  * prefs cache as well as any prefs overrides from other domain
  * (thanks to Vlogin, etc.).
  * 
  * @param $data_dir string The location of the SM data directory as necessary.
  * @param $userid   string The full IMAP username of the target user.
  * @param $pref     string The desired preference name.
  * @param $default  mixed  The default value if the pref isn't found.
  *
  * @return mixed The preference value as returned from SM's getPref()
  *
  */
function vadmin_getPrefForUser($data_dir, $userid, $pref, $default) {

    $me = 'vadmin_getPrefForUser';
    spew("$me: Getting pref $pref for user $userid (with data_dir $data_dir and default value of $default).");

    global $prefs_are_cached, $prefs_cache, $force_username_lowercase;

    $prefs_are_cached = false;
    $prefs_cache = array();
    sqsession_unregister('prefs_are_cached');
    sqsession_unregister('prefs_cache');
    
//    $saved_globals = $GLOBALS;
//    $saved_session = $_SESSION;

    if ($force_username_lowercase)
       $value = getPref($data_dir, strtolower($userid), $pref, $default);
    else
       $value = getPref($data_dir, $userid, $pref, $default);

//    $GLOBALS = $saved_globals;
//    $_SESSION = $saved_session;

    $prefs_are_cached = false;
    $prefs_cache = array();
    sqsession_unregister('prefs_are_cached');
    sqsession_unregister('prefs_cache');

    spew("$me: Found $value.");
    return $value;
}

/**
  * Get ALL SquirrelMail preferences for any user
  *
  * Collects all preferences stored for the desired user, which
  * can be a user who is currently NOT logged in.  This function
  * avoids polluting the current user's prefs cache as well as 
  * any prefs overrides from other domain (thanks to Vlogin, etc.).
  * 
  * @param $data_dir string The location of the SM data directory as necessary.
  * @param $userid   string The full IMAP username of the target user.
  *
  * @return array A list of all the user's preference keys and values.
  *
  */
function vadmin_get_all_user_prefs($data_dir, $userid) {

    $me = 'vadmin_get_all_user_prefs';
    spew("$me: Getting all prefs for user $userid (with data_dir $data_dir).");

    global $prefs_are_cached, $prefs_cache, $force_username_lowercase;

    $prefs_are_cached = false;
    $prefs_cache = array();
    sqsession_unregister('prefs_are_cached');
    sqsession_unregister('prefs_cache');

//    $saved_globals = $GLOBALS;
//    $saved_session = $_SESSION;

    if ($force_username_lowercase)
       $ignore = getPref($data_dir, strtolower($userid), 'dummy');
    else
       $ignore = getPref($data_dir, $userid, 'dummy');

    $all_user_preferences = $prefs_cache;

//    $GLOBALS = $saved_globals;
//    $_SESSION = $saved_session;

    $prefs_are_cached = false;
    $prefs_cache = array();
    sqsession_unregister('prefs_are_cached');
    sqsession_unregister('prefs_cache');

    spew("$me: Done.");
    return $all_user_preferences;
}

/**
  * Override ALL SquirrelMail preferences for any user
  *
  * Resets all user preferences for the desired user with
  * the given array of preference settings.  The user can
  * be a user who is currently NOT logged in.  This function
  * avoids polluting the current user's prefs cache as well
  * as any prefs overrides from other domain (thanks to Vlogin, etc.).
  *
  * @param $data_dir  string The location of the SM data directory as necessary.
  * @param $userid    string The full IMAP username of the target user.
  * @param $new_prefs array  The array of new (override) preference
  *                          keys and values (pref name => pref value)
  *
  * @return boolean TRUE upon success, FALSE otherwise
//FIXME: there is no error detection and no place that currently returns FALSE herein; I'm not sure exactly where we'd do that...
  *
  */
function vadmin_put_all_user_prefs($data_dir, $userid, $new_prefs) {

    $me = 'vadmin_put_all_user_prefs';
    spew("$me: Resetting all prefs for user $userid (with data_dir $data_dir).");

    global $prefs_are_cached, $prefs_cache, $force_username_lowercase;

    $prefs_are_cached = false;
    $prefs_cache = array();
    sqsession_unregister('prefs_are_cached');
    sqsession_unregister('prefs_cache');

//    $saved_globals = $GLOBALS;
//    $saved_session = $_SESSION;

    if ($force_username_lowercase)
       $ignore = getPref($data_dir, strtolower($userid), 'dummy');
    else
       $ignore = getPref($data_dir, $userid, 'dummy');

    $all_user_preferences = $prefs_cache;

    // now that we have the current list of user preferences,
    // the first thing to do is iterate through it and remove
    // all of them
    //
    foreach ($all_user_preferences as $key => $val)
        setPref($data_dir, $userid, $key, '');

    // and next we re-populate with our override list
    //
    foreach ($new_prefs as $key => $val)
        setPref($data_dir, $userid, $key, $val);

//    $GLOBALS = $saved_globals;
//    $_SESSION = $saved_session;

    $prefs_are_cached = false;
    $prefs_cache = array();
    sqsession_unregister('prefs_are_cached');
    sqsession_unregister('prefs_cache');

    spew("$me: Done.");
    return TRUE;
}

/**
  * Retrieve user webmail stats
  *
  * @param $userid string The full user for which to get stats.
  * @param $domain string The domain that the user belongs to.
  *
  * @return array Associative list of user webmail stats with the following
  *               entries: LAST_LOGIN           Full date of last login
  *                        LAST_LOGIN_SHORT     Abbreviated date of last login
  *                        TOTAL_LOGINS         Number of overall logins
  *                        NEVER_LOGGED_IN      Boolean TRUE/FALSE
  *
  */
function vadmin_get_user_webmail_stats($userid, $domain) {
    $me = 'vadmin_get_user_webmail_stats';

/*
    if (!vadmin_domain_passwd_issaved($domain))
        return array(
                     'LAST_LOGIN'         => _("Never"),
                     'LAST_LOGIN_SHORT'   => _("Never"),
                     'TOTAL_LOGINS'       => 0,
                     'NEVER_LOGGED_IN'    => TRUE,
                    );
*/

    $trackusage = vadmin_get_pref($domain, 'track_usage');
    if ($trackusage == FALSE)
        return array(
                     'LAST_LOGIN'         => _("Never"),
                     'LAST_LOGIN_SHORT'   => _("Never"),
                     'TOTAL_LOGINS'       => 0,
                     'NEVER_LOGGED_IN'    => TRUE,
                    );

    $datadir = vadmin_get_per_domain_sm_setting('data_dir', $domain);

    $delim = vadmin_getvar('BACKEND', 'delimiters');
    $delim = substr($delim, 0, 1);

    if (vadmin_IMAP_usernames_have_domain($domain) && strpos($userid, $delim) === FALSE)
        $full_prefs_username = "$userid$delim$domain";
    else
        $full_prefs_username = $userid;

    $lastLogin = vadmin_getPrefForUser($datadir, $full_prefs_username,  
                                       'vadmin_last_login_date', 'NEVER');
    if ($lastLogin != 'NEVER') {
        $last_login = date('Y/m/d H:i:sO', $lastLogin);
        $last_login_short = date('Y/m/d', $lastLogin);
    } else {
        $last_login = _("Never");
        $last_login_short = _("Never");
    }

    $total_logins = vadmin_getPrefForUser($datadir, $full_prefs_username, 
                                          'vadmin_total_logins', 0);

    return array(
                 'LAST_LOGIN'         => $last_login,
                 'LAST_LOGIN_SHORT'   => $last_login_short,
                 'TOTAL_LOGINS'       => $total_logins,
                 'NEVER_LOGGED_IN'    => ((int)$total_logins > 0 ? FALSE : TRUE),
                );
}

/**
  * Determine if a given user has the permissions
  * needed to edit another.
  *
  * @param string $domain   Domain of the user to be edited
  * @param string $myname   The name of the user requesting edit permissions
  * @param string $mydesig  The Vadmin administrator designation for the
  *                         user requesting edit permissions
  * @param string $username The name of the user to be edited
  *                         (usually full email address format
  *                         except maybe for systems that don't
  *                         use domains in the username format)
  *
  * @return boolean TRUE if the requesting user is allowed to
  *                 edit the other; FALSE otherwise
  *
  */
function vadmin_get_edit_perms($domain, $myname, $mydesig, $username){
    $me = 'vadmin_get_edit_perms';
    if ($mydesig == 'ELVIS'){
        spew("$me: ELVIS can edit anyone");
        return true;
    }

    // check if the user's domain is controlable by the current user
    //
    else {

        // lowlies can only edit within their own domain
        //
        if ($mydesig == 'LOWLY') {
            if ($domain != vadmin_getdomain())
                return false;
        }

        // cross admins can only edit what they have been allowed
        //
        else if ($mydesig == 'CROSS') {
            $domain_ary = vadmin_get_xdomain_array($myname);
            if (!in_array($domain, $domain_ary))
                return false;
        }

    }

    spew("$me: checking if this is an admin of any sort");
    $udesig = vadmin_get_user_designation($domain, $username);
    spew("$me: udesig is '$udesig'");
    if ($udesig == 'NONER'){
        spew("$me: everyone can edit 'NONER'");
        return true;
    }
    if ($myname == $username){
        /**
         * They can edit themselves all they want
         */
        return true;
    }

    // test for self in systems where IMAP usernames don't have @domain in them
    //
    list($uname, $ignore) = vadmin_get_user_unamedomain($username);
    if (!vadmin_IMAP_usernames_have_domain($domain) && $myname == $uname){
        /**
         * They can edit themselves all they want
         */
        return true;
    }

    spew("$me: mydesig is '$mydesig'");
    if ($mydesig == 'LOWLY'){
        /**
         * Lowly admin cannot edit any other admins besides themselves.
         */
        return false;
    } elseif ($mydesig == 'CROSS'){
        /**
         * CROSS can only edit LOWLY
         */
        if ($udesig == 'LOWLY'){
            return true;
        }
    }
    return false;
}

/**
 * Parses some text that contains an autoresponder
 * message and possibly other message headers.
 *
 * @param string $text The text glob to be parsed
 *
 * @return array An associative list of the fields
 *               where the message text (body) itself
 *               is keyed by "message", and other
 *               headers are keyed by the header name
 *               (in all lower case), such as "from"
 *               or "subject".
 *
 */
function vadmin_parse_autoresponder_message($text)
{

   $header_list = vadmin_getvar('CONFIG', 'autoresponder.message_contains_headers');

   if (empty($header_list)) return array('message' => $text);


   // split the text into an array with one line per entry
   // and iterate, stopping at the first blank line
   //
   $text = preg_split("/\r\n|\r|\n/", $text);
   $return_array = array();
   $message = array();
   $finished_headers = FALSE;
   foreach ($text as $line)
   {
      if (empty($line))
      {
         $finished_headers = TRUE;
         continue;
      }

      if (!$finished_headers)
      { 
         if (preg_match('/^([a-z_-]+):\s*(.+)$/i', $line, $matches))
            $return_array[strtolower($matches[1])] = $matches[2];
      }
      else
         $message[] = $line;
   }

   
   // the rest of the text is the message body
   //
   $return_array['message'] = implode("\n", $message);

   
   return $return_array;

}

/**
 * This utility function notifies the user that they have attempted
 * an action that was deemed as violating security by the system.
 * It used to carry an "INTRUDER ALERT ON DECK 5", but I changed it
 * to be more non-geek friendly. ;)
 * 
 * @return Never returns.
 */
function vadmin_security_breach(){
    $me = 'vadmin_security_breach';
    spew("$me: I was invoked!");
    echo _("You have attempted an action that was flagged by the system as violating the security measures in place. Your access has been denied. If you believe it was an error on our part, please try your action again, or e-mail your system administrator.");
    exit;
}

/**
 * This function displays an error message if the error was due to 
 * system malfunction and not user input.
 *
 * @param $message A text message to display to the user.
 * @return         Never returns.
 */
function vadmin_system_error($message){
    $me = 'vadmin_system_error';
    spew("$me: Invoked with message '$message'");
    spew("$me: checking if we're in vadmin_main");
    $SCRIPT_NAME = vadmin_getvar('SERVER', 'SCRIPT_NAME');
    if (!strstr($SCRIPT_NAME, 'vadmin')){
        spew("$me: no, just printing the error message and exiting.");

        // if configtest is being run, print error in that style
        //
        if (strpos($SCRIPT_NAME, '/src/configtest.php') !== FALSE)
            do_err('Vadmin plugin encountered a fatal error: ' . $message, TRUE);
        else
            echo sprintf(_("VADMIN SYSTEM ERROR: %s"), $message);

        // why is this return here?  I though this function wasn't supposed to return?
        //return;
        exit;
    }
    $warning = sprintf(_("<strong>SYSTEM ERROR: %s</strong>. We apologize for the inconvenience, but your request cannot be completed at this time. Please contact your system administrator if the problem persists."), $message);
    $body = sprintf('<hr/><p>%s</p><hr/>', $warning);
    vadmin_make_page(_("SYSTEM ERROR"), null, $body, false, false);
    exit;
}

/**
 * This function displays an error message if the error was due to the
 * user input.
 *
 * @param $message A text message to display to the user.
 * @return         Never returns.
 */
function vadmin_user_error($message){
    $me = 'vadmin_user_error';
    spew("$me: Invoked with message '$message'");
    spew("$me: checking if we're in vadmin_main");
    $SCRIPT_NAME = vadmin_getvar('SERVER', 'SCRIPT_NAME');
    if (!strstr($SCRIPT_NAME, 'vadmin')){
        spew("$me: no, just printing the error message and exiting.");
        echo sprintf(_("VADMIN SYSTEM ERROR: %s"), $message);
        return;
    }
    $warning = sprintf(_("<strong>Error: %s</strong>. Please return to the previous page and try to correct your input. If you need assistance or believe this is a software error, please contact your system administrator."), $message);
    $body = sprintf('<p>%s</p>', $warning);
    vadmin_make_page(_("Input Error"), null, $body, false, false);
    exit;
}

/**
 * This function is the authentication core of vadmin. Read the
 * docs/SECURITY for the in-depth explanation on how the security
 * is handled in vadmin.
 *
 * @return $VADMIN_AUTHCODE, which is a string with the 
 *         authentication information.
 */
function vadmin_auth(){
    $me = 'vadmin_auth';
    $username = vadmin_getvar('SQMAIL',  'username');
    $authcode = vadmin_getvar('SESSION', 'VADMIN_AUTHCODE');
    $domain   = vadmin_getdomain();
    
    /**
     * If VADMIN_AUTHCODE is set, then they have already authenticated.
     */
    if ($authcode != false){
        spew("$me: AUTHCODE is already in session with '$authcode'");
        return ($authcode);
    }
    if ($domain == false){
        spew("$me: doesn't look like this domain is configured with vadmin");
        spew("$me: setting AUTHCODE to 'NONER' and returning");
        vadmin_putvar('SESSION', 'VADMIN_AUTHCODE', 'NONER');
        return 'NONER';
    }
    $authcode = vadmin_get_user_designation($domain, $username);
    vadmin_putvar('SESSION', 'VADMIN_AUTHCODE', $authcode);
    return $authcode;
}

/**
  * Gets current login user's password
  *
  * Please use with care.
  *
  * @return string The current users's password
  *
  */
function vadmin_get_current_pwd() {
   $key = vadmin_getvar('COOKIE', 'key');
   $onetimepad = vadmin_getvar('SQMAIL_SESSION', 'onetimepad');
   return OneTimePadDecrypt($key, $onetimepad);
}

/**
  * Determines if IMAP username format is such that usernames
  * are of the format "user@domain.tld" and not just "user".  
  * It's very sad that people build mail systems like the latter...
  *
  * If the value for the username.username_format_includes_domain
  * setting in the main Vadmin conf file is set to "yes" or "no",
  * that is what is used, but if it is set to "auto", then...
  *
  * This is determined by looking at a domain pref that should
  * have been set when it was enabled, since doing it on the 
  * fly based on the current username could be a sensitive issue.  
  * If that pref isn't there but ELVIS is logged in, we can look 
  * at his username format.
  *
  * @param $domain The vadmin domain.
  *
  * @return bool TRUE if IMAP usernames have the domain in them
  *
  */
function vadmin_IMAP_usernames_have_domain($domain)
{

   static $IMAP_usernames_have_domain_in_them = FALSE;

   $me = 'vadmin_IMAP_usernames_have_domain';
   spew("$me: detecting IMAP username format, current value is '$IMAP_usernames_have_domain_in_them'");


   // only need to look it up once (should we also put it in session?
   //
   if ($IMAP_usernames_have_domain_in_them !== FALSE)
      return ($IMAP_usernames_have_domain_in_them === 'yes');


   // if this setting is given in our config, we can luckily
   // just use that
   //
   spew("$me: looking in config settings for IMAP username format");
   $IMAP_usernames_have_domain_in_them 
    = vadmin_getvar('CONFIG', 'username.username_format_includes_domain');
   if ($IMAP_usernames_have_domain_in_them === 'yes')
      return TRUE;
   else if ($IMAP_usernames_have_domain_in_them === 'no')
      return FALSE;


   // grab from vadmin db
   //
   spew("$me: looking in vadmin settings for IMAP username format");
   $IMAP_usernames_have_domain_in_them 
       = dbastor_get($domain, 'IMAP_usernames_have_domain_in_them');

 
   // still can't find it?  well, if we are elvis, let's figure it out
   // and put it in the vadmin db for this domain
   //
   if ($IMAP_usernames_have_domain_in_them === FALSE)
   {
      $username = vadmin_getvar('SQMAIL', 'username');
      if (vadmin_get_user_designation($domain, $username, TRUE) === 'ELVIS') {
         spew("$me: geez, well, elvis can always lead us out of the darkness");
         $delim = vadmin_getvar('BACKEND', 'delimiters');
         $delim = substr($delim, 0, 1);
         if (strpos($username, $delim) !== FALSE)
            $IMAP_usernames_have_domain_in_them = "yes";
         else
            $IMAP_usernames_have_domain_in_them = "no";
         spew("$me: storing IMAP username format for $domain "
            . "as \"$IMAP_usernames_have_domain_in_them\"");
         dbastor_put($domain, 'IMAP_usernames_have_domain_in_them', 
                     $IMAP_usernames_have_domain_in_them);
      }
//FIXME: will this error show up when we don't necessarily want it to?  If so, perhaps return NULL to indicate indeterminate value??  But then all callers need to check for that condition, so I'd rather leave this here for now.
      else 
      {
         spew("$me: well, that was hopeless - FAILED");
         vadmin_system_error(_("Unable to determine username format"));
      }
   }


   spew("$me: this is what I got: '$IMAP_usernames_have_domain_in_them'");
   return ($IMAP_usernames_have_domain_in_them === 'yes');

}

/**
 * Looks at the user's login name, which should be either user@domain.com
 * or user:domain.com, and separates the userid from the domain name,
 * returning them in an array.  When IMAP usernames for the current domain
 * don't have the domain in them, this function will return the given 
 * username and the global SquirrelMail domain unless $strict is turned on.
 *
 * @param string $unamedomain A string in the format username{sep}domainname
 * @param bool   $strict      When username format does not include the domain
 *                            in it, do not allow SquirrelMail global domain
 *                            value to be substituted for the domain returned.
 *                            (OPTIONAL; default is FALSE)
 *
 * @return array An array with the username and the domainname 
 *               (domain only will be empty if $strict is TRUE
 *               and usernames don't have @domain in them),
 *               or array with two empty fields if unsuccessful.
 */
function vadmin_get_user_unamedomain($unamedomain, $strict=FALSE){
    $me = 'vadmin_get_user_unamedomain';
    spew("$me: analyzing '$unamedomain'");
    $delimiters = vadmin_getvar('BACKEND', 'delimiters');
    for($ct = 0; $ct < strlen($delimiters); $ct++){
        $separator = substr($delimiters, $ct, 1);

        // @ sign to quiet instances where no delimiter is found 
        @list($uname, $domain) = explode($separator, $unamedomain, 2);
        if (!empty($domain)){
            spew("$me: domain '$domain' found with separator '$separator'");
            break;
        }
    }
    if (!empty($domain)){
        spew("$me: username: $uname, domain: $domain");
        $ret_ary = array(strtolower($uname), strtolower($domain));
    } else {

        // check if username format for this domain does not 
        // include domain information in it
        //
        $vadmin_domain = vadmin_getvar('VADMIN', 'VADMIN_DOMAIN');
        if (empty($vadmin_domain))
            $vadmin_domain = vadmin_getvar('SQMAIL', 'domain');
        if (!vadmin_IMAP_usernames_have_domain($vadmin_domain)) {
            if ($strict) {
                spew("$me: No domain in '$unamedomain', but that's OK; but cannot use SquirrelMail domain");
                return array($unamedomain, '');
            } else {
                spew("$me: No domain in '$unamedomain', but that's OK; using SquirrelMail domain");
                return array($unamedomain, $vadmin_domain);
            }
        }

        spew("$me: Couldn't grok what the hell '$unamedomain' is!");
        $ret_ary = array('', '');
    }
    return $ret_ary;
}

/**
 * This is a GUI wrap-around for every generated page, since I'm too
 * lazy to write one for each generated page, nor should I. ;)
 * $scriptsrc is not currently used in vadmin, but I preserved it
 * from SquirrelSpell in case I will add this functionality later.
 *
 * @param $title          the title of the page.
 * @param $scriptsrc      the name of a javascript file to load before loading
 *                        the page itself.
 * @param $body           the contents of the page.
 * @param $returntomain   show "return to main menu" link? true/false
 * @param $logoutlink     show logout link? true/false
 * @param $previouslink   optional text (full HTML for link and displayed text)
 *                        to be shown as a "return to previous page" or other
 *                        such link just above the "Back to Main Menu link"
 * @param $blockOnLoad    when TRUE, stops SquirrelMail from adding its usual
 *                        JavaScript code that focuses in the first text box 
 *                        on the page - defaults to FALSE
 *
 * @return               void.
 */
function vadmin_make_page($title, $scriptsrc, $body, $returntomain, 
                          $logoutlink, $previouslink='', $blockOnLoad=FALSE){
    $me = 'vadmin_make_page';
    spew("$me: building a page '$title'");
    $color    = vadmin_getvar('SQMAIL', 'color');
    $PHP_SELF = vadmin_getvar('SERVER', 'PHP_SELF');
    $LVL      = vadmin_getvar('GET', 'LVL');
    /**
     * Call squirrelmail's "displayPageHeader" function"
     * (make sure we're in SquirrelMail text domain first)
     * In 1.5.1 the signature of displayPageHeader changed
     */
    sq_change_text_domain('squirrelmail');
    if ($blockOnLoad) {
        if (check_sm_version(1, 5, 1))  
            displayPageHeader($color, '', '', 'onload=""');
        else {
            global $javascript_on;
            $temp_javascript_on = $javascript_on;
            $javascript_on = FALSE;
            displayPageHeader($color, '');
            $javascript_on = $temp_javascript_on;
        }
    } else
        displayPageHeader($color, '');
    sq_change_text_domain('vadmin');
    if(isset($scriptsrc) && $scriptsrc){
        $scriptsrc = sprintf('<script type="text/javascript" src="js/%s"></script>', $scriptsrc);
    } else {
        $scriptsrc = '';
    }
    spew("$me: scriptsrc=$scriptsrc");
    /**
     * Use "previouslink" if given
     */
    $returnlink = $previouslink;

    /**
     * This checks whether we should include a "return to main menu"
     * link at the bottom for convenience.
     */
    if ($returntomain == true){
        spew("$me: Providing a 'return to main' link");
        list($start_mod, $start_act) = vadmin_get_startup_location();
        $link = vadmin_mkform_action('admin', $start_mod, $start_act);
        $str = ''
            . ' <tr>'
            . '  <td align="center">'
            . '   <a href="%s">&lt;&lt; %s</a>'
            . '  </td>'
            . '</tr>';
        $returnlink .= sprintf($str, $link, 
                                _("Back to &quot;Main Menu&quot;"));
    } 
    spew("$me: returnlink=$returnlink");
    /** 
     * Provide them with a logout link if requested.
     */
    if ($logoutlink == true){ 
        spew("$me: Providing a 'logout' link");
        $link = vadmin_mkform_action('admin', 'logout', 'main');
        $str = '(<strong><a href="%s">%s</a></strong>)';
        $logoutlink = sprintf($str, $link, _("Admin Logout"));
    } else {
        $logoutlink = '';
    }

    $versionline = '&nbsp;';
    $debug = vadmin_debug_level();
    if ($debug > 0) 
        $versionline = sprintf(_("Vadmin Plugin version %s"), vadmin_version());
    $str = '&nbsp;<br/>'
        . '%s'
        . '<table width="95%%" align="center" border="0" cellpadding="2" '
        . 'cellspacing="0">'
        . ' <tr>'
        . '  <td bgcolor="%s" align="center">'
        . '   <strong>%s</strong>'
        . '  </td>'
        . ' </tr>'
        . ' <tr><td><hr/></td></tr>'
        . ' <tr>'
        . '  <td>'
        . '   %s'
        . '  </td>'
        . ' </tr>'
        . ' %s'
        . ' <tr><td><hr/></td></tr>'
        . ' <tr>'
        . '  <td bgcolor="%s" align="center">'
        . '    %s %s'
        . '  </td>'
        . ' </tr>'
        . '</table>';
    print sprintf($str, $scriptsrc, $color[9], $title, $body, 
                  $returnlink, $color[9], $versionline, $logoutlink);
}

/**
 * Makes a menu of links
 *
 * @param array $menu_contents List of arrays describing each menu entry
 *                             with key/values of: 'title' = link title 
 *                             attribute text, 'words' = link text, and 
 *                             'LVL', 'MOD' and 'ACT' point to the target 
 *                             action.  If the entry has a key called 
 *                             'separator', just some blank space is placed 
 *                             on the menu output.
 *
 * @return string The HTML for the full menu
 *
 */
function vadmin_menu_make_linkmenu($menu_contents){

    if (check_sm_version(1, 5, 2))
        $hook_return = do_hook('vadmin_menu_make_linkmenu', $menu_contents);
    else
        $hook_return = do_hook_function('vadmin_menu_make_linkmenu', $menu_contents);

    if (is_array($hook_return))
        $menu_contents = $hook_return;

    $color = vadmin_getvar('SQMAIL', 'color');
    $head = ''
        . '<table border="0" align="center" width="40%">'
        . ' <tr>'
        . '  <th bgcolor="' . $color[9] . '">'
        .     _("Make your selection:")
        . '  </th>'
        . ' </tr>'
        . ' <tr>'
        . '  <td>'
        . '   <ul style="margin-top:0; margin-bottom:0">';
    $foot = ''
        . '   </ul>'
        . '  </td>'
        . ' </tr>'
        . ' <tr>'
        . '  <th bgcolor="' . $color[9] . '">&nbsp;</th>'
        . ' </tr>'
        . '</table>';
    $body = $head;
    foreach ($menu_contents as $menu){
        if (!empty($menu['separator']))
        {
            $body .= '<br /><br />';
            //$body .= '</ul><hr width="90%" /><ul style="margin-top:0; margin-bottom:0">';
        }
        else
        {
        $link = vadmin_mkform_action($menu{'LVL'}, $menu{'MOD'}, $menu{'ACT'});
        $title = $menu{'title'};
        $words = $menu{'words'};
        $body .= '<li><a href="' . $link . '" title="' . $title . '">'
            . $words . '</a></li>';
        }
    }
    $body .= $foot;
    return $body;
}

/**
 * This function stolen nearly verbatim from hastymail. 
 * http://hastymail.sf.net/
 */
function vadmin_rc4_crypt($input, $key) {
    $me = 'vadmin_rc4_crypt';
    spew("$me: encrypting using the builtin rc4");
    $k_tmp = preg_split('//', $key, -1, PREG_SPLIT_NO_EMPTY);
    foreach($k_tmp as $char) {
        $k[] = ord($char);
    }
    unset($k_tmp); 
    $message = preg_split('//', $input, -1, PREG_SPLIT_NO_EMPTY);
    $rep = count($k);
    for ($n=0;$n<$rep;$n++) {
        $s[] = $n;
    }
    $i = 0;
    $f = 0;
    for ($i = 0;$i<$rep;$i++) {
        $f = (($f + $s[$i] + $k[$i]) % $rep);
        $tmp = $s[$i];
        $s[$i] = $s[$f];
        $s[$f] = $tmp;
    }
    $i = 0;
    $f = 0;
    foreach($message as $letter) {
        $i = (($i + 1) % $rep);
        $f = (($f + $s[$i]) % $rep);
        $tmp = $s[$i];
        $s[$i] = $s[$f];
        $s[$f] = $tmp;
        $t = $s[$i] + $s[$f];
        $done = ($t^(ord($letter)));
        $i++;
        $f++;
        $enc_array[] = chr($done);
    }
    $coded = implode('', $enc_array);
    return $coded;
}


/**
 * This function does the encryption and decryption of sensitive
 * data stored on the HDD. It uses the CRYPTO_HASH_LINE as key.
 * See docs for more info and/or explanation of the security
 * structure of VADMIN.
 *
 * @param $input the contents to encrypt/decrypt.
 * @param $mode  a string that can be either "decrypt" or "encrypt"
 *               depending on which action needs to be performed.
 * @return       the results of encryption/decryption.
 */
function vadmin_crypto($input, $mode){
    $me = 'vadmin_crypto';
    $CRYPTO_HASH_LINE   = vadmin_getvar('SERVER', 'CRYPTO_HASH_LINE');
    $MCRYPT_ALGO        = vadmin_getvar('SERVER', 'MCRYPT_ALGO');

    if ($CRYPTO_HASH_LINE == 'LLAMA')
        vadmin_system_error(_("Vadmin's apache.conf is misconfigured: CRYPTO_HASH_LINE has not been changed from its default value"));

    /**
     * See if we have everything needed for
     * encryption/decryption. This includes checking for the
     * encryption functions, the algorithm, and the hash line.
     */
    if (!$CRYPTO_HASH_LINE || !$MCRYPT_ALGO){
        $message = '';
        if (!$CRYPTO_HASH_LINE){
            $message .= _("Could not find CRYPTO_HASH_LINE! ");
        }
        if (!$MCRYPT_ALGO){
            $message .= _("Could not find MCRYPT_ALGO! ");
        }
        spew("$me: Can't use crypt functions for the following reasons:");
        spew("$me: $message");
        vadmin_system_error(sprintf(_("Vamin is misconfigured: %s"),$message));
    }
    $key = $CRYPTO_HASH_LINE;
    if ($MCRYPT_ALGO == 'rc4_builtin'){
        spew("$me: using builtin rc4 encryption/decryption");
        spew("$me: thank you jason. :)");
        switch($mode){
        case 'encrypt':
            $endresult = base64_encode(vadmin_rc4_crypt($input, $key));
            spew("$me: encrypted successfully");
            break;
        case 'decrypt':
            $endresult = vadmin_rc4_crypt(base64_decode($input), $key);
            spew("$me: decrypted successfully");
            break;
        }
    } else {
        if (!function_exists('mcrypt_generic')){
            $message = _("An algorithm other than 'rc4_builtin' specified, but mcrypt support not found.");
            spew("$me: MCRYPT_ALGO is: $MCRYPT_ALGO");
            spew("$me: mcrypt functions are not found!");
            vadmin_system_error(sprintf(_("Vadmin is misconfigured: %s"), 
                                        $message));
        }
        $td = mcrypt_module_open($MCRYPT_ALGO, '', MCRYPT_MODE_ECB, '');
        $iv = mcrypt_create_iv(mcrypt_enc_get_iv_size ($td), MCRYPT_RAND);
        @mcrypt_generic_init($td, $key, $iv);
        switch ($mode){
        case 'encrypt':
            $endresult = base64_encode(mcrypt_generic($td, $input));
            spew("$me: encrypted successfully");
            break;
        case 'decrypt':
            $endresult = mdecrypt_generic($td, base64_decode($input));
            /**
             * There will be trailing nul's on the end. Remove
             * these. They suck.
             */
            $endresult = rtrim($endresult);
            spew("$me: decrypted successfully");
            break;
        }
        mcrypt_generic_deinit($td);
    }
    return $endresult;
}

/**
  * Send command to system management proxy.
  *
  * Depending on Vadmin configuration, this may open a
  * socket or execute a SUID script.
  *
  * @param string $domain         The vadmin domain.
  * @param string $domainPassword The domain password.
  * @param string $cmdXtra        The additional action-specific
  *                               arguments to be sent to the proxy.
  *
  * @return array A two-element array containing first an integer
  *               error code returned from the proxy and then a
  *               string error message (which may be empty, even
  *               when an error occurs).  If no system management
  *               proxy has been configured, the first element
  *               will be -1.
  *
  */
function vadmin_system_manager_proxy($domain, $domainPassword, $cmdXtra) {

    $me = 'vadmin_system_manager_proxy';

    $system_manager_proxy = vadmin_getvar('CONFIG', 'system_manager_proxy.path');
    if (empty($system_manager_proxy))
        return array(-1, _("Backend proxy has not been configured"));

    if (preg_match('/^\w+:\/\//', $system_manager_proxy))
        return vadmin_socket_exec($system_manager_proxy,
                                  $domain,
                                  $domainPassword,
                                  $cmdXtra);

    else
        return array(vadmin_suid_script_exec($system_manager_proxy,
                                             $domain,
                                             $domainPassword,
                                             $cmdXtra), '');

}

/**
  * Send a command to a system manager socket for backends
  * that need it.
  *
  * Note that the protocol the socket must understand is
  * hard-coded herein.
  *
  * The data sent to the socket is constructed as follows:
  *
  *  - A two-byte representation of the total data payload
  *    length
  *  - One byte indicating the total number of individual
  *    arguments embedded in the payload
  *  - A two-byte representation of the length of the 
  *    command name
  *  - The name of the command being requested, which is
  *    assumed to be the first command argument as defined
  *    in vadmin.conf.php (for example, the
  *    "system_manager_proxy.account_creation_flags" setting)
  *  - A two-byte representation of the length of the 
  *    domain being acted upon
  *  - The name of the domain being acted upon
  *  - A two-byte representation of the length of the 
  *    domain password
  *  - The domain password
  *
  * After this, each of the command arguments and their
  * values as defined in vadmin.conf.php (for example, the
  * "system_manager_proxy.account_creation_flags"), EXCEPT
  * the first argument (which was already given above),
  * are prepended with a two-byte representation of their
  * length and added to the payload. 
  *
  * The socket application should always validate the domain
  * password before handling the command request.  If possible,
  * it should also check that the current (logged in) user is a
  * valid IMAP account, check that that user is also a valid
  * Vadmin administrator.  The user data for the current user
  * can optionally be passed to the socket in the command
  * arguments that are defined in vadmin.conf.php.
  *
  * The socket is then expected to return a stream of data
  * containing a one-byte response code (which is returned to the
  * caller of this function) and then a two-byte message length
  * indicator which is followed by the command response message
  * of the indicated length.  If the message length is set to zero,
  * anything that follows in the data stream until EOF is
  * interpreted as the command response message, however intentional
  * use of this format is discouraged.  The message is also returned
  * directly to the caller of this function.
  *
  * @param string $socket_addr    The address to the target socket.
  * @param string $domain         The vadmin domain.
  * @param string $domainPassword The domain password.
  * @param string $args           The additional action-specific
  *                               arguments to be sent to the script.
  *
  * @return array A two-element array containing first an integer
  *               error code returned from the proxy and then a
  *               string error message (which may be empty, even
  *               when an error occurs).
  *
  */
function vadmin_socket_exec($socket_addr, $domain, $domainPassword, $args) {

    $me = 'vadmin_socket_exec';

    // get port number from the socket address if one is given
    // therein? (format of regular expression basically the
    //  same as in SquirrelMail's core functions/url_parser.php)
    //
    if (preg_match('/\w+:\/\/(\[?[0-9]{1,3}(\.[0-9]{1,3}){3}\]?|[0-9a-z]([-.]?[0-9a-z])*\.[a-z][a-z]+):(\d+)/', $socket_addr, $matches) && !empty($matches[4]))
    {
//FIXME: we parsed out the port, so should we just use the hostname parsed out ($matches[1]) here without the port in it?  but then what would happen with any crap on the end of the URI, if there is any?  really there should not be or it should not have meaning, but hmmm... well, we'll just leave it all alone unless someone complains that keeping the port or other info in the socket address causes problems
        $port = $matches[4];
    }
    else
    {
        $port = 0;
    }


    // try to connect...
    //
    $errno = 0;
    $errstr = '';
    $socket_open_timeout = vadmin_getvar('CONFIG', 'system_manager_proxy.socket_open_timeout');
    $SOCKET = sq_call_function_suppress_errors('fsockopen', array($socket_addr, $port, $errno, $errstr, $socket_open_timeout));


    // sometimes unix sockets seem to only work if you remove the "unix://"
    // from the front - maybe PHP version dependent???
    // just for kicks, we try to connect w/out the protocol on the front
    // of the socket address
    //
    if (!$SOCKET)
        $SOCKET = sq_call_function_suppress_errors('fsockopen', array(substr($socket_addr, strpos($socket_addr, '://') + 3), $port, $errno, $errstr, $socket_open_timeout));


    // bail if cannot make connection
    //
    if (!$SOCKET) {
        spew("me: Failed to open socket \"$socket_addr\".  Socket returned $errno: $errstr.  Check web server error log for possibly more details.");
        vadmin_system_error(_("Failed to open socket."));
    }


    // build data to be passed through the socket -
    // all arguments will be prepended with a two-byte
    // length representation and then concatenated
    // together into one long string to be sent over
    // the socket
    //
    $data = '';
    $args = preg_split('/\s+/', trim($args));
    $args = array_merge(array(array_shift($args), $domain, $domainPassword), $args);
    foreach ($args as $arg) {
        $arg_length = strlen($arg);
        $high = ($arg_length & (0xFF << 8)) >> 8;
        $low = ($arg_length & 0xFF);
        $data .= sprintf('%c%c%s', $high, $low, $arg);
    }
    // debugging
    //echo $data . '<hr />';


    // prepend the header, which consists of another two byte length
    // representation, the number of arguments being passed, and the
    // data string created above
    //
    $args_length = sizeof($args) - 1;
    $data_length = strlen($data);
    $high = ($data_length & (255 << 8)) >> 8;
    $low = $data_length & 255;
    $full_data_stream = sprintf("\002%c%c%c%s", $high, $low + 1, $args_length, $data);


    // send data to the socket
    //
    $bytes_written = fwrite($SOCKET, $full_data_stream);
    if (!$bytes_written) {
        vadmin_system_error(_("Could not communicate with mangement interface"));
    }


    // get the response code and any message being sent back
    //
    $response = ord(fgetc($SOCKET));
    $message_length = (ord(fgetc($SOCKET)) << 8) + ord(fgetc($SOCKET));
    if ($message_length == 0) {
        $out = '';
        while (!feof($SOCKET))
            $out .= fread($SOCKET, 65535);
        fclose($SOCKET);
        spew("$me: socket returned $response but did not return a message.");
//FIXME: why does the vmailmgr socket code only return $out in this case and not the array per usual?  I realize this is probably an erroneous situation, but still...  UPDATE: when using over tcp/network connection, and the client is rejected, the response seems to be zero, so we get what looks like a valid response code -- because of that, we'll force an error here
        $response = 99;
        $out = _("Socket protocol error");
        return array($response, $out);
    }


    // get message from socket
    //
    $message = fread($SOCKET, $message_length);
    spew("$me: wrote $bytes_written bytes; socket returned $response: $message.");


    // close the socket
    //
    fclose($SOCKET);
    return array($response, $message);

}

/**
  * Execute SUID script for backends that need it.
  *
  * Note that the protocol the SUID script must understand
  * is hard-coded herein.
  *
  * The script is called with a variable number of arguments
  * on the command line, which is defined for each type of
  * action in vadmin.conf.php.  These arguments are intended
  * to be passed along to the agent that processes the command
  * request after the SUID script has authenticated the
  * requesting user.
  *
  * The script is then fed authorization information via STDIN
  * across the pipe that is established between PHP and the
  * script.  The authorization informtaion consists of a series
  * of values, each on their own line (newlines separating each
  * value) and are provided in this order:
  *
  *    Vadmin encryption algorithm
  *    Vadmin encryption hashline
  *    Domain for this operation
  *    Password for the domain
  *    Current (logged in) user's username
  *    Current (logged in) user's password
  *
  * It is recommended that you use the SUID script that is
  * included with Vadmin, because it is careful to validate
  * several things before passing the command request along
  * to another program of your choice: check the current
  * (logged in) user is a valid IMAP account, check that that
  * user is also a valid Vadmin administrator, check that the
  * domain for the requested operation and its password are
  * valid in Vadmin's eyes (according to Vadmin's stored and
  * encrypted domain passwords).
  *
  * @param string $suid_script    The path to the script being executed.
  * @param string $domain         The vadmin domain.
  * @param string $domainPassword The domain password.
  * @param string $cmdXtra        The additional action-specific
  *                               arguments to be sent to the script.
  *
  * @return int Return value from script execution
  *
  */
function vadmin_suid_script_exec($suid_script, $domain, $domainPassword, $cmdXtra) {

    $me = 'vadmin_suid_script_exec';

    global $username;

    // open pipe - note that even when command is not found, popen() returns
    // a valid resource, which is intended to help get any errors back from
    // the shell, but as this is a write pipe, that's not possible... so we
    // first check if the file exists ourselves
    //
    if (!file_exists($suid_script))
        vadmin_system_error(_("Mangement interface not found"));
    spew("$me: executing SUID script: $suid_script $cmdXtra");
    $pipe = popen("$suid_script $cmdXtra", 'w');
    if (!$pipe)
        vadmin_system_error(_("Could not connect to mangement interface"));

    $algo = vadmin_getvar('SERVER', 'MCRYPT_ALGO');
    $hashline = vadmin_getvar('SERVER', 'CRYPTO_HASH_LINE');
    $password = vadmin_get_current_pwd();


    // pass all credentials to pipe, check for zero or FALSE
    // bytes written, indicating the pipe failed to accept our input
    //
    $bytes_written = fwrite($pipe, "$algo\n$hashline\n$domain\n$domainPassword\n$username\n$password\n");
    if (!$bytes_written)
        vadmin_system_error(_("Could not communicate with mangement interface"));


    // get results of command and return them
    //
    $result = pclose($pipe);
    // simulate WEXITSTATUS($result) -- this is not portable
    /////$result = $result / 256;

    spew("$me: done; wrote $bytes_written bytes and backend returned $result");
    return $result;

}

/**
  * Internal file_put_contents() function
  *
  * Writes a string (or array) to a file.
  *
  * Similar in most ways to PHP5's file_put_contents().  See:
  * http://www.php.net/manual/function.file-put-contents.php
  * This function does not implement writing stream data 
  * and only implements the FILE_APPEND flag (by means of the
  * third parameter, which is slightly different than the
  * native PHP5 function).
  *
  * @param string  $filename The file to be written to
  * @param mixed   $data     The string or array to be written
  * @param boolean $append   When TRUE, the $data will be
  *                          added to the end of the $filename
  *                          (OPTIONAL; default = FALSE)
  *
  * @return mixed FALSE when the file could not be written, or
  *               the number of bytes written when it could.
  *
  */
function vadmin_file_put_contents($filename, $data, $append=FALSE) {

    $me = 'vadmin_file_put_contents';

    if (!sq_is_writable($filename)) {
        spew("$me: File \"$filename\" is not writable; check file permissions and ownership");
        return FALSE;
    }

    $mode = ($append ? 'a' : 'w');
    $file = sq_call_function_suppress_errors('fopen', array($filename, $mode));
    if ($file === FALSE) {
        return FALSE;
    } else {
        if (is_array($data)) $data = implode($data);
        $bytes_written = sq_call_function_suppress_errors('fwrite', array($file, $data));
        sq_call_function_suppress_errors('fclose', array($file));
        return $bytes_written;
    }

}

function vadmin_mkform_action($tolvl, $tomod, $toact){
    $me = 'vadmin_mkform_action';
    spew("$me: LVL='$tolvl', MOD='$tomod', ACT='$toact'");
    $PHP_SELF = vadmin_getvar('SERVER', 'PHP_SELF');
    $action = sprintf('%s?LVL=%s&amp;MOD=%s&amp;ACT=%s',
                      $PHP_SELF, $tolvl, $tomod, $toact);
    spew("$me: $action");
    return $action;
}

function vadmin_redirect($lvl, $mod, $act, $stor_params){
    $me = 'vadmin_redirect';
    spew("$me: storing the params in session");
    vadmin_putvar('SESSION', 'VADMIN_REDIRECT_STOR', $stor_params);
    spew("$me: redirecting to LVL: $lvl, MOD: $mod, ACT: $act");
    $self = vadmin_getvar('SERVER', 'PHP_SELF');
    $loc = sprintf('%s?LVL=%s&MOD=%s&ACT=%s', $self, $lvl, $mod, $act);
    header('Location: ' . $loc);
    spew("$me: Exiting, done processing.");
    exit;
}

function vadmin_get_storparams(){
    $me = 'vadmin_get_storparams';
    spew("$me: grabbing the storred redirect params from the session");
    $stor_params = vadmin_getvar('SESSION', 'VADMIN_REDIRECT_STOR');
    vadmin_putvar('SESSION', 'VADMIN_REDIRECT_STOR', '');
    return $stor_params;
}

function vadmin_getdomain(){
    $me = 'vadmin_getdomain';

    // get out of session since the username is not
    // globalized yet when called during the
    // loading_constants hook
    $username = vadmin_getvar('SQMAIL_SESSION', 'username');

    if (empty($username)){
        spew("$me: no username, returning false");
        return false;
    }
    list($uname, $domain) = vadmin_get_user_unamedomain($username);
    spew("$me: returning $domain");
    return $domain;
}

function vadmin_gethost(){
    $me = 'vadmin_gethostdomain';
    $HTTP_HOST = vadmin_getvar('SERVER', 'HTTP_HOST');
    $domain = preg_replace('/:.*/s', '', $HTTP_HOST);
    spew("$me: preliminary domain is '$domain'");
    if (vadmin_domain_exists($domain)){
        spew("$me: found it on the first try");
    } else {
        spew("$me: dropping levels until found or second level reached");
        $dom_ary = explode('.', $domain);
        do {
            array_shift($dom_ary);
            $domain = join('.', $dom_ary);
            spew("$me: trying '$domain'");
        } while (sizeof($dom_ary) > 2 && !vadmin_domain_exists($domain));
        if (!vadmin_domain_exists($domain)){
            spew("$me: domain not configured, it seems!");
            $domain = false;
        } else {
            spew("$me: domain found as '$domain'");
        }
    }
    return $domain;
}

/**
  * Determines what limit may be applicable for the
  * given setting depending on what kind of user is
  * attempting to change it.
  *
  * @param string $domain The domain within which the
  *                       limit is being asked for
  * @param string $level  The authentication level of
  *                       the user attempting to change
  *                       the setting (LOWLY?  CROSS?  ELVIS?)
  * @param string $def    The setting name for which the 
  *                       level is being determined
  *
  * @return mixed The setting's limit, or FALSE if none
  *               is applicable.
  *
  */
function vadmin_smart_limit($domain, $level, $def){
    $me = 'vadmin_smart_limit';
    spew("$me: fetching smart limit '$def' for level '$level'");
    $limit = vadmin_get_limit($domain, $level, $def);

    // if no limit is set for LOWLY admin, give it
    // the same limit as for CROSS admins
    //
    if ($limit == false && $level == 'LOWLY'){
        spew("$me: limit not set for LOWLY, checking CROSS");
        $limit = vadmin_get_limit($domain, 'CROSS', $def);
    }

    spew("$me: smart limit set to '$limit'");
    return $limit;
}

function vadmin_get_user_number($domain){
    $me = 'vadmin_get_user_number';
    spew("$me: getting the number of real users in domain '$domain'");
    $crypto   = vadmin_getvar('SESSION', 'VADMIN_SECRET');
    $secret   = vadmin_crypto($crypto, 'decrypt');
    $userbunch = listdomain($domain, $secret);
    $mboxes = 0;
    foreach ($userbunch as $userary){

        if (!is_alias($userary)){
            $mboxes++;
        }

    }
    spew("$me: total number of real users is: $mboxes");
    return $mboxes;
}

/**
  * Determine startup location for the given designation
  *
  * @param string $desig The Vadmin designation to check startup
  *                      location for.  If not given, this function
  *                      will look up the designation for the user
  *                      who is currently logged in (OPTIONAL;
  *                      default = not given).
  *
  * @return array A two-element array where the first element is
  *               the startup module and the second is the startup
  *               action.
  *
  */
function vadmin_get_startup_location($desig='') {

    if (empty($desig)) {
        $username = vadmin_getvar('SQMAIL', 'username');
        $domain   = vadmin_getvar('VADMIN',  'VADMIN_DOMAIN');
        $desig = vadmin_get_user_designation($domain, $username);
    }

    if ($desig == 'ELVIS') {
        $start_mod = vadmin_getvar('CONFIG', 'startup.elvis_module');
        $start_act = vadmin_getvar('CONFIG', 'startup.elvis_action');
    }

    if ($desig == 'CROSS') {
        $start_mod = vadmin_getvar('CONFIG', 'startup.cross_module');
        $start_act = vadmin_getvar('CONFIG', 'startup.cross_action');
    }

    if ($desig == 'LOWLY') {
        $start_mod = vadmin_getvar('CONFIG', 'startup.lowly_module');
        $start_act = vadmin_getvar('CONFIG', 'startup.lowly_action');
    }

    // if either module or action are not found for
    // this designation, just use the default one
    //
    if (empty($start_mod))
        $start_mod = vadmin_getvar('CONFIG', 'startup.module');
    if (empty($start_act))
        $start_act = vadmin_getvar('CONFIG', 'startup.action');

    return array($start_mod, $start_act);
}

/** 
  * Retrieves list of displayable fields for the designated screen
  *
  * @param string $screen_name The screen for which to get displayable fields
  * @param string $default A (comma-separated) list of default fields in case 
  *                        nothing is found for this screen in the Vadmin 
  *                        configuration.
  *
  * @return array A list of the displayable fields for this screen.
  *
  */
function vadmin_get_displayable_fields($screen_name, $default) {

    $domain   = vadmin_getvar('VADMIN',  'VADMIN_DOMAIN');
    $username = vadmin_getvar('SQMAIL', 'username');
    $desig    = vadmin_get_user_designation($domain, $username);

    if ($desig == 'ELVIS')
        $fields = vadmin_getvar('CONFIG', 'displayable_fields.' 
                                          . $screen_name . '_elvis');
    else if ($desig == 'CROSS')
        $fields = vadmin_getvar('CONFIG', 'displayable_fields.' 
                                          . $screen_name . '_cross');
    else
        $fields = vadmin_getvar('CONFIG', 'displayable_fields.' 
                                          . $screen_name . '_lowly');

    $fields = trim($fields);
    $fields = preg_replace("/\s+/s", ' ', $fields);

    // use default if nothing else is given
    //
    if (!strlen($fields))
        $fields = $default;

    if (strpos($fields, ',') !== false)
    {
        $fields = explode(',', $fields);
        @array_walk($fields, 'vadmin_trim_array');
    } else {
        $fields = array($fields);
    }

    return $fields;

}

function vadmin_trim_array(&$member){
    $member = trim($member);
}

/**
  * Validate a string against a set of defined valid characters.
  *
  *
  * @param $compareStr string The string to check
  * @param $validChars string The set of valid characters to check against
  *
  * @return boolean true/false (success/failure)
  *                 that is, if compareStr is valid, we return true
  *
  */
function vadmin_verify_character_input($compareStr, $validChars)
{
    // escape any chars in valid character list as needed
    // for use in a regexp
    $validChars = str_replace(array(']', '/', '\''), 
                              array('\]', '\/', '\\\''), $validChars);

    // do a regex ^ match for allowed chars
    // if there is a match, there are unallowed chars
    $allowed = '[^' . $validChars . ']';
    $badMatch = preg_match("/$allowed/", $compareStr);

    // could probably return !$badMatch, but given PHP doc warnings,
    // I don't want to make assumptions about
    if ($badMatch)
        return(false);
    else
        return(true);
}

/**
  * Escapes single quotes and backslashes in a string.
TODO: Will we need to make this configurable?  Will some other backends (LDAP for example?) or simply another kind of database beside MySQL (PostgreSQL?) have different quoting such that we only want to escpae quotes but not backslashes?  Or vice versa?  Or not need escaping at all?
  *
  * @param string $string The string to be escaped.
  *
  * @return string The escaped string.
  *
  */
function str_esc($string)
{
   return str_replace(array('\\', '\''), array('\\\\', '\\\''), $string);
}

/**
  * Sorts two user accounts or aliases
  * by user (alias) name.
  *
  * @param $a array The first user/alias array.
  * @param $b array The second user/alias array.
  *
  * @return int -1 if $a is to come before $b,
  *              1 if $b is to come before $a,
  *              0 if $a and $b are equal
  *
  */
function sortUserArrayByName($a, $b)
{
   return strcasecmp($a[0], $b[0]);
}

/**
  * Sorts two user arrays by the last login time.
  *
  * Webmail login stats are expected to be an array
  * in outer array index number 13; if not, will
  * try to look it up ourselves, otherwise the
  * item will sort to the end of the list.  Superusers
  * will also get sorted down if the current user is
  * not also a superuser.
  *
  * If they have equal login times, attempts to sort
  * based on user name.
  *
  * @param $a array User info array for user 1
  * @param $b array User info array for user 2
  *
  * @return int -1 if $a is to come before $b,
  *              1 if $b is to come before $a,
  *              0 if $a and $b are equal
  *
  */
function sortUserArrayByLoginTime(&$a, &$b)
{

   global $sortdomain, $sortcatchall_alias, $sortusername,
          $sortdesig, $sort_add_delim;
   $domain = $sortdomain;
   $catchall_alias = $sortcatchall_alias;
   $username = $sortusername;
   $desig = $sortdesig;

   if (!isset($a[13]['LAST_LOGIN']))
   {

      // try to get user webmail stats
      //
      $userid = $a[0];
      if ($userid == $catchall_alias)
      {
         if ($sort_add_delim)
         {
//FIXME: why using @ instead of $delim???
//FIXME: should we use $catchall_alias instead of *?  does * work for all backends?
            $userid = '*@' . $domain;
            $usertitle = $userid;
         }
         else
         {
//FIXME: should we use $catchall_alias instead of *?  does * work for all backends?
            $userid = '*';
//FIXME: why using @ instead of $delim???
            $usertitle = "*@$domain";
         }
      }
      else
      {
//FIXME: why using @ instead of $delim???
         $usertitle = "$userid@$domain";
      }
      $can_edit = vadmin_get_edit_perms($domain, $username, $desig,
                                          $usertitle);
      if ($can_edit && !is_alias($a))
      {
         $a[13] = vadmin_get_user_webmail_stats($a[0], $domain);
      }

   }

   if (!isset($b[13]['LAST_LOGIN']))
   {

      // try to get user webmail stats
      //
      $userid = $b[0];
      if ($userid == $catchall_alias){
         if ($sort_add_delim)
         {
//FIXME: why using @ instead of $delim???
//FIXME: should we use $catchall_alias instead of *?  does * work for all backends?
            $userid = '*@' . $domain;
            $usertitle = $userid;
         }
         else
         {
//FIXME: should we use $catchall_alias instead of *?  does * work for all backends?
            $userid = '*';
//FIXME: why using @ instead of $delim???
            $usertitle = "*@$domain";
         }
      }
      else
      {
//FIXME: why using @ instead of $delim???
         $usertitle = "$userid@$domain";
      }
      $can_edit = vadmin_get_edit_perms($domain, $username, $desig,
                                          $usertitle);
      if ($can_edit && !is_alias($b))
      {
         $b[13] = vadmin_get_user_webmail_stats($b[0], $domain);
      }

   }

   if (!isset($a[13]['LAST_LOGIN']) || !isset($b[13]['LAST_LOGIN']))
   {

      // sort aliases together
      //
      if (!is_alias($a) && is_alias($b))
         return -1;
      if (is_alias($a) && !is_alias($b))
         return 1;

      // superusers...
      //
      if (!isset($a[13]['LAST_LOGIN']) && !is_alias($b))
         return 1;
      if (!isset($b[13]['LAST_LOGIN']) && !is_alias($a))
         return -1;

      // otherwise sort by name
      //
      return strcasecmp($a[0], $b[0]);
   }


   // "Never" should get pushed to the end of the list
   //
   if ($a[13]['NEVER_LOGGED_IN'] && !$b[13]['NEVER_LOGGED_IN'])
      return 1;
   if (!$a[13]['NEVER_LOGGED_IN'] && $b[13]['NEVER_LOGGED_IN'])
      return -1;
   if ($a[13]['NEVER_LOGGED_IN'] && $b[13]['NEVER_LOGGED_IN'])
      return strcasecmp($a[0], $b[0]);


   // just compare regular login times!
   //
   if ($a[13]['LAST_LOGIN'] < $b[13]['LAST_LOGIN'])
      return 1;

   if ($a[13]['LAST_LOGIN'] > $b[13]['LAST_LOGIN'])
      return -1;

   return strcasecmp($a[0], $b[0]);
}

