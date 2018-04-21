<?php
/**
 * autores.mod
 * ------------
 * This module lets users set autoresponders.
 *
 * Licensed under GNU GPL v2. Please see COPYING for full terms.
 *
 * $Id: autores.mod,v 1.6 2008/07/29 19:44:00 pdontthink Exp $
 * 
 * @author Konstantin Riabitsev ($Author: pdontthink $)
 * @version $Date: 2008/07/29 19:44:00 $
 */

$ACT = vadmin_getvar('VADMIN', 'ACT');
$LVL = vadmin_getvar('VADMIN', 'LVL');
$MOD = 'autores';
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
$autores = vadmin_get_pref($domain, 'autoresponder');
if (!$autores) {
    spew("$me: You're not supposed to be here! Bad boy!");
    vadmin_security_breach();
}

$color = vadmin_getvar('SQMAIL', 'color');
$secret = vadmin_get_domain_passwd($domain);
$title = _("Your autoresponder settings");

switch ($ACT){
    /******************************* main *******************************/    
 case 'main':
     list($code, $stat) = vautoresponsestatus($domain, $secret, $uname);
     if ($stat == 'enabled'){
         $action = vadmin_mkform_action($LVL, $MOD, 'remove');
         $repl = vreadautoresponse($domain, $secret, $uname);
         $message_and_headers = vadmin_parse_autoresponder_message($repl[1]);
         $message = nl2br(htmlspecialchars($message_and_headers['message']));
         unset($message_and_headers['message']);
         $show = ''
             . '<tr>'
             . ' <th bgcolor="' . $color[9] . '">'
             .    _("Your autoresponder is enabled")
             . ' </th>'
             . '</tr>';

         foreach ($message_and_headers as $header => $header_val) {
             if ($header == 'from') continue;
             $show .= ''
                 . '<tr>'
                 . ' <td>'
                 . '  <p><strong>' . htmlspecialchars(_(ucfirst($header))) . ':</strong><br />'
                 .     htmlspecialchars($header_val)
                 . '  </p>'
                 . ' </td>'
                 . '</tr>';
         }

         $show .= ''
             . '<tr>'
             . ' <td>'
             . '  <p><strong>' . _("Message") . ':</strong><br />'
             .     $message
             . '  </p>'
             . ' </td>'
             . '</tr>'
             . '<tr>'
             . ' <td bgcolor="' . $color[9] . '" align="center">'
             . '  <input type="submit" '
             . '    value="' . _("Disable autoresponder") . '" />'
             . ' </td>'
             . '</tr>';
     } else {
         $action = vadmin_mkform_action($LVL, $MOD, 'set');
         $header_list = vadmin_getvar('CONFIG', 'autoresponder.message_contains_headers');
         if (strpos($header_list, ',') !== false) {
             $header_list = explode(',', $header_list);
             @array_walk($header_list, 'vadmin_trim_array');
         } else {
             $header_list = array($header_list);
         }

         $repl = vreadautoresponse($domain, $secret, $uname);
         if ($repl[0] == 0){
             $message_and_headers = vadmin_parse_autoresponder_message($repl[1]);
             $message = htmlspecialchars($message_and_headers['message']);
             unset($message_and_headers['message']);
         } else {
             $message = _("I am currently away from my email. I will respond to your message when I return.");
             $message_and_headers = array();
         }
         $show = ''
             . '<tr>'
             . ' <th bgcolor="' . $color[9] . '">'
                  . _("Your autoresponder is not enabled")
             . ' </th>'
             . '</tr>';

         foreach ($header_list as $header) {

             if (empty($header) || $header == 'from') continue;

             if (!empty($message_and_headers[$header]))
                 $header_val = $message_and_headers[$header];
             else
                 $header_val = '';

             $show .= ''
                 . '<tr>'
                 . ' <td>'
                 . '  <p><strong>' . htmlspecialchars(_(ucfirst($header))) . ':</strong><br />'
                 . '   <input type="text" size="40" name="' . $header . '" value="' . htmlspecialchars($header_val) . '" />'
                 . '  </p>'
                 . ' </td>'
                 . '</tr>';
         }

         $show .= ''
             . '<tr>'
             . ' <td>'
             . '  <p><strong>' . _("Message") . ':</strong><br />'
             . '   <textarea name="message" cols="70" rows="10" '
             . '    wrap="hard">' . $message . '</textarea>'
             . '  </p>'
             . ' </td>'
             . '</tr>'
             . '<tr>'
             . ' <td bgcolor="' . $color[9] . '" align="center">'
             . '  <input type="submit" '
             . '   value="' . _("Enable autoresponder") . '" />'
             . ' </td>'
             . '</tr>';
     }
     $body = ''
         . '<form method="post" action="' . $action . '">'
         . ' <table border="0" width="70%" align="center">'
         .    $show
         . ' </table>'
         . '</form>';
     vadmin_make_page($title, null, $body, false, false);
     break;

     /******************************* set ****************************/
 case 'set':

     $header_list = vadmin_getvar('CONFIG', 'autoresponder.message_contains_headers');
     if (empty($header_list))
     { /* no-op */ }
     else if (strpos($header_list, ',') !== false) {
         $header_list = explode(',', $header_list);
         @array_walk($header_list, 'vadmin_trim_array');
     } else {
         $header_list = array($header_list);
     }

     /**
      * Get the message directly from $_POST as opposed to vadmin_getvar
      * so we don't have automatic anti-xss, which doesn't look so good in
      * an email message.
      */
     $message = $_POST{'message'};
     if ($message == ''){
         vadmin_user_error(_("Please provide an autoresponse message. It cannot be an empty string."));
     }

     // get each header as well
     //
     $header_text = '';
     if (!empty($header_list)) {
         
         foreach ($header_list as $header) {

             if (empty($header)) continue;

             $header_val = $_POST{$header};

             if ($header == 'from') {
                 $delim = vadmin_getvar('BACKEND', 'delimiters');
                 $delim = substr($delim, 0, 1);
                 $header_text .= 'From: ' . $uname . $delim . $udomain . "\n";
                 continue;
             }

             $header_text .= ucfirst($header) . ': ' . $header_val . "\n";

         }

         $header_text .= "\n";
         $message = $header_text . $message;

     }

     list($code, $stat) = vautoresponsestatus($domain, $secret, $uname);
     if ($stat == 'disabled'){
         $repl = venableautoresponse($domain, $secret, $uname);
         if ($repl[0]){
             vadmin_system_error($repl[1]);
         }
     }
     $repl = vwriteautoresponse($domain, $secret, $uname, $message);
     if ($repl[0]){
         vadmin_system_error($repl[1]);
     }
     if ($stat == 'nonexistant'){
         $repl = venableautoresponse($domain, $secret, $uname);
         if ($repl[0]){
             vadmin_system_error($repl[1]);
         }
     }         
     /**
      * Redirect back to main
      */
     vadmin_redirect($LVL, $MOD, 'main', null);
     break;

     /******************************* remove **************************/
 case 'remove':
     $repl = vdisableautoresponse($domain, $secret, $uname);
     if ($repl[0]){
         vadmin_system_error($repl[1]);
     }
     /**
      * Redirect back to main
      */
     vadmin_redirect($LVL, $MOD, 'main', null);
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
