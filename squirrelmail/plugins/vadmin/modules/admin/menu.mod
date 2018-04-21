<?php
/**
 * menu.mod
 * ----------------
 * Show a nice choice of destinations to the admin.
 *
 * Licensed under GNU GPL v2. Please see COPYING for full terms.
 *
 * $Id: menu.mod,v 1.30 2009/01/27 18:20:25 pdontthink Exp $
 *
 * @author Konstantin Riabitsev ($Author: pdontthink $)
 * @version $Date: 2009/01/27 18:20:25 $
 */

function vadmin_menu_make_formmenu($title, $formbody, $LVL, $MOD, $ACT){
    $color = vadmin_getvar('SQMAIL', 'color');
    $action = vadmin_mkform_action($LVL, $MOD, $ACT);
    $body = ''
        . '<form style="display:inline; margin:0" method="POST" action="' . $action . '">'
        . ' <table border="0" align="center" width="40%">'
        . '  <tr>'
        . '   <th bgcolor="' . $color[9] . '">' . $title . '</th>'
        . '  </tr>'
        . '  <tr>'
        . '   <td>' . $formbody . '</td>'
        . '  </tr>'
        . '  <tr>'
        . '   <th bgcolor="' . $color[9] . '">'
        . '    <input type="submit" value="' . _("Proceed") . ' &gt;&gt" />'
        . '   </th>'
        . '  </tr>'
        . ' </table>'
        . '</form>';
    return $body;
}

$LVL = vadmin_getvar('VADMIN', 'LVL');
$ACT = vadmin_getvar('VADMIN', 'ACT');
$MOD = 'menu';

$me = "$MOD.mod/$ACT";
spew("$me: taking over");

$color = vadmin_getvar('SQMAIL', 'color');

switch ($ACT){
    /************************** main *********************************/
 case 'main':
     $domain      = vadmin_getvar('VADMIN', 'VADMIN_DOMAIN');
     $username    = vadmin_getvar('SQMAIL', 'username');

     // change menubar setting if necessary
     //
     $data_dir  = vadmin_getvar('SQMAIL', 'data_dir');
     $user  = vadmin_getvar('SQMAIL', 'username');
     $change_vadmin_in_menubar = vadmin_getvar('POST', 'change_vadmin_in_menubar');
     $vadmin_in_menubar        = vadmin_getvar('POST', 'vadmin_in_menubar');
     if ($change_vadmin_in_menubar) {
         setPref($data_dir, $user, 'vadmin_in_menubar', $vadmin_in_menubar);
     }
     $designation = vadmin_get_user_designation($domain, $username);
     $cross_limits = vadmin_getvar('CONFIG', 'permissions.cross_can_set_domain_limits');

     $menu = array();

     $member = array(
         'words' => _("Email administration"),
         'title' => _("Manage email accounts in this domain"),
         'LVL'   => $LVL,
         'MOD'   => 'email',
         'ACT'   => 'main');
     array_push($menu, $member);

     if ($designation == 'ELVIS' 
      || ($designation == 'CROSS' && $cross_limits == 'yes')){

         $member = array(
             'separator' => 1);
         array_push($menu, $member);

         $member = array(
            'words' => _("Set domain limits"),
            'title' => _("Set limits for this domain: maximum number of users, maximum quotas, etc."),
            'LVL'   => $LVL,
            'MOD'   => 'limits',
            'ACT'   => 'main');
         array_push($menu, $member);

         if ($designation == 'ELVIS'){
             $member = array(
                 'words' => _("Configure cross-admins"),
                 'title' => _("Edit cross-administrators"),
                 'LVL'   => $LVL,
                 'MOD'   => $MOD,
                 'ACT'   => 'cross');
             array_push($menu, $member);
             
             $member = array(
                 'words' => _("Set up domain templates"),
                 'title' => _("Set up some templates for new domains"),
                 'LVL'   => $LVL,
                 'MOD'   => 'templates',
                 'ACT'   => 'main');
             array_push($menu, $member);
         } 
     }

     $body = vadmin_menu_make_linkmenu($menu);

     $js_on  = vadmin_getvar('SQMAIL', 'javascript_on');
         $vadmin_in_menubar = getPref($data_dir, $user, 'vadmin_in_menubar', 0);
         $body .= '<center><form style="display:inline; margin:0" name= "vadmin_in_menubar_form" action="'
               . vadmin_mkform_action($LVL, $MOD, 'main')
               . '" method="POST"><input type="checkbox" name="vadmin_in_menubar" '
               . ' id="vadmin_in_menubarID" value="1" '
               . ($vadmin_in_menubar ? 'CHECKED' : '') 
               . ($js_on ? ' onclick="document.vadmin_in_menubar_form.submit()"' : '')
               . ' /><input type="hidden" name="change_vadmin_in_menubar" value="1" />'
               . '<label for="vadmin_in_menubarID">' . _("Show admin link in menu bar")
               . '</label>'
               . ($js_on ? '' : ' <input type="submit" value="' . _("Go") . '" />')
               . '</form></center>';
     $title = sprintf(_("Domain Administration: %s"), $domain);
     list($start_mod, $start_act) = vadmin_get_startup_location($designation);
     if ($start_mod == $MOD)
         $show_return_to_main_link = FALSE;
     vadmin_make_page($title, null, $body, $show_return_to_main_link, true);
     break;
   
     /*************************** userlookup **************************/
//FIXME: This action is covered under the admin/email.mod module; a quick grep does not seem to show anywhere that uses this code here - can it be removed? (ah, except it looks like the "original" menu uses it; see admin/menu_orig.mod)
 case 'userlookup':
     $title = _("Search for username");
     $formbody = '<p style="text-align: center">' . _("Username:")
         . ' <input name="userid" /></p>';
     $body = vadmin_menu_make_formmenu($title, $formbody, $LVL, 'accounts',
                                       'getuser');
     $vadmin_backend = vadmin_getvar('CONFIG', 'backend.type');
     if ($vadmin_backend != 'vmailmgr')
     {
         $body .= '<table border="0" cellpadding="0" cellspacing="0" align="center" width="40%"><tr>'
             . '  <td>'
             . _("Use an asterisk (*) as a wildcard character to search for more than one user.")
             . '  </td>'
             . ' </tr>'
             . '</table>';
     }
     vadmin_make_page($title, null, $body, true, true);
     break;
     
     /**************************** cross ******************************/
 case 'cross':
     /**
      * Only an ELVIS can be here.
      */
     $username = vadmin_getvar('SQMAIL', 'username');
     $domain = vadmin_getvar('VADMIN', 'VADMIN_DOMAIN');
     $designation = vadmin_get_user_designation($domain, $username);
     if ($designation != 'ELVIS'){
         spew("$me: you are not elvis!");
         vadmin_security_breach();
     }
     $menu = array();
     $member = array(
         'words' => _("List all cross-admins by domain"),
         'title' => _("Show a listing of ALL cross-admins on this system sorted by the domain"),
         'LVL'   => $LVL,
         'MOD'   => 'cross',
         'ACT'   => 'listall');
     array_push($menu, $member);
     
     $member = array(
         'words' => _("List cross-admins in a specific domain"),
         'title' => _("Lists all cross-admins in a domain you specify"),
         'LVL'   => $LVL,
         'MOD'   => $MOD,
         'ACT'   => 'crosslookup');
     array_push($menu, $member);
     
     $member = array(
         'words' => _("List domains belonging to each cross-admin"),
         'title' => _("This lets you list all domians belonging to a certain cross-admin"),
         'LVL' => $LVL,
         'MOD' => 'cross',
         'ACT' => 'bycrossname');
     array_push($menu, $member);
     $body = vadmin_menu_make_linkmenu($menu);
     $title = _("Cross-admins administration menu");
     vadmin_make_page($title, null, $body, true, true);
     break; 

     /*************************** crosslookup **************************/
 case 'crosslookup':
     /**
      * Only an ELVIS can be here.
      */
     $username = vadmin_getvar('SQMAIL', 'username');
     $domain = vadmin_getvar('VADMIN', 'VADMIN_DOMAIN');
     $designation = vadmin_get_user_designation($domain, $username);
     if ($designation != 'ELVIS'){
         spew("$me: you are not elvis!");
         vadmin_security_breach();
     }
     
     $title = _("List cross-admins for a domain");
     $formbody = '<p style="text-align: center">' . _("Choose a domain:")
         . ' <select name="vdomain">';
     $domain_ary = vadmin_list_enabled_domains();
     foreach ($domain_ary as $domain){
         $formbody .= '<option>' . $domain . '</option>';
     }
     $formbody .= '</select></p>';
     $body = vadmin_menu_make_formmenu($title, $formbody, $LVL, 'cross',
                                       'listdomain');
     $previous_link = '<tr><td align="center"><a href="'
                    . vadmin_mkform_action($LVL, $MOD, 'cross')
                    . '">&lt; ' . _("Previous") . '</a></td></tr>';
     vadmin_make_page($title, null, $body, true, true, $previous_link);
     break;
     
     /**************************** default ****************************/
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

