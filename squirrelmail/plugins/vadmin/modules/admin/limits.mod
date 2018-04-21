<?php
/**
 * limits.mod
 * ----------
 * This module operates the limits set per domain.
 *
 * Licensed under GNU GPL v2. Please see COPYING for full terms.
 *
 * $Id: limits.mod,v 1.15 2009/09/04 21:36:51 pdontthink Exp $
 * 
 * @author Konstantin Riabitsev ($Author: pdontthink $)
 * @version $Date: 2009/09/04 21:36:51 $
 */

function vadmin_zval($val){
    if ($val == false){
        $val = '-';
    } elseif ($val == '-'){
        $val = false;
    }
    return $val;
}

$ACT = vadmin_getvar('VADMIN', 'ACT');
$LVL = vadmin_getvar('VADMIN', 'LVL');
$MOD = 'limits';
$me = "$MOD.mod/$ACT";
spew("$me: taking over");

$domain = vadmin_getvar('VADMIN',  'VADMIN_DOMAIN');
$color = vadmin_getvar('SQMAIL', 'color');
$username = vadmin_getvar('SQMAIL', 'username');
$designation = vadmin_get_user_designation($domain, $username);
if ($designation == 'ELVIS'){
    $level = 'CROSS';
} else {
    $level = 'LOWLY';
}

$cross_limits = vadmin_getvar('CONFIG', 'permissions.cross_can_set_domain_limits');
if (!($designation == 'ELVIS' || ($designation == 'CROSS' && $cross_limits == 'yes')))
   vadmin_security_breach();


// what fields are to be displayed on this screen?
//
$displayablefields = vadmin_get_displayable_fields('domain_limits', 'mailboxes, hardquota, size, count, imgsize');


$fields = array(
    'mailboxes' => _("Maximum number of mailboxes"),
    // Reduce confusion between soft/hard - the limit is of course
    // hard, as soft limit is nearly meaningless, but some admins
    // seem to be confused why there is not a soft limit
    //'hardquota' => _("Maximum user hard quota limit (MiB)"),
    'hardquota' => _("Maximum user quota limit (MiB)"),
    'softquota' => _("Maximum user soft quota limit (MiB)"),
    'size'      => _("Maximum user message size limit (MiB)"),
    'count'     => _("Maximum user message count limit"),
    'imgsize'   => _("Maxium login page image size (KiB)")
    );


switch ($ACT){
    /******************************* main *****************************/
 case 'main':
     $action = vadmin_mkform_action($LVL, $MOD, 'setlimits');
     $body = ''
         . '<form method="post" action="' . $action . '">'
         . ' <table border="0" align="center">'
         . '  <tr>'
         . '   <th bgcolor="' . $color[9] . '">' 
         .      _("Field Description") 
         . '   </th>'
         . '   <th bgcolor="' . $color[9] . '">' . _("Current value") . '</th>'
         . '  </tr>';
     foreach ($fields as $field=>$descr){
         if (!in_array($field, $displayablefields)) continue;

         $val = vadmin_zval(vadmin_smart_limit($domain, $level, $field));
         $body .= ''
             . '<tr>'
             . ' <td align="right">' . $descr . ':</th>'
             . ' <td>'
             . '  <input name="' . $field . '" size="5" value="' . $val .'" />'
             . ' </td>'
             . '</tr>';
     }
     $body .= ''
         . '<tr>'
         . ' <th colspan="2" bgcolor="' . $color[9] . '">'
         . '  <input type="submit" value="' 
         .     _("Set these limits") . ' &gt;&gt;" />'
         . ' </th>'
         . '</tr></table></form>';
     $title = sprintf(_("Set domain limits for %s"), $domain);
     vadmin_make_page($title, null, $body, true, true);
     break;

     /***************************** setlimits ****************************/
 case 'setlimits':
     $body = ''
         . '<table border="0" align="center">'
         . ' <tr>'
         . '  <th bgcolor="' . $color[9] . '">' 
         .     _("Field Description") 
         . '  </th>'
         . '  <th bgcolor="' . $color[9] . '">' . _("Result") . '</th>'
         . ' </tr>';
     foreach ($fields as $field=>$descr){
         if (!in_array($field, $displayablefields)) continue;

         $val = vadmin_zval(trim(vadmin_getvar('POST', $field)));
         if ($val == '-') $val = false;
         $result = '';
         if (strval(intval($val)) != $val){
             spew("$me: $val is not an int. Ignoring.");
             $result = sprintf(_("%s is not an integer. Ignored"), $val);
         } else {
             spew("$me: new setting for '$field': $val");
             if ($val != false && $level == 'LOWLY'){
                 spew("$me: This is a CROSS. Checking against master.");
                 $master = vadmin_get_limit($domain, 'CROSS', $field);
                 if ($master > 0 && $val > $master){
                     spew("$me: Tried to override the limit from above.");
                     $result = sprintf(_("%d is over the master limit of %d. Maximum setting of %d used"), $val, $master, $master);
                     $val = $master;
                 }
             }
             $oldval = vadmin_get_limit($domain, $level, $field);
             if ($val != $oldval){
                 vadmin_put_limit($domain, $level, $field, $val);
                 if ($result == ''){
                     if ($val == false){
                         $result = _("OK, set to unlimited");
                     } else {
                         $result = sprintf(_("OK, set to %d"), $val);
                     }
                 }
             } else {
                 if ($result == ''){
                     $result = _("No change");
                 }
             }
         }
         $body .= ''
             . '<tr>'
             . ' <td>' . $descr . ':</th>'
             . ' <td>' . $result . '</td>'
             . '</tr>';
     }
     $body .= ''
         . '<tr>'
         . ' <th colspan="2" bgcolor="' . $color[9] . '">&nbsp;</th>'
         . '</tr></table>';
     $title = sprintf(_("Recording domain limits for %s"), $domain);
     $previous_link = '<tr><td align="center"><a href="'
                    . vadmin_mkform_action($LVL, $MOD, 'main')
                    . '">&lt; ' . _("Previous") . '</a></td></tr>';
     vadmin_make_page($title, null, $body, true, true, $previous_link);
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
