<?php
/**
 * logout.mod
 * -----------
 * Logs out the user by wiping his session variables with the AUTHCODE
 * and domain password
 *
 * Licensed under GNU GPL v2. Please see COPYING for full terms.
 *
 * $Id: logout.mod,v 1.8 2007/07/01 08:29:51 pdontthink Exp $
 *
 * @author Konstantin Riabitsev ($Author: pdontthink $)
 * @version $Date: 2007/07/01 08:29:51 $
 */

$ACT = vadmin_getvar('VADMIN', 'ACT');
$LVL = vadmin_getvar('VADMIN', 'LVL');
$MOD = 'logout';
$me = "$MOD.mod/$ACT";
spew("$me: taking over");

$color    = vadmin_getvar('SQMAIL', 'color');
$PHP_SELF = vadmin_getvar('SERVER', 'PHP_SELF');
switch ($ACT){
 case 'main':
     $_SESSION{'VADMIN_SESSION_VARS'} = '';
     $body = ''
         . '<table border="0" align="center" width="50%">'
         . ' <tr>'
         . '  <th bgcolor="' . $color[9] . '">' . _("Logged out") . '</th>'
         . ' </tr>'
         . ' <tr>'
         . '  <td>'
         .     _("You have been logged out of the administrator interface. You may still use your mailbox, but you will need to re-login to the this interface if you wish to administer users again.")
         . '  </td>'
         . ' </tr>'
         . ' <tr>'
         . '  <th bgcolor="' . $color[9] . '">'
         . '   <a href="' . $PHP_SELF . '?LVL=admin">' 
         .      _("Log back in") . '</a>'
         . '  </th>'
         . ' </tr>'
         . '</table>';
     $title = _("Logged Out");
     vadmin_make_page($title, null, $body, false, false);
     break;
 default:
     vadmin_system_error(
         sprintf(_("Invalid request, handler for '%s' does not exist"), $ACT));
     break;
}

echo '</body></html>';

/**
 * For emacs weenies:
 * Local variables:
 * mode: php
 * End:
 */

