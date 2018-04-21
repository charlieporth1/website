<?php /*
# Please do not change the line above or you will break Vadmin


##
# Vadmin SQL backend configuration file.
# --------------------------------------
# This configuration file is in a simple win.ini-style format.
# Lines starting with "#" and ";" are comments.
#
# CACHING NOTE: Vadmin caches the configuration in session, so if you
# have made changes to this file, you will need to sign out of squirrelmail
# and sign back in to see the changes.
# 
# $Id: sql.conf_example.php,v 1.2 2008/12/16 06:42:40 pdontthink Exp $
#


[sql]


##
#
# delimiters
#   Make sure that any and all characters used to separate the domain portion
#   from the username portion of your email addresses are included in this
#   setting. If the example value given below contains any characters found
#   in your usernames or domain names, remove those characters (and ask 
#   yourself why you are using those characters like that). The first character
#   in this list should always be the preferred delimiter character that
#   will be used to construct full email addresses when necessary.
#
delimiters = @:%


##
#
# database_wildcard
#   This should be set to whatever the multiple-character wildcard is for 
#   your database implementation.
#
database_wildcard = %


##
#
# catchall_alias
#   Set this to the username portion of catchall addresses on your system.
#   Blank values are acceptable and correspond to "@example.com".
#
catchall_alias = 


##
#
# search_enabled
#   Set this to "no" if the user "lookup" mechanism does not understand
#   search syntax (any username lookup with an asterisk in it).
#
search_enabled = yes


##
#
# vmail_path
#   This is the path to the file that contains the bulk of the code that 
#   implements this backend's functionality.  You should only need to 
#   adjust the base path for this setting.
#
;vmail_path = /path/to/squirrelmail/plugins/vadmin/includes/sql/vmail.inc
vmail_path = /usr/share/squirrelmail/plugins/vadmin/includes/sql/vmail.inc


##
#
# password encryption
#   The type of password encryption to be used.  Currently, these are
#   supported:
#     md5plain      (uses plain MD5 hash)
#     md5crypt      (uses MD5-enhanced crypt(3) algorithm)
#     saslcrypt     (uses MySQL password() function)
#     unixcrypt     (uses MySQL encrypt() function, which
#                   should be the same as system crypt(3))
#     vadmincrypt   (uses the encryption method chosen in vadmin Apache setup)
#
password_encryption = unixcrypt


##
#
# pagination
#   Set this to the default number of items to show in one page if this 
#   backend supports pagination and you want to allow users to paginate 
#   user lists.  Set to zero to disable pagination.
#
#   "Fancy" pagination refers to whether or not the pagination drop-down
#   list will have the username at the top of each page listed next to
#   the page number.
#
pagination = 25
fancy_pagination = yes


##
#
# db_dsn
#   Theoretically, any SQL database supported by Pear should be supported
#   here.  The DSN (data source name) must contain the information needed
#   to connect to your database backend. A MySQL example is included below.
#   For more details about DSN syntax and list of supported database types,
#   please see:
#     http://pear.php.net/manual/en/package.database.db.intro-dsn.php
#
db_dsn = mysql://user:password@localhost/database


##
#
# forwardsCSV
#   Set this to "yes" if your email system allows multiple forwarding
#   (or alias destination) addresses all together in comma-separated-
#   values format (most MTAs do)
#
forwardsCSV = yes
;forwardsCSV = no


##
#
# custom_user_account_fields
#   This is a list of database fields that are particular to your 
#   email system implementation.  Extra field names defined here
#   will be expected as part of all user account queries, inserts
#   and updates (as long as you also add these fields to those 
#   queries as defined elsewhere in this configuration file - in
#   the same order).  This should be a comma-separated list of the
#   field names as they will be presented in the user interface 
#   (spaces in field names are OK and should be used if fields are
#   multi-word).
#
#   Currently, no more than 26 fields are supported, but this can
#   be improved if someone really needs it....
#
;custom_user_account_fields = POP3


##
#
# custom_field_types
#   Provide expected field types for your custom fields (as defined 
#   with the custom_user_account_fields setting).  The types should
#   all be given keyed by the name of the setting, exactly as given
#   for custom_user_account_fields but with spaces replaced with
#   underscores and followed by "_type".  Supported types must be 
#   one of:
#     int        integer values only; input taken in standard simple
#                one-line text field
#     text       all string values OK; input taken in standard simple
#                one-line text field
#     largetext  all string values OK; input taken in multiple-line
#                text input box (textarea)
#     boolean    0/1 or TRUE/FALSE values only; input taken in radio
#                buttons
#     checkbox   0/1 or TRUE/FALSE values only; input taken in the
#                form of a checkbox
#
;POP3_type = boolean


##
#
# custom_field_suffix_text
#   You can indicate any text that you'd like to place after a
#   custom field's input widget (text box, radio buttons, checkbox, 
#   etc.) with this setting.  These descriptions should all be 
#   given keyed by the name of the setting, exactly as given
#   for custom_user_account_fields but with spaces replaced with
#   underscores and followed by "_suffix".
#
;POP3_suffix =


##
#
# custom_field_descriptions
#   Provide descriptions for your custom fields (as defined with the
#   custom_user_account_fields setting).  These descriptions should
#   all be given keyed by the name of the setting, exactly as given
#   for custom_user_account_fields but with spaces replaced with
#   underscores.
#
;POP3 = This allows you to turn a user's POP access on and off.


##
#
# account_deletion_queries
#   This is a list of the names of any queries defined below that
#   are to be run when removing a user account from the system.
#   You may add additional queries as needed.  Queries are run
#   in the order they are listed here.  Query names are to be 
#   separated by commas.  You may also leave this setting empty
#   to leave all deletion responsibility to your system management
#   script instead.
#
#   Note that all queries in this list will have the following
#   variable substitution available to them - no more than this,
#   even if specified in the documentation for the query itself.
#
#     $1 in each query will be replaced with the user name (user name only;
#        the domain portion of the email address will not be included)
#     $2 in each query will be replaced with the domain name
#
#   For security, any single quotes and/or backslashes inside all
#   of the replacements above will be escaped (ab\cd'ef becomes ab\\cd\'ef).
#
;account_deletion_queries = delete_all_user_aliases_query, delete_all_user_forwards_query, delete_account_query
account_deletion_queries =


##
#
# domain_list_query
#   The SQL query that will gather a list of unique domains hosted on your
#   system. The query must return domain names ONLY.
#
domain_list_query = SELECT DISTINCT domain FROM email_users ORDER BY domain


##
#
# domain_password_query
#   The SQL query that will validate the password for the given domain. The
#   query should return 1 if the given domain and password pair correctly
#   match and 0 (zero) if they do not, usually by counting the number of rows 
#   that match the pair. If you do not keep domain passwords, you will have 
#   to improvise - one idea is to have the password be the same as the domain,
#   so this query would be similar to the domain_list_query except that it has 
#   two WHERE clauses that both ask the domain name to match the given 
#   password and domain.
#
#     $1 in this query will be replaced with the domain name
#     $2 in this query will be replaced with the password
#
#   For security, any single quotes and/or backslashes inside all
#   of the replacements above will be escaped (ab\cd'ef becomes ab\\cd\'ef).
#
domain_password_query = SELECT count(*) FROM domain_administration WHERE domain = '$1' AND crypt_password = encrypt('$2', crypt_password)
;domain_password_query = SELECT IF(count(*), 1, 0) FROM email_users WHERE domain = '$1' AND domain = '$2'


##
#
# list_all_accounts_query
#   The SQL query that gathers a list of all user accounts for the given
#   domain.  The query must return account information in a very specific
#   order, and if any one of the specified fields is not available in your
#   database, you must return a hard-coded "===N/A===" in its place (you 
#   will also want to remove those fields from the "displayable_fields"
#   for the user_details screen in the main vadmin configuration file).  
#   The list of fields (in the order that they must be returned) is:
#     account name               (Note that this may be just the username or
#                                it may contain the domain portion (full email
#                                address) as long as the separator is listed
#                                in the "delimiters" setting.)
#     password                   
#     mailbox location
#     <dummy>                    (Please specify "===N/A===" here.)
#     personal info
#     hard quota                 (Courier-style quotas with both total size
#                                and message count quotas in the same field
#                                will be parsed correctly here)
#     soft quota
#     message size limit
#     message count limit        (Courier-style quotas with both total size
#                                and message count quotas in the same field
#                                will be parsed correctly here)
#     account creation date/time
#     account expiry time
#     account flags
#     <plus any custom fields as defined in custom_user_account_fields>
#
#   Note also that:
#
#     $1 in this query will be replaced with the domain name
#     $2 in this query will be replaced with "all_accounts_search_clause"
#        when necessary
#
#   For security, any single quotes and/or backslashes inside all
#   of the replacements above will be escaped (ab\cd'ef becomes ab\\cd\'ef).
#
;list_all_accounts_query = SELECT username, crypt_password, maildir, '===N/A===', real_name, quota, '===N/A===', '===N/A===', quota, account_created_on, '===N/A===', enabled, pop3_enabled FROM email_users WHERE domain = '$1' $2 ORDER BY username
list_all_accounts_query = SELECT username, crypt_password, maildir, '===N/A===', real_name, quota, '===N/A===', '===N/A===', quota, account_created_on, '===N/A===', enabled FROM email_users WHERE domain = '$1' $2 ORDER BY username


##
#
# all_accounts_search_clause
#   This clause is inserted into "list_all_accounts_query" when 
#   the account lookup needs to be limited to a subset of all 
#   user accounts.
#
#     $1 in this query will be replaced with the search phrase
#
#   For security, any single quotes and/or backslashes inside all
#   of the replacements above will be escaped (ab\cd'ef becomes ab\\cd\'ef).
#
all_accounts_search_clause = AND username LIKE '$1'


##
#
# vadmin_get_all_usernames_for_domain_query
#   The SQL query that gathers a quick list of all user accounts names 
#   for the given domain.  This is the shortened version of 
#   "list_all_accounts_query", and it should only return a list of 
#   usernames and no more.  Usernames may be returned with only the
#   username portion, or they may contain the domain portion (full email
#   address) as long as the separator is listed in the "delimiters" setting.)
#
#     $1 in this query will be replaced with the domain name
#     $2 in this query will be replaced with "all_accounts_search_clause"
#        when necessary
#
#   For security, any single quotes and/or backslashes inside all
#   of the replacements above will be escaped (ab\cd'ef becomes ab\\cd\'ef).
#
;vadmin_get_all_usernames_for_domain_query = SELECT username FROM email_users WHERE username LIKE '%@$1' $2 ORDER BY username
vadmin_get_all_usernames_for_domain_query = SELECT username FROM email_users WHERE domain = '$1' $2 ORDER BY username


##
#
# find_user_account_query
#   The SQL query that looks up a single user account for the given domain.
#   The query must return all the fields described above for the 
#   list_all_accounts_query just for one user.
#
#     $1 in this query will be replaced with the user name (user name only;
#        the domain portion of the email address will not be included)
#     $2 in this query will be replaced with the domain name
#
#   For security, any single quotes and/or backslashes inside all
#   of the replacements above will be escaped (ab\cd'ef becomes ab\\cd\'ef).
#     
;find_user_account_query = SELECT username, crypt_password, maildir, '===N/A===', real_name, quota, '===N/A===', '===N/A===', quota, account_created_on, '===N/A===', enabled, pop3_enabled FROM email_users WHERE username = '$1@$2'
;find_user_account_query = SELECT username, crypt_password, maildir, '===N/A===', real_name, quota, '===N/A===', '===N/A===', quota, account_created_on, '===N/A===', enabled FROM email_users WHERE user_name = '$1' AND domain = '$2'
find_user_account_query = SELECT username, crypt_password, maildir, '===N/A===', real_name, quota, '===N/A===', '===N/A===', quota, account_created_on, '===N/A===', enabled FROM email_users WHERE username = '$1@$2'


##
#
# find_alias_query
#   The SQL query that looks up a single email alias for the given domain.
#   The query must return all the fields described above (including any 
#   custom fields) for the list_all_accounts_query just for one alias, 
#   however, many systems may only provide a username/email address for 
#   each alias, in which case all other fields should be hard-coded 
#   to "===N/A===".
#
#     $1 in this query will be replaced with the alias name (alias name only;
#        the domain portion of the email address will not be included)
#     $2 in this query will be replaced with the domain name
#
#   For security, any single quotes and/or backslashes inside all
#   of the replacements above will be escaped (ab\cd'ef becomes ab\\cd\'ef).
#     
find_alias_query = SELECT address, '===N/A===', '===N/A===', '===N/A===', '===N/A===', '===N/A===', '===N/A===', '===N/A===', '===N/A===', '===N/A===', '===N/A===', '===N/A===' FROM virtual WHERE address = '$1@$2'


##
#
# find_all_aliases_query
#   The SQL query that returns a list of all aliases for the given domain. 
#   The query must return all the fields (including any custom fields) described 
#   above for the list_all_accounts_query for each alias, however, many systems 
#   may only provide a username/email address for each alias, in which case all 
#   other fields should be hard-coded to "===N/A===". 
#
#     $1 in this query will be replaced with the domain name
#
#   For security, any single quotes and/or backslashes inside all
#   of the replacements above will be escaped (ab\cd'ef becomes ab\\cd\'ef).
#
find_all_aliases_query = SELECT address, '===N/A===', '===N/A===', '===N/A===', '===N/A===', '===N/A===', '===N/A===', '===N/A===', '===N/A===', '===N/A===', '===N/A===', '===N/A===' FROM virtual WHERE address LIKE '%@$1' ORDER BY address


##
#
# find_user_aliases_query
#   The SQL query that returns a list of all aliases (usually full email 
#   addresses) that point to the given username and domain. The query must
#   only return the aliases and no other fields.
;FIXME - is it acceptable that all aliases are possibly returned in a CSV string here too??  this query is not currently in use (but users, PLEASE don't skip it in case we make use of it in a future version!), so we will answer that question when we start using this query (for reference, see how we did it with find_user_forwards_query)
#
#     $1 in this query will be replaced with the user name (user name only;
#        the domain portion of the email address will not be included)
#     $2 in this query will be replaced with the domain name
#
#   For security, any single quotes and/or backslashes inside all
#   of the replacements above will be escaped (ab\cd'ef becomes ab\\cd\'ef).
#
find_user_aliases_query = SELECT address FROM virtual WHERE goto = '$1@$2' ORDER BY address


##
#
# find_real_account_for_alias_query
#   The SQL query that returns the real account name that the given 
#   alias points to.  The query must return just the username of the
#   target account (domain portion of address optional, but if not
#   provided, current domain will be assumed, which might not always
#   be a good assumption). If more than one result is found (which is
#   not usually expected, so behavior may be unpredictable), result
#   may be in multple rows or in comma-separated-values format.
#
#     $1 in this query will be replaced with the alias name
#     $2 in this query will be replaced with the domain name of the alias
#
#   For security, any single quotes and/or backslashes inside all
#   of the replacements above will be escaped (ab\cd'ef becomes ab\\cd\'ef).
#
find_real_account_for_alias_query = SELECT goto FROM virtual WHERE address = '$1@$2' ORDER BY goto


##
#
# find_user_forwards_query
#   The SQL query that gathers a list of all forwards (usually full email
#   addresses) specified for the given username and domain. The query must
#   only return the forwards and no other fields. If all forwards are 
#   returned as a single string in comma-separated-values format, this
#   is also acceptable (and the example query should work).
#
#     $1 in this query will be replaced with the user name (user name only;
#        the domain portion of the email address will not be included)
#     $2 in this query will be replaced with the domain name
#
#   For security, any single quotes and/or backslashes inside all
#   of the replacements above will be escaped (ab\cd'ef becomes ab\\cd\'ef).
#
find_user_forwards_query = SELECT goto FROM virtual WHERE address = '$1@$2' ORDER BY goto


##
#
# insert_user_query
#   The SQL query that inserts a new user account into the system.  
#   If left blank, no query will be run (your system management
#   script will be responsible for adding the user instead).
#   The following fields are to be included in this query:
#     account name               (Note that this may be just the username or
#                                it may contain the domain portion (full email
#                                address) as long as the separator is listed
#                                in the "delimiters" setting.)
#     password                   (Vadmin will put quotes around this field's
#                                value if necessary.  This query should NOT
#                                include quotes around this field.)
#     hard quota
#     message count limit
#     courier quota              (If used, this field will be automatically
#                                constructed in Courier-compatible format 
#                                with size-based and/or message count-based 
#                                quotas.)
#     account flags
#     date of last modification  (The time that this query is run will be 
#                                inserted into this field.)
#   
#   If your system does not use some of these fields, you may leave any 
#   of the following replacement keys out of the query text, in which 
#   case the corresponding field will be ignored.
#
#     $1 in this query will be replaced with the user name (user name only;
#        the domain portion of the email address will not be included)
#     $2 in this query will be replaced with the domain name
#     $3 in this query will be replaced with the new password
#     $6 in this query will be replaced with the new hard quota
#     $9 in this query will be replaced with the new message count limit
#     $a in this query will be replaced with the new Courier-style quota
#     $d in this query will be replaced with the current date and time
#
#   NOTE: currently, $6, $9, and $a are NOT used.  Instead, the user is 
#         created without blank values for these fields, after which user
#         attributes are changed for those fields.
#
#   For security, any single quotes and/or backslashes inside all
#   of the replacements above will be escaped (ab\cd'ef becomes ab\\cd\'ef).
#
;insert_user_query = INSERT email_users (username, crypt_password, quota, pop3_enabled) VALUES ('$1@$2', $3, '$a', 0)
;insert_user_query = INSERT email_users (user_name, domain, crypt_password, quota) VALUES ('$1', '$2', $3, '$6')
;insert_user_query = INSERT email_users (username, crypt_password, quota) VALUES ('$1@$2', $3, '$a')
insert_user_query = 


##
#
# update_user_attributes_query
#   The SQL query that updates basic user account information.  You may
#   also leave this setting empty to leave all update responsibility to
#   your system management script instead.  The following fields are to 
#   be updated in this query:
#     account name               (Note that this may be just the username or
#                                it may contain the domain portion (full email
#                                address) as long as the separator is listed
#                                in the "delimiters" setting.)
#     password                   (Vadmin will put quotes around this field's
#                                value if necessary.  This query should NOT
#                                include quotes around this field.)
#     mailbox location
#     personal info
#     hard quota
#     soft quota
#     message size limit
#     message count limit
#     courier quota              (If used, this field will be automatically
#                                constructed in Courier-compatible format 
#                                with size-based and/or message count-based 
#                                quotas.)
#     account expiry time        (Usually given in number of days.)
#     account flags
#     date of last modification  (The time that this query is run will be 
#                                inserted into this field.)
#     <plus any custom fields as defined in custom_user_account_fields>
#   
#   If your system does not use some of these fields, you may leave any 
#   of the following replacement keys out of the query text, in which 
#   case the corresponding field will be ignored.
#
#     $1 in this query will be replaced with the user name (user name only;
#        the domain portion of the email address will not be included)
#     $2 in this query will be replaced with the domain name
#     $3 in this query will be replaced with the new password
#     $4 in this query will be replaced with the new mailbox location
#     $5 in this query will be replaced with the new personal info
#     $6 in this query will be replaced with the new hard quota
#     $7 in this query will be replaced with the new soft quota
#     $8 in this query will be replaced with the new message size limit
#     $9 in this query will be replaced with the new message count limit
#     $a in this query will be replaced with the new Courier-style quota
#     $b in this query will be replaced with the new account expiry time
#     $c in this query will be replaced with the new account flags
#     $d in this query will be replaced with the current date and time
#     $_a in this query will be replaced with the first defined custom field (if any)
#     $_b in this query will be replaced with the second defined custom field (if any)
#     $_c in this query will be replaced with the third defined custom field (if any)
#     ...
#     ...
#     ...
#     $_z in this query will be replaced with the 26th defined custom field (if any)
#
#   For security, any single quotes and/or backslashes inside all
#   of the replacements above will be escaped (ab\cd'ef becomes ab\\cd\'ef).
#
;update_user_attributes_query =
;update_user_attributes_query = UPDATE email_users SET crypt_password = $3, maildir = '$4', real_name = '$5', quota = '$6', pop3_enabled = $_a WHERE user_name = '$1' AND domain = '$2'
update_user_attributes_query = UPDATE email_users SET crypt_password = $3, maildir = '$4', real_name = '$5', quota = '$a' WHERE username = '$1@$2'


##
#
# insert_alias_query
#   The SQL query that adds a single alias to the system.
#
#     $1 in this query will be replaced with the alias name (full email address)
#     $2 in this query will be replaced with the destination (full email address)
#
#   For security, any single quotes and/or backslashes inside all
#   of the replacements above will be escaped (ab\cd'ef becomes ab\\cd\'ef).
#
insert_alias_query = INSERT INTO virtual (address, goto) VALUES('$1', '$2')


##
#
# insert_forward_query
#   The SQL query that adds a single forward to the system.
#
#     $1 in this query will be replaced with the forward source (full email address)
#     $2 in this query will be replaced with the destination (full email address)
#
#   For security, any single quotes and/or backslashes inside all
#   of the replacements above will be escaped (ab\cd'ef becomes ab\\cd\'ef).
#
insert_forward_query = INSERT INTO virtual (address, goto) VALUES('$1', '$2')


##
#
# delete_alias_query
#   The SQL query that removes a single alias from the system.
#
#     $1 in this query will be replaced with the alias name (full email address)
#
#   For security, any single quotes and/or backslashes inside all
#   of the replacements above will be escaped (ab\cd'ef becomes ab\\cd\'ef).
#
delete_alias_query = DELETE FROM virtual WHERE address = '$1'


##
#
# delete_forward_query
#   The SQL query that removes a single forward from the system.
#
#     $1 in this query will be replaced with the forward source (full email address)
#
#   For security, any single quotes and/or backslashes inside all
#   of the replacements above will be escaped (ab\cd'ef becomes ab\\cd\'ef).
#
delete_forward_query = DELETE FROM virtual WHERE address = '$1'


##
#
# delete_all_user_aliases_query
#   The SQL query that removes any aliases that point to a certain
#   real user account.
#
#     $1 in this query will be replaced with the user name (user name only;
#        the domain portion of the email address will not be included)
#     $2 in this query will be replaced with the domain name
#
#   For security, any single quotes and/or backslashes inside all
#   of the replacements above will be escaped (ab\cd'ef becomes ab\\cd\'ef).
#
delete_all_user_aliases_query = DELETE FROM virtual WHERE goto = '$1@$2'


##
#
# delete_all_user_forwards_query
#   The SQL query that removes any forwards for a certain user.
#
#     $1 in this query will be replaced with the user name (user name only;
#        the domain portion of the email address will not be included)
#     $2 in this query will be replaced with the domain name
#
#   For security, any single quotes and/or backslashes inside all
#   of the replacements above will be escaped (ab\cd'ef becomes ab\\cd\'ef).
#
delete_all_user_forwards_query = DELETE FROM virtual WHERE address = '$1@$2'


##
#
# delete_account_query
#   The SQL query that removes a user account from the system.
#
#     $1 in this query will be replaced with the user name (user name only;
#        the domain portion of the email address will not be included)
#     $2 in this query will be replaced with the domain name
#
#   For security, any single quotes and/or backslashes inside all
#   of the replacements above will be escaped (ab\cd'ef becomes ab\\cd\'ef).
#
;delete_account_query = DELETE FROM email_users WHERE user_name = '$1' AND domain = '$2'
delete_account_query = DELETE FROM email_users WHERE username = '$1@$2'



# Please do not change the following line or you will break Vadmin
# */

