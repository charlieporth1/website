<?php
/**
 * Copyright (C) 2000 Mike Bell <mike@mikebell.org>
 *
 * Examples available at http://mailtest.mikebell.org/
 * Latest version will be included in each vmailmgr release
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Cleaned up and made work with E_ALL in Feb, 2003 
 * by Konstantin Riabitsev <icon@duke.edu>
 *
 * Made compatible with VMailMgr listening on an IP address
 * and better socket file autosensing in Oct, 2007 by Paul 
 * Lesniewski <paul@squirrelmail.org>
 *
 */

function vm_daemon_raw($args){

    // where is vmailmgr listening?
    //
    $cfgfile = '/etc/vmailmgr/socket-file';
    if (file_exists($cfgfile)){
        $socketfile = file($cfgfile);
        $socketfile = trim($socketfile[0]);
    } else {
        $socketfile = '/tmp/.vmailmgrd';
    }

    // on an IP address?  (format xx.xx.xx.xx or xx.xx.xx.xx:xx)
    //
    if (preg_match('/^\d+\.\d+\.\d+\.\d+($|:(\d+)$)/', $socketfile, $matches)){
        if (empty($matches[2]))
            $port = 322;
        else
            $port = $matches[2];
    } else
        $port = 0;

    $vmailsock = fsockopen($socketfile, $port, $errno, $errstr, 4);
    if (!$vmailsock)
        $vmailsock = fsockopen('unix://' . $socketfile, $port, $errno, $errstr, 4);
    if (!$vmailsock){
        $msg = 'Failed to open socket file "%s". Is vmailmgrd running?';
        die(sprintf($msg, $socketfile));
    }
    
    /**      
     * Parse $args, which should be an array of arguments to pass to the
     * daemon, into a glob consisting of each argument proceeded by a
     * two-byte representation of its length. 
     */
    $command = '';
    foreach ($args as $arg){
        $commandlength = strlen($arg);
        $high = ($commandlength & (0xFF << 8)) >> 8;
        $low = ($commandlength & 0xFF);
        $command .= sprintf('%c%c%s', $high, $low, $arg);
    }
    
    /**
     * Create the header, which consists of another two byte length
     * representation, the number of arguments being passed, and the
     * command string created above. 
     */
    $arglen = sizeof($args) - 1;
    $commandlength = strlen($command);
    $high = ($commandlength & (255 << 8)) >> 8;
    $low = $commandlength & 255;
    $commandstr = sprintf("\002%c%c%c%s", $high, $low + 1, $arglen, $command);
    /**
     * Pass it all to the daemon 
     */
    fputs($vmailsock, $commandstr);
    
    /**
     * Get the response
     */
    $value = ord(fgetc($vmailsock));
    $length = (ord(fgetc($vmailsock)) << 8) + ord(fgetc($vmailsock));
    
    if ($length == 0) {
        $out = '';
        while (!feof($vmailsock))
            $out .= fread($vmailsock, 65535);
        fclose($vmailsock);
        return $out;
    }

    $message = fread($vmailsock, $length);
    
    /**
     * Close the socket	
     */
    fclose($vmailsock);
    return array($value, $message);
}

function listdomain_parse_userdata($line, $username){
	/**
     * Grab the protocol version
     */
	$ver = ord(substr($line, 0, 1));
	if ($ver != 2){
        $msg = 'Error communicating with vmailmgrd, or incorrect protocol '
            . 'version used. Protocol version 2 expected.';
        die($msg);
    }
	/**
     * Chop off the version and grab the flags.
     */
	$line = substr($line, 1);
	while (substr($line, 0, 1) != chr(0)){
		$flags[ord(substr($line, 0, 1))] = ord(substr($line, 1, 1));
		$line = substr($line, 2);
	}
	/**
     * Explode the remainder into an array based on the NULs used to
     * end the fields
     */
	$user = explode(chr(0), $line);
	$x = 1;
    $password = '';
	if ($user[$x++] != '*')
        $password = 'Set';
	$mailbox = $user[$x++];
    $aliases = Array();
	while($user[$x] != '')
		$aliases[] = $user[$x++];
	$x++;
	$PersonalInfo = $user[$x++];
	$HardQuota = $user[$x++];
	$SoftQuota = $user[$x++];
	$SizeLimit = $user[$x++];
	$CountLimit = $user[$x++];
	$CreationTime = $user[$x++];
	$ExpiryTime = $user[$x++];
	
	return array($username, $password, $mailbox, $aliases,
                 $PersonalInfo, $HardQuota, $SoftQuota, $SizeLimit, 
                 $CountLimit, $CreationTime, $ExpiryTime, $flags);
}

/**
 * Parses the lines from listdomain into fields. 
 */
function listdomain_parse_line($line){
	/**
     *Grab the user's name
     */
	$data_position = strpos($line, chr(0));
	$username = substr($line, 0, $data_position);
	/**
     * Send that user's data to be parsed. The $username is nessesary
     * because array manipulation in PHP3 is so limited.
     */
	return listdomain_parse_userdata(substr($line, $data_position + 1), 
                                     $username);
}

/**
 * Does the ugly stuff for listdomain, and calls listdomain_parse_line
 * once for each user
 */
function listdomain_parse($output){
	$cur = 1;
    $ret_ary = Array();
	while (true) {
        $linelength = (ord(substr($output, $cur++, 1)) << 8 ) 
            + ord(substr($output, $cur++, 1));
        if ($linelength == 0) break;
        $data = listdomain_parse_line(substr($output, $cur, $linelength));
        array_push($ret_ary, $data);
        $cur += $linelength + 1;
    }
    return $ret_ary;
}

/**
  * Get full account list for a given domain.
  * Output is parsed as stated in listdomain_parse_userdata
  *     
  * Queries out a list of users and aliases and their
  * account attributes for any one domain.
  * 
  * NOTE that if pagination is being used, a global variable
  * called $LISTDOMAIN_TOTAL_USERS will be set to the number
  * of overall users and aliases in the database so the
  * caller can calculate the total number of pages to display
  * to the user.  Also, an extra (optional) global array called
  * $LISTDOMAIN_FIRST_USER_PER_PAGE will be made available,
  * containing a list of the usernames that are first on
  * every page; the array starts with key number zero (for page
  * number one).
  *   
  * @param string $domain The vadmin domain.
  * @param string $password The domain password.
  * @param int $limit If given, the number of results to limit the
  *                   query to for pagination. (optional; default = 0;
  *                   no pagination)
  * @param int $pageNumber If given, the page number of result sets to
  *                        limit the query to for pagination (note this
  *                        is NOT the offset
  *                        (offset = ($pageNumber - 1) * $limit)).
  *                        (optional; default first page if there is pagination)
  * @param string $sortCol If given, indicates which data element
  *                        should be sorted upon. (optional; default='',
  *                        sort by name)
  * @param string $searchTerms If given, specifies a pattern upon which
  *                            users should be searhced instead of
  *                            returning all users, returns only those
  *                            that match the pattern (optional; default = '')
  * @param int $activeLookup If -1, only inactive users will be returned
  *                          (accounts that have never used webmail, no
  *                          aliases), if 1, only active accounts will be
  *                          returned (users that have used webmail at least
  *                          once, no aliases), if 0, all accounts and aliases
  *                          are returned.  (optional; default = 0)
  *
  * @return array An array of user information arrays; each user
  *               information array contains the fields as described
  *               in listdomain_parse_userdata:
  *                    username
  *                    password
  *                    mailbox
  *                    aliases
  *                    PersonalInfo
  *                    HardQuota
  *                    SoftQuota
  *                    SizeLimit
  *                    CountLimit
  *                    CreationTime
  *                    ExpiryTime
  *                    flags
  *
  */
function listdomain($domain, $password, $limit=0, $pageNumber=0, $sortCol='', $searchTerms='', $activeLookup=0)
{
    $command = array('listdomain', $domain, $password);
    $temp = vm_daemon_raw(array('listdomain', $domain, $password));
    if (is_array($temp)) return $temp;

    $users = listdomain_parse($temp);


   // if active or inactive users only should be returned,
   // strip out anything else here
   //
   if ($activeLookup != 0)
   {

      $newUsers = array();
      foreach ($users as $user)
      {

         if (!isset($user[13]['LAST_LOGIN']))
            $user[13] = vadmin_get_user_webmail_stats($user[0], $domain);

         if ($activeLookup == -1
          && $user[13]['NEVER_LOGGED_IN'] && !is_alias($user))
            $newUsers[] = $user;

         else if ($activeLookup == 1 && !$user[13]['NEVER_LOGGED_IN'])
            $newUsers[] = $user;

      }

      $users = $newUsers;

   }


   // was this a search?  filter out users that don't match search terms
   //
   if (!empty($searchTerms))
   {

      // build regexp
      //
      $searchTerms = str_replace('*', '.*?', $searchTerms);

      $newUsers = array();
      foreach ($users as $user)
      {

         if (preg_match("/^$searchTerms\$/i", $user[0]))
            $newUsers[] = $user;

      }

      $users = $newUsers;

   }



   // sort as needed
   //
   if ($sortCol == 'login') {
      global $sortdomain, $sortcatchall_alias, $sortusername,
             $sortdesig, $sort_add_delim;
      $sortdomain = $domain;
      $sortcatchall_alias = vadmin_getvar('BACKEND', 'catchall_alias');
      $sortusername = vadmin_getvar('SQMAIL', 'username');
      $sort_add_delim = vadmin_IMAP_usernames_have_domain($domain);
      $sortdesig = vadmin_get_user_designation($domain, $sortusername);
      usort($users, 'sortUserArrayByLoginTime');
   }
   else
      usort($users, 'sortUserArrayByName');



   // unfortunately, we have to paginate manually...
   //
   if ($limit)
   {

      $fancy_pagination = vadmin_getvar('BACKEND', 'fancy_pagination');

      global $LISTDOMAIN_TOTAL_USERS;
      $LISTDOMAIN_TOTAL_USERS = count($users);

      if ($fancy_pagination == 'yes')
      {
         global $LISTDOMAIN_FIRST_USER_PER_PAGE;
         $LISTDOMAIN_FIRST_USER_PER_PAGE = array();
      }

      if (!$pageNumber) $pageNumber = 1;
      $offset = ($pageNumber - 1) * $limit;

      $newUsers = array();
      $i = -1;
      foreach ($users as $user)
      {
         if ($fancy_pagination == 'yes' && (($i + 1) % $limit == 0))
            if ($sortCol == 'login')
               $LISTDOMAIN_FIRST_USER_PER_PAGE[] = $user[13]['LAST_LOGIN_SHORT'] . ' (' . $user[0] . ')';
            else
               $LISTDOMAIN_FIRST_USER_PER_PAGE[] = $user[0];

         if (++$i < $offset) continue;
         if ($i >= $offset + $limit)
         {
            if ($fancy_pagination == 'yes')
               continue;
            else
               break;
         }

         $newUsers[] = $user;
      }

      $users = $newUsers;

   }


    return $users;
}

/**
 * Lookup, returns for a single virtual user what listdomain does for
 * an entire domain.
 */
function lookup($domain, $user, $password){
	$command = array("lookup", $domain, $user, $password);
	$tmp = vm_daemon_raw($command);
	return listdomain_parse_userdata($tmp[1], $user);
}
			
/**
 * vadduser, takes domain name, password, username, userpassword, and
 * an array of forwarding desinations, returns an array consisting of
 * an integer exit code and message.
 */
function vadduser($domain, $password, $username, $userpass, $forwards){
	$command = array('adduser2', $domain, $username, $password,
                     $userpass, $username);
    foreach ($forwards as $forward)
		if ($forward != '') array_push($command, $forward);
    return vm_daemon_raw($command);
}

/**
 * vaddalias, takes domain name, password, username, userpassword, and
 * an array of forwarding desinations, returns an array consisting of
 * an integer exit code and message. If the user's password is left
 * empty an alias with no password will be created.
*/
function vaddalias($domain, $password, $username, $userpass, $forwards){
	$command = array('adduser2', $domain, $username, $password,
                     $userpass, '');
    foreach ($forwards as $forward)
		if ($forward != '') array_push($command, $forward);
    return vm_daemon_raw($command);
}

/**
 * vdeluser, takes domain name, password, and username, returns an
 * array consisting of an integer exit code and message.
 */
function vdeluser($domain, $password, $username){
	$command = array('deluser', $domain, $username, $password);
    return vm_daemon_raw($command);
}

/**
 * vchpass, takes domain name, password, username and a new password,
 * returns an array consisting of an integer exit code and
 * message. Scripts allowing users to change their own passwords
 * should check the password was entered correctly by having the user
 * enter it twice and checking these are equal
*/
function vchpass($domain, $password, $username, $newpass){
	$command = array('chattr', $domain, $username, $password, '1', $newpass);
    return vm_daemon_raw($command);
}

/**
 * vchforward, takes domain name, password, username and an array of
 * forwarding addresses, returns an array consisting of an integer
 * exit code and message.
 */ 
function vchforward($domain, $password, $username, $forwards){
	$command = array('chattr', $domain, $username, $password, '2');
    if (sizeof($forwards) > 0){
        foreach ($forwards as $forward)
            if ($forward != '') array_push($command, $forward);
    } else {
        array_push($command, '');
    }
    return vm_daemon_raw($command);
}

function vchattr($domain, $password, $username, $attr, $value){
	$ATTR = array(
                  'PASS' => '1',
                  'DEST' => '2',
                  'HARDQUOTA' => '3',
                  'SOFTQUOTA' => '4',
                  'MSGSIZE' => '5',
                  'MSGCOUNT' => '6',
                  'EXPIRY' => '7',
                  'MAILBOX_ENABLED' => '8',
                  'PERSONAL' => '9'
                  );
	$command = array ('chattr', $domain, $username, $password, $ATTR[$attr], 
                      $value);
    return vm_daemon_raw($command);
}

function vwriteautoresponse($domain, $password, $username, $message){
	$command = array('autoresponse', $domain, $username, $password, 'write', 
                     $message);
	return vm_daemon_raw($command);
}

function vreadautoresponse($domain, $password, $username){
	$command = array('autoresponse', $domain, $username, $password, 'read');
	return vm_daemon_raw($command);
}

function vdisableautoresponse($domain, $password, $username){
	$command = array('autoresponse', $domain, $username, $password, 'disable');
	return vm_daemon_raw($command);
}

function venableautoresponse($domain, $password, $username){
    $command = array('autoresponse', $domain, $username, $password, 'enable');
	return vm_daemon_raw($command);
}

function vautoresponsestatus($domain, $password, $username){
	$command = array('autoresponse', $domain, $username, $password, 'status');
	return vm_daemon_raw($command);
}

