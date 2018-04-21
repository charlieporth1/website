<?php
/**
 * vchpass.mod
 * ------------
 * This module lets users change their passwords.
 *
 * Licensed under GNU GPL v2. Please see COPYING for full terms.
 *
 * $Id: vchpass.mod,v 1.7 2008/07/24 18:18:24 pdontthink Exp $
 * 
 * @author Konstantin Riabitsev ($Author: pdontthink $)
 * @version $Date: 2008/07/24 18:18:24 $
 */

$ACT = vadmin_getvar('VADMIN', 'ACT');
$LVL = vadmin_getvar('VADMIN', 'LVL');
$MOD = 'vchpass';
$me = "$MOD.mod/$ACT";
spew("$me: taking over");

$domain = vadmin_getdomain();
$username = vadmin_getvar('SQMAIL', 'username');
spew("$me: making sure they are in the same domain");
list($uname, $udomain) = vadmin_get_user_unamedomain($username);
//FIXME: the check below seems useless because the code above seems to do the same thing twice
if ($udomain != $domain){
    spew("$me: You're not supposed to be here! Bad boy!");
    vadmin_security_breach();
}

/**
 * Make sure this feature is enabled
 */
$passchange = vadmin_get_pref($domain, 'password_change');
if (!$passchange) {
    spew("$me: You're not supposed to be here! Bad boy!");
    vadmin_security_breach();
}

$color = vadmin_getvar('SQMAIL', 'color');

switch ($ACT){
    /******************************* main *******************************/    
 case 'main':
     $action = vadmin_mkform_action($LVL, $MOD, 'setpass');
     $body = ''
         . '<form method="POST" action="' . $action . '">'
         . ' <table border="0" width="60%" align="center">'
         . '  <tr>'
         . '   <th colspan="2" bgcolor="' . $color[9] . '">'
         .      _("Change Password") 
         . '   </th>'
         . '  </tr>'
         . '  <tr>'
         . '   <td align="right">' . _("Old Password") . ': </td>'
         . '   <td align="left"><input type="password" name="oldpass" /></td>'
         . '  </tr>'
         . '  <tr>'
         . '   <td align="right">' . _("New Password") . ': </td>'
         . '   <td align="left"><input type="password" name="newpass" /></td>'
         . '  </tr>'
         . '  <tr>'
         . '   <td align="right">' . _("Repeat Password") . ': </td>'
         . '   <td align="left"><input type="password" name="reppass" />'
         . '  </tr>'
         . '  <tr>'
         . '   <th colspan="2" bgcolor="' . $color[9] . '">'
         . '    <input type="submit" value="' 
         .       _("Change Password") . ' &gt;&gt;" />'
         . '   </th>'
         . '  </tr>'
         . ' </table>'
         . '</form>';
     $title = _("Change your password");
     vadmin_make_page($title, null, $body, false, false);
     break;

     /***************************** setpass ***************************/
 case 'setpass':
     $oldpass = vadmin_getvar('POST', 'oldpass');
     $newpass = vadmin_getvar('POST', 'newpass');
     $reppass = vadmin_getvar('POST', 'reppass');
//TODO: added in SM 1.4.11 and 1.5.1; change this after a while longer (instead of next three lines):  $clear_key = sqauth_read_password();
     $key = $_COOKIE{'key'};
     $pad = $_SESSION{'onetimepad'};
     $clear_key = OneTimePadDecrypt($key, $pad);

     if ($oldpass != $clear_key){
         vadmin_user_error(_("Your old password did not match"));
     }
     if ($newpass != $reppass){
         vadmin_user_error(_("New password did not match repeat password"));
     }
     if ($newpass == $clear_key){
         vadmin_user_error(_("New password is the same as old one"));
     }

     /**
      * Verify password integrity
      */
     $valid_pwd = vadmin_validate_password($newpass, $domain, $uname, FALSE);
     if ($valid_pwd[0]) {
         vadmin_user_error($valid_pwd[1]);
     }
// old code...
//     if (preg_match("/[\W]/i", $newpass)){
//         vadmin_user_error(_("Invalid characters in password. Passwords can only contain latin characters and numbers"));
//     }
//     if (strlen($newpass) < 6){
//         vadmin_user_error(_("Password must be at least 6 characters long"));
//     }

     /**
      * Get the domain password.
      */
     $secret = vadmin_get_domain_passwd($domain);
     if(!vadmin_domain_passwd_validate($domain, $secret)){
         vadmin_system_error(_("Sorry, but this service is not available at this time. Please try again later, or notify somebody in charge that this feature is not working."));
     }
 
     /**
      * Call vchpass(), provided by vmail.inc.
      */
     $repl = vchpass($domain, $secret, $uname, $newpass);
     if ($repl[0]){
         vadmin_system_error($repl[1]);
     }
     /**
      * Update the cookie so the user can continue with his SquirrelMail
      * session without having to re-login.
      * Get out of the plugins dir to set it.
      */
//TODO: added in SM 1.4.16 and 1.5.1; change this after a while longer (instead of next two lines and one above):  sqauth_save_password($newpass);
     $base_uri = sqm_baseuri();
     vadmin_setcookie('key', OneTimePadEncrypt($newpass, $pad), 0, $base_uri);
     
     $body = ''
         . '<table border="0" width="50%" align="center">'
         . ' <tr>'
         . '  <th bgcolor="' . $color[9] . '">'
         .     _("Password changed successfully")
         . '  </th>'
         . ' </tr>'
         . ' <tr>'
         . '  <td align="center">'
         .     _("Your password was changed successfully. Next time you log in, please use your new password.")
         . '  </td>'
         . ' </tr>'
         . ' <tr>'
         . '  <td bgcolor="' . $color[9] . '">&nbsp;</td>'
         . ' </tr>'
         . '</table>';

     $title = _("Password changed");
     vadmin_make_page($title, null, $body, false, false);
     break;

   /******************************* default ***************************/
 default:
     vadmin_system_error(
         sprintf(_("Invalid request, handler for '%s' does not exist"), $ACT));
     break;
}

echo '</body></html>';

/**
 * For emacs weenies among us:
 * Local variables:
 * mode: php
 * End:
 */
