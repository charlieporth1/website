<?php
/**
 * vmailmgr_functions.inc
 * ----------------------
 * All VmailMgr backend functions are in this file.
 *
 * Licensed under GNU GPL v2. Please see COPYING for full terms.
 *
 * $Id: vmailmgr_functions.inc,v 1.3 2008/12/30 09:59:51 pdontthink Exp $
 *
 * @author Konstantin Riabitsev ($Author: pdontthink $)
 * @version $Date: 2008/12/30 09:59:51 $
 *
 */

function vadmin_list_virtual_domains(){
    $me = 'vadmin_list_virtual_domains';
    $control = vadmin_getvar('CONFIG', 'paths.qmailcontrol');
    $vfile = "$control/virtualdomains";
    spew("$me: Looking at $vfile now");
    if (!file_exists($vfile) || !is_readable($vfile)){
        spew("$me: Could not find $vfile!");
        $error = sprintf(_("Could not find virtual domains file %s!"), $vfile);
        vadmin_system_error($error);
    }
    $fd = fopen($vfile, 'r');
    $contents = fread($fd, filesize($vfile));
    $contents = rtrim($contents);
    if (strlen($contents) > 0){
        spew("$me: got $vfile, size is " . strlen($contents));
        $domuary = explode("\n", $contents);
        $domary = array();
        foreach ($domuary as $domu){
            if ($domu{0} != '#'){
                $domain = preg_replace('/:.*/s', '', $domu);
                spew("$me: found $domain");
                array_push($domary, $domain);
            }
        }
        return $domary;
    } else {
        spew("$me: Uhm... No domains seem to be defined.");
        $error = sprintf(_("No virtual domains defined in %s!"), $vfile);
        vadmin_system_error($error);
    }
}

/**
 * Tries to validate the supplied domain/password pair by
 * connecting to the vmailmgr daemon and getting a
 * listing for the domain in question.
 *
 * @param $domain vadmin domain.
 * @param $passwd the password for this domain.
 * @return        true if successful, or false if not.
 */
function vadmin_domain_passwd_validate($domain, $passwd){
    $me = 'vadmin_domain_passwd_validate';
    spew("$me: Trying to validate password for domain '$domain'");
    $reply = vm_daemon_raw(array('listdomain', $domain, $passwd));
    if (is_array($reply)){
        /**
         * did not validate. Return the error message.
         */
        spew("$me: password did not validate!");
        spew("$me: Error: " . $reply[1]);
        return false;
    } else {
        /**
         * validation successful. Return false.
         */
        spew("$me: password validated successfully");
        return true;
    }
}

/**
  * Determines if the given data represents
  * an alias or not
  *
  * @param array Typical account/alias data array as returned
  *              from a lookup() call
  *
  * @return boolean TRUE if the data represents an alias,
  *                 FALSE if not.
  *
  */
function is_alias($user_data)
{
   return empty($user_data[2]);
}

