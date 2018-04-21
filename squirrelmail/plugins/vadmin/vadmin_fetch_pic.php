<?php
/**
 * vadmin_fetch_pic.php
 * ---------------------
 * Fetches a custom picture for each domain. If QUERY_STRING
 * is not set, it tries to determine which domain it is by 
 * looking at the access URI. The images are pulled from the
 * pic file in $VADMIN_DIR/domain directory.
 *
 * Licensed under GNU GPL v2. Please see COPYING for full terms.
 *
 * $Id: vadmin_fetch_pic.php,v 1.6 2007/07/01 07:59:40 pdontthink Exp $
 *
 * @author Konstantin Riabitsev ($Author: pdontthink $)
 * @version $Date: 2007/07/01 07:59:40 $
 */

/**
 * Load the config parser before we chdir.
 */
include_once('config_parser.php');

$config_path = vadmin_get_conf_location();
$config_file = 'vadmin.conf.php';

$vadmin_config = vadmin_parse_config($config_path . $config_file);

/**
 * Load main functions.
 */
$GLOBALS{'VADMIN_CONFIG'} = $vadmin_config;
$include_dir = $vadmin_config{'paths'}{'includes'};
include_once($include_dir . '/vadmin_functions.inc');

/**
 * See what setting is storage.type set to and load the needed
 * file. This is the only place where error checking is implemented
 * since many different storage types are possible.
 */
$storage_type = vadmin_getvar('CONFIG', 'storage.type');
$storage_functions_file = sprintf('%s/%s_functions.inc', 
                                  $include_dir, $storage_type);
if (file_exists($storage_functions_file) 
    && is_readable($storage_functions_file)){
    include_once($storage_functions_file);
} else {
    spew("$me: FATAL: could not load the storage functions file!");
    vadmin_system_error(sprintf(
        _("Could not load function file for storage type %s"), $storage_type));
}

$domain = vadmin_getvar('GET', 'DOM');
spew("vadmin_fetch_pic.php: I was asked to get the pic for domain: $domain");

if ($domain == ''){
    spew("vadmin_fetch_pic.php: empty domain; attempting to get default pic");
    /**
     * Load whatever the default image is (per SM config)
     */
    //$default_login_image = vadmin_getvar('SQMAIL', 'org_logo');
    $default_login_image = vadmin_get_per_domain_sm_setting('org_logo');
    if (strpos($default_login_image, '../../') !== 0)
        $default_login_image = '../' . $default_login_image;
    header('Content-type: image/png');
    readfile($default_login_image);
    //readfile('../../images/sm_logo.png');
    exit;
}

$contents = vadmin_get_pic($domain);

if ($contents == false){
    spew("vadmin_fetch_pic.php: Pic not found, using default");
    /**
     * Load whatever the default image is for this domain (defined outside of vadmin)
     */
    $default_login_image = vadmin_get_per_domain_sm_setting('org_logo', $domain);
    if (strpos($default_login_image, '../../') !== 0)
        $default_login_image = '../' . $default_login_image;
    header('Content-type: image/png');
    readfile('../' . $default_login_image);
    //readfile('../../images/sm_logo.png');
    exit;
} else {
    spew("vadmin_fetch_pic.php: Pic found");
    $mimetype = vadmin_get_pref($domain, 'mimetype');
    header('Pragma: no-cache');
    header('Content-type: ' . $mimetype);
    echo $contents;
}
