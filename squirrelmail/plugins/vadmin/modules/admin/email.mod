<?php
/**
 * email.mod
 * ----------------
 * Show a list of email administrative functions to the admin.
 *
 * Licensed under GNU GPL v2. Please see COPYING for full terms.
 *
 * $Id: email.mod,v 1.8 2009/03/05 04:04:40 pdontthink Exp $
 *
 * @author Konstantin Riabitsev ($Author: pdontthink $)
 * @version $Date: 2009/03/05 04:04:40 $
 */

function vadmin_menu_make_formmenu($title, $formbody, $LVL, $MOD, $ACT){
    $color = vadmin_getvar('SQMAIL', 'color');
    $action = vadmin_mkform_action($LVL, $MOD, $ACT);
    $body = ''
        . '<form style="margin:0" method="POST" action="' . $action . '">'
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
$MOD = 'email';

$me = "$MOD.mod/$ACT";
spew("$me: taking over");

$color = vadmin_getvar('SQMAIL', 'color');

switch ($ACT){
    /************************** main *********************************/
 case 'main':
     $domain      = vadmin_getvar('VADMIN', 'VADMIN_DOMAIN');
     $username    = vadmin_getvar('SQMAIL', 'username');

     $designation = vadmin_get_user_designation($domain, $username);

     // change menubar setting if necessary
     //
     $data_dir  = vadmin_getvar('SQMAIL', 'data_dir');
     $user  = vadmin_getvar('SQMAIL', 'username');
     $change_vadmin_in_menubar = vadmin_getvar('POST', 'change_vadmin_in_menubar');
     $vadmin_in_menubar        = vadmin_getvar('POST', 'vadmin_in_menubar');
     if ($change_vadmin_in_menubar) {
         setPref($data_dir, $user, 'vadmin_in_menubar', $vadmin_in_menubar);
     }

     $menu = array();

     $member = array(
         'words' => _("Manage users"),
         'title' => _("List, edit and delete accounts in this domain"),
         'LVL'   => $LVL,
         'MOD'   => $MOD,
         'ACT'   => 'acctlist');
     array_push($menu, $member);

     $support_aliases = vadmin_getvar('CONFIG', 'backend.support_aliases');
     if ($support_aliases == 'yes')
     {
        $member = array(
            'words' => _("Add new user or alias"),
            'title' => _("Create new users or new aliases for this domain"),
            'LVL'   => $LVL,
            'MOD'   => $MOD,
            'ACT'   => 'addnew');
     }
     else
     {
        $member = array(
            'words' => _("Add new user(s)"),
            'title' => _("Create new users for this domain"),
            'LVL'   => $LVL,
            'MOD'   => $MOD,
            'ACT'   => 'addnew');
     }
     array_push($menu, $member);

     // don't show link to domain customizations menu if it will be empty!
     //
     // NOTE: this makes an assumption about what is in that menu!  Need to
     //       keep this updated
     //
     $pass_plugin = vadmin_getvar('CONFIG', 'backend.user_password_plugin');
     $autores_plugin = vadmin_getvar('CONFIG', 'backend.user_autoresponder_plugin');
     $mail_fwd_plugin = vadmin_getvar('CONFIG', 'backend.user_mail_forwarding_plugin');
     $displayablefields = vadmin_get_displayable_fields('domain_customizations', '');
     if (in_array('login_screen_customize', $displayablefields)

      // check that this menu item is displayable AND that it has at least
      // one menu item inside of it to be shown
      //
      || (in_array('set_user_permissions', $displayablefields)
       && (!empty($pass_plugin) || !empty($autores_plugin)
        || !empty($mail_fwd_plugin)))) 
     {
        $member = array(
            'words' => _("Customize this domain"),
            'title' => _("Change the look of your domain and set account defaults"),
            'LVL'   => $LVL,
            'MOD'   => $MOD,
            'ACT'   => 'prefmain');
        array_push($menu, $member);
     }

     $body = vadmin_menu_make_linkmenu($menu);

     list($start_mod, $start_act) = vadmin_get_startup_location($designation);
     if ($start_mod == $MOD)
     {
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
     }

     $title = sprintf(_("Email Administration: %s"), $domain);
     $show_return_to_main_link = TRUE;
     if ($start_mod == $MOD)
         $show_return_to_main_link = FALSE;
     vadmin_make_page($title, null, $body, $show_return_to_main_link, true);

     // get data_dir for this domain now so vlogin can't screw us up too much
     // the trick is to do this at an end of the request so vlogin does
     // not accidentally override any critical globals, etc.  This will
     // stash the value in session for the next page, so don't need to
     // to do anything with the return value here
     //
     vadmin_get_per_domain_sm_setting('data_dir', $domain);

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
         'words' => _("Quick list"),
         'title' => _("Show an abbreviated listing of ALL users in this domain"),
         'LVL'   => $LVL,
         'MOD'   => 'accounts',
         'ACT'   => 'quicklistall');
     array_push($menu, $member);

     $search_enabled = vadmin_getvar('BACKEND', 'search_enabled');
     if ($search_enabled == 'yes')
     {
        $member = array(
            'words' => _("Search for user(s)"),
            'title' => _("Look up a specific username or search for similarly named users"),
            'LVL'   => $LVL,
            'MOD'   => $MOD,
            'ACT'   => 'userlookup');
        array_push($menu, $member);
     }

     $backend = vadmin_getvar('CONFIG', 'backend.type');
     if ($backend != 'vmailmgr') {
         $member = array(
             'separator' => 1);
         array_push($menu, $member);

         $member = array(
             'words' => _("Active users only"),
             'title' => _("List all user accounts that have used webmail at least once"),
             'LVL'   => $LVL,
             'MOD'   => 'accounts',
             'ACT'   => 'activelookup');
         array_push($menu, $member);

         $member = array(
             'words' => _("Inactive users only"),
             'title' => _("List all user accounts that have not used webmail at least once"),
             'LVL'   => $LVL,
             'MOD'   => 'accounts',
             'ACT'   => 'inactivelookup');
         array_push($menu, $member);
     }

     $body = vadmin_menu_make_linkmenu($menu);
     $domain = vadmin_getvar('VADMIN', 'VADMIN_DOMAIN');
     $title = sprintf(_("Accounts for %s"), $domain);
     $previous_link = '<tr><td align="center"><a href="' 
                    . vadmin_mkform_action($LVL, $MOD, 'main') 
                    . '">&lt; ' . _("Previous") . '</a></td></tr>';
     vadmin_make_page($title, null, $body, true, true, $previous_link);

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
         array_push($menu, $member);
         $member = array(
             'words' => _("Create multiple users"),
             'title' => _("Create one or more real users at once"),
             'LVL'   => $LVL,
             'MOD'   => 'accounts',
             'ACT'   => 'multiplenewusers');
     }
     array_push($menu, $member);

     $support_aliases = vadmin_getvar('CONFIG', 'backend.support_aliases');
     if ($support_aliases == 'yes') {
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
     }
     $body = vadmin_menu_make_linkmenu($menu);
     if ($support_aliases == 'yes')
         $title = _("Add New User or Alias");
     else
         $title = _("Add New User(s)");
     $previous_link = '<tr><td align="center"><a href="' 
                    . vadmin_mkform_action($LVL, $MOD, 'main') 
                    . '">&lt; ' . _("Previous") . '</a></td></tr>';
     vadmin_make_page($title, null, $body, true, true, $previous_link);
     break;

     /*************************** prefmain *************************/

 case 'prefmain':

     $displayablefields = vadmin_get_displayable_fields('domain_customizations', '');

     $menu = array();

     if (in_array('login_screen_customize', $displayablefields))
     {
        $member = array(
            'words' => _("Configure your login screen"),
            'title' => _("Set your login screen title and picture"),
            'LVL'   => $LVL,
            'MOD'   => 'domprefs',
            'ACT'   => 'title');
        array_push($menu, $member);
     }

     // don't show link to user permissions menu if it will be empty!
     //
     // NOTE: this makes an assumption about what is in that menu!  Need to
     //       keep this updated
     //
     $pass_plugin = vadmin_getvar('CONFIG', 'backend.user_password_plugin');
     $autores_plugin = vadmin_getvar('CONFIG', 'backend.user_autoresponder_plugin');
     $mail_fwd_plugin = vadmin_getvar('CONFIG', 'backend.user_mail_forwarding_plugin');
     if (!empty($pass_plugin) || !empty($autores_plugin)
      || !empty($mail_fwd_plugin)) {

         if (in_array('set_user_permissions', $displayablefields)) {
            $member = array(
                'words' => _("Manage webmail features"),
                'title' => _("Configure what your users are allowed to do"),
                'LVL'   => $LVL,
                'MOD'   => 'domprefs',
                'ACT'   => 'perms');
            array_push($menu, $member);
         }

     }

     $body = vadmin_menu_make_linkmenu($menu);
     $title = _("Customize your domain");
     $previous_link = '<tr><td align="center"><a href="' 
                    . vadmin_mkform_action($LVL, $MOD, 'main') 
                    . '">&lt; ' . _("Previous") . '</a></td></tr>';
     vadmin_make_page($title, null, $body, true, true, $previous_link);

     // get default login page image so vlogin can't screw us up too much
     // the trick is to do this at an end of the request so vlogin does
     // not accidentally override any critical globals, etc.  This will 
     // stash the value in session for the next page, so don't need to
     // to do anything with the return value here
     //
     vadmin_get_per_domain_sm_setting('org_logo', vadmin_getvar('VADMIN', 'VADMIN_DOMAIN'));

     break;

     /*************************** userlookup **************************/
 case 'userlookup':
     $title = _("Search for user(s)");
     $formbody = '<p style="text-align: center">' . _("Username:")
         . '<input type="hidden" name="camefrommod" value="' . $MOD . '">'
         . '<input type="hidden" name="camefromact" value="' . $ACT . '">'
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
     $previous_link = '<tr><td align="center"><a href="' 
                    . vadmin_mkform_action($LVL, $MOD, 'acctlist') 
                    . '">&lt; ' . _("Previous") . '</a></td></tr>';
     vadmin_make_page($title, null, $body, true, true, $previous_link);
     break;
     
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

