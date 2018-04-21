<?php /*
# Please do not change the line above or you will break Vadmin


##
#
# Vadmin configuration file.
# --------------------------
# This configuration file is in a simple win.ini-style format.
# Lines starting with "#" and ";" are comments.
#
# CACHING NOTE: Vadmin caches the configuration in session, so if you
# have made changes to this file, you will need to sign out of squirrelmail
# and sign back in to see the changes.
# 
# $Id: vadmin.conf_example.php,v 1.12 2009/09/05 05:27:06 pdontthink Exp $
#


##
#
# [startup]
# These settings put the user in the right place immediately
# after first logging in to Vadmin.  This allows you to insert
# and rearrange the menu system as needed.  This only applies
# to modules in the modules/admin directory.
#
# Note that you can specify the startup location for each
# administrator type (elvis, cross-administrator, lowly),
# and if not found, the generic settings here will apply.
#
# Set module to "menu_orig" for old (pre v3.0) Vadmin behavior
#
# NOTE that this is NOT intended as a security feature!
#
[startup]
    module = menu
;    module = menu_orig
    action = main
;    elvis_module = menu
;    elvis_action = main
;    cross_module = menu
;    cross_action = main
;    lowly_module = email
;    lowly_action = main


##
#
# [paths]
# These should be self-evident.
#
# Note that the qmailcontrol setting is only important 
# when using the VMailMgr backend and can usually be 
# ignored otherwise.
#
[paths]
    plugin = /usr/share/squirrelmail/plugins/vadmin
    modules = /usr/share/squirrelmail/plugins/vadmin/modules
    includes = /usr/share/squirrelmail/plugins/vadmin/includes
    qmailcontrol = /var/qmail/control


##
#
# [backend]
# Choose the backend that is used on your system.  Note that
# you will also need to review and edit the settings in the 
# configuration file that matches your backend selection.
#
# type
#   The name of your backend.  Currently, these are supported:
#     vmailmgr
#     sql
#
# zero_is_unlimited
#   Tell Vadmin what user attribute fields should be treated 
#   as "unlimited" when given as zero.  This should be a comma 
#   separated list where possible values are:
#     hardquota
#     softquota
#     sizelimit
#     countlimit
#   For the quota-related fields here, you'll need to know what
#   the meaning of a zero is in your mail system for a quota 
#   field.  For example, Courier (IMAP/Maildrop) considers zero 
#   to indicate that the user has no quota limit whatsoever,
#   whereas VMailMgr/qmail take zero literally.  The sample
#   below should work with systems using Courier-based quotas.
#
# support_aliases
#   If your backend system does not support email aliases, set
#   this to "no".
#
# user_password_plugin
#   When users are allowed to change their passwords, which
#   SquirrelMail plugin should be enabled?  The exact name
#   of that plugin's directory should be given here.  Of
#   course, this plugin needs to be downloaded, installed
#   and configured separately.  However, DO NOT activate the
#   plugin in the SquirrelMail configuration, since it will
#   be managed in the Vadmin interface instead.  Vadmin also
#   provides an internal mechanism that may be used instead
#   of an external plugin by specifying "internal" for this
#   setting.  If nothing is specified here, this feature will
#   not be available to users.
#
# user_autoresponder_plugin
#   When users are allowed to set up an autoresponder message,
#   which SquirrelMail plugin should be enabled?  The exact name
#   of that plugin's directory should be given here.  Of course,
#   this plugin needs to be downloaded, installed and configured
#   separately.  However, DO NOT activate the plugin in the
#   SquirrelMail configuration, since it will be managed in the
#   Vadmin interface instead.  It is possible to use an internal
#   mechanism by specifying "internal" for this setting (only
#   supported for some backends), but you are encouraged to use
#   an external plugin that is designed for your system.  If
#   nothing is specified here, this feature will not be
#   available to users.
#
# user_mail_forwarding_plugin
#   When users are allowed to manage their mail forwarding
#   (alias) settings, which SquirrelMail plugin should be
#   enabled?  The exact name of that plugin's directory should be
#   given here.  Of course, this plugin needs to be downloaded,
#   installed and configured separately.  However, DO NOT
#   activate the plugin in the SquirrelMail configuration, since
#   it will be managed in the Vadmin interface instead.  Vadmin
#   also provides an internal mechanism that may be used instead
#   of an external plugin by specifying "internal" for this
#   setting.  If nothing is specified here, this feature will
#   not be available to users.
#
[backend]
    type = vmailmgr
;    zero_is_unlimited = hardquota, countlimit
    support_aliases = yes
;    user_password_plugin = internal
;    user_autoresponder_plugin = local_autorespond_forward
;    user_mail_forwarding_plugin = internal
    user_password_plugin =
    user_autoresponder_plugin =
    user_mail_forwarding_plugin =


##
#
# [storage]
# Theoretically, many different storage mechanisms are possible, however
# at the moment only one is supported -- dba abstraction interface.
# The following options are possible:
#
# type
#   This can only be set to "dba" at the moment, unless you have written
#   your own api implementation that saves to some other format.
#
# flavor
#   PHP's "dba" allows saving in many different formats, including dbm,
#   gdbm, db2, db3, db4, etc.  However, do not idly change the setting here,
#   as support for each database flavor needs to be compiled into PHP.  Many
#   distributions ship php binaries pre-built with "gdbm" enabled, so it's 
#   a safe choice, however check your phpinfo to see which ones are enabled.
#   More info: http://www.php.net/dba
#
# locking
#   This is the database lock method that Vadmin will use when accessing
#   the Vadmin database.  Which method you use may depend on the "flavor"
#   setting you have selected.  The gdbm flavor can usually use "l", most
#   Berkeley variants (db3, db4, etc.) should use "d".  For more details,
#   see: http://www.php.net/manual/function.dba-open.php and also check
#   your web server error log to make sure the database commands in Vadmin
#   are not causing locking notices.
#
# suffix
#   This is what the filenames will be ending with. ".db" is a sane default.
#
# dir
#   This is where Vadmin will save its files.  /var/lib/vadmin is a very
#   sane and FHS-compliant default.
#
[storage]
    type = dba
    flavor = gdbm
    locking = l
    suffix = .db
    dir = /var/lib/vadmin


##
#
# [auth]
# This section deals with things related to authentication and general
# functioning.
#
# method
#   Two settings are possible -- "user" and "system".  In "user" mode, 
#   Vadmin will present the administrator with a login screen that asks for
#   their *mailbox* password, and then use the stored domain password to
#   communicate with vmailmgr.  On the other hand, "system" mode will ask
#   the domain password at the login screen.  The advantage of "system" vs
#   "user" is that "system" doesn't require the domain passwords to be
#   saved on disk, which is a relative security problem.  The drawbacks,
#   however, are significant -- a saved domain password is required for
#   things like autoresponders and password changes, not to mention that
#   "lowly" admins (the ones who can only administer the domains they belong
#   to) cannot access the system without a saved password.  The "user" 
#   method is generally recommended.
#
# force_https
#   If you want to make sure that elvis/cross-admins log in only using HTTPS.
#   A generally good idea, as domain passwords (which are system passwords!)
#   will otherwise travel across the network in the clear.  This is only
#   relevant for the superuser and cross-administrators -- common admins
#   do not get to use system passwords anyway.
#
# elvis
#   This is the superuser.  There can be more than one elvis (well, at least
#   in terms of Vadmin).  If you have more than one superuser, specify
#   them all on one line separated by commas.  E.g.:
#   elvis = albus@hogwarts.jk, minerva@hogwarts.jk
#   NOTE: this user has to exist and be able to log in to SquirrelMail.
#   Putting a username here will not create an account automatically.
#  
[auth]
    method = user
    force_https = no
    elvis = albus@hogwarts.jk


##
#
# [permissions]
# Configure allowable actions by certain admins.
#
# cross_can_set_domain_limits
#   Set to yes/no to turn the ability of cross admins
#   to edit domain limits for maximum mailboxes, quotas, etc.
#
# never_show_admins
#   Setting this to "yes" will ensure that users who may be
#   administrators in some capacity will never be indicated
#   as such on user listings.  You should always leave this
#   set to "no" for maximum clarity.
#
[permissions]
    cross_can_set_domain_limits = yes
    never_show_admins = no


##
#
# [redirect]
# This is useful for people who want to enforce https, but only have one
# valid https certificate.  When a person comes to http://mail.theirdomain.com,
# they will be transparently redirected to the domain set in "host".
#
# https
#   Whether the redirect should go to https vs http
#
# host
#   The hostname where to redirect
#
# path
#   The path to squirrelmail installation.  DO NOT OMIT THE TRAILING SLASH!
#   E.g. if your squirrelmail lives in yourdomain.com/webmail, the
#   path is:
#     path = /webmail/
#
;[redirect]
;    https = yes
;    host = mail.quibbler.jk
;    path = /


##
#
# [username]
# This allows controlling how valid usernames are defined.  This also
# applies to alias names.  Set these as empty if Vadmin should not validate
# usernames (perhaps your backend does that for you).
#
# username_format_includes_domain
#   If all the domains on your system have usernames in the format
#   (specifically, the exact username format needed to log into the
#   IMAP server) where the domain information is part of the username
#   ("user@example.clom"), then set this to "yes".  Vadmin can support
#   systems where usernames are only "user" without the domain part
#   although such systems have their limits.  Set this to "no" if, for
#   example, your mail accounts are also local user accounts.  You can
#   also set this to "auto" and Vadmin will try to autodetect username
#   format separately for each domain.
#
# minimum_length
#   Specify minimum length of usernames.
#
# maximum_length
#   Specify maximum length of usernames.
#
# valid_characters
#   Specify the valid characters allowed in a username.
#   Note that due to how this configuration file is parsed, pound (#)
#   signs and semicolons (;) are not allowed in this setting.
#
# quicklist_columns
#   Specifies the number of columns shown for listing users on the
#   "quick list" screen.
#
# multiple_add_user_columns
#   Specifies the number of columns shown for listing users to be
#   added on the confirmation screen for creating multiple users.
#
# multiple_add_error_columns
#   Specifies the number of columns shown for listing erroneous
#   users on the confirmation screen for creating multiple users.
#
# multiple_delete_user_columns
#   Specifies the number of columns shown for listing users to be
#   deleted on the confirmation screen for deleting multiple users.
#
# multiple_delete_error_columns
#   Specifies the number of columns shown for listing erroneous
#   users on the confirmation screen for deleting multiple users.
#
[username]
    username_format_includes_domain = yes
;    username_format_includes_domain = auto
    minimum_length = 3
    maximum_length = 25
    valid_characters = _.abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789
    quicklist_columns = 4
    multiple_add_user_columns = 3
    multiple_add_error_columns = 2
    multiple_delete_user_columns = 3
    multiple_delete_error_columns = 2


##
#
# [password]
# This allows controlling how valid passwords are defined.  Set
# these as empty (or to "no" as applicable) if Vadmin should not
# validate passwords (perhaps your backend does that for you).
#
# minimum_length
#   Specify minimum length of passwords.
#
# maximum_length
#   Specify maximum length of passwords.
#
# differ_from_username
#   Set to "yes" to specify that the password must differ
#   from the username, "no" otherwise.  It is highly recommended
#   to leave this turned on.
#
# alpha_numeric
#   Set to "yes" to indicate that passwords must contain both
#   letters (a-z) and numerals (0-9).
#
# valid_characters
#   Specify the valid characters allowed in a password.
#   Note that due to how this configuration file is parsed, pound (#)
#   signs and semicolons (;) are not allowed in this setting.
#
# cracklib_dict
#   In order to subject passwords to strength testing by the Cracklib
#   library, set this setting to the path of a password dictionary.
#   This will have no effect if the current PHP build does not include 
#   support for Cracklib or if the specified dictionary cannot be
#   opened.  Check the Vadmin logfile or src/configtest.php to 
#   determine if Cracklib is successfully configured.  See:
#   http://www.php.net/manual/ref.crack.php
#
# override
#   Administrators can override these restrictions (except
#   valid_characters) if their administrative level is listed
#   in this setting (comma-separated list of "lowly", "cross",
#   and/or "elvis").  You may also leave this empty so that
#   no one may override these restrictions.
#
[password]
;    override = elvis, cross
;    override = 
    override = elvis
    minimum_length = 6
    maximum_length = 12
    differ_from_username = yes
    alpha_numeric = yes
;    cracklib_dict = /usr/local/lib/pw_dict
    cracklib_dict = 
    valid_characters = |}{:][?/abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789%$^@!*()._,+-=
# TODO: the following chars didn't seem to get thru probably due to some
#       escaping somewhere along the way:   '&
#       it's not really worth my time trying to make them available


##
#
# [forwarding]
# This allows controls how valid mail forwarding aliases are defined.
# Set these as empty (or to "no" as applicable) if Vadmin should not
# validate forwarding aliases (perhaps your backend does that for you).
#
# restriction_list
#   Specify any number of restricted forwarding addresses, which may
#   contain wildcards "*" and "?", which mean "any number of (or zero)
#   characters" and "one alphanumeric character" respectively.  This
#   is a comma-separated list.
#
# local_addresses_ok
#   Set to "yes" to indicate that forwarding addresses may be given
#   sans the domain portion of the address (local delivery).  Otherwise,
#   only full email addresses will be acceptable.
#
# override
#   Administrators can override these restrictions (except
#   local_addresses_ok) if their administrative level is listed
#   in this setting (comma-separated list of "lowly", "cross",
#   and/or "elvis").  You may also leave this empty so that
#   no one may override these restrictions.
#
[forwarding]
;    restriction_list = *@yahoo.com, *@hotmail.com
;    local_addresses_ok = no
    restriction_list = 
    local_addresses_ok = yes
;    override = elvis, cross
;    override = 
    override = elvis


##
#
# [displayable_fields]
# This section specifies what information will be shown on certain screens 
# depending upon what level of access the current admin user has.
#
[displayable_fields]

#
# user_list_elvis
# user_list_cross
# user_list_lowly
#   These each determine the fields that are shown on the user summary/
#   user list screen for each type of administrator (normal/"lowly" admin, 
#   multiple/cross-domain admin, superuser/"elvis").  The only acceptable 
#   values for these settings may be any combination (separated by commas) of:
#     delete_checkbox
#     username
#     info
#     webmail_stats
#     mailbox
#     forwards
#   Note that "webmail_stats" will have no effect if webmail usage tracking
#   is not enabled for a given domain.  To enable webmail usage tracking,
#   make use of a domain template with it turned on or turn it on
#   individually under the "Manage webmail features" menu.
#
    user_list_elvis = delete_checkbox, username, info, webmail_stats, mailbox, forwards
    user_list_cross = delete_checkbox, username, info, webmail_stats, forwards
    user_list_lowly = delete_checkbox, username, info, webmail_stats, forwards

#
# user_details_elvis
# user_details_cross
# user_details_lowly
#   These each determine the fields that are shown on the user details
#   screen for each type of administrator (normal/"lowly" admin, multiple/cross-
#   domain admin, superuser/"elvis").  The only acceptable values for these 
#   settings may be any combination (separated by commas) of:
#     info
#     mailbox
#     password
#     expiry
#     hardquota
#     softquota
#     sizelimit
#     countlimit
#     forwards
#     webmail_stats
#     prefs
#     <plus any custom fields you have defined in your 
#      backend custom_user_account_fields setting>
#   Note that "webmail_stats" will have no effect if webmail usage tracking
#   is not enabled for a given domain.  To enable webmail usage tracking,
#   make use of a domain template with it turned on or turn it on
#   individually under the "Manage webmail features" menu.
#     
;    user_details_elvis = info, mailbox, password, expiry, hardquota, countlimit, forwards, webmail_stats, prefs, POP3
;    user_details_cross = info, password, hardquota, countlimit, forwards, POP3, webmail_stats
;    user_details_lowly = info, password, hardquota, countlimit, forwards, webmail_stats
    user_details_elvis = info, mailbox, password, expiry, hardquota, countlimit, forwards, webmail_stats, prefs
    user_details_cross = info, password, hardquota, countlimit, forwards, webmail_stats
    user_details_lowly = info, password, hardquota, countlimit, forwards, webmail_stats

#
# domain_customizations_elvis
# domain_customizations_cross
# domain_customizations_lowly
#   These each determine the menu choices that are shown on the domain
#   customizations menu for each type of administrator (normal/"lowly" admin,
#   multiple/cross-domain admin, superuser/"elvis").  The only acceptable
#   values for these settings may be any combination (separated by commas) of:
#     login_screen_customize
#     set_user_permissions
#
    domain_customizations_elvis = login_screen_customize, set_user_permissions
    domain_customizations_cross = login_screen_customize, set_user_permissions
    domain_customizations_lowly = login_screen_customize, set_user_permissions

#
# extra_delete_files_dirs_elvis
# extra_delete_files_dirs_cross
# extra_delete_files_dirs_lowly
#   This is a list of files and directories that are typically deleted
#   when a user account is removed.  This setting is ONLY used for
#   displaying what files and directories may have been removed upon
#   account deletion; these are NOT related to any actual backend
#   functionality whatsoever!  Each file or directory name should be
#   separated with a comma, and when displayed, will typically be on
#   a line of its own.  $1 anywhere in the text of these settings will
#   be replaced with the username when it is displayed.
#
    extra_delete_files_dirs_elvis =
    extra_delete_files_dirs_cross =
    extra_delete_files_dirs_lowly =

#
# domain_limits_elvis
# domain_limits_cross
#   These are the elements that administrators can place limits upon
#   in the "Set domain limits" part of the Vadmin interface.  Lowly
#   administrators cannot set domain limits, which is why there are
#   only two of these settings.  Some systems may not use fields such
#   as maximum message size limitation, so such fields can be removed.
#   The only acceptable values for these settings may be any combination 
#   (separated by commas) of:
#     mailboxes
#     hardquota
#     softquota
#     size
#     count
#     imgsize
#
    domain_limits_elvis = mailboxes, hardquota, size, count, imgsize
    domain_limits_cross = mailboxes, hardquota, size, count, imgsize


##
#
# [system_manager_proxy]
# For backends that require it, Vadmin can send command details
# to a system management script or socket in addition to any
# internal Vadmin actions.  The proxy has to be programmed into
# a backend, therefore you only need to configure these options
# if your backend uses it.  Currently, these backends make use
# of it:
#
#   sql
#
# This proxy allows Vadmin to trigger the execution of actions that
# may only be done by the root user, such as purging mail spools,
# creating new accounts and the like.  You can specify which actions
# should and should not be passed along to the proxy application
# (leave the settings for undesired actions blank below to turn thenm
# off), and these actions may be done *in addition* to whatever
# actions are already accomplished in the backend that uses the proxy.
#
# You may specify your own proxy script or socket address here, but
# Vadmin also provides a set-uid (SUID) proxy application that is
# meant to be a secure gateway to running your own system management
# shell scripts or other regular programs.
#
# If you choose to use your own SUID proxy script here, it must
# understand Vadmin's protocol (see the function vadmin_suid_script_exec()
# in includes/vadmin_functions.inc for details), and should make as
# many security guarantees as possible.  The included SUID proxy verifies
# the domain password, checks that the current user is a legal Vadmin
# administrator, checks that the user has a valid IMAP login, and uses
# the same Vadmin cryptography features that are a part of the main PHP
# code.  Only after all these checks have succeeded does it pass
# execution (along with any arguments it was given) to a script or
# application of your choosing, called with root (superuser) privileges.
#
# Before you can use it, however, you must go into the vadmin/includes/suid/
# directory and compile and install the proxy application.  Please refer
# to the README file in that directory for guidance on how to compile
# and install it.
#
# If you are using a socket application, it must also conform to
# Vadmin's internal protocol, which is defined in the function
# vadmin_socket_exec() in includes/vadmin_functions.inc, wherein certain
# authentication information is sent to the socket, after which the
# action-specific flags defined below are sent.
#
[system_manager_proxy]

#
# path
#   The path to the proxy script or socket.  When using a custom socket
#   application, it MUST be preceeded with the socket protocol, such
#   as "unix://" or "tcp://", etc.  If it is not, Vadmin will assume you
#   are using a SUID application.  When using Vadmin's included SUID
#   proxy application, we recommend you install it in a place more
#   appropriate than the SquirrelMail plugins directory.  You will also
#   need to read the documentation in the vadmin/includes/suid directory
#   and compile the proxy application yourself.
#
;    path = unix:///path/to/system/manager/socket
;    path = unix:///tmp/.sysmand
;    path = tcp://127.0.0.1:4990
;    path = /path/to/squirrelmail/plugins/vadmin/includes/suid/vadmin_auth
;    path = /usr/share/squirrelmail/plugins/vadmin/includes/suid/vadmin_auth
    path = /usr/local/sbin/vadmin_auth

#
# socket_open_timeout
#   When the path (see above) is set to use a socket address, this is
#   the number of seconds to wait before giving up when trying to open
#   a connection to the socket.
#
    socket_open_timeout = 4

#
# sanitize_flag_arguments
#   When set to "yes", this tells the backend to escape and quote the
#   user input replacements in all of the "*_flags" settings below.
#   This should always be turned on unless the data is passed to a
#   backend that takes care of sanitizing the data itself (the manner
#   that the arguments are passed to a socket backend may allow for
#   this to be turned off).
#
    sanitize_flag_arguments = yes

#
# account_creation_flags
#   This is a string of options that will be passed on the command
#   line to your system management script by the proxy when a user
#   account is being created.  If empty, the proxy will not be used
#   for this action.
#     $1 in this string will be replaced with the user name (user name only;
#        the domain portion of the email address will not be included)
#     $2 in this string will be replaced with the domain name
#     $3 in this string will be replaced with the new password (raw, unencrypted)
#     $6 in this string will be replaced with the new hard quota
#     $9 in this string will be replaced with the new message count limit
#     $a in this string will be replaced with the new Courier-style quota
#     $u in this string will be replaced with the username of the user who is
#        invoking this action (NOT the username being created, which is $1)
#     $p in this string will be replaced with the (raw, unencrypted) password
#        of the user who is invoking this action (NOT that of the user being
#        created, which is $3) (CAREFUL: only use this if necessary)
#   For security, all of the above replacements will be placed in single
#   quotes and any single quotes inside of them will be escaped.
#
;    account_creation_flags = create $1 $2 $3 $u $p
    account_creation_flags = --create --user $1 --domain $2 --password $3

#
# alias_creation_flags
#   This is a string of options that will be passed on the command
#   line to your system management script by the proxy when an alias
#   is being created.  If empty, the proxy will not be used for this 
#   action.
#     $1 in this string will be replaced with the alias name (full email
#        address)
#     $2 in this string will be replaced with the destination address(es)
#        for the alias (full email address(es))
#     $u in this string will be replaced with the username of the user who is
#        invoking this action (NOT the alias name being created, which is $1)
#     $p in this string will be replaced with the (raw, unencrypted) password
#        of the user who is invoking this action (CAREFUL: only use this if
#        necessary)
#   For security, all of the above replacements will be placed in single
#   quotes and any single quotes inside of them will be escaped.
#
    alias_creation_flags =

#
# account_deletion_flags
#   This is a string of options that will be passed on the command
#   line to your system management script by the proxy when a user
#   account is being deleted.  If empty, the proxy will not be used
#   for this action.
#     $1 in this string will be replaced with the user name (user name only;
#        the domain portion of the email address will not be included)
#     $2 in this string will be replaced with the domain name
#     $u in this string will be replaced with the username of the user who is
#        invoking this action (NOT the username being deleted, which is $1)
#     $p in this string will be replaced with the (raw, unencrypted) password
#        of the user who is invoking this action (NOT that of the user being
#        deleted) (CAREFUL: only use this if necessary)
#   For security, all of the above replacements will be placed in single
#   quotes and any single quotes inside of them will be escaped.
#
;    account_deletion_flags = delete $1 $2 $u $p
    account_deletion_flags = --delete --user $1 --domain $2

#
# alias_deletion_flags
#   This is a string of options that will be passed on the command
#   line to your system management script by the proxy when an alias
#   is being deleted.  If empty, the proxy will not be used for this
#   action.
#     $1 in this string will be replaced with the alias name (full email
#        address)
#     $u in this string will be replaced with the username of the user who is
#        invoking this action (NOT the alias name being deleted, which is $1)
#     $p in this string will be replaced with the (raw, unencrypted) password
#        of the user who is invoking this action (CAREFUL: only use this if
#        necessary)
#   For security, all of the above replacements will be placed in single
#   quotes and any single quotes inside of them will be escaped.
#
    alias_deletion_flags =

#
# account_change_flags
#   This is a string of options that will be passed on the command
#   line to your system management script by the proxy when a user
#   account attribute is being changed.  If empty, the proxy will
#   not be used for this action.
#     $1 in this string will be replaced with the user name (user name only;
#        the domain portion of the email address will not be included)
#     $2 in this string will be replaced with the domain name
#     $3 in this string will be replaced with a numeric key that
#        indicates which attribute is being changed:
#           1 : password
#           2 : destination (mailbox)
#           3 : hard quota
#           4 : soft quota
#           5 : message size limit
#           6 : message count limit
#           7 : account expiry
#           8 : mailbox flags
#           9 : personal info (personal/real name information)
#          10 : forwards (non-vmailmgr; added for SQL backend)
#          <or the name of a custom field if it is being changed,
#           with spaces changed to uderscores>
#     $4 in this string will be replaced with the new attribute value
#     $u in this string will be replaced with the username of the user who is
#        invoking this action (NOT the username being changed, which is $1)
#     $p in this string will be replaced with the (raw, unencrypted) password
#        of the user who is invoking this action (NOT that of the user being
#        changed) (CAREFUL: only use this if necessary)
#   For security, all of the above replacements will be placed in single
#   quotes and any single quotes inside of them will be escaped.
#
;    account_change_flags = update $1 $2 $3 $4 $u $p
    account_change_flags = --update $3 --value $4 --user $1 --domain $2


##
#
# [limits]
# Certain setting limitations are managed by administrators in
# the Vadmin interface.  These settings outline how they are
# managed.  Note that the defaults defined here will apply to
# new domain limit templates, and will NOT apply to any one
# domain unless such a template is applied to it.
#
# default_mailbox_limit
#   This is the default limitation on the number of mailboxes
#   allowed per domain.  If set to no value (left empty),
#   mailboxes will be unlimited.
#
# default_hard_quota_limit
#   This is the default maximum (hard) quota (in MiB) allowed
#   per domain.  If set to no value (left empty), quotas will
#   be unlimited.
#
# default_message_size_limit
#   This is the default limitation on message sizes (in MiB)
#   allowed per domain.  If set to no value (left empty),
#   message sizes will be unlimited.
#
# default_message_count_limit
#   This is the default maximum number of messages allowed for
#   any one user in a domain.  If set to no value (left empty),
#   message maximums will be unlimited.
#
# default_image_size_limit
#   This is the default maximum login image size (in KiB)
#   allowed per domain.  If set to no value (left empty),
#   image sizes will be unlimited.
#
# default_allow_password_change
#   This is the default state of whether or not users will be
#   able to change their passwords.  1 = yes, 0 = no.
#
# default_allow_autoresponder
#   This is the default state of whether or not users will be
#   able to use the autoresponder.  1 = yes, 0 = no.
#
# default_allow_mail_forwarding
#   This is the default state of whether or not users will be
#   able to use mail forwarding.  1 = yes, 0 = no.
#
# default_enable_usage_tracking
#   This is the default state of whether or not webmail usage
#   will be tracked.  1 = yes, 0 = no.
#
[limits]
    default_mailbox_limit =
    default_hard_quota_limit =
    default_message_size_limit =
    default_message_count_limit =
    default_image_size_limit = 100
    default_allow_password_change = 1
    default_allow_autoresponder = 0
    default_allow_mail_forwarding = 0
    default_enable_usage_tracking = 0


##
#
# [autoresponder]
# The internal autoresponder can be configured to work with
# slightly different implementations.  These settings will
# not affect any external autoresponder that may be used with
# Vadmin.
#
# message_contains_headers
#   When empty, the autoresponse message stored for each user
#   will be treated as ONLY containing the message text and no
#   more.  When the message is stored with additional headers
#   (qmail-autoresponder can use this format), they are stored
#   before the message, each on one line, separated from the
#   message text with a blank line.  Any headers may be specified
#   here, but only standard ones such as from and subject are
#   recommended.  When this list includes "from", it will be
#   automatically populated with the user's email address, but
#   any other fields are shown as user-controllable input fields.
#
[autoresponder]
;    message_contains_headers = subject, from
    message_contains_headers =


##
#
# [upgrade]
# This option will be useful to you only if you are upgrading from using
# vadmin-1.0.x.  Before you do anything here please read doc/UPGRADING
# VERY CAREFULLY.  Just uncommenting this section will not work.
#
# upgrade
#   Set to "yes" to enable auto-upgrading of preferences.
#
# dir
#   If you want to keep old and new preferences separately, you may
#   move the old vadmin directory to vadmin-old, and point this value
#   to that location.  The new and old settings do not clash, though, 
#   so keeping this setting the same as the one in [storage] should 
#   be just fine.
#
# cleanup
#   Whether to remove the old-style preferences once they have been 
#   processed and upgraded.  This shouldn't be necessary, unless you 
#   want to err on the side of caution.
#
;[upgrade]
;    upgrade = yes
;    dir = /var/lib/vadmin
;    cleanup = yes


##
#
# [debug]
# These are the debugging settings.  On production systems they should 
# be commented out.
#
# level
#   When debugging is enabled, this should be set to something
#   other than zero.  Level 1 enables Vadmin diagnostic logging
#   per the "output" setting below.  Level 2 enables certain
#   PHP error information to be logged or displayed on screen
#   (as long as PHP is not configured to hide such information)
#   in addition to level 1 functionality.
#
# output
#   Vadmin will put all debugging output in this file, which will be 
#   appended with the username of the person accessing the system.
#
;[debug]
;    level = 1
;    output = /tmp/vadmin-debug.txt



# Please do not change the following line or you will break Vadmin
# */

