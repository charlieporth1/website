<?php
/**
 * menu_orig.mod
 * ----------------
 * Main menu; old style (Vadmin 2.0 and previous).
 *
 * Licensed under GNU GPL v2. Please see COPYING for full terms.
 *
 * $Id: menu_orig.mod,v 1.5 2009/03/05 04:04:40 pdontthink Exp $
 *
 * @author Konstantin Riabitsev ($Author: pdontthink $)
 * @version $Date: 2009/03/05 04:04:40 $
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
         'words' => _("List users"),
         'title' => _("List accounts in this domain"),
         'LVL'   => $LVL,
         'MOD'   => $MOD,
         'ACT'   => 'acctlist');
     array_push($menu, $member);

     $member = array(
         'words' => _("Add new user or alias"),
         'title' => _("Create new users or new aliases for this domain"),
         'LVL'   => $LVL,
         'MOD'   => $MOD,
         'ACT'   => 'addnew');
     array_push($menu, $member);

     $member = array(
         'words' => _("Customize this domain"),
         'title' => _("Change the look of your domain and set account defaults"),
         'LVL'   => $LVL,
         'MOD'   => $MOD,
         'ACT'   => 'prefmain');
     array_push($menu, $member);

     if ($designation == 'ELVIS' 
      || ($designation == 'CROSS' && $cross_limits == 'yes')){
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
     if ($js_on) {
         $vadmin_in_menubar = getPref($data_dir, $user, 'vadmin_in_menubar', 0);
         $body .= '<center><form style="display:inline; margin:0" name= "vadmin_in_menubar_form" action="'
               . vadmin_mkform_action($LVL, $MOD, 'main')
               . '" method="POST"><input type="checkbox" name="vadmin_in_menubar" '
               . ' id="vadmin_in_menubarID" value="1" '
               . ($vadmin_in_menubar ? 'CHECKED' : '') 
               . ' onclick="document.vadmin_in_menubar_form.submit()" />'
               . '<input type="hidden" name="change_vadmin_in_menubar" value="1" />'
               . '<label for="vadmin_in_menubarID">' . _("Show admin link in menu bar")
               . '</label></form></center>';
     }
     $title = sprintf(_("Vadmin Plugin: %s"), $domain);
     vadmin_make_page($title, null, $body, false, true);
     break;
   
     /**************************** acctlist ***************************/
 case 'acctlist':
     $menu = array();
     $member = array(
         'words' => _("List all users"),
         'title' => _("Show a listing of ALL users in this domain"),
         'LVL'   => $LVL,
         'MOD'   => 'accounts',
         'ACT'   => 'listall');
     array_push($menu, $member);

     $member = array(
         'words' => _("Search for username"),
         'title' => _("Look up a specific username or search for similarly named users"),
         'LVL'   => $LVL,
         'MOD'   => $MOD,
         'ACT'   => 'userlookup');
     array_push($menu, $member);
     $body = vadmin_menu_make_linkmenu($menu);
     $domain = vadmin_getvar('VADMIN', 'VADMIN_DOMAIN');
     $title = sprintf(_("Accounts for %s"), $domain);
     vadmin_make_page($title, null, $body, true, true);
     break;

     /***************************** addnew ****************************/

 case 'addnew':
     $menu = array();
     $domain = vadmin_getvar('VADMIN', 'VADMIN_DOMAIN');
     $username = vadmin_getvar('SQMAIL', 'username');
     $desig = vadmin_get_user_designation($domain, $username);
     $mailbox_limit = vadmin_smart_limit($domain, $desig, 'mailboxes');
     $member = false;
     if ($mailbox_limit != false){
         $mboxes = vadmin_get_user_number($domain);
         if ($mboxes >= $mailbox_limit){
             $member = array(
                 'words' => _("Mailbox Limit Reached"),
                 'title' => _("Cannot add any more real users, please delete some or ask your administrator for a higher mailbox quota"),
                 'LVL'   => $LVL,
                 'MOD'   => $MOD,
                 'ACT'   => $ACT);
         }
     }
     if ($member == false){
         $member = array(
             'words' => _("Create a new user"),
             'title' => _("Create a real user with a real mailbox"),
             'LVL'   => $LVL,
             'MOD'   => 'accounts',
             'ACT'   => 'newuser');
     }
     array_push($menu, $member);

     $member = array(
         'words' => _("Create a new alias"),
         'title' => _("Don't create a mailbox, just a forwarding address"),
         'LVL'   => $LVL,
         'MOD'   => 'accounts',
         'ACT'   => 'newalias');
     array_push($menu, $member);

     spew("$me: See if we already have a catchall alias");
     $domain = vadmin_getvar('VADMIN', 'VADMIN_DOMAIN');
     $crypto = vadmin_getvar('SESSION','VADMIN_SECRET');
     $secret = vadmin_crypto($crypto, 'decrypt');

     $catchall_alias = vadmin_getvar('BACKEND', 'catchall_alias');
     $command = array('lookup', $domain, $catchall_alias, $secret);
     $tmp = vm_daemon_raw($command);
     if ($tmp[0]){
         spew("$me: no catchall user yet. Showing option to set one up.");
         $member = array(
             'words' => _("Create a 'catchall' account"),
             'title' => _("Create a default forwarding alias for this domain"),
             'LVL'   => $LVL,
             'MOD'   => 'accounts',
             'ACT'   => 'newcatchall');
         array_push($menu, $member);
     }
     $body = vadmin_menu_make_linkmenu($menu);
     $title = _("Real user or Alias?");
     vadmin_make_page($title, null, $body, true, true);
     break;

     /*************************** prefmain *************************/

 case 'prefmain':
     $menu = array();
     $member = array(
         'words' => _("Configure your login screen"),
         'title' => _("Set your login screen title and picture"),
         'LVL'   => $LVL,
         'MOD'   => 'domprefs',
         'ACT'   => 'title');
     array_push($menu, $member);

     $member = array(
         'words' => _("Manage webmail features"),
         'title' => _("Configure what your users are allowed to do"),
         'LVL'   => $LVL,
         'MOD'   => 'domprefs',
         'ACT'   => 'perms');
     array_push($menu, $member);

     $body = vadmin_menu_make_linkmenu($menu);
     $title = _("Customize your domain");
     vadmin_make_page($title, null, $body, true, true);
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
     $formbody .= '</p>';
     $body = vadmin_menu_make_formmenu($title, $formbody, $LVL, 'cross',
                                       'listdomain');
     vadmin_make_page($title, null, $body, true, true);
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

