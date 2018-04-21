<?php /*
# Please do not change the line above or you will break Vadmin


##
# Vadmin Vmailmgr backend configuration file.
# -------------------------------------------
# This configuration file is in a simple win.ini-style format.
# Lines starting with "#" and ";" are comments.
#
# CACHING NOTE: Vadmin caches the configuration in session, so if you
# have made changes to this file, you will need to sign out of SquirrelMail
# and sign back in to see the changes.
# 
# $Id: vmailmgr.conf_example.php,v 1.1 2008/09/03 19:45:55 pdontthink Exp $
#


[vmailmgr]


##
#
# delimiters
#   You shouldn't need to edit this unless you have made changes to VMailMgr.
#   The first character in this list should always be the preferred delimiter 
#   character that will be used to construct full email addresses when necessary.
#
delimiters = @:


##
#
# catchall_alias
#   You shouldn't need to edit this unless you have made changes to VMailMgr.
#   Set this to the username portion of catchall addresses on your system.
#   Blank values are acceptable and correspond to "@example.com".
#
catchall_alias = +


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
#   This is the path to the PHP interface to VMailMgr.  You should only 
#   need to adjust the base path for this setting.  One of the examples
#   below may help get you started.  You can also link to the vmail.inc
#   that is provided with the VMailMgr package, but the one that is 
#   included with Vadmin is be modified for optimal use with SquirrelMail, 
#   so you should use that one if at all possible.
#
;vmail_path = /path/to/squirrelmail/plugins/vadmin/includes/vmailmgr/vmail.inc
;vmail_path = /usr/share/squirrelmail/plugins/vadmin/includes/vmailmgr/vmail.inc
vmail_path = /usr/share/vadmin/includes/vmailmgr/vmail.inc


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



# Please do not change the following line or you will break Vadmin
# */

