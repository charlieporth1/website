<?php
/**
 * domprefs.mod
 * ------------
 * This module allows setting domain preferences, such as login pictures, and
 * user permissions (autoresponder, password change, etc).
 *
 * Licensed under GNU GPL v2. Please see COPYING for full terms.
 *
 * $Id: domprefs.mod,v 1.17 2009/09/05 02:45:10 pdontthink Exp $
 * 
 * @author Konstantin Riabitsev ($Author: pdontthink $)
 * @version $Date: 2009/09/05 02:45:10 $
 */

$ACT = vadmin_getvar('VADMIN', 'ACT');
$LVL = vadmin_getvar('VADMIN', 'LVL');
$MOD = 'domprefs';
$me = "$MOD.mod/$ACT";
spew("$me: taking over");

$domain = vadmin_getvar('VADMIN',  'VADMIN_DOMAIN');
$color = vadmin_getvar('SQMAIL', 'color');
$username = vadmin_getvar('SQMAIL', 'username');
$desig = vadmin_get_user_designation($domain, $username);
$sizelimit = vadmin_smart_limit($domain, $desig, 'imgsize');
if ($sizelimit != false){
    $sizelimit = $sizelimit*1024;
} else {
    /**
     * Make it something silly
     */
    $sizelimit = 1000000000;
}
$good_types = array('image/gif', 'image/jpeg', 'image/png');


switch ($ACT){
    /****************************** title *****************************/
 case 'title':
     $title = vadmin_get_pref($domain, 'title');
     $action = vadmin_mkform_action($LVL, $MOD, 'settitle');
     $body = ''
         . '<form method="post" enctype="multipart/form-data" '
         . ' action="' . $action . '">'
         . ' <table border="0" align="center">'
         . '  <tr>'
         . '   <td align="center">';
     spew("$me: figure out if they have an image");
     $contents = vadmin_get_pic($domain);
     if ($contents == false){
         $default_login_image = vadmin_get_per_domain_sm_setting('org_logo', $domain);
         if (strpos($default_login_image, '../../') !== 0)
             $default_login_image = '../' . $default_login_image;
         spew("$me: No uploaded image. Get default (it's already in our session: $default_login_image).");

         $body .= ''
             //TODO: try this:
             //. '<p><img src="' . htmlspecialchars($default_login_image) . '" border="1" '
             . '<p><img src="' . $default_login_image . '" border="1" '
             . ' alt="' . _("The default login image") . '" /></p>';
     } else {
         spew("$me: Show custom image and a 'delete' option");
         $body .= ''
             . '<p><img src="vadmin_fetch_pic.php?DOM=' . $domain . '"'
             . ' border="1"'
             . ' alt="' . _("Your custom login image") . '" /><br />'
             . ' <input type="checkbox" name="rmcustom" id="rmcustomid" value="1" />'
             . '<label for="rmcustomid">'
             .   _("Restore the default login image") . '</label></p>';
     }
     $body .= ''
         . '    <p>'
         . '     <input type="hidden" name="MAX_FILE_SIZE" '
         . '      value="' . $sizelimit . '" />'
         .       _("Upload custom image") . ': '
         . '     <input type="file" name="titleimage" />'
         . '    </p>'
         . '   </td>'
         . '  </tr>'
         . '  <tr>'
         . '   <td align="center">'
         . '    <strong>' . _("Title") . ':</strong>'
         . '    <input name="title" size="40" value="' . $title . '" />'
         . '   </td>'
         . '  </tr>'
         . '  <tr>'
         . '   <td bgcolor="' . $color[9] . '" align="center">'
         . '    <input type="submit" '
         . '      value="' . _("Make these changes") . '&gt;&gt;" />'
         . '   </td>'
         . '  </tr>'
         . ' </table>'
         . '</form>';
     $title = _("Customize the login page look");
     $previous_link = '<tr><td align="center"><a href="'
                    . vadmin_mkform_action($LVL, 'email', 'prefmain')
                    . '">&lt; ' . _("Previous") . '</a></td></tr>';
     vadmin_make_page($title, null, $body, true, true, $previous_link);
     break;

     /****************************** settitle ***************************/
 case 'settitle':
     $title = vadmin_getvar('POST', 'title');
     $rmcustom = vadmin_getvar('POST', 'rmcustom');
     if (isset($rmcustom) && $rmcustom == '1'){
         vadmin_del_pic($domain);
     } else {
         $titleimage = vadmin_getvar('FILES', 'titleimage');
         if (isset($titleimage) && $titleimage{'error'} == 0){
             $type = strtolower($titleimage{'type'});
             if (!in_array($type, $good_types)){
                 $msg = _("Only GIF, PNG, and JPEG type images are permitted");
                 vadmin_user_error($msg);
             }
             $size = $titleimage{'size'};
             if ($size > $sizelimit){
                 $msg = sprintf(_("The image is over the limit of %d KiB"),
                                $sizelimit/1024);
                 vadmin_user_error($msg);
             }
             spew("$me: image passes. Load and store.");
             $imgfile = $titleimage{'tmp_name'};
             $fp = fopen($imgfile, 'r');
             $contents = fread($fp, filesize($imgfile));
             fclose($fp);
             unlink($imgfile);
             vadmin_put_pref($domain, 'mimetype', $type);
             vadmin_put_pic($domain, $contents);
         }
     }
     vadmin_put_pref($domain, 'title', $title);

     // if no pic, vadmin_fetch_pic might not work, so just 
     // put the URI in to the default image instead 
     //
     $pic = vadmin_get_pic($domain);
     if ($pic == false){ 
         $default_image_location = vadmin_get_per_domain_sm_setting('org_logo', $domain);
         if (strpos($default_image_location, '../../') !== 0)
             $default_image_location = '../' . $default_image_location;
     }

     $body = ''
         . '<table border="0" width="60%" align="center">'
         . ' <tr>'
         . '  <th bgcolor="' . $color[9] . '">'
         .     _("Login page preferences saved")
         . '  </th>'
         . ' </tr>'
         . ' <tr>'
         . '  <td align="center">'
         . ($pic != FALSE ? '   <img border="1" src="vadmin_fetch_pic.php?DOM='
                            . $domain . '"'
                          : '   <img border="1" src="' . $default_image_location . '"')
         . '    alt="' . _("Login page image") . '" />'
         . '  </td>'
         . ' </tr>'
         . ' <tr>'
         . '  <td align="center">'
         . '   <strong>' . _("Title") . ':</strong> ' . $title
         . '  </td>'
         . ' </tr>'
         . ' <tr>'
         . '  <td bgcolor="' . $color[9] . '">&nbsp;</td>'
         . ' </tr>'
         . '</table>';
     $title = _("Login page preferences");
     $previous_link = '<tr><td align="center"><a href="'
                    . vadmin_mkform_action($LVL, $MOD, 'title')
                    . '">&lt; ' . _("Previous") . '</a></td></tr>';
     vadmin_make_page($title, null, $body, true, true, $previous_link);
     break;

     /******************************** perms **************************/
 case 'perms':

//FIXME: what does this code do?  How is it possible to get here without
//       having first entered a domain password??  don't you have to have
//       a domain password stored in order to log in, meaning ANYTHING
     if (!vadmin_domain_passwd_issaved($domain)){
         spew("$me: this operation requires a stored password. Redirect.");
         vadmin_redirect($LVL, $MOD, 'needdompass', null);
     }

     $passchange = vadmin_get_pref($domain, 'password_change');
     if ($passchange != false){
         $passon = ' checked="yes"';
     } else {
         $passon = '';
     }

     $autores = vadmin_get_pref($domain, 'autoresponder');
     if ($autores != false){
         $autoreson = ' checked="yes"';
     } else {
         $autoreson = '';
     }

     $mailfwd = vadmin_get_pref($domain, 'mail_forwarding');
     if ($mailfwd != false){
         $mailfwdon = ' checked="yes"';
     } else {
         $mailfwdon = '';
     }

     $track_usage = vadmin_get_pref($domain, 'track_usage');
     if ($track_usage != false){
         $track_usage_on = ' checked="yes"';
     } else {
         $track_usage_on = '';
     }

     $pass_plugin = vadmin_getvar('CONFIG', 'backend.user_password_plugin');
     $autores_plugin = vadmin_getvar('CONFIG', 'backend.user_autoresponder_plugin');
     $mail_fwd_plugin = vadmin_getvar('CONFIG', 'backend.user_mail_forwarding_plugin');

     $action = vadmin_mkform_action($LVL, $MOD, 'setperms');
     $body = ''
         . '<form method="post" action="' . $action . '">'
         . ' <table border="0" align="center" width="50%">'
         . '  <tr>'
         . '   <th bgcolor="' . $color[9] . '">'
         .      _("You may set the following features") . ':'
         . '   </th>'
         . '  </tr>';

     if (!empty($pass_plugin))
         $body .= ''
             . '  <tr>'
             . '   <td>'
             . '    <input type="checkbox" name="passchange" id="passchange" value="1"'
             .       $passon . ' /><label for="passchange"> ' . _("Users may change their passwords")
             . '   </label></td>'
             . '  </tr>';

     if (!empty($autores_plugin))
         $body .= ''
             . '  <tr>'
             . '   <td>'
             . '    <input type="checkbox" name="autores" id="autores" value="1"'
             .       $autoreson . ' /><label for="autores"> ' . _("Users may use autoresponder")
             . '   </label></td>'
             . '  </tr>';

     if (!empty($mail_fwd_plugin))
         $body .= ''
             . '  <tr>'
             . '   <td>'
             . '    <input type="checkbox" name="mailfwd" id="mailfwd" value="1"'
             .       $mailfwdon . ' /><label for="mailfwd"> ' . _("Users may control mail forwarding")
             . '   </label></td>'
             . '  </tr>';

     $body .= ''
         . '  <tr>'
         . '   <td>'
         . '    <input type="checkbox" name="track_usage" id="track_usage" value="1"'
         .       $track_usage_on . ' /><label for="track_usage"> ' . _("Track webmail usage")
         . '   </label></td>'
         . '  </tr>';

     // need to put the following hook calls inside an eval()
     // because even just the syntax of the 1.4.x hook call
     // evokes a PHP error in 1.5.x, even though it's not
     // actually called
     //
     $temp = array($VADMIN_DOMAIN, $desig);
     if (check_sm_version(1, 5, 2))
         eval('$more_options = do_hook(\'vadmin_domain_perms_menu\', $temp);');
     else
         eval('$more_options = do_hook_function(\'vadmin_domain_perms_menu\', $temp);');
     if (!empty($more_options)) $body .= $more_options;

     $body .= ''
         . '  <tr>'
         . '   <td bgcolor="' . $color[9] . '" align="center">'
         . '    <input type="submit"'
         . '     value="' . _("Set these features") . ' &gt;&gt;" />'
         . '   </td>'
         . '  </tr>'
         . ' </table>'
         . '</form>';
     $title = sprintf(_("Manage webmail features for %s"), $domain);
     $previous_link = '<tr><td align="center"><a href="'
                    . vadmin_mkform_action($LVL, 'email', 'prefmain')
                    . '">&lt; ' . _("Previous") . '</a></td></tr>';
     vadmin_make_page($title, null, $body, true, true, $previous_link);
     break;

     /****************************** setperms *************************/
 case 'setperms':

     $passchange = vadmin_getvar('POST', 'passchange');
     if (isset($passchange) && $passchange == '1'){
         $passchange = 1;
     } else {
         $passchange = false;
     }

     $autores = vadmin_getvar('POST', 'autores');
     if (isset($autores) && $autores == '1'){
         $autores = 1;
     } else {
         $autores = false;
     }

     $mailfwd = vadmin_getvar('POST', 'mailfwd');
     if (isset($mailfwd) && $mailfwd == '1'){
         $mailfwd = 1;
     } else {
         $mailfwd = false;
     }

     $track_usage = vadmin_getvar('POST', 'track_usage');
     if (isset($track_usage) && $track_usage == '1'){
         $track_usage = 1;
     } else {
         $track_usage = false;
     }

     $pass_plugin = vadmin_getvar('CONFIG', 'backend.user_password_plugin');
     $autores_plugin = vadmin_getvar('CONFIG', 'backend.user_autoresponder_plugin');
     $mail_fwd_plugin = vadmin_getvar('CONFIG', 'backend.user_mail_forwarding_plugin');

     $body = ''
         . '<table border="0" align="center" width="50%">'
         . ' <tr>'
         . '  <th bgcolor="' . $color[9] . '">' . _("Action taken") . '</th>'
         . '  <th bgcolor="' . $color[9] . '">' . _("Result") . '</th>'
         . ' </tr>';

     if (!empty($pass_plugin))
     {
         if ($passchange != vadmin_get_pref($domain, 'password_change')){
             spew("$me: changing the preference for password change");
             vadmin_put_pref($domain, 'password_change', $passchange);
             if ($passchange){
                 $result = _("Enabled");
             } else {
                 $result = _("Disabled");
             }
         } else {
             $result = _("No change");
         }
         $body .= ''
             . '<tr>'
             . ' <td>' . _("Changing password change settings") . ':</td>'
             . ' <td>' . $result . '</td>'
             . '</tr>';
     }
       
     if (!empty($autores_plugin))
     {
         if ($autores != vadmin_get_pref($domain, 'autoresponder')){
             spew("$me: changing the preference for autoresponder");
             vadmin_put_pref($domain, 'autoresponder', $autores);
             if ($autores){
                 $result = _("Enabled");
             } else {
                 $result = _("Disabled");
             }
         } else {
             $result = _("No change");
         }
         $body .= ''
             . '<tr>'
             . ' <td>' . _("Changing autoresponder settings") . ':</td>'
             . ' <td>' . $result . '</td>'
             . '</tr>';
     }
     
     if (!empty($mail_fwd_plugin))
     {
         if ($mailfwd != vadmin_get_pref($domain, 'mail_forwarding')){
             spew("$me: changing the preference for mail forwarding");
             vadmin_put_pref($domain, 'mail_forwarding', $mailfwd);
             if ($mailfwd){
                 $result = _("Enabled");
             } else {
                 $result = _("Disabled");
             }
         } else {
             $result = _("No change");
         }
         $body .= ''
             . '<tr>'
             . ' <td>' . _("Changing mail forwarding settings") . ':</td>'
             . ' <td>' . $result . '</td>'
             . '</tr>';
     }
     
     if ($track_usage != vadmin_get_pref($domain, 'track_usage')){
         spew("$me: changing the preference for webmail usage tracking");
         vadmin_put_pref($domain, 'track_usage', $track_usage);
         if ($track_usage){
             $result = _("Enabled");
         } else {
             $result = _("Disabled");
         }
     } else {
         $result = _("No change");
     }
     $body .= ''
         . '<tr>'
         . ' <td>' . _("Changing webmail usage tracking settings") . ':</td>'
         . ' <td>' . $result . '</td>'
         . '</tr>';

     // need to put the following hook calls inside an eval()
     // because even just the syntax of the 1.4.x hook call
     // evokes a PHP error in 1.5.x, even though it's not
     // actually called
     //
     $temp = array($VADMIN_DOMAIN, $desig);
     if (check_sm_version(1, 5, 2))
         eval('$more_options = do_hook(\'vadmin_domain_perms_menu_submit\', $temp);');
     else
         eval('$more_options = do_hook_function(\'vadmin_domain_perms_menu_submit\', $temp);');
     if (!empty($more_options)) $body .= $more_options;

     $body .= ''
         . '<tr>'
         . ' <td colspan="2" bgcolor="' . $color[9] . '">&nbsp;</td>'
         . '</tr>'
         . '</table>';
     $title = sprintf(_("Setting webmail features for %s"), $domain);
     $previous_link = '<tr><td align="center"><a href="'
                    . vadmin_mkform_action($LVL, $MOD, 'perms')
                    . '">&lt; ' . _("Previous") . '</a></td></tr>';
     vadmin_make_page($title, null, $body, true, true, $previous_link);
     break;

     /****************************** needdompass ************************/
 case 'needdompass':
     $action = vadmin_mkform_action($LVL, $MOD, 'setdompass');
     $body = ''
         . '<form method="post" action="' . $action . '">'
         . ' <table border="0" align="center" width="60%">'
         . '  <tr>'
         . '   <th bgcolor="' . $color[9] . '">'
         .      _("This option requires domain password")
         . '   </th>'
         . '  <tr>'
         . '   <td>'
         .      _("To let users change passwords, configure an autoresponder, and/or control mail forwarding, there will need to be a copy of the system password saved on the disk.")
         . '   </td>'
         . '  </tr>'
         . '  <tr>'
         . '   <td bgcolor="' . $color[9] . '" align="center">'
         . '    <input type="submit" '
         . '     value="' . _("Save domain password") . ' &gt;&gt;" />'
         . '   </td>'
         . '  </tr>'
         . ' </table>'
         . '</form>';
     $title = _("Domain password required");
     $previous_link = '<tr><td align="center"><a href="'
                    . vadmin_mkform_action($LVL, 'email', 'prefmain')
                    . '">&lt; ' . _("Previous") . '</a></td></tr>';
     vadmin_make_page($title, null, $body, true, true, $previous_link);
     break;

     /****************************** setdompass ***********************/
 case 'setdompass':
     $crypto = vadmin_getvar('SESSION', 'VADMIN_SECRET');
     $secret   = vadmin_crypto($crypto, 'decrypt');
     vadmin_put_domain_passwd($domain, $secret);
     vadmin_redirect($LVL, $MOD, 'perms', null);
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
