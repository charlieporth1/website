<?php
/**
 * dba_functions.inc
 * ------------------
 * This include file implements Vadmin 2.0 storage API.
 *
 * Licensed under GNU GPL v2. Please see COPYING for full terms.
 *
 * $Id: dba_functions.inc,v 1.15 2009/08/13 22:41:41 pdontthink Exp $
 * 
 * @author Konstantin Riabitsev ($Author: pdontthink $)
 * @version $Date: 2009/08/13 22:41:41 $
 */

/**
 * Functions starting with vadmin_* implement the storage API.
 * Functions starting with dbastor_* are internal to this implementation.
 */

function vadmin_domain_exists($domain){
    $me = 'vadmin_domain_exists';
    spew("$me: checking if domain '$domain' is enabled");
    spew("$me: making a quick check");
    $ret = dbastor_check_db($domain, true);

    // check for upgrade
    if ($ret == false){
        spew("$me: checking if it needs upgrading");
        $ret = vadmin_upgrade_v1($domain);
    }

    if ($ret == false){
        spew("$me: check if this domain belongs to elvis");
        $elvis = vadmin_getvar('CONFIG', 'auth.elvis');
        if (stristr($elvis, "@$domain") !== FALSE){
            spew("$me: this is the master domain. Enabling.");
            vadmin_enable_domain($domain);
            $ret = true;
        }
    }

    // if $ret is still false, check if we have local accounts-
    // based system, where no domain is in the username to check
    // for being the master domain... and in this case, we'll just
    // enable any domain as long as it's elvis that is currently
    // logged in
    //
    if ($ret == false){
        if (!vadmin_IMAP_usernames_have_domain($domain)) {
            $username = vadmin_getvar('SQMAIL', 'username');
            if (vadmin_get_user_designation($domain, $username) === 'ELVIS') {
                spew("$me: this is the master domain I guess. Enabling.");
                vadmin_enable_domain($domain);
                $ret = true;
            }
        }
    }

    return $ret;
}

function vadmin_list_enabled_domains(){
    $me = 'vadmin_list_enabled_domains';
    spew("$me: Getting all available virtual domains");
    $vdomain_ary = vadmin_list_virtual_domains();
    $domain_ary = array();
    foreach (dbastor_list_all() as $dbname){
        if ($dbname == 'master'){
            continue;
        }
        if (in_array($dbname, $vdomain_ary)){
            spew("$me: adding $dbname");
            array_push($domain_ary, $dbname);
        }
    }
    return $domain_ary;
}

function vadmin_domain_passwd_issaved($domain){
    $me = "vadmin_domain_passwd_issaved";
    $passwd = dbastor_get($domain, 'passwd');
    if ($passwd != false){
        spew("$me: passwd is stored.");
        return true;
    } else {
        spew("$me: there is no passwd stored.");
        return false;
    }
}

function vadmin_enable_domain($domain){
    $me = 'vadmin_enable_domain';
    spew("$me: enabling domain $domain");
    dbastor_create_db($domain);

    $IMAP_usernames_format_config
     = vadmin_getvar('CONFIG', 'username.username_format_includes_domain');
    if ($IMAP_usernames_format_config === 'auto') {
        $username = vadmin_getvar('SQMAIL', 'username');
        $delim = vadmin_getvar('BACKEND', 'delimiters');
        $delim = substr($delim, 0, 1);
        if (strpos($username, $delim) !== FALSE)
            $IMAP_usernames_have_domain_in_them = "yes";
        else
            $IMAP_usernames_have_domain_in_them = "no";

        spew("$me: storing IMAP username format for $domain as \"$IMAP_usernames_have_domain_in_them\"");
        dbastor_put($domain, 'IMAP_usernames_have_domain_in_them', $IMAP_usernames_have_domain_in_them);
    }
}

/**
 * This function will return a stored domain password or false if
 * there isn't one.
 *
 * @param $domain  a vadmin domain.
 * @return         the domain password stored in $domain/passwd, or
 *                 false if that file is empty.
 */
function vadmin_get_domain_passwd($domain){
    $me = "vadmin_get_domain_passwd";
    $clearpass = false;
    /**
     * Let's see if we have it in session first.
     */
    $pacpass = vadmin_getvar('SESSION', 'VADMIN_SECRET');
    if (!isset($pacpass) || !strlen($pacpass)){
        spew("$me: password is not in session");
        $clearpass = dbastor_get($domain, 'passwd');
    } else {
        spew("$me: got encrypted password from session, decrypting");
        $clearpass = vadmin_crypto($pacpass, 'decrypt');
    }
    return $clearpass;
}


function vadmin_put_domain_passwd($domain, $clearpass){
    $me = "vadmin_put_domain_passwd";
    dbastor_put($domain, 'passwd', $clearpass);
}

function vadmin_get_pic($domain){
    $me = 'vadmin_get_pic';
    $contents = dbastor_get($domain, 'pic', false);
    $md5sum = vadmin_get_pref($domain, 'imgmd5');
    spew("$me: checking md5s");
    if (md5($contents) != $md5sum){
        spew("$me: ewww, no match. Returning false.");
        return false;
    }
    spew("$me: all checks passed. Returning contents.");
    return $contents;
}

function vadmin_put_pic($domain, $contents){
    $me = 'vadmin_put_pic';
    $md5sum = md5($contents);
    spew("$me: md5sum is '$md5sum'");
    spew("$me: putting md5sum in prefs");
    vadmin_put_pref($domain, 'imgmd5', $md5sum);
    spew("$me: storing the pic");
    dbastor_put($domain, 'pic', $contents, false);
}

function vadmin_del_pic($domain){
    $me = 'vadmin_del_pic';
    spew("$me: removing stuff from prefs");
    vadmin_put_pref($domain, 'imgmd5', false);
    vadmin_put_pref($domain, 'mimetype', false);
    dbastor_put($domain, 'pic', false);
    spew("$me: custom pic for '$domain' removed");
}   

function vadmin_get_template_list(){
    $me = 'vadmin_get_template_list';
    spew("$me: getting a list of all templates");
    $tmpl_ary = dbastor_get_templates_ary();
    $names = array();
    foreach ($tmpl_ary as $name=>$contents){
        array_push($names, $name);
    }
    return $names;
}

function vadmin_get_template($tmplname){
    $me = 'vadmin_get_template';
    $tmpl_ary = dbastor_get_templates_ary();
    if (isset($tmpl_ary{$tmplname})){
        spew("$me: returning template '$tmplname'");
        return $tmpl_ary{$tmplname};
    }
    spew("$me: no such template '$tmplname'");
    return false;
}

function vadmin_put_template($tmplname, $contents){
    $me = 'vadmin_put_template';
    $tmpl_ary = dbastor_get_templates_ary();
    spew("$me: storing template '$tmplname'");
    $tmpl_ary{$tmplname} = $contents;
    dbastor_put_templates_ary($tmpl_ary);
}

function vadmin_delete_template($tmplname){
    $me = 'vadmin_delete_template';
    $tmpl_ary = dbastor_get_templates_ary();
    if (count($tmpl_ary) && isset($tmpl_ary{$tmplname})){
        unset($tmpl_ary{$tmplname});
        dbastor_put_templates_ary($tmpl_ary);
    }
}


/**
 * This function finds out the designation of a given user -- see if the
 * user is just a user, a lowly administrator, a cross, or an elvis.
 *
 * @param string $domain                A domain name to look up designation for.
 * @param string $username              A username to look up designation for.
 * @param bool   $dont_molest_username  Take username as given, don't try to 
 *                                      adjust it at all (OPTIONAL; default FALSE)
 *
 * @return            a string with one of the following:
 *                    'NONER' if this is just a user.
 *                    'LOWLY' if this is a lowly admin
 *                    'CROSS' if this is a cross-admin
 *                    'ELVIS' if this is elvis!
 */
function vadmin_get_user_designation($domain, $username, $dont_molest_username=FALSE){
    $me = "vadmin_get_user_designation";
    $username = strtolower($username);
    $designation = 'NONER';
    
    // strip off @domain from username if IMAP usernames
    // are of same format
    //
    if (!$dont_molest_username && !vadmin_IMAP_usernames_have_domain($domain)) 
        list($username, $ignore) = vadmin_get_user_unamedomain($username);

    /**
     * First off, see if this is Elvis.
     */
    spew("$me: checking if this is elvis");
    $elvises = vadmin_getvar('CONFIG', 'auth.elvis');
    /**
     * Take care of pesky whitespaces.
     */
    $elvises = trim($elvises);
    $elvises = preg_replace("/\s+/s", ' ', $elvises);
    spew("$me: elvises are: $elvises");
    if (!strlen($elvises)){
        spew("$me: WARNING! No elvises defined!");
        spew("$me: vadmin will still work, but this is not normal!");
    } elseif (strpos($elvises, ',') !== false){
        /**
         * Explode along the commas
         */
        $elvis_ary = explode(',', $elvises);
        @array_walk($elvis_ary, 'vadmin_trim_array');
        if (in_array($username, $elvis_ary)){
            $designation = 'ELVIS';
            spew("$me: found '$username' among other elvises");
        }
    } elseif ($elvises == $username){
        $designation = 'ELVIS';
        spew("$me: '$username' is the only defined elvis");
    }
    
    /**
     * If we didn't find ELVIS, let's look in other sources.
     */
    if ($designation == 'NONER'){
        spew("$me: checking if '$username' is among cross-admins");
        $cross_ary = vadmin_get_cross_array($domain);
        if (in_array($username, $cross_ary)){
            spew("$me: found '$username' among cross-admins");
            $designation = 'CROSS';
        }
    }
    
    /**
     * What, still nothing?
     */
    if ($designation == 'NONER'){
        /**
         * Let's look among LOWLY admins, then.
         */
        spew("$me: checking if '$username' is among lowly admins");
        $lowly_ary = vadmin_get_lowly_array($domain);
        if (is_array($lowly_ary) && in_array($username, $lowly_ary)){
            spew("$me: found '$username' among lowly admins");
            $designation = 'LOWLY';
        }
    }
    spew("$me: final designation is '$designation'");
    return $designation;
}

function vadmin_get_lowly_array($domain){
    $ret = dbastor_get_lowly($domain);
    if (!is_array($ret)){
        $ret = array();
    }
    return $ret;
}

function vadmin_put_lowly_array($domain, $lowly_ary){
    dbastor_put_lowly($domain, $lowly_ary);
}

/**
 * This function returns an array with all CROSS admins for the
 * domain in question.
 * 
 * @param  $domain  the name of the domain to look for.
 * @return          an array with the usernames.
 */
function vadmin_get_cross_array($domain){
    $me = 'vadmin_get_cross_array';
    $all_cross_ary = dbastor_get_crosses();
    if($all_cross_ary != false){
        spew("$me: grabbing only admins for '$domain'");
        if (isset($all_cross_ary{$domain}) &&
            is_array($all_cross_ary{$domain})){
            $cross_ary = $all_cross_ary{$domain};
        } else {
            spew("$me: No cross-admins defined for this domain");
            $cross_ary = array();
        }
    } else {
        $cross_ary = array();
    }
    return $cross_ary;
}

function vadmin_get_xdomain_array($username){
    $me = 'vadmin_get_xdomain_array';
    $all_cross_ary = dbastor_get_crosses();
    if ($all_cross_ary != false){
        $xdomain_ary = array();
        foreach ($all_cross_ary as $domain=>$xadmins){
            if (in_array($username, $xadmins)){
                array_push($xdomain_ary, $domain);
            }
        }
    } else {
        $xdomain_ary = array();
    }
    return $xdomain_ary;
}

function vadmin_add_cross($domain, $cross){
    $me = 'vadmin_add_cross';
    spew("$me: Adding a cross-admin $cross to $domain");
    $cross_ary = dbastor_get_crosses();
    if ($cross_ary == false){
        $cross_ary = array();
    }
    if (!isset($cross_ary{$domain})){
        $cross_ary{$domain} = array();
    }
    array_push($cross_ary{$domain}, $cross);
    dbastor_put_crosses($cross_ary);
}

function vadmin_delete_cross($domain, $cross){
    $me = 'vadmin_delete_cross';
    spew("$me: deleting $cross from $domain");
    $cross_ary = dbastor_get_crosses();
    if ($cross_ary != false){
        if (isset($cross_ary{$domain}) && is_array($cross_ary{$domain})){
            $new_crosses = array();
            foreach ($cross_ary{$domain} as $ecross){
                if ($ecross != $cross){
                    spew("$me: keeping $ecross");
                    array_push($new_crosses, $ecross);
                }
            }
            if (sizeof($new_crosses) > 0){
                spew("$me: size of new array is " . sizeof($new_crosses));
                $cross_ary{$domain} = $new_crosses;
            } else {
                spew("$me: no more crosses left in $domain. Unsetting.");
                unset($cross_ary{$domain});
            }
            dbastor_put_crosses($cross_ary);
        }
    }
}

/**
 * This function returns the stored preference.
 *
 * @param  $domain    the name of the domain to look up the preference for.
 * @param  $prefname  the preference name to return.
 * @return            string with the value of the preference
 */
function vadmin_get_pref($domain, $prefname){
    $me = 'vadmin_get_pref';
    spew("$me: fetching the preference '$prefname'");
    $prefs_ary = dbastor_get_prefs($domain);
    if (isset($prefs_ary{$prefname})){
        $prefval = $prefs_ary{$prefname};
    } else {
        $prefval = false;
    }
    spew("$me: prefval is '$prefval'");
    return $prefval;
}

/**
 * This function stores the preference.
 *
 * @param  $domain    the name of the domain to store the pref for.
 * @param  $prefname  the name of the preference to store.
 * @param  $prefval   the value of the preference.
 * @return            void.
 */
function vadmin_put_pref($domain, $prefname, $prefval){
    $me = 'vadmin_put_pref';
    spew("$me: storing '$prefname' with value '$prefval'");
    $prefs_ary = dbastor_get_prefs($domain);
    $prefs_ary{$prefname} = $prefval;
    dbastor_put_prefs($domain, $prefs_ary);
}

/**
  * Retrieves the limit set for the given setting
  * and the given authentication level.
  *
  * @param string $domain The domain within which the
  *                       limit is being asked for
  * @param string $level  The authentication level 
  *                       (LOWLY?  CROSS?  ELVIS?)
  * @param string $def    The setting name for which the
  *                       level is being determined
  *
  * @return mixed The setting's limit, or FALSE if none
  *               is applicable.
  *
  */
function vadmin_get_limit($domain, $level, $def){

    $me = 'vadmin_get_limit';
    spew("$me: fetching the limit definition '$def' for level '$level'");


    // get limits as stored in vadmin database
    // the returned array is keyed by level,
    // with sub-arrays of setting->limit key
    // value pairs
    //
    $lim_ary = dbastor_get_limits($domain);


    // no limits found at all for the requested level?
    // return FALSE
    //
    if (!isset($lim_ary{$level})){
        spew("$me: no limits set for level '$level'");
        $retval = false;


    // we found limits for the given level,
    // is there a limit for the given setting?
    //
    } else {

        // no, return FALSE
        //
        if (!isset($lim_ary{$level}{$def})){
            spew("$me: limit '$def' not set for level '$level'");
            $retval = false;

        // yes, return it
        //
        } else {
            $retval = $lim_ary{$level}{$def};
            spew("$me: limit '$def' for level '$level' is: $retval");
        }
    }

    return $retval;
}

function vadmin_put_limit($domain, $level, $def, $value){
    $me = 'vadmin_put_limit';
    spew("$me: storing limit '$def' for level '$level' with value: $value");
    spew("$me: get limits array first");
    $lim_ary = dbastor_get_limits($domain);
    if (!isset($lim_ary{$level})){
        spew("$me: level '$level' does not exist. Creating.");
        $lim_ary{$level} = array();
    }
    if ($value != false){
        $lim_ary{$level}{$def} = $value;
        spew("$me: set limit '$def' for level '$level' with value: $value");
    } else {
        unset($lim_ary{$level}{$def});
        spew("$me: removed limit '$def' for level '$level'");
    }
    dbastor_put_limits($domain, $lim_ary);
}

#------------------------------------------------------------------------------
# DBA ROUTINES
#------------------------------------------------------------------------------

function dbastor_get_dbfile($dbname){
    $me = 'dbastor_get_dbfile';
    $stordir = vadmin_getvar('CONFIG', 'storage.dir');
    $suffix = vadmin_getvar('CONFIG', 'storage.suffix');
    $dbfile = $stordir . '/' . $dbname . $suffix;
    spew("$me: dbfile for '$dbname' is '$dbfile'");
    return $dbfile;
}

function dbastor_check_db($dbname, $quick=false){
    $me = 'dbastor_check_db';
    $dbfile = dbastor_get_dbfile($dbname);
    if (!file_exists($dbfile)){
        spew("$me: no such file, '$dbfile'");
        return false;
    }
    if ($quick){
        return true;
    }
    $flavor = vadmin_getvar('CONFIG', 'storage.flavor');
    $db_locking = vadmin_getvar('CONFIG', 'storage.locking');
    spew("$me: '$dbfile' claims to be flavor '$flavor'");
    spew("$me: checking if we have the identifier key set");
    //$db = @dba_open($dbfile, 'r' . $db_locking, $flavor);
    $db = sq_call_function_suppress_errors('dba_open', array($dbfile, 'r' . $db_locking, $flavor));
    if (!$db){
        spew("$me: ruh-roh, db open failed!");
        return false;
    }
    $key = 'vadmin_apiver';
    if (dba_exists($key, $db)){
        global $STORAGE_API;
        $apiver = dba_fetch($key, $db);
        if ($apiver != $STORAGE_API){
            spew("$me: wrong storage api version. Returning false atm.");
            return false;
        }
    } else {
        spew("$me: 'vadmin_apiver' key not found.");
        return false;
    }
    spew("$me: all checks successful");
    return true;
}

function dbastor_create_db($dbname){
    $me = 'dbastor_create_db';
    $dbfile = dbastor_get_dbfile($dbname);
    if (file_exists($dbfile)){
        spew("$me: '$dbfile' already exists");
        $msg = sprintf(_("%s already exists!"), $dbfile);
        vadmin_system_error($msg);
    }
    $flavor = vadmin_getvar('CONFIG', 'storage.flavor');
    $db_locking = vadmin_getvar('CONFIG', 'storage.locking');
    spew("$me: '$dbfile' claims to be flavor '$flavor'");
    //$db = @dba_open($dbfile, 'c' . $db_locking, $flavor);
    $db = sq_call_function_suppress_errors('dba_open', array($dbfile, 'c' . $db_locking, $flavor));
    if (!$db){
        spew("$me: ruh-roh, db creation failed!");
        $msg = sprintf(_("Failed to create %s!"), $dbfile);
        vadmin_system_error($msg);
    }
    $key = 'vadmin_apiver';
    global $STORAGE_API;
    dba_replace($key, $STORAGE_API, $db);
    dba_close($db);
    spew("$me: '$dbfile' created successfully");
}

function dbastor_list_all(){
    $me = 'dbastor_list_all';
    $stordir = vadmin_getvar('CONFIG', 'storage.dir');
    $suffix = vadmin_getvar('CONFIG', 'storage.suffix');
    spew("$me: listing all database files in $stordir");
    $dbnames = array();
    //if ($dir = @opendir($stordir)) {
    if ($dir = sq_call_function_suppress_errors('opendir', array($stordir))) {
        while (($file = readdir($dir)) !== false) {
            spew("$me: found $file");
            if (strstr($file, $suffix) !== false){
                $dbname = substr($file, 0, -strlen($suffix));
                spew("$me: adding '$dbname' to the array");
                array_push($dbnames, $dbname);
            }
        }
        closedir($dir);
    }
    return $dbnames;
}
    
function dbastor_get($dbname, $key, $crypto=true){
    $me = 'dbastor_get';
    spew("$me: I was asked to get '$key' from db '$dbname'");
    $dbkey = vadmin_crypto($dbname . $key, 'encrypt');
    $dbfile = dbastor_get_dbfile($dbname);
    $flavor = vadmin_getvar('CONFIG', 'storage.flavor');
    $db_locking = vadmin_getvar('CONFIG', 'storage.locking');
    spew("$me: flavor is $flavor");
    //$db = @dba_open($dbfile, 'r' . $db_locking, $flavor);
    $db = sq_call_function_suppress_errors('dba_open', array($dbfile, 'r' . $db_locking, $flavor));
    $ret = false;
    if ($db && dba_exists($dbkey, $db)){
        if ($crypto){
            $cryptdata = dba_fetch($dbkey, $db);
            $serdata = vadmin_crypto($cryptdata, 'decrypt');
        } else {
            $serdata = dba_fetch($dbkey, $db);
        }
        //$data = @unserialize($serdata);
        $data = sq_call_function_suppress_errors('unserialize', array($serdata));
        if ($data != false){
            $ret = $data;
            spew("$me: unserialized successfully!");
        } else {
            spew("$me: failed to unserialize!");
        }
    } else {
        spew("$me: key '$key' ($dbkey) not found, or dbopen failed!");
    }
    if ($db){
        dba_close($db);
    }
    return $ret;
}

function dbastor_put($dbname, $key, $data, $crypto=true){
    $me = 'dbastor_put';
    spew("$me: I was asked to put '$key' into db '$dbname'");
    $dbkey = vadmin_crypto($dbname . $key, 'encrypt');
    $dbfile = dbastor_get_dbfile($dbname);
    $flavor = vadmin_getvar('CONFIG', 'storage.flavor');
    $db_locking = vadmin_getvar('CONFIG', 'storage.locking');
    if (!file_exists($dbfile)){
        spew("$me: Hmm... $dbfile doesn't exist! Creating.");
        dbastor_create_db($dbname);
    }
    //$db = @dba_open($dbfile, 'w' . $db_locking, $flavor);
    $db = sq_call_function_suppress_errors('dba_open', array($dbfile, 'w' . $db_locking, $flavor));
    if (!$db){
        spew("$me: ruh-roh, db open failed for $dbfile!");
        $msg = sprintf(_("Could not open %s for writing"), $dbfile);
        vadmin_system_error($msg);
    }
    if ($data){
        $serdata = serialize($data);
        if ($crypto){
            $cryptdata = vadmin_crypto($serdata, 'encrypt');
            $ret = dba_replace($dbkey, $cryptdata, $db);
        } else {
            $ret = dba_replace($dbkey, $serdata, $db);
        }
    } else {
        if (dba_exists($dbkey, $db)){
            $ret = dba_delete($dbkey, $db);
        } else {
            spew("$me: Hm... key '$dbkey' doesn't exist anyway");
            $ret = true;
        }
    }
    dba_close($db);
    if ($ret){
        spew("$me:  $key ($dbkey) in '$dbfile' processed successfully");
    } else {
        $msg = sprintf(_("Could not write data in %s!"), $dbfile);
        vadmin_system_error($msg);
    }
}

function dbastor_delete($dbname, $key){
    dbastor_put($dbname, $key, false);
}

/**
 * This function saves the domain preferences into the "prefs"
 * file located in the domain directory specified by the
 * $VADMIN_DIR global var.
 *
 * @param $prefs  An associative array with the preferences
 *                data for this domain.
 * @return        void.
 */
function dbastor_put_prefs($domain, $prefs_ary){
    $me = "dbastor_put_prefs";
    dbastor_put($domain, 'prefs', $prefs_ary);
    spew("$me: updating prefs in session");
    vadmin_putvar('SESSION', 'prefs', $prefs_ary);
}

/**
 * This function parses the prefs file into an array and returns it. It
 * also uses the session for caching, so if the prefs are already in session,
 * it loads them from there.
 *
 * @return    a nested array with all domain prefs.
 */
function dbastor_get_prefs($domain){
    $me = "dbastor_get_prefs";
    $prefs_ary = vadmin_getvar('SESSION', 'prefs');
    if ($prefs_ary == false){
        spew("$me: prefs_file not found in session");
        $prefs_ary = dbastor_get($domain, 'prefs');
        spew("$me: putting prefs into session");
        vadmin_putvar('SESSION', 'prefs', $prefs_ary);
    } else {
        spew("$me: Found prefs in session");
    }
    spew("$me: size of prefs is: " . sizeof($prefs_ary));
    return $prefs_ary;
}

function dbastor_get_templates_ary(){
    $me = 'dbastor_get_templates_ary';
    spew("$me: getting templates from master db");
    $dbname = 'master';
    $dbfile = dbastor_get_dbfile($dbname);
    if (!file_exists($dbfile)) {
        spew("$me: oops, no master db... this is normal if no cross admins or templates have been added to the system yet");
        return FALSE;
    }
    $tmpl_ary = dbastor_get($dbname, 'templates');
    if (!is_array($tmpl_ary)){
        spew("$me: creating an empty templates array");
        $tmpl_ary = array();
    }
    return $tmpl_ary;
}

function dbastor_put_templates_ary($tmpl_ary){
    $me = 'dbastor_put_templates_ary';
    spew("$me: storing templates in the master db");
    dbastor_put('master', 'templates', $tmpl_ary);
}

function dbastor_get_crosses(){
    $me = 'dbastor_get_crosses';
    $dbname = 'master';
    $dbfile = dbastor_get_dbfile($dbname);
    if (!file_exists($dbfile)) return FALSE;
    $cross_ary = dbastor_get($dbname, 'cross-admins');
    return $cross_ary;
}

function dbastor_put_crosses($cross_ary){
    $me = 'dbastor_put_crosses';
    spew("$me: putting cross-admins into master db");
    dbastor_put('master', 'cross-admins', $cross_ary);
}

function dbastor_get_lowly($domain){
    $me = 'dbastor_get_lowly';
    spew("$me: geting lowly admins array from $domain");
    $lowly_ary = dbastor_get($domain, 'admins');
    return $lowly_ary;
}

function dbastor_put_lowly($domain, $lowly_ary){
    $me = 'dbastor_put_lowly';
    spew("$me: storing lowly admins array in $domain");
    dbastor_put($domain, 'admins', $lowly_ary);
}

function dbastor_get_limits($domain){
    $me = "dbastor_get_limits";
    $lim_ary = dbastor_get($domain, 'limits');
    spew("$me: size of lim_ary is: " . sizeof($lim_ary));
    return $lim_ary;
}

function dbastor_put_limits($domain, $lim_ary){
    $me = "dbastor_put_limits";
    spew("$me: storing limits for domain '$domain'");
    dbastor_put($domain, 'limits', $lim_ary);
}

/**
  * Removes the given user as a cross admin and
  * lowly admin for any and all domains on the
  * system.  This is usually needed when deleting
  * the user account in question.
  *
  * @param $user The user (FULL username including
  *              domain if usernames are in such
  *              format)
  *
  */
function delete_user_from_all_admin($user){

    list($uname, $domain) = vadmin_get_user_unamedomain($user);

    // remove from lowly admins for user's domain
    //
    $lowly_ary = vadmin_get_lowly_array($domain);
    if (($key = array_search($user, $lowly_ary)) !== FALSE){
        unset($lowly_ary[$key]);
        vadmin_put_lowly_array($domain, $lowly_ary);
    }

    // remove as cross admin for any domains
    //
    $cross_ary = dbastor_get_crosses();
    if ($cross_ary != false && is_array($cross_ary)){
        $changed_crosses = FALSE;
        foreach ($cross_ary as $dom => $crosses){
            $new_crosses = array();
            $changed = FALSE;
            if (is_array($crosses)){
                foreach ($crosses as $cross){
                    if ($cross != $user) $new_crosses[] = $cross;
                    else $changed = TRUE;
                }
                if ($changed){
                    $changed_crosses = TRUE;
                    if (sizeof($new_crosses) > 0)
                        $cross_ary[$dom] = $new_crosses;
                    else
                        unset($cross_ary[$dom]);
                }
            }
        }
        if ($changed_crosses)
            dbastor_put_crosses($cross_ary);
    }

}

/**
  * Synchronize Vadmin Admins Data Store
  *
  * Looks at the current real list of domains
  * on the machine and makes sure that there
  * are no cross administrators or lowly
  * administrators in the Vadmin data store
  * for domains that no longer exist on the
  * machine.
  *
  * Note that this function will not clean up
  * domain db files (with lowly admin info in
  * them) for domains that did not have any
  * cross admins configured.
  *
  */
function vadmin_resynchronize_admins() {

    // get master domain list
    //
    $real_domains = vadmin_list_virtual_domains();


    // first, check the cross admins
    //
    $cross_ary = dbastor_get_crosses();
    $changes_made = FALSE;
    if ($cross_ary != false && is_array($cross_ary)){
        foreach ($cross_ary as $dom => $crosses){
            if (!in_array($dom, $real_domains)){
                unset($cross_ary[$dom]);
                $changes_made = TRUE;

                // delete any lowlies by removing the
                // entire db file for this domain
                // (note that this does not catch
                // domains for which there were no
                // cross admins configured)
                //
                $dbfile = dbastor_get_dbfile($dom);
                if (file_exists($dbfile)){
                    sq_call_function_suppress_errors('unlink', array($dbfile));
                }
            }
        }
    }
    if ($changes_made) dbastor_put_crosses($cross_ary);

}

#------------------------------------------------------------------------------
# UPGRADE ROUTINES
#------------------------------------------------------------------------------

function vadmin_upgrade_v1($domain){
    $me = 'vadmin_upgrade_v1';
    if (!$domain){
        return false;
    }
    if (vadmin_getvar('CONFIG', 'upgrade.upgrade') != 'yes'){
        return false;
    }
    $upgraded = dbastor_get($domain, '_upgraded_');
    if (isset($upgraded) && $upgraded){
        return false;
    }
    $vadmin_dir = vadmin_getvar('CONFIG', 'upgrade.dir');
    if (!is_dir("$vadmin_dir/$domain")){
        return false;
    }
    /**
     * OK, looks like we'll need to upgrade.
     */
    if (!sq_is_writable($vadmin_dir)){
        spew("$me: UPGRADE FAILURE! $vadmin_dir must be writable by apache!");
        spew("$me: SEE UPGRADING for more info");
        spew("$me: upgrader quitting");
        $msg =  sprintf(_("Vadmin is misconfigured: No writing permissions to %s"), $vadmin_dir);
        vadmin_system_error($msg);
    }
    $adminsfile = "$vadmin_dir/$domain/admins";
    $passwdfile = "$vadmin_dir/$domain/passwd";
    $prefsfile  = "$vadmin_dir/$domain/prefs";
    $picfile    = "$vadmin_dir/$domain/pic";
    if (file_exists($adminsfile) && filesize($adminsfile)){
        spew("$me: upgrading file $adminsfile");
        /**
         * Old format: one username per line
         */
        $fp = fopen($adminsfile, 'r');
        $contents = fread($fp, filesize($adminsfile));
        fclose($fp);
        $contents = trim($contents);
        $admins = explode("\n", $contents);
        @array_walk($admins, 'vadmin_trim_array');
        vadmin_put_lowly_array($domain, $admins);
        spew("$me: done with $adminsfile");
    }
    if (file_exists($passwdfile) && filesize($passwdfile)){
        spew("$me: upgrading file $passwdfile");
        /**
         * Old format: encrypted data.
         */
        $fp = fopen($passwdfile, 'r');
        $contents = fread($fp, filesize($passwdfile));
        $contents = trim($contents);
        fclose($fp);
        $clearpass = vadmin_crypto($contents, 'decrypt');
        vadmin_put_domain_passwd($domain, $clearpass);
        spew("$me: done with $passwdfile");
    }
    if (file_exists($prefsfile) && filesize($prefsfile)){
        spew("$me: upgrading file $prefsfile");
        /**
         * Old format: three possible options:
         * org: The title
         * pic: Mimetype of the picture
         * pwc: Allow changing passwords or not (yes/no)
         */
        $fp = fopen($prefsfile, 'r');
        $contents = fread($fp, filesize($prefsfile));
        fclose($fp);
        $contents = trim($contents);
        $prefs = explode("\n", $contents);
        foreach ($prefs as $pref){
            spew("$me: pref is: $pref");
            if (strpos($pref, 'org:') === 0){
                $title = substr($pref, 5);
                spew("$me: found title pref: $title");
                vadmin_put_pref($domain, 'title', $title);
            } elseif (strpos($pref, 'pic:') === 0){
                $mimetype = substr($pref, 5);
                spew("$me: found mimetype pref: $mimetype");
                vadmin_put_pref($domain, 'mimetype', $mimetype);
            } elseif (strpos($pref, 'pwc:') === 0){
                $pwc = substr($pref, 5);
                spew("$me: found pwc preff: $pwc");
                if ($pwc == 'yes'){
                    vadmin_put_pref($domain, 'password_change', 1);
                }
            }
        }
        spew("$me: Done with $prefsfile");
    }
    if (file_exists($picfile) && filesize($picfile)){
        spew("$me: upgrading file $picfile");
        /**
         * Read the pic in
         */
        $fd = fopen($picfile, 'r');
        $contents = fread($fd, filesize($picfile));
        fclose($fd);
        $imgmd5 = md5($contents);
        vadmin_put_pref($domain, 'imgmd5', $imgmd5);
        vadmin_put_pic($domain, $contents);
        spew("$me: done with file $picfile");
    }
    /**
     * Check on cross-admins
     */
    $cross_ary = dbastor_get_crosses();
    if (!isset($cross_ary{'_upgraded_'})){
        spew("$me: uh-oh, cross-admins don't look upgraded to me.");
        spew("$me: upgrading cross-admins");
        $cross_dir = "$vadmin_dir/cross-admins";
        if (is_dir($cross_dir)){
            $handle = opendir($cross_dir);
            while (false !== ($xadmin = readdir($handle))){
                if ($xadmin != "." && $xadmin != ".."){
                    spew("$me: found cross-admin $xadmin");
                    $crossfile = "$cross_dir/$xadmin";
                    $fp = fopen($crossfile, 'r');
                    $contents = fread($fp, filesize($crossfile));
                    fclose($fp);
                    $contents = trim($contents);
                    $domains = explode("\n", $contents);
                    foreach ($domains as $xdomain){
                        $xdomain = trim($xdomain);
                        if ($xdomain == 'elvis' || $xdomain == 'ELVIS'){
                            spew("$me: Found elvis $crossfile");
                            spew("$me: ignoring, can't do not'n");
                        } else {
                            spew("$me: $xadmin is cross-admin of $xdomain");
                            if (!isset($cross_ary{$xdomain})){
                                $cross_ary{$xdomain} = array();
                            }
                            array_push($cross_ary{$xdomain}, $xadmin);
                        }
                    }
                }
            }
            dbastor_put_crosses($cross_ary);
            closedir($handle);
        }
        spew("$me: done with upgrading the cross-admins");
        vadmin_add_cross('_upgraded_', '_upgraded_');
    }
    dbastor_put($domain, '_upgraded_', 1);
    $clean = vadmin_getvar('CONFIG', 'upgrade.cleanup');
    if ($clean == 'yes'){
        spew("$me: cleaning up");
        recursive_delete("$vadmin_dir/$domain");
        recursive_delete("$vadmin_dir/cross-admins");
    }
    spew("$me: done upgrading $domain");
    return true;
}

function recursive_delete($dir){
    $me = 'recursive_delete';
    spew("$me: recursively deleting $dir");
    if (!is_dir($dir)){
        return;
    }
    $dd = opendir($dir);
    while($entry = readdir($dd)){
        if ($entry == '.' || $entry == '..'){
            continue;
        }
        $fpath = "$dir/$entry";
        if (is_dir($fpath)){
            recursive_delete($fpath);
        } else {
            unlink($fpath);
        }
    }
    closedir($dd);
    rmdir($dir);
}

/**
 * Don't edit this var -- it defines what API this storage component
 * implements.
 */
global $STORAGE_API;
$STORAGE_API = 2;

