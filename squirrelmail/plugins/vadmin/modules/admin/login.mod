<?php
/**
 * login.mod
 * ----------
 * Ah, the very first module loaded. Brings up all those funny
 * dialogs about providing passwords and such.
 *
 * Licensed under GNU GPL v2. Please see COPYING for full terms.
 *
 * $Id: login.mod,v 1.39 2010/03/05 19:54:43 pdontthink Exp $
 *
 * @author Konstantin Riabitsev ($Author: pdontthink $)
 * @version $Date: 2010/03/05 19:54:43 $
 */

$LVL = vadmin_getvar('VADMIN', 'LVL');
$ACT = vadmin_getvar('VADMIN', 'ACT');
$MOD = 'login';

$me = "$MOD.mod/$ACT";
spew("$me: taking over");

switch ($ACT){
    /***************************** main *********************************/
 case 'main':
     /**
      * Display the main login screen.
      */
     $username    = vadmin_getvar('SQMAIL', 'username');
     $domain      = vadmin_getdomain();
     $current_hostname = preg_replace('/:.*/s', '', vadmin_getvar('SERVER', 'HTTP_HOST'));
     $designation = vadmin_getvar('SESSION', 'VADMIN_AUTHCODE');
     spew("$me: designation is '$designation'");
     $action = vadmin_mkform_action($LVL, $MOD, 'check');
     $str = ''
         . '<form method="post" action="%s">'
         . ' <table border="0" align="center" width="50%%">'
         . '  <tr>'
         . '   <td align="center">';
     $body = sprintf($str, $action);
     $user_auth_note = _("<strong>Password</strong> is the same as your <em>mailbox password</em> you used during login procedure. You are asked to type it in second time as an extra security precaution.");
     $system_auth_note = _("<strong>Password</strong> is the <em>system password</em> corresponding to the domain you have selected. Once you type it in, it will be stored on the server in an encrypted format until you sign out of the vadmin interface.");

     // Before we present the login screen, we take this moment
     // to re-synch the real domain list with the list of crosses
     // and lowlies (if a domain was removed from the machine,
     // it need to be removed from those Vadmin data stores)
     //
     vadmin_resynchronize_admins();

     /**
      * Figure out what to do.
      */
     switch ($designation){
     case 'LOWLY':
         /**
          * "LOWLY" means a lowly admin.
          * They only have permission to edit users in the current domain.
          */
         spew("$me: This is a lowly admin");
         $str = '<input type="hidden" name="vdomain" value="%s"/>'
             . '<strong>%s</strong>';
         $body .= sprintf($str, $domain, sprintf(_("Domain: %s"), $domain));
         break;
     case 'CROSS':
         /**
          * "CROSS" is a cross-admin. They have a drop-down selection of
          * domains they can edit. Get the list.
          */
         spew("$me: This is a cross-admin");
         $username = vadmin_getvar('SQMAIL', 'username');
         $domain_ary = vadmin_get_xdomain_array($username);
         if (sizeof($domain_ary) < 2) {
             $str = '<input type="hidden" name="vdomain" value="%s"/>'
                 . '<strong>%s</strong>';
             $body .= sprintf($str, $domain_ary[0], sprintf(_("Domain: %s"), 
                                                     $domain_ary[0]));
         } else {
             $body .= sprintf('<strong>%s: <select name="vdomain">', _("Domain"));
             natcasesort($domain_ary);
             foreach ($domain_ary as $domain){

                 // default select the current login domain for convenience
                 //
                 if ($domain == $current_hostname) $selected = ' selected="yes"';
                 else $selected = '';

                 $body .= sprintf('<option%s>%s</option>', $selected, $domain);
             }
             $body .= '</select>';
         }
         break;
     case 'ELVIS':
         /**
          * This is elvis! Display the list of all configured virtual domains
          * so the king can pick one.
          */
         spew("$me: This is elvis! Long live the king, baby!");
         $domain_ary = vadmin_list_virtual_domains();
         if (sizeof($domain_ary) < 2) {
             $str = '<input type="hidden" name="vdomain" value="%s"/>'
                 . '<strong>%s</strong>';
             $body .= sprintf($str, $domain_ary[0], sprintf(_("Domain: %s"), 
                                                     $domain_ary[0]));
         } else {
             $body .= sprintf('<strong>%s: <select name="vdomain">', _("Domain"));
             natcasesort($domain_ary);
             foreach ($domain_ary as $domain){

                 // default select the current login domain for convenience
                 //
                 if ($domain == $current_hostname) $selected = ' selected="yes"';
                 else $selected = '';

                 $body .= sprintf('<option%s>%s</option>', $selected, $domain);
             }
             $body .= '</select>';
         }
         break;
     }
     $str = ''
         . ' </td>'
         . '</tr>'
         . '<tr>'
         . ' <td align="center">'
         . '  <strong>%s:</strong> '
         . '  <input type="password" name="vsecret" />'
         . ' </td>'
         . '</tr>'
         . '<tr>'
         . ' <td align="center">'
         . '  <input type="submit" value="%s &gt;&gt;"/>'
         . ' </td>'
         . '</tr>'
         . '<tr>'
         . ' <td><hr/></td>'
         . '</tr>'
         . '<tr>'
         . ' <td>';
     $body .= sprintf($str, _("Password"), _("Log in"));
     $auth_method = vadmin_getvar('CONFIG', 'auth.method');
     if ($designation == 'LOWLY' || strtolower($auth_method) == 'user'){
         $body .= $user_auth_note;
     } else {
         $body .= $system_auth_note;
     }
     $body .= ''
         . ' </td>'
         . '</tr>'
         . '<tr>'
         . ' <td><hr/></td>'
         . '</tr>'
         . '</table></form>';
     vadmin_make_page(_("Administrator Login"), null, $body, false, false);
     break;

 /******************************* check *******************************/
 case 'check':
     $AUTHCODE    = vadmin_getvar('SESSION', 'VADMIN_AUTHCODE');
     $vdomain     = vadmin_getvar('POST', 'vdomain');
     $vsecret     = vadmin_getvar('POST', 'vsecret');
     $domain      = vadmin_getdomain();

     /**
      * Make sure nobody is trying to be nasty by hand-coding the admin domain
      * names before we authorize them.
      */
     if ($AUTHCODE == 'LOWLY'){
         if ($vdomain != $domain){
             /**
              * This person tried to override the domain name in the
              * POST/GET data.  Kick them in the shins and spank them
              * until they see the error of their ways.
              */
             spew("$me: This person tried to override the hard-coded domain");
             vadmin_security_breach();
         }
     }
     
     if ($AUTHCODE == 'CROSS'){
         /**
          * Make sure this domain is actually in the list of domains in this
          * admin's name.
          */
         spew("$me: Checking whether this cross-admin is allowed in domain");
         $username = vadmin_getvar('SQMAIL', 'username');
         $domain_ary = vadmin_get_xdomain_array($username);
         if (!in_array($vdomain, $domain_ary)){
             /**
              * This cross-admin tried to override the list of available
              * domains by hand. Beat the snot out of them.
              */
             spew("$me: Nope. Kicking them off.");
             vadmin_security_breach();
         }
     }

     /**
      * Make sure this domain exists in virtual domains.
      */
     spew("$me: Checking among existing virtual domains");
     $domain_ary = vadmin_list_virtual_domains();
     if (!in_array($vdomain, $domain_ary)){
         $error = sprintf(_("Domain '%s' doesn't seem to be configured as a virtual domain"), $vdomain);
         vadmin_system_error($error);
     }

     /**
      * If this is a lowly admin, OR if the authentication method for
      * cross-admins is set to "user", the $vsecret will be this admin's
      * mailbox password. Get the password from squirrelmail and try to 
      * verify.
      */
     $domain_enabled = true;
     $auth_method = vadmin_getvar('CONFIG', 'auth.method');
     if ($AUTHCODE == 'LOWLY' || $auth_method == 'user'){
         spew("$me: verifying the mailbox password");
//TODO: added in SM 1.4.11 and 1.5.1; change this after a while longer (instead of next three lines):  $clear_key = sqauth_read_password();
         $key = $_COOKIE{'key'};
         $pad = $_SESSION{'onetimepad'};
         $clear_key = OneTimePadDecrypt($key, $pad);
         if ($vsecret == $clear_key){
             spew("$me: Mailbox password verified");
             spew("$me: Checking if the domain exists");
             if (vadmin_domain_exists($vdomain)){
                 /**
                  * Ok! Now check to see whether we have passwd file
                  * for this domain. If we do get one, then load it
                  * into $vsecret so we can continue this module.
                  */
                 $vsecret = vadmin_get_domain_passwd($vdomain);
                 if (!isset($vsecret) || !strlen($vsecret)){
                     /**
                      * Nope. Since it makes no sense to work in
                      * AUTH_METHOD="USER" without a stored password,
                      * load the askpasswd module, but only if this is
                      * not a LOWLY.
                      */
                     spew("$me: Could not get domain password from system!");
                     if ($AUTHCODE == 'LOWLY'){
                         $error = sprintf(_("Sorry, domain password for %s is not stored within the system. Please contact your systems administrator."), $vdomain);
                         vadmin_system_error($error);
                     } else {
                         spew("$me: switching to asking a password");
                         vadmin_redirect($LVL, 'login', 'askdompass', 
                                         $vdomain);
                     }
                 }
             } else {
                 spew("$me: domain is not enabled. Try upgrading?");
                 $domain_enabled = vadmin_upgrade_v1($vdomain);
             }
         } else {
             vadmin_user_error(_("The password you provided is incorrect."));
         }
     } else {
         if(!vadmin_domain_passwd_validate($vdomain, $vsecret)){
             spew("$me: telling them to try again.");
             vadmin_user_error(_("The password you provided did not verify."));
         }
         if (!vadmin_domain_exists($vdomain)){
             $domain_enabled = false;
         }
     }

     /**
      * Now make sure the requested domain is configured.
      */
     spew("$me: checking if this domain is configured");
     if ($domain_enabled == false){
         spew("$me: This domain is not enabled within vadmin");
         if ($AUTHCODE == 'ELVIS'){
             spew("$me: Redirecting to the domain setup page");
             vadmin_redirect($LVL, 'login', 'setdomainask', $vdomain);
         } else {
             spew("$me: erring out");
             $error = sprintf(_("Domain %s is not enabled for administration using this interface. Please ask your administrator to configure it first."), $vdomain);
             vadmin_system_error($error);
         }
     }
     
     if ($AUTHCODE == 'LOWLY' || $auth_method == 'user'){
         if (!vadmin_domain_passwd_validate($vdomain, $vsecret)){
             spew("$me: the password did NOT verify");
             /**
              * See if we need to zero out the password.
              */
             if ($AUTHCODE == 'LOWLY'){
                 spew("$me: Stored password did not verify! Removing it");
                 vadmin_put_domain_passwd($vdomain, false);
                 $msg = _("The stored password file was invalid and has been removed. Please contact your systems administrator.");
                 vadmin_system_error($msg);
             }
             /**
              * This is a CROSS or ELVIS with auth=user. This still
              * means that the stored password is incorrect, but we
              * can redirect them to the askdompass instead of just
              * booting them off.
              */
             spew("$me: Stored password did not verify! Asking again.");
             vadmin_put_domain_passwd($vdomain, false);
             vadmin_redirect($LVL, 'login', 'askdompass', $vdomain);
         }
     }
     
     /**
      * Set the session data, encrypting it before sticking into the
      * apache-readable /tmp/.sess*. The things I do out of my
      * paranoia.. ;)
      */
     vadmin_putvar('VADMIN', 'VADMIN_DOMAIN', $vdomain);
     spew("$me: stor VADMIN_AUTHCODE and VADMIN_SECRET in session, encrypted");
     vadmin_putvar('SESSION', 'VADMIN_AUTHCODE', 
                   vadmin_crypto($vdomain, 'encrypt'));
     vadmin_putvar('SESSION', 'VADMIN_SECRET', 
                   vadmin_crypto($vsecret, 'encrypt'));
     list($start_mod, $start_act) = vadmin_get_startup_location();
     vadmin_redirect($LVL, $start_mod, $start_act, null);
     break;
     
     /****************************** askdompass ***************************/
 case 'askdompass':
     spew("$me: Grabbing stored domain from redirect_stor");
     $vdomain = vadmin_get_storparams();

     // if password entered incorrectly, pressing the back button as
     // told to do by instructions results in the redirect domain parameter
     // being missing -- TODO/FIXME: is it actually OK to assume that the
     // user's domain is what we want?  It might have been something else!
     //
     if (empty($vdomain))
         $vdomain = vadmin_getdomain();

     $AUTHCODE = vadmin_getvar('SESSION', 'VADMIN_AUTHCODE');
     if ($AUTHCODE == 'LOWLY'){
         /** 
          * This is a lowly admin.
          */
         $msg = _("There is no stored password for this domain. Your admin will have to first finish all needed configuration for this domain, and only then you will be able to log in and administer users. Please contact the person in charge and remind them about this problem.");
         $body = "<p>$msg</p>";
         $title = _("Cannot continue");
     } else {
         /** 
          * This is either a Cross-admin or an Elvis.
          */
         $msg = _("You will need to store the domain password on the server before you can proceed. It will be stored in an encrypted format, so it's not easily hijacked. Please provide the password in the form below. If you don't know the password, please contact the people who do so they can set it up to be stored on the server.");
         $formact = vadmin_mkform_action('admin', 'login', 'dompasscheck');
         $str = '<p>%s</p>'
             . '<form method="post" action="%s">'
             . ' <input type="hidden" name="vdomain" value="%s" />'
             . ' <table border="0" align="center">'
             . '  <tr>'
             . '   <th bgcolor="%s">%s</th>'
             . '  </tr>'
             . '  <tr>'
             . '   <td align="center">'
             . '    %s <input type="password" name="vsecret">'
             . '   </td>'
             . '  </tr>'
             . '  <tr>'
             . '   <td align="center" bgcolor="%s">'
             . '    <input type="submit" value="%s &gt;&gt;"/>'
             . '   </td>'
             . '  </tr>'
             . ' </table>'
             . '</form>';
         $body = sprintf($str, $msg, $formact, $vdomain, $color[9],
                          sprintf(_("Domain: %s"), $vdomain), _("Domain Password:"),
                          $color[9], _("Store Password"));
         $title = sprintf(_("Need domain password for '%s'"), $vdomain);
     }
     vadmin_make_page($title, null, $body, false, false);
     break;

 /****************************** dompasscheck **************************/
 case 'dompasscheck':
     $username = vadmin_getvar('SQMAIL', 'username');
     $vdomain  = vadmin_getvar('POST', 'vdomain');
     $vsecret  = vadmin_getvar('POST', 'vsecret');
     spew("$me: making sure '$username' has access to '$vdomain'");
     $designation = vadmin_get_user_designation($vdomain, $username);
     if ($designation == 'NONER' || $designation == 'LOWLY'){
         /**
          * They have no right to be accessing this page. Kick them.
          */
         spew("$me: '$username' is not ELVIS or CROSS for '$vdomain'. Denied");
         vadmin_security_breach();
     }
     /**
      * Verify if the password is correct.
      */
     spew("$me: validating the password provided.");
     if (vadmin_domain_passwd_validate($vdomain, $vsecret)){
         spew("$me: password validates, storing.");
         vadmin_put_domain_passwd($vdomain, $vsecret);
         /**
          * Set the session data, encrypting it before sticking into the
          * apache-readable /tmp/.sess*. The things I do out of my
          * paranoia.. ;)
          */
         spew("$me: storing data in session, encrypted");
         vadmin_putvar('SESSION', 'VADMIN_AUTHCODE', 
                       vadmin_crypto($vdomain, 'encrypt'));
         vadmin_putvar('SESSION', 'VADMIN_SECRET', 
                       vadmin_crypto($vsecret, 'encrypt'));
         list($start_mod, $start_act) = vadmin_get_startup_location($designation);
         vadmin_redirect($LVL, $start_mod, $start_act, null);
     } else {
         spew("$me: nope, the password is still incorrect.");
         $err = _("The password you provided did not validate.");
         vadmin_user_error($err);
     }
     break;
     /***************************** needhttps ******************************/
 case 'needhttps':
     $str = ''
         . '<table border="0" width="70%%" align="center">'
         . ' <tr>'
         . '  <th bgcolor="%s">%s</th>'
         . ' </tr>'
         . ' <tr>'
         . '  <td align="center">'
         . '   <p style="width: 90%%; text-align: left">%s</p>'
         . '  </td>'
         . ' </tr>'
         . ' <tr>'
         . '  <th bgcolor="%s">&nbsp;</th>'
         . ' </tr>'
         . '</table>';
     $msg = _("Sorry, but you cannot login over a clear-text HTTP connection. Please sign out and login using a secure HTTPS server instead. This is for your own protection. Thank you!");
     $body = sprintf($str, $color[9], _("Secure connection required"), $msg,
                     $color[9]);
     $title = _("Cannot log in at this time");
     vadmin_make_page($title, null, $body, false, false);
     break;
     
     /****************************** setdomainask **************************/
 case 'setdomainask':
     /**
      * Only elvis can be here.
      */
     $vdomain  = vadmin_get_storparams();
     $AUTHCODE = vadmin_getvar('SESSION', 'VADMIN_AUTHCODE');
     if ($AUTHCODE != 'ELVIS'){
         spew("$me: hey! You're not elvis!");
         vadmin_security_breach();
     }
     $action = vadmin_mkform_action($LVL, $MOD, 'enabledomain');
     $msg = sprintf(_("Domain <strong>'%s'</strong> is currently not enabled for administration using this interface. If you would like to enable it, then click the button below, otherwise return to the previous page and choose a different domain."), $vdomain);
     spew("$me: checking if there are domain templates defined");
     $templates = vadmin_get_template_list();
     if (count($templates)){
         $tmplmsg = ''
             . _("Use this template") . ': '
             . '<select name="tmplname">';
         foreach ($templates as $template){
             $tmplmsg .= '<option>' . $template . '</option>';
         }
         $tmplmsg .= '</select></p><p style="text-align: center">';
     } else {
         $tmplmsg = '';
     }
     $str = ''
         . '<table border="0" width="70%%" align="center">'
         . ' <tr>'
         . '  <th bgcolor="%s">%s</th>'
         . ' </tr>'
         . ' <tr>'
         . '  <td align="center">'
         . '   <p style="width: 90%%; text-align: left">%s</p>'
         . '   <form method="post" action="%s">'
         . '    <input type="hidden" name="vdomain" value="%s"/>'
         . '    <p style="text-align: center">%s'
         . '     <input type="submit" value="%s"/>'
         . '    </p>'
         . '   </form>'
         . '  </td>'
         . ' </tr>'
         . ' <tr>'
         . '  <th bgcolor="%s">&nbsp;</th>'
         . ' </tr>'
         . '</table>';
     $body = sprintf($str, $color[9], _("Domain not enabled"), $msg, $action,
                     $vdomain, $tmplmsg, sprintf(_("Enable %s"), $vdomain),
                     $color[9]);
     $title = sprintf(_("%s is not enabled"), $vdomain);
     vadmin_make_page($title, null, $body, false, false);
     break;

   /***************************** enabledomain ***************************/
 case 'enabledomain':
     /**
      * Only elvis can be here!
      */
     $AUTHCODE = vadmin_getvar('SESSION', 'VADMIN_AUTHCODE');
     if ($AUTHCODE != 'ELVIS'){
         spew("$me: hey! You're not elvis!");
         vadmin_security_breach();
     }
     spew("$me: Making sure this domain exists in the list of domains");
     $vdomain = vadmin_getvar('POST', 'vdomain');
     $domain_ary = vadmin_list_virtual_domains();
     if (!in_array($vdomain, $domain_ary)){
         spew("$me: $vdomain not found in virtualdomains!");
         $error = sprintf(_("Sorry, the domain name you supplied, %s, is not configured among the virtual domains on this system"), $vdomain);
         vadmin_system_error($error);
     }
     $result = vadmin_enable_domain($vdomain);
     /**
      * Process the template if we have any
      */
     $tmplname = vadmin_getvar('POST', 'tmplname');
     if (isset($tmplname) && $tmplname){
         spew("$me: found template '$tmplname'");
         $contents = vadmin_get_template($tmplname);
         $limits = array('mailboxes', 'hardquota', 'size', 'count',
                         'imgsize');
         $prefs = array('password_change', 'autoresponder', 'mail_forwarding', 'track_usage');
         foreach ($limits as $limit){
             if (isset($contents{$limit}) && $contents{$limit}){
                 spew("$me: setting limit '$limit' to ".$contents{$limit});
                 vadmin_put_limit($vdomain, 'CROSS', $limit, 
                                  $contents{$limit});
             }
         }
         foreach ($prefs as $pref){
             if (isset($contents{$pref}) && $contents{$pref}){
                 spew("$me: setting pref '$pref' to ".$contents{$pref});
                 vadmin_put_pref($vdomain, $pref, $contents{$pref});
             }
         }
     }
     spew("$me: domain enabled successfully");
     /**
      * If auth.method is 'user', then bounce them to set the password.
      */
     $auth_method = vadmin_getvar('CONFIG', 'auth.method');
     if ($auth_method == 'user'){
         spew("$me: Redirecting them to set the password.");
         vadmin_redirect($LVL, 'login', 'askdompass', $vdomain);
     }
     $str = ''
         . '<table border="0" width="70%%" align="center">'
         . ' <tr>'
         . '  <th bgcolor="%s">%s</th>'
         . ' </tr>'
         . ' <tr>'
         . '  <td align="center">'
         . '   <p style="width: 90%; text-align: left">%s</p>'
         . '  </td>'
         . ' </tr>'
         . ' <tr>'
         . '  <th bgcolor="%s">&nbsp;</th>'
         . ' </tr>'
         . '</table>';
     $msg = sprintf(_("Domain <strong>'%s'</strong> enabled successfully. Click the 'Admin' link at the top and log in again to access the domain."), $vdomain);
     $title = _("Domain enabled successfully");
     
     $body = sprintf($msg, $color[9], $title, $msg, $color[9]);
     vadmin_make_page($title, null, $body, false, false);
     break;


     /****************************** default *******************************/
 default:
     vadmin_system_error(sprintf(_("Invalid request, handler for '%s' does not exist"), $ACT));
     break;
}

echo '</body></html>';

/**
 * For the emacs weenies among us.
 * Local variables:
 * mode: php
 * End:
 */

