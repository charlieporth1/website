<?php
/**
 * cross.mod
 * ------------
 * This module lets an elvis operate cross-admins.
 *
 * Licensed under GNU GPL v2. Please see COPYING for full terms.
 *
 * $Id: cross.mod,v 1.9 2008/12/31 11:57:50 pdontthink Exp $
 * 
 * @author Konstantin Riabitsev ($Author: pdontthink $)
 * @version $Date: 2008/12/31 11:57:50 $
 */

function vadmin_crossmod_get_crosses($domain){
    $me = 'vadmin_crossmod_get_crosses';
    $color = vadmin_getvar('SQMAIL', 'color');
    $camefrom = vadmin_getvar('VADMIN', 'ACT');
    $retstr = ''
        . '<tr bgcolor="' . $color[9] . '">'
        . '<th colspan="2">' . $domain . '</th>'
        . '</tr>';
    $LVL = vadmin_getvar('VADMIN', 'LVL');
    $MOD = 'cross';
    $action = vadmin_mkform_action($LVL, $MOD, 'delcross');
    $crosses = vadmin_get_cross_array($domain);
    $flip = true;
    foreach ($crosses as $cross){
        if ($flip = !$flip){
            $retstr .= '<tr bgcolor="' . $color[0] . '">';
        } else {
            $retstr .= '<tr>';
        }
        $retstr .= ''
            . '<form method="post" action="' . $action . '">'
            . ' <input type="hidden" name="vdomain" value="' . $domain . '" />'
            . ' <input type="hidden" name="vcross"  value="' . $cross . '" />'
            . ' <input type="hidden" name="camefrom" value="'.$camefrom.'" />'
            . ' <td align="right">' . $cross . '</td>'
            . ' <td align="left">'
            . '  <input type="submit" value="  ' . _("Delete") . '  " />'
            . ' </td>'
            . '</form></tr>';
    }
    $action = vadmin_mkform_action($LVL, $MOD, 'addcross');
    if ($flip = !$flip){
        $retstr .= '<tr bgcolor="' . $color[0] . '">';
    } else {
        $retstr .= '<tr>';
    }
    $retstr .= ''
        . '<form method="post" action="' . $action . '">'
        . ' <input type="hidden" name="vdomain" value="' . $domain . '" />'
        . ' <input type="hidden" name="camefrom" value="' . $camefrom . '" />'
        . ' <td align="right">'
        . '  <input name="vcross" size="30">'
        . ' </td>'
        . ' <td align="left">'
        . '  <input type="submit" value="  ' . _("Add") . '  " />'
        . ' </td>'
        . '</form></tr>';
    return $retstr;
}

function vadmin_crossmod_descript(){
    $color = vadmin_getvar('SQMAIL', 'color');
    $retstr = ''
        . '<tr>'
        . ' <th bgcolor="' . $color[9] . '" colspan="2">'
        .    _("Editing instructions")
        . ' </th>'
        . '</tr>'
        . '<tr>'
        . ' <td colspan="2">'
        . '  <p>' 
        .     _("To delete a cross-administrator, press the 'Delete' key next to the username. To add a cross-administrator, put in the full username into the field for the domain to which you wish to add one, and press 'Add'. The term 'full username' means both the username and the domain name, e.g.: joe@domain.com.")
        . '  </p>'
        . ' </td>'
        . '</tr>';
    return $retstr;
}

$LVL = vadmin_getvar('VADMIN', 'LVL');
$ACT = vadmin_getvar('VADMIN', 'ACT');
$MOD = 'cross';

$me = "$MOD.mod/$ACT";
spew("$me: taking over");

/**
 * Only ELVIS can be here.
 */
$domain      = vadmin_getvar('VADMIN', 'VADMIN_DOMAIN');
$username    = vadmin_getvar('SQMAIL', 'username');
$designation = vadmin_get_user_designation($domain, $username);

spew("$me: The user is $username, with designation $designation");
if ($designation != 'ELVIS'){
    spew("$me: This is not elvis!");
    vadmin_security_breach();
}

switch ($ACT){
    /****************************** listall ******************************/
 case 'listall':
     $body = '<table border="0" align="center" width="50%">';
     spew("$me: getting a list of all enabled domains");
     $domain_ary = vadmin_list_enabled_domains();
     foreach ($domain_ary as $domain){
         $body .= vadmin_crossmod_get_crosses($domain);
     }
     $body .= vadmin_crossmod_descript();
     $body .= '</table>';
     $title = _("Listing cross-admins for all configured domains");
     $previous_link = '<tr><td align="center"><a href="'
                    . vadmin_mkform_action($LVL, 'menu', 'cross')
                    . '">&lt; ' . _("Previous") . '</a></td></tr>';
     vadmin_make_page($title, null, $body, true, true, $previous_link);
     break;
     
     /****************************** addcross *****************************/
 case 'addcross':
     $cross    = vadmin_getvar('POST', 'vcross');
     $domain   = vadmin_getvar('POST', 'vdomain');
     $camefrom = vadmin_getvar('POST', 'camefrom');
     spew("$me: cross=$cross, domain=$domain, camefrom=$camefrom");

     $delim = vadmin_getvar('BACKEND', 'delimiters');
     $delim = substr($delim, 0, 1);
     if (vadmin_IMAP_usernames_have_domain($domain) && strpos($cross, $delim) === FALSE)
     {
         // TODO: warn that username format may not be correct?  warnings not easy to do because this redirects below.... or auto-adjust by appending "$delim$domain" to $cross?
     }
     else if (!vadmin_IMAP_usernames_have_domain($domain) && strpos($cross, $delim) !== FALSE)
     {
         // TODO: warn that username format may not be correct?  warnings not easy to do because this redirects below.... or auto-adjust by removing everything after $delim from $cross?
     }

     if (isset($cross) && strlen($cross) > 0){
         spew("$me: making sure $cross is not already elvis");
         $cdesignation = vadmin_get_user_designation($domain, $cross);
         if ($cdesignation != 'ELVIS'){
             spew("$me: making sure $cross isn't already a cross for $domain");
             $cross_ary = vadmin_get_cross_array($domain);
             if (!in_array($cross, $cross_ary)){
                 spew("$me: calling vadmin_add_cross");
                 vadmin_add_cross($domain, $cross);
             } else {
                 spew("$me: $cross is already a cross for $domain, silly.");
             }
         } else {
             spew("$me: $cross is already an elvis, silly.");
         }
     }
     vadmin_redirect($LVL, $MOD, $camefrom, $domain);
     break;
     
     /****************************** delcross *****************************/
 case 'delcross':
     $cross    = vadmin_getvar('POST', 'vcross');
     $domain   = vadmin_getvar('POST', 'vdomain');
     $camefrom = vadmin_getvar('POST', 'camefrom');
     spew("$me: cross=$cross, domain=$domain, camefrom=$camefrom");
     if (isset($cross) && strlen($cross) > 0){
         spew("$me: calling vadmin_delete_cross");
         vadmin_delete_cross($domain, $cross);
     }
     vadmin_redirect($LVL, $MOD, $camefrom, $domain);
     break;
     
     /***************************** listdomain *****************************/
 case 'listdomain':
     $domain = vadmin_getvar('POST', 'vdomain');
     if (empty($domain)){
         $domain = vadmin_get_storparams();
     }
     $body = '<table border="0" align="center" width="50%">';
     $body .= vadmin_crossmod_get_crosses($domain);
     $body .= vadmin_crossmod_descript();
     $body .= '</table>';
     $title = sprintf(_("Listing cross-admins for %s"), $domain);
     $previous_link = '<tr><td align="center"><a href="'
                    . vadmin_mkform_action($LVL, 'menu', 'crosslookup')
                    . '">&lt; ' . _("Previous") . '</a></td></tr>';
     vadmin_make_page($title, null, $body, true, true, $previous_link);
     break;
     
     /*************************** bycrossname *****************************/
 case 'bycrossname':
     spew("$me: making an array of all cross-admins on this system");
     $domary = vadmin_list_enabled_domains();
     $crosses = array();
     foreach ($domary as $domain){
         $domcrossary = vadmin_get_cross_array($domain);
         if (is_array($domcrossary) && sizeof($domcrossary) > 0){
             foreach($domcrossary as $cross){
                 if (!in_array($cross, $crosses)){
                     spew("$me: pushing $cross into the array");
                     array_push($crosses, $cross);
                 }
             }
         }
     }
     if (sizeof($crosses)){
         sort($crosses);
         $action = vadmin_mkform_action($LVL, $MOD, 'showcrossdomains');
         $body = ''
             . '<table border="0" align="center" width="50%">'
             . ' <tr bgcolor="' . $color[9] . '">'
             . '  <th colspan="2">' 
             .     _("Choose a cross-admin to view") 
             . '  </th>'
             . ' </tr>'
             . ' <tr>'
             . '  <form method="post" action="' . $action . '">'
             . '   <td align="right">'
             .      _("Cross-admin:") 
             . '    <select name="vcross">';
         foreach ($crosses as $cross){
             $body .= '<option>' . $cross . '</option>';
         }
         $body .= ''
             . '    </select>'
             . '   </td>'
             . '   <td align="left">'
             . '    <input type="submit" value="  ' . _("Edit") . '  " />'
             . '   </td>'
             . '  </form>'
             . ' </tr>'
             . ' <tr>'
             . '  <form method="post" action="' . $action . '">'
             . '   <td align="right">'
             . '    <input name="vcross" size="30" />'
             . '   </td>'
             . '   <td aligh="left">'
             . '    <input type="submit" value="  ' . _("Add") . '  " />'
             . '   </td>'
             . '  </form>'
             . ' </tr>'
             . ' <tr bgcolor="' . $color[9] . '">'
             . '  <th colspan="2">' . _("Instructions") . '</th>'
             . ' </tr>'
             . ' <tr>'
             . '  <td colspan="2">'
             . '   <p>'
             .      _("To edit a cross-admin, select the admin's username from the drop-down dialog box and then click 'Edit'. To add a new one, provide the full username in the text box and click the 'Add' button next to it. The 'full' username means both the user's name and the domain, e.g.: joe@domain.com.")
             . '   </p>'
             . '  </td>'
             . ' </tr>'
             . '</table>';
     } else {
         $error = _("No cross-admins found. You should add some first.");
         vadmin_user_error($error);
     }
     $title = _("Choose a cross-admin to view");
     $previous_link = '<tr><td align="center"><a href="'
                    . vadmin_mkform_action($LVL, 'menu', 'cross')
                    . '">&lt; ' . _("Previous") . '</a></td></tr>';
     vadmin_make_page($title, null, $body, true, true, $previous_link);
     break;
     
     /************************* showcrossdomains **************************/
 case 'showcrossdomains':
     $cross = vadmin_getvar('POST', 'vcross');
     if (!isset($cross) || !$cross){
         $cross = vadmin_get_storparams();
     }
     $body = ''
         . '<table width="50%" border="0" align="center">'
         . ' <tr bgcolor="' . $color[9] . '">'
         . '  <th colspan="2">'
         .     sprintf(_("Viewing domains for %s"), $cross)
         . '  </th>'
         . ' </tr>';
     $action = vadmin_mkform_action($LVL, $MOD, 'deldomain');
     $domains = vadmin_get_xdomain_array($cross);
     $flip = true;
     foreach ($domains as $domain){
         if ($flip = !$flip){
             $body .= '<tr bgcolor="' . $color[0] . '">';
         } else {
             $body .= '<tr>';
         }
         $body .= ''
             . '<form method="post" action="' . $action . '">'
             . ' <input type="hidden" name="vdomain" value="'.$domain.'" />'
             . ' <input type="hidden" name="vcross"  value="'.$cross.'" />'
             . ' <td align="right">' . $domain . '</td>'
             . ' <td align="left">'
             . '  <input type="submit" value="  ' . _("Delete") . '  " />'
             . ' </td>'
             . '</form></tr>';
     }
     $endomains = vadmin_list_enabled_domains();
     $showdomains = array();
     foreach ($endomains as $endomain){
         if (!in_array($endomain, $domains)){
             array_push($showdomains, $endomain);
         }
     }
     if (sizeof($showdomains)){
         $action = vadmin_mkform_action($LVL, $MOD, 'adddomain');
         if ($flip = !$flip){
             $body .= '<tr bgcolor="' . $color[0] . '">';
         } else {
             $body .= '<tr>';
         }
         $body .= ''
             . '<form method="post" action="' . $action . '">'
             . ' <input type="hidden" name="vcross" value="' . $cross . '" />'
             . ' <td align="right">'
             . '  <select name="vdomain">';
         foreach ($showdomains as $showdomain){
             $body .= '<option>' . $showdomain . '</option>';
         }
         $body .= ''
             . '  </select>'
             . ' </td>'
             . ' <td align="left">'
             . '  <input type="submit" value="  ' . _("Add") . '  " />'
             . ' </td>'
             . '</form></tr>';
     }
     $body .= '</table>';
     $title = sprintf(_("Viewing Cross-administrator %s"), $cross);
     $previous_link = '<tr><td align="center"><a href="'
                    . vadmin_mkform_action($LVL, $MOD, 'bycrossname')
                    . '">&lt; ' . _("Previous") . '</a></td></tr>';
     vadmin_make_page($title, null, $body, true, true, $previous_link);
     break;

     /***************************** deldomain *****************************/
 case 'deldomain':
     $domain = vadmin_getvar('POST', 'vdomain');
     $cross  = vadmin_getvar('POST', 'vcross');
     spew("$me: domain=$domain, cross=$cross");
     vadmin_delete_cross($domain, $cross);
     vadmin_redirect($LVL, $MOD, 'showcrossdomains', $cross);
     break;

     /***************************** adddomain ***************************/
 case 'adddomain':
     $domain = vadmin_getvar('POST', 'vdomain');
     $cross  = vadmin_getvar('POST', 'vcross');
     spew("$me: domain=$domain, cross=$cross");
     vadmin_add_cross($domain, $cross);
     vadmin_redirect($LVL, $MOD, 'showcrossdomains', $cross);
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

