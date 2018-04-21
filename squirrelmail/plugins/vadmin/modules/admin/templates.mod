<?php
/**
 * templates.mod
 * -------------
 * This module lets you define templates for new domains.
 *
 * Licensed under GNU GPL v2. Please see COPYING for full terms.
 *
 * $Id: templates.mod,v 1.13 2009/09/05 05:27:06 pdontthink Exp $
 * 
 * @author Konstantin Riabitsev ($Author: pdontthink $)
 * @version $Date: 2009/09/05 05:27:06 $
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
$MOD = 'templates';
$me = "$MOD.mod/$ACT";
spew("$me: taking over");

$domain = vadmin_getvar('VADMIN',  'VADMIN_DOMAIN');
$color = vadmin_getvar('SQMAIL', 'color');
$username = vadmin_getvar('SQMAIL', 'username');
$designation = vadmin_get_user_designation($domain, $username);
if ($designation != 'ELVIS'){
    spew("$me: only elvis can be here");
    vadmin_security_breach();
}

// what fields are to be displayed on this screen?
//
$displayablefields = vadmin_get_displayable_fields('domain_limits', 'mailboxes, hardquota, size, count, imgsize');


switch ($ACT){
    /******************************* main *****************************/
 case 'main':
     $tmplname = vadmin_get_storparams();
     if (!isset($tmplname) || !$tmplname){
         $tmplname = '';
         $smalltitle = _("Domain Creation Templates");
     } else {
         $smalltitle = sprintf(_("Template '%s' processed"), $tmplname);
     }
     $body = ''
         . '<table border="0" width="50%" align="center">'
         . ' <tr>'
         . '  <th bgcolor="' . $color[9] . '">' . $smalltitle . '</th>'
         . ' </tr>';
     $names = vadmin_get_template_list();
     $action = vadmin_mkform_action($LVL, $MOD, 'edit');
     if (count($names)){
         $body .= ''
             . '<tr><td align="center">'
             . '<form method="post" action="' . $action . '">'
             . '<select name="tmplname">';
         foreach ($names as $name){
             if ($name == $tmplname){
                 $body .= '<option selected="yes">';
             } else {
                 $body .= '<option>';
             }
             $body .= $name . '</option>';
         }
         $body .= ''
             . '</select>&nbsp;'
             . '<input type="submit" value="  ' . _("Edit") . '  " />'
             . '</form>'
             . '</td></tr>';
     } else {
         $body .= ''
             . '<tr><td><p>'
             . _("No domain templates are currently defined. Please click the button below to create one")
             .'</p></td></tr>';
     }
     $body .= ''
         . '<tr><th bgcolor="' . $color[9] . '">'
         . '<form method="post" action="' . $action . '">'
         . ' <input type="hidden" name="addnew" value="1" />'
         . ' <input type="submit" value="' . _("Create a New Template") .'"/>'
         . '</form>'
         . '</th></tr>'
         . '</table>';
     $title = _("Domain creation templates");
     vadmin_make_page($title, null, $body, true, true);
     break;


     /***************************** edit ******************************/
 case 'edit':
     $addnew = vadmin_getvar('POST', 'addnew');

     $default_contents = array(
        'mailboxes' => vadmin_getvar('CONFIG', 'limits.default_mailbox_limit'),
        'hardquota' => vadmin_getvar('CONFIG', 'limits.default_hard_quota_limit'),
        'size' => vadmin_getvar('CONFIG', 'limits.default_message_size_limit'),
        'count' => vadmin_getvar('CONFIG', 'limits.default_message_count_limit'),
        'imgsize' => vadmin_getvar('CONFIG', 'limits.default_image_size_limit'),
        'password_change' => vadmin_getvar('CONFIG', 'limits.default_allow_password_change'),
        'autoresponder' => vadmin_getvar('CONFIG', 'limits.default_allow_autoresponder'),
        'mail_forwarding' => vadmin_getvar('CONFIG', 'limits.default_allow_mail_forwarding'),
        'track_usage' => vadmin_getvar('CONFIG', 'limits.default_enable_usage_tracking'),
     );

     if (!isset($addnew) || $addnew != '1') {
         $tmplname = vadmin_getvar('POST', 'tmplname');
         $oldtmplname = $tmplname;
         $contents = vadmin_get_template($tmplname);
         foreach ($displayablefields as $field)
             if (!isset($contents[$field]))
                 $contents[$field] = $default_contents[$field];
         $title = sprintf(_("Editing template '%s'"), $tmplname);
     } else {
         $oldtmplname = '__new__';
         $tmplname = '';

         $contents = $default_contents;

         $title = _("Creating a new domain template");
     }
     $pass_plugin = vadmin_getvar('CONFIG', 'backend.user_password_plugin');
     $autores_plugin = vadmin_getvar('CONFIG', 'backend.user_autoresponder_plugin');
     $mail_fwd_plugin = vadmin_getvar('CONFIG', 'backend.user_mail_forwarding_plugin');
     $action = vadmin_mkform_action($LVL, $MOD, 'add');
     $body = ''
         . '<form method="post" action="' . $action . '">'
         . '<input type="hidden" name="oldtmplname" value="' 
         .    $oldtmplname . '" />'
         . '<table border="0" width="60%" align="center">'
         . ' <tr>'
         . '  <th bgcolor="' . $color[9] . '" colspan="2">'
         .     _("Set template parameters")
         . '  </th>'
         . ' </tr>'
         . ' <tr>'
         . '  <td>' . _("Template name") . '</td>'
         . '  <td><input size="15" name="tmplname" value="'
         .       $tmplname . '" /></td>'
         . (in_array('mailboxes', $displayablefields)
         ? ' <tr>'
         . '  <td>' . _("Maximum allowed mailboxes") . '</td>'
         . '  <td><input size="5" name="mailboxes" value="' 
         .       vadmin_zval($contents['mailboxes']) .'"/></td>'
         . ' </tr>'
         : '')
         . (in_array('hardquota', $displayablefields)
         ? '<tr>'
         . '  <td>' . _("Maximum allowed hard quota per user (MiB)") . '</td>'
         . '  <td><input size="5" name="hardquota" value="' 
         .       vadmin_zval($contents['hardquota']) . '" /></td>'
         . ' </tr>'
         : '')
         . (in_array('size', $displayablefields)
         ? '<tr>'
         . '  <td>' . _("Maximum allowed message size per user (MiB)") . '</td>'
         . '  <td><input size="5" name="size" value="' 
         .       vadmin_zval($contents['size']) . '" /></td>'
         . ' </tr>'
         : '')
         . (in_array('count', $displayablefields)
         ? '<tr>'
         . '  <td>' . _("Maximum message count per user") . '</td>'
         . '  <td><input size="5" name="count" value="' 
         .       vadmin_zval($contents['count']) . '" /></td>'
         . ' </tr>'
         : '')
         . (in_array('imgsize', $displayablefields)
         ? '<tr>'
         . '  <td>' . _("Maximum front page image size (KiB)") . '</td>'
         . '  <td><input size="5" name="imgsize" value="' 
         .       vadmin_zval($contents['imgsize']) . '" /></td>'
         . ' </tr>'
         : '')
         . '<tr>';

     if (!empty($pass_plugin)) {
         $body .= ''
             . '  <td><label for="password_change">' 
             . _("Allow users to change their passwords") . '</label></td>'
             . '  <td><input type="checkbox" name="password_change"'
             . '       id="password_change" value="1"';
         if (!empty($contents['password_change'])){
             $body .= ' checked="yes"';
         }
         $body .= ''
             . '       /></td>'
             . ' </tr>';
     }

     if (!empty($autores_plugin)) {
         $backend_name = vadmin_getvar('CONFIG', 'backend.type');
         $body .= ''
             . ' <tr>'
             . '  <td><label for="autoresponder">' 
             . ($backend_name == 'vmailmgr' && $autores_plugin == 'internal'
             ? _("Let users use the autoresponder (requires qmail-autoresponder)")
             : _("Let users use the autoresponder"))
             . '</label></td>'
             . '  <td><input type="checkbox" name="autoresponder"'
             . '             id="autoresponder" value="1"';
         if (!empty($contents['autoresponder'])){
             $body .= ' checked="yes"';
         }
         $body .= ''
             . '       /></td>'
             . ' </tr>';
     }

     if (!empty($mail_fwd_plugin)) {
         $body .= ''
             . ' <tr>'
             . '  <td><label for="mailfwd">' 
             . _("Let users control mail forwarding")
             . '</label></td>'
             . '  <td><input type="checkbox" name="mailfwd"'
             . '             id="mailfwd" value="1"';
         if (!empty($contents['mail_forwarding'])){
             $body .= ' checked="yes"';
         }
         $body .= ''
             . '       /></td>'
             . ' </tr>';
     }

     $body .= ''
         . ' <tr>'
         . '  <td><label for="track_usage">' 
         . _("Track webmail usage")
         . '</label></td>'
         . '  <td><input type="checkbox" name="track_usage"'
         . '             id="track_usage" value="1"';
     if (!empty($contents['track_usage'])){
         $body .= ' checked="yes"';
     }
     $body .= ''
         . '       /></td>'
         . ' </tr>';

     $body .= ''
         . ' <tr>'
         . '  <th colspan="2" bgcolor="' . $color[9] . '">'
         . '   <input type="submit" value="' . _("Save template")
         .      ' &gt;&gt;" />'
         . '  </th></form>';
     if ($oldtmplname != '__new__'){
         $action = vadmin_mkform_action($LVL, $MOD, 'delete');
         $body .= ''
             . ' </tr><form method="post" action="' . $action . '">'
             . ' <input type="hidden" name="tmplname" value="'.$tmplname.'" />'
             . ' <tr>'
             . '  <th colspan="2" bgcolor="' . $color[9] . '">'
             . '   <input type="submit" value="' . _("Delete template") . '"/>'
             . '  </th>';
     }
     $body .= ''
         . ' </tr><tr>'
         . '  <td colspan="2">'
         . '   <p>'
         . _("Domain templates let you specify a set of default parameters that can be applied to the domains you enable. These parameters include limits, restrictions, and some preferences as well")
         . '   </p>'
         . '  </td>'
         . ' </tr>'
         . '</table></form>';
     $previous_link = '<tr><td align="center"><a href="'
                    . vadmin_mkform_action($LVL, $MOD, 'main')
                    . '">&lt; ' . _("Previous") . '</a></td></tr>';
     vadmin_make_page($title, null, $body, true, true, $previous_link);
     break;

     /****************************** add *****************************/
 case 'add':
     $tmplname = vadmin_getvar('POST', 'tmplname');
     if (!isset($tmplname) || $tmplname == ''){
         $msg = _("Please provide a template name");
         vadmin_user_error($msg);
     }
     $oldtmplname = vadmin_getvar('POST', 'oldtmplname');
     $mailboxes = vadmin_getvar('POST', 'mailboxes');
     $hardquota = vadmin_getvar('POST', 'hardquota');
     $size = vadmin_getvar('POST', 'size');
     $count = vadmin_getvar('POST', 'count');
     $imgsize = vadmin_getvar('POST', 'imgsize');
     $password_change = vadmin_getvar('POST', 'password_change');
     if (!isset($password_change) || $password_change != '1'){
         $password_change = 0;
     } else {
         $password_change = 1;
     }
     $autoresponder = vadmin_getvar('POST', 'autoresponder');
     if (!isset($autoresponder) || $autoresponder != '1'){
         $autoresponder = 0;
     } else {
         $autoresponder = 1;
     }
     $mail_forwarding = vadmin_getvar('POST', 'mailfwd');
     if (!isset($mail_forwarding) || $mail_forwarding != '1'){
         $mail_forwarding = 0;
     } else {
         $mail_forwarding = 1;
     }
     $track_usage = vadmin_getvar('POST', 'track_usage');
     if (!isset($track_usage) || $track_usage != '1'){
         $track_usage = 0;
     } else {
         $track_usage = 1;
     }

     $contents = array(
                       'password_change' => $password_change,
                       'autoresponder' => $autoresponder,
                       'mail_forwarding' => $mail_forwarding,
                       'track_usage' => $track_usage,
                      );
     if (in_array('mailboxes', $displayablefields))
         $contents['mailboxes'] = vadmin_zval($mailboxes);
     if (in_array('hardquota', $displayablefields))
         $contents['hardquota'] = vadmin_zval($hardquota);
     if (in_array('size', $displayablefields))
         $contents['size'] = vadmin_zval($size);
     if (in_array('count', $displayablefields))
         $contents['count'] = vadmin_zval($count);
     if (in_array('imgsize', $displayablefields))
         $contents['imgsize'] = vadmin_zval($imgsize);

     if ($oldtmplname != '__new__' && $oldtmplname != $tmplname){
         spew("$me: deleting old template '$tmplname'");
         vadmin_delete_template($oldtmplname);
     }
     vadmin_put_template($tmplname, $contents);
     /**
      * I'm lazy. Just redirect them back to main
      */
     vadmin_redirect($LVL, $MOD, 'main', $tmplname);
     break;
     
     /***************************** delete *****************************/
 case 'delete':
     $tmplname = vadmin_getvar('POST', 'tmplname');
     vadmin_delete_template($tmplname);
     spew("$me: deleted template '$tmplname'");
     vadmin_redirect($LVL, $MOD, 'main', $tmplname);
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
