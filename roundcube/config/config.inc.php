<?php

/* Local configuration for Roundcube Webmail */

// ----------------------------------
// SQL DATABASE
// ----------------------------------
// Database connection string (DSN) for read+write operations
// Format (compatible with PEAR MDB2): db_provider://user:password@host/database
// Currently supported db_providers: mysql, pgsql, sqlite, mssql, sqlsrv, oracle
// For examples see http://pear.php.net/manual/en/package.database.mdb2.intro-dsn.php
// NOTE: for SQLite use absolute path (Linux): 'sqlite:////full/path/to/sqlite.db?mode=0646'
//       or (Windows): 'sqlite:///C:/full/path/to/sqlite.db'
$config['db_dsnw'] = 'mysql://roundcube:NetTechScience99%23%21%21%24%3D@localhost/roundcube';

/* Do not set db_dsnw here, use dpkg-reconfigure roundcube-core to configure database ! */// The mail host chosen to perform the log-in.
// Leave blank to show a textbox at login, give a list of hosts
// to display a pulldown menu or set one host as string.
// To use SSL/TLS connection, enter hostname with prefix ssl:// or tls://
// Supported replacement variables:
// %n - hostname ($_SERVER['SERVER_NAME'])
// %t - hostname without the first part
// %d - domain (http hostname $_SERVER['HTTP_HOST'] without the first part)
// %s - domain name after the '@' from e-mail address provided at login screen
// For example %n = mail.domain.tld, %t = domain.tld
$config['default_host'] = 'localhost:143';

// SMTP username (if required) if you use %u as the username Roundcube
// will use the current username for login
//$config['smtp_user'] = '%u';
// SMTP password (if required) if you use %p as the password Roundcube
// will use the current user's password for login
//$config['smtp_pass'] = '%p';
// SMTP port (default is 25; use 587 for STARTTLS or 465 for the
// deprecated SSL over SMTP (aka SMTPS))
//$config['smtp_port'] = 25;
// SMTP username (if required) if you use %u as the username Roundcube
// will use the current username for login
//$config['smtp_user'] = '';
// SMTP password (if required) if you use %p as the password Roundcube
// will use the current user's password for login
//$config['smtp_pass'] = '';
// provide an URL where a user can get support for this Roundcube installation
// PLEASE DO NOT LINK TO THE ROUNDCUBE.NET WEBSITE HERE!
$config['support_url'] = '';

// check client IP in session authorization
$config['ip_check'] = true;

// this key is used to encrypt the users imap password which is stored
// in the session record (and the client cookie if remember password is enabled).
// please provide a string of exactly 24 chars.
// YOUR KEY MUST BE DIFFERENT THAN THE SAMPLE VALUE FOR SECURITY REASONS
$config['des_key'] = 'D]QAVlOyLPa[Bil6IMlJ[rnm';

// Automatically add this domain to user names for login
// Only for IMAP servers that require full e-mail addresses for login
// Specify an array with 'host' => 'domain' values to support multiple hosts
// Supported replacement variables:
// %h - user's IMAP hostname
// %n - hostname ($_SERVER['SERVER_NAME'])
// %t - hostname without the first part
// %d - domain (http hostname $_SERVER['HTTP_HOST'] without the first part)
// %z - IMAP domain (IMAP hostname without the first part)
// For example %n = mail.domain.tld, %t = domain.tld
$config['username_domain'] = 'otih-oith.us.to';

// List of active plugins (in plugins/ directory)
$config['plugins'] = array('archive', 'attachment_reminder', 'autologon', 'database_attachments', 'emoticons', 'enigma', 'filesystem_attachments', 'http_authentication', 'identicon', 'identity_select', 'managesieve', 'markasjunk', 'new_user_dialog', 'new_user_identity', 'newmail_notifier', 'password', 'vcard_attachments', 'zipdownload');

