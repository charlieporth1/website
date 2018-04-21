<?php
/**
 * setup.php
 * ---------
 * This is a standard SquirrelMail API for plugins.
 *
 * Licensed under GNU GPL v2. Please see COPYING for full terms.
 * 
 * @author Konstantin Riabitsev ($Author: pdontthink $)
 * @version $Date: 2010/03/06 19:53:25 $
 *
 * $Id: setup.php,v 1.41 2010/03/06 19:53:25 pdontthink Exp $
 */

/**
 * Main intialization function. SquirrelMail calls it whenever
 * the plugin is initialized. Plugin hooks are defined within so
 * SM knows which place to put which function on.
 *
 * @return void
 */
function squirrelmail_plugin_init_vadmin(){
    global $squirrelmail_plugin_hooks;

    $squirrelmail_plugin_hooks['login_cookie']['vadmin'] 
        = 'vadmin_reroute_and_login_prefs';
    $squirrelmail_plugin_hooks['webmail_top']['vadmin'] 
        = 'vadmin_set_user_domain_cookie';
    $squirrelmail_plugin_hooks['menuline']['vadmin'] 
        = 'vadmin_menulink';
    $squirrelmail_plugin_hooks['template_construct_page_header.tpl']['vadmin'] 
        = 'vadmin_menulink';
    $squirrelmail_plugin_hooks['optpage_register_block']['vadmin'] 
        = 'vadmin_options_page';
    $squirrelmail_plugin_hooks['left_main_after']['vadmin'] 
        = 'vadmin_autores_notify';
    $squirrelmail_plugin_hooks['login_verified']['vadmin'] 
        = 'vadmin_log_login';
    $squirrelmail_plugin_hooks['configtest']['vadmin'] 
        = 'vadmin_check_configuration';
    $squirrelmail_plugin_hooks['reset_user_prefs']['vadmin'] 
        = 'vadmin_sensitive_preference_names';
    $squirrelmail_plugin_hooks['loading_constants']['vadmin'] 
        = 'vadmin_override_config';
    $squirrelmail_plugin_hooks['optpage_loadhook_accessibility']['vadmin'] 
        = 'vadmin_accesskey_options';

    // use prefs_backend hook for config_override in 1.5.2
    $squirrelmail_plugin_hooks['prefs_backend']['vadmin'] 
        = 'vadmin_override_config';
}

/**
  * Returns info about this plugin
  *
  * @return array An array containing detailed plugin information 
  *               and requirements.
  */
function vadmin_info()
{

   return array(
             'english_name' => 'Vadmin',
             'authors' => array(
                'Konstantin Riabitsev' => array(
                   'email' => 'graf25@users.sourceforge.net',
                ),
                'Paul Lesniewski' => array(
                   'email' => 'paul@squirrelmail.org',
                ),
             ),
             'summary' => 'A control panel system that fits inside SquirrelMail.  Features include very tight security, encryption, multiple levels of administrators, and much more.',
             'details' => 'Vadmin is a control panel system that fits inside of SquirrelMail.  It comes with features that allow system administrators to create, manage and delete user accounts amongst many other things.  It can also be extended to manage any other aspects of the environment in which it is installed.<br /><br />Vadmin is designed to support any number of system architectures by means of configurable backends.  Currently, backends for VMailMgr and SQL database backed systems are included, and a proprietary local user accounts backend is also available.  Other backends may be added in the future.<br /><br />Vadmin incorporates tight security features, encryption, multiple levels of administrators, and many convenient options to make the life of an administrator easier.  It also provides users with optional helpful features such as password change, auto-responder and mail forwarding systems.<br /><br />This plugin also allows domain customizations so login pages look different depending on which URI it is being accessed at.  This makes it very convenient to have just one SquirrelMail install per every virtual domain.<br /><br />A sampling of features in Vadmin are as follows:<ul><li>Manage multiple domains</li><li>Multiple levels of administrators</li><li>Administrators can be assigned to one or more domains</li><li>Extensible (can add custom modules that allow access to things such as database management, DNS management, server management, etc.)</li><li>Manage image on login screen for each domain</li><li>Add account/add multiple accounts</li><li>Create aliases</li><li>Delete account/delete multiple accounts</li><li>Search for user account(s)</li><li>Paginated accounts list (detailed or quick view)</li><li>Control user access to autoresponder/password change/mail forwarding</li><li>View/edit user webmail preferences (does not matter whether they are stored in a database or file based)</li><li>Show users who have never logged in or only those that have logged in (webmail only)</li><li>Extensible account attribute system (can add custom account settings to match mail system backend)</li><li>Manage account quotas, SMTP/POP/IMAP/webmail access, password, forwards, webmail preference settings, etc.</li><li>Parent/child account mangement (with add-on)</li><li>Many Vadmin backends can be used to enable user signup systems</li><li>Much, much more!</li></ul>',
             'external_project_uri' => 'http://sourceforge.net/projects/vadmin-plugin/',
             'version' => '3.0',
             'required_sm_version' => '1.4.0',
             'requires_configuration' => 1,
             'requires_source_patch' => 0,
             'required_plugins' => array(
                'compatibility' => array(
                   'version' => '2.0.14',
                   'activate' => FALSE,
                )
             ),
          );

}

/**
  * Returns version information about this plugin.
  *
  * @return string The current plugin version.
  *
  */
function vadmin_version(){
   $info = vadmin_info();
   return $info['version'];
}

/**
  * Returns a list of preference names that should not
  * be controllable by the end user
  *
  * @return string A comma-delimited list of preference
  *                names that the end-user should not
  *                have control over.
  */
function vadmin_sensitive_preference_names(){
    include_once(SM_PATH . 'plugins/vadmin/vadmin_start.php');
    return vadmin_sensitive_preference_names_do();
}

/**
 * This function hooks in immediately after login in order to
 * log user's webmail usage.
 *
 * @return void
 */
function vadmin_log_login(){
    include_once(SM_PATH . 'plugins/vadmin/vadmin_start.php');
    vadmin_log_login_do();
}

/**
 * This function hooks to the options page for the end-users and
 * allows them to change their passwords or set up an autoresponder.
 * It also places the link to the administrative interface on the
 * options page (for admins only, of course).
 *
 * @return void
 */
function vadmin_options_page(){
    include_once(SM_PATH . 'plugins/vadmin/vadmin_start.php');
    vadmin_options_page_do();
}

/**
 * This function hooks in very early in every page request which 
 * allows Vadmin the ability to override SquirrelMail configuration
 * settings or enable/disable plugins, etc.
 *
 * @return void
 */
function vadmin_override_config($args){
    include_once(SM_PATH . 'plugins/vadmin/vadmin_start.php');
    vadmin_override_config_do($args);
}

/** 
 * Puts a link to vadmin in SquirrelMail menu bar if prefs permit
 *
 * @return void
 */
function vadmin_menulink(){
    include_once(SM_PATH . 'plugins/vadmin/vadmin_start.php');
    return vadmin_menulink_do();
}

/** 
 * Lets users define different access key/shortcuts
 *
 * @return void
 */
function vadmin_accesskey_options(){
    include_once(SM_PATH . 'plugins/vadmin/vadmin_start.php');
    vadmin_accesskey_options_do();
}

/**
 * This function puts a link to the autoresponder below the
 * SquirrelMail folder list when it is enabled
 *
 * @return void
 */
function vadmin_autores_notify(){
    include_once(SM_PATH . 'plugins/vadmin/vadmin_start.php');
    vadmin_autores_notify_do();
}

/**
 * This function redirects the user to another host/path if 
 * needed; usually in order to transparently serve SSL to 
 * multiple domains using just one SSL certificate.
 *
 * After that, it sets the front page preferences per-domain. 
 * It will check if the virtual domain has custom name and 
 * picture and will substitute the default ones with them.
 *
 * @return void
 */
function vadmin_reroute_and_login_prefs(){
    include_once(SM_PATH . 'plugins/vadmin/vadmin_start.php');
    vadmin_reroute_and_login_prefs_do();
}

/**
 * This function sets up the vadmin domain cookie
 *
 * @return void
 */
function vadmin_set_user_domain_cookie(){
    include_once(SM_PATH . 'plugins/vadmin/vadmin_start.php');
    vadmin_set_user_domain_cookie_do();
}

/**
 * This function checks that Vadmin is correctly installed
 * and configured and that the current server environment
 * is also suited for the correct functioning of Vadmin.
 *
 * @return boolean TRUE if there were any configuration or
 *                 other such errors, FALSE if there are
 *                 no problems and the plugin is ready to
 *                 rock.
 */
function vadmin_check_configuration(){
    include_once(SM_PATH . 'plugins/vadmin/configtest.php');
    return vadmin_check_configuration_do();
}

/**
  * Force the getpot script to pick up these translations
  *
  * @ignore
  *
  */
function v_no_op()
{
   $ignore = _("Subject");
}

