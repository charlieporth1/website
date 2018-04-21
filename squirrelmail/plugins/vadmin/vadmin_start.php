<?php
/**
 * vadmin_start.php
 * ----------------
 * Entry point for the vadmin plugin. This file is accessed
 * by all plugin hook calls registered in setup.php.
 *
 * Licensed under GNU GPL v2. Please see COPYING for full terms.
 *
 * $Id: vadmin_start.php,v 1.18 2009/09/05 02:45:10 pdontthink Exp $
 *
 * @author Konstantin Riabitsev ($Author: pdontthink $)
 * @version $Date: 2009/09/05 02:45:10 $
 */

/**
 * This function gets us started; loads up Vadmin and
 * its backend configuration settings as well as all
 * needed code libraries.
 *
 * @return void
 */
function vadmin_prep(){
    include_once(SM_PATH . 'plugins/vadmin/config_parser.php');
    /**
     * Parse the main and backend config files.
     */
    $config_path = vadmin_get_conf_location();
    $config_file = 'vadmin.conf.php';

    $vadmin_config = vadmin_parse_config($config_path . $config_file);
    $GLOBALS{'VADMIN_CONFIG'} = $vadmin_config;
    $vadmin_backend_config = vadmin_parse_config($config_path
                             . $vadmin_config{'backend'}{'type'} . '.conf.php');
    $GLOBALS{'VADMIN_BACKEND_CONFIG'} = $vadmin_backend_config;
    /**
     * Load main functions.
     */
    $include_dir = $vadmin_config{'paths'}{'includes'};
    include_once($include_dir . '/vadmin_functions.inc');
    /**
     * See what setting is storage.type set to and load the needed
     * file. This is the only place where error checking is implemented
     * since many different storage types are possible.
     */
    $storage_type = vadmin_getvar('CONFIG', 'storage.type');
    $storage_functions_file = sprintf('%s/%s_functions.inc', 
                                      $include_dir, $storage_type); 
    if (file_exists($storage_functions_file) 
        && is_readable($storage_functions_file)){
        include_once($storage_functions_file);
    } else {
        /**
         * Don't fail. Just return.
         */
        return;
    }
}

/**
 * This function places an optional "Admin" link in the main 
 * "menuline" at the top of the main frame in SquirrelMail.
 * The user has to have enabled its display for it to show up,
 * however.
 *
 * @return void
 */
function vadmin_menulink_do(){
    vadmin_prep();
    $domain = vadmin_getdomain();
    if (!empty($domain)) {
        sq_change_text_domain('vadmin');
        $AUTHCODE = vadmin_auth();
        $data_dir  = vadmin_getvar('SQMAIL', 'data_dir');
        $user  = vadmin_getvar('SQMAIL', 'username');
        $vadmin_in_menubar = getPref($data_dir, $user, 'vadmin_in_menubar', 0);
        $accesskey = getPref($data_dir, $user, 'accesskey_vadmin_menubar', 'NONE');
        if ($vadmin_in_menubar && $AUTHCODE != 'NONER'){

            // output is different for 1.5.2+
            //
            if (check_sm_version(1, 5, 2)) {
                global $oTemplate, $nbsp;
                $output = makeInternalLink('plugins/vadmin/vadmin_main.php?LVL=admin', _("Admin"), '', $accesskey)
                        . $nbsp . $nbsp;
                sq_change_text_domain('squirrelmail');
                return array('menuline' => $output);
            } else {
                displayInternalLink('plugins/vadmin/vadmin_main.php?LVL=admin', _("Admin"));
                echo "&nbsp;&nbsp;\n";
            }
        }

        /**
         * Rebind back to squirrelmail
         */
        sq_change_text_domain('squirrelmail');
    }
}

/**
 * This function hooks in immediately after login in order to
 * log user's webmail usage, if allowable.
 *
 * @return void
 */
function vadmin_log_login_do(){

   global $username, $data_dir;
   $domain = vadmin_getdomain();

   if (!empty($domain))
   {
      list($uname, $udomain) = vadmin_get_user_unamedomain($username);
//FIXME: $domain and $udomain seem to ultimately come from the same place - are they supposed to be different?
      if (vadmin_domain_passwd_issaved($domain) && $domain == $udomain)
      {
         $trackusage = vadmin_get_pref($domain, 'track_usage');
         if ($trackusage != false)
         {
            $logins = getPref($data_dir, $username, 'vadmin_total_logins');
            setPref($data_dir, $username, 'vadmin_total_logins', ++$logins);
            setPref($data_dir, $username, 'vadmin_last_login_date', time());
         }
      }
   }
}

/**
 * This function hooks in very early in every page request which
 * allows Vadmin the ability to override SquirrelMail configuration
 * settings or enable/disable plugins, etc.
 *
 * @return void
 */
function vadmin_override_config_do($args){
    vadmin_prep();
    $domain = vadmin_getdomain();
    if (!empty($domain)){

        // get out of session since the username is not
        // globalized yet when called during the
        // loading_constants hook
        $username = vadmin_getvar('SQMAIL_SESSION', 'username');
        list($uname, $udomain) = vadmin_get_user_unamedomain($username);
//FIXME: $domain and $udomain seem to ultimately come from the same place - are they supposed to be different?
        if (vadmin_domain_passwd_issaved($domain) && $domain == $udomain){

            $passchange = vadmin_get_pref($domain, 'password_change');
            $autores = vadmin_get_pref($domain, 'autoresponder');
            $mailfwd = vadmin_get_pref($domain, 'mail_forwarding');
            $pass_plugin = vadmin_getvar('CONFIG', 'backend.user_password_plugin');
            $autores_plugin = vadmin_getvar('CONFIG', 'backend.user_autoresponder_plugin');
            $mail_fwd_plugin = vadmin_getvar('CONFIG', 'backend.user_mail_forwarding_plugin');

            if ($passchange != false 
             && !empty($pass_plugin) && $pass_plugin != 'internal'){

                // this function is found in the Compatibility plugin
                add_plugin($pass_plugin, $args);
            }

            if ($autores != false 
             && !empty($autores_plugin) && $autores_plugin != 'internal'){

                // this function is found in the Compatibility plugin
                add_plugin($autores_plugin, $args);
            }

            if ($mailfwd != false 
             && !empty($mail_fwd_plugin) && $mail_fwd_plugin != 'internal'){

                // this function is found in the Compatibility plugin
                add_plugin($mail_fwd_plugin, $args);
            }

        }

    }
}

/**
 * This function hooks to the options page for the end-users and 
 * allows them to change their passwords or set up an autoresponder.
 * It also places the link to the administrative interface on the
 * options page (for admins only, of course).
 *
 * @return void
 */
function vadmin_options_page_do(){
    vadmin_prep();
    $domain = vadmin_getdomain();
    if (!empty($domain)){
        /**
         * Upgrade check. Will return if not useful
         */
        vadmin_upgrade_v1($domain);
        sq_change_text_domain('vadmin');
        global $optpage_blocks;
        $AUTHCODE = vadmin_auth();
        if ($AUTHCODE != 'NONER'){
            $optpage_blocks[] = 
                array(
                    'name' => _("Administrator Interface"),
                    'url'  => "../plugins/vadmin/vadmin_main.php?LVL=admin",
                    'desc' => _("Access the administrator interface to set up and configure users, mailboxes, and other domain settings."),
                    'js'   => false);
        }


        /**
         * The following two are only useful in certain cases.
         */
        $username = vadmin_getvar('SQMAIL', 'username');
        list($uname, $udomain) = vadmin_get_user_unamedomain($username);
//FIXME: $domain and $udomain seem to ultimately come from the same place - are they supposed to be different?
        if (vadmin_domain_passwd_issaved($domain) && $domain == $udomain){

            $passchange = vadmin_get_pref($domain, 'password_change');
            $autores = vadmin_get_pref($domain, 'autoresponder');
            $mailfwd = vadmin_get_pref($domain, 'mail_forwarding');
            $pass_plugin = vadmin_getvar('CONFIG', 'backend.user_password_plugin');
            $autores_plugin = vadmin_getvar('CONFIG', 'backend.user_autoresponder_plugin');
            $mail_fwd_plugin = vadmin_getvar('CONFIG', 'backend.user_mail_forwarding_plugin');

            // internal user password change setup
            //
            if ($passchange != false
             && !empty($pass_plugin) && $pass_plugin == 'internal'){

                $optpage_blocks[] = 
                    array(
                          'name' => _("Change Password"),
                          'url'  => "../plugins/vadmin/vadmin_main.php?LVL=user&MOD=vchpass&ACT=main",
                          'desc' => _("Change the password that you use to log in and check your e-mail."),
                          'js'   => false);

            }

            // internal user autoresponder setup
            //
//FIXME: this functionality has not been tested nor updated since v3.0 work began
            if ($autores != false
             && !empty($autores_plugin) && $autores_plugin == 'internal'){

                $optpage_blocks[] = 
                    array(
                          'name' => _("Autoresponder"),
                          'url'  => "../plugins/vadmin/vadmin_main.php?LVL=user&MOD=autores&ACT=main",
                          'desc' => _("Set up an auto-reply message for you incoming email. This can be useful, for example, for notifying your correspondents that you are on vacation."),
                          'js'   => false);

            }

            // internal user mail forwarding setup
            //
            if ($mailfwd != false
             && !empty($mail_fwd_plugin) && $mail_fwd_plugin == 'internal'){

                $optpage_blocks[] = 
                    array(
                          'name' => _("Mail Forwarding"),
                          'url'  => "../plugins/vadmin/vadmin_main.php?LVL=user&MOD=mail_forwarding&ACT=main",
                          'desc' => _("Set up other email addresses to which your incoming messages will be forwarded."),
                          'js'   => false);

            }

        }


        /**
         * Rebind back to squirrelmail
         */
        sq_change_text_domain('squirrelmail');
    }
}

/**
 * This function puts a link to the autoresponder below the 
 * SquirrelMail folder list when it is enabled
 *
 * @return void
 */
function vadmin_autores_notify_do(){
    vadmin_prep();
    /**
     * Load vmail.inc.
     */
    $vmail_inc = vadmin_getvar('BACKEND', 'vmail_path');
    include_once($vmail_inc);
    $domain = vadmin_getdomain();
    if (empty($domain)){
        return;
    }
    vadmin_upgrade_v1($domain);

    // don't do anything here unless internal autoresponder is
    // in use and it is enabled for the current user domain
    //
    if (vadmin_get_pref($domain, 'autoresponder') == false
     || vadmin_getvar('CONFIG', 'backend.user_autoresponder_plugin') != 'internal') {
        return;
    }

    $username = vadmin_getvar('SQMAIL', 'username');
    list($uname, $udomain) = vadmin_get_user_unamedomain($username);

    if ($domain && $udomain == $domain){
        $secret = vadmin_get_domain_passwd($domain);
        list($code, $stat) = vautoresponsestatus($domain, $secret, $uname);
        if ($stat == 'enabled'){
            sq_change_text_domain('vadmin');
            echo '<br /><small><center>(<a '
                . ' href="../plugins/vadmin/vadmin_main.php?LVL=user&amp;'
                . 'MOD=autores&amp;ACT=main" target="right">' 
                . _("autoresponder&nbsp;on") . '</a>)</center></small>';
            sq_change_text_domain('squirrelmail');
        }
    }
}

/**
 * This function sets the front page preferences per-domain. It will
 * check if the virtual domain has custom name and picture and will
 * substitute the default ones with them.
 *
 * @return void
 */
function vadmin_login_prefs_do(){

    // not needed because this function is now called from
    // somewhere that has already done this (vadmin_reroute_and_login_prefs_do())
    //vadmin_prep();

    $domain = vadmin_getvar('VADMIN', 'vadmin_user_domain');
    if (empty($domain)){
        $domain = vadmin_getvar('COOKIE', 'vadmin_user_domain');
    }
    if (empty($domain)){
        $domain = vadmin_gethost();
    }
    if (isset($domain) && $domain){
        vadmin_upgrade_v1($domain);
        global $org_name, $org_logo, $org_logo_width, $org_logo_height;
        $contents = vadmin_get_pic($domain);
        $title = vadmin_get_pref($domain, 'title');
        if ($title != false){
            $org_name = $title;
        }
        if ($contents != false){
            $org_logo_width = $org_logo_height = 0;
            $org_logo = '../plugins/vadmin/vadmin_fetch_pic.php?DOM=' 
                . urlencode($domain);
        }
    }
}

/**
 * This function redirects the user to another host/path if
 * needed; usually in order to transparently serve SSL to
 * multiple domains using just one SSL certificate.
 *
 * After that, it sets the front page preferences per-domain. 
 * It will check if the virtual domain has custom name and 
 * picture and will substitute the default ones with them.
 *
 * @return void
 */
function vadmin_reroute_and_login_prefs_do(){

    vadmin_prep();

    $rhost = vadmin_getvar('CONFIG', 'redirect.host');
    $rpath = vadmin_getvar('CONFIG', 'redirect.path');

    // bail if no need to redirect
    //
    if (empty($rhost))
        return vadmin_login_prefs_do();

    $HTTP_HOST = vadmin_getvar('SERVER', 'HTTP_HOST');
    $HTTP_HOST = preg_replace('/:.*/s', '', $HTTP_HOST);
    $need_https = vadmin_getvar('CONFIG', 'redirect.https');
    global $is_secure_connection;
    if ($HTTP_HOST == $rhost && 
        (($need_https == 'yes' && $is_secure_connection)
      || ($need_https == 'no'))) { // Why were we requiring NON-SSL to log in when SSL isn't required?....  && !$is_secure_connection))) {
        /**
         * We are here!
         * Set the cookie if we have a ?domain
         * Cookie life = 31536000 seconds = 365 days
         */
        $domain = vadmin_getvar('GET', 'domain');
        if (!empty($domain)) {
            vadmin_setcookie('vadmin_user_domain', $domain, time()+31536000, 
                             $rpath, $HTTP_HOST);
            /**
             * And stick into VADMIN, so it's immediately accessible.
             */
            vadmin_putvar('VADMIN', 'vadmin_user_domain', $domain);
        }
        return vadmin_login_prefs_do();
    }
    /**
     * Figure out domain
     */
    $domain = vadmin_gethost();
    if (!$domain){
        /**
         * Oh well.
         */
        return vadmin_login_prefs_do();
    }
    /**
     * Redirect.
     */
    $proto = 'http';
    if ($need_https == 'yes'){
        $proto = 'https';
    }
    $loc = sprintf('%s://%s%s%s?domain=%s', $proto, $rhost, $rpath, 
                   'src/login.php', urlencode($domain));
    header('Location: ' . $loc);
    exit;
}

/**
 * This function sets up the vadmin domain cookie
 *
 * @return void
 */
function vadmin_set_user_domain_cookie_do(){
    vadmin_prep();
    $username = vadmin_getvar('SQMAIL', 'username');
    $path = vadmin_getvar('SQMAIL', 'base_uri');
    list($uname, $domain) = vadmin_get_user_unamedomain($username);
    if (!empty($domain)){
        // Cookie life = 31536000 seconds = 365 days
        vadmin_setcookie('vadmin_user_domain', $domain, time()+31536000, $path);
    }
}

/**
  * Returns a list of preference names that should not
  * be controllable by the end user
  *
  * @return string A comma-delimited list of preference
  *                names that the end-user should not
  *                have control over.
  */
function vadmin_sensitive_preference_names_do(){
    return 'vadmin_last_login_date, vadmin_total_logins, ';
}

/**
  * Lets users define different access key/shortcuts
  *
  */
function vadmin_accesskey_options_do() {

   // SquirrelMail 1.4?  bail.
   //
   if (!check_sm_version(1, 5, 2))
      return;

   global $optpage_data, $username, $data_dir, $a_to_z;
   
   // i18n: this string is intentionally in the SquirrelMail domain - no need to translate this here
   $my_a_to_z = array_merge(array('NONE' => _("Not used")), $a_to_z);

   $accesskey_vadmin_menubar = getPref($data_dir, $username, 'accesskey_vadmin_menubar', 'NONE');

   sq_change_text_domain('vadmin');

   $optpage_data['vals'][0][] = array(
      'name'          => 'accesskey_vadmin_menubar',
      'caption'       => _("Admin"),
      'type'          => SMOPT_TYPE_STRLIST,
      'refresh'       => SMOPT_REFRESH_NONE,
      'posvals'       => $my_a_to_z,
      'initial_value' => $accesskey_vadmin_menubar,
   );

   sq_change_text_domain('squirrelmail');

}

