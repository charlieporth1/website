<?php
/**
 * mail_forwarding.mod
 * -------------------
 * This module lets users change their mail forwarding (aliases).
 *
 * Licensed under GNU GPL v2. Please see COPYING for full terms.
 *
 * $Id: mail_forwarding.mod,v 1.3 2008/10/16 03:27:42 pdontthink Exp $
 * 
 * @author Paul Lesniewski ($Author: pdontthink $)
 * @version $Date: 2008/10/16 03:27:42 $
 */

$ACT = vadmin_getvar('VADMIN', 'ACT');
$LVL = vadmin_getvar('VADMIN', 'LVL');
$MOD = 'mail_forwarding';
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
$mailfwd = vadmin_get_pref($domain, 'mail_forwarding');
if (!$mailfwd) {
    spew("$me: You're not supposed to be here! Bad boy!");
    vadmin_security_breach();
}

/**
 * Get the domain password.
 */
$secret = vadmin_get_domain_passwd($domain);
if(!vadmin_domain_passwd_validate($domain, $secret)){
    vadmin_system_error(_("Sorry, but this service is not available at this time. Please try again later, or notify somebody in charge that this feature is not working."));
}

/**
 * Get user info and extract list of forwarding aliases
 */
//FIXME: shouldn't we be using $delim instead of @??
$command = array('lookup', $domain, $uname, $secret);
$tmp = vm_daemon_raw($command);
if ($tmp[0]){
    spew("$me: no such user or alias '$username'");
    $support_aliases = vadmin_getvar('CONFIG', 'backend.support_aliases');
    if ($support_aliases == 'yes')
        $msg = sprintf(_("No such user or alias exists: %s"), $username);
    else
        $msg = sprintf(_("No such user exists: %s"), $username);
    vadmin_user_error($msg);
}

/**
 * Call lookup(), a function provided by vmail.inc.
 */
spew("$me: fetching the user data");
$user_ary = listdomain_parse_userdata($tmp[1], $uname);

/**
 * Aliases are all we want out of it
 */
if (is_array($user_ary[3])){
    $aliases = join(', ', $user_ary[3]);
} else {
    $aliases = '';
}
spew("$me: aliases is '$aliases'");

$color = vadmin_getvar('SQMAIL', 'color');

switch ($ACT){
    /******************************* main *******************************/    
 case 'main':

     $action = vadmin_mkform_action($LVL, $MOD, 'chngfwds');
     $body = ''
         . '<form method="POST" action="' . $action . '">'
         . ' <table border="0" width="60%" align="center">'
         . '  <tr>'
         . '   <th colspan="2" bgcolor="' . $color[9] . '">'
         .      _("Forward mail to (separate by commas):")
         . '   </th>'
         . '  </tr>'
         . '  <tr>'
         . '   <td colspan="2" align="center">'
         . '     <textarea name="valiases" cols="60" rows="2">' . htmlspecialchars($aliases)
         . '</textarea>'
         . '   </td>'
         . '  </tr>'
         . '  <tr>'
         . '   <th colspan="2" bgcolor="' . $color[9] . '">'
         . '    <input type="submit" value="' 
         .       _("Change Forwarding") . ' &gt;&gt;" />'
         . '   </th>'
         . '  </tr>'
         . ' </table>'
         . '</form>';
     $title = _("Change mail forwarding");
     vadmin_make_page($title, null, $body, false, false);
     break;

     /***************************** chngfwds ***************************/
 case 'chngfwds':

     /**
      * Prepare the aliases by collapsing them into a string. This allows us
      * to comare the new values with the old ones.
      */
     if (!sqGetGlobalVar('valiases', $valiases, SQ_POST)) {
         vadmin_user_error(_("Did not find mail forwarding list"));
     }

     // rid ourselves of newlines in user input, they
     // only cause trouble
     //
     $valiases = str_replace(array("\r\n", "\r", "\n"), ',', $valiases);

     spew("$me: (forwards) aliases is: $aliases");
     spew("$me: (forwards) valiases is: $valiases");

     if ($valiases != $aliases){
         /**
          * Change forwards to a new value.
          */
         if (strlen($valiases)){
             $fwds = explode(',', $valiases);
             /**
              * Trim the values to avoid issues with spaces,
              * also remove empty entries
              */
             $forwards = array();
             $forward_errors = array();
             foreach ($fwds as $addr) {
                 $addr = trim($addr);
                 if (!empty($addr)) {
                     $result = validate_restricted_user_mail_forwarding($addr, $domain, $uname, FALSE);
                     if (!$result[0])
                        $forwards[] = $addr;
                     else {
                        $forward_errors[] = $result[1];
                        break;
                     }
                 }
             }
         } else {
             $forwards = array();
         }

         // if there were errors, report 
         //
         if (!empty($forward_errors)) {
             $errormsg = implode(' -- ', $forward_errors);
             spew("$me: forward address error: " . $errormsg);
             $msg = sprintf(_("Error: %s"), $errormsg);
             $subtitle = _("Mail forwarding not changed");

         } else {

             spew("$me: total size of forwads is: " . sizeof($forwards));

             /**
              * Call the vchforward function provided by vmail.inc.
              */
             spew("$me: calling vchforward");
             $repl = vchforward($domain, $secret, $uname, $forwards);
             if ($repl[0]){
                 spew("$me: error returned: " . $repl[1]);
                 vadmin_system_error($repl[1]);
             } else {
                 $msg = sprintf(_("Updated forwarding to '%s'"), $valiases);
                 $subtitle = _("Mail forwarding changed successfully");
             }
         }
     } else {
         $msg = _("No change requested");
         $subtitle = _("Mail forwarding not changed");
     }

     $body = ''
         . '<table border="0" width="50%" align="center">'
         . ' <tr>'
         . '  <th bgcolor="' . $color[9] . '">'
         .     $subtitle
         . '  </th>'
         . ' </tr>'
         . ' <tr>'
         . '  <td align="center">'
         .     htmlspecialchars($msg)
         . '  </td>'
         . ' </tr>'
         . ' <tr>'
         . '  <td bgcolor="' . $color[9] . '">&nbsp;</td>'
         . ' </tr>'
         . '</table>';

     $title = _("Change mail forwarding");
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
