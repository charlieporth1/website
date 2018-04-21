#!/bin/sh
#
# This script tests that vadmin_auth can be connected to and
# authenticated against as it would normally be used in the
# Vadmin plugin context.  This test should be run after
# ensuring that vadmin_auth was successfully compiled.
#
# This is only a test, however, of whether or not a valid
# connection and authentication can be made.  It does not
# pass any real commands through vadmin_auth.
#
# You can change the configuration variables immediately below
# if needed.  Always remember to remove the file called "test_creds"
# when done testing!
#
# Licensed under GNU GPL v2.  Please see COPYING for full terms.
#
#

WEBSERVER=apache
#WEBSERVER=nobody
#WEBSERVER=www
#WEBSERVER=www-data


# To run the command as the PHP files will, need to change to a different
# directory first.  If Vadmin home is changed, this must be changed also.
#
RUNFROM=../../../../plugins/vadmin
SCRIPTDIR=../../plugins/vadmin/includes/suid




echo


# need a valid username/password and other vars for testing
#
if [ ! -s test_creds ]; then 
   read -p "algo (see MCRYPT_ALGO in conf/apache.conf_example): " ALGO
   read -p "hashline (see CRYPTO_HASH_LINE in conf/apache.conf_example): " HASHLINE
   read -p "domain: " DOMAIN
   read -p "domain password: " DOM_PWD
   read -p "IMAP username: " USER
   read -p "IMAP password: " PASS
   echo $ALGO > test_creds
   echo $HASHLINE >> test_creds
   echo $DOMAIN >> test_creds
   echo $DOM_PWD >> test_creds
   echo $USER >> test_creds
   echo $PASS >> test_creds
   echo
fi


cd $RUNFROM


# if root, allow executing as web server for best test
#
SUDO=x
if test $UID = 0; then 
   while [ $SUDO != 'y' ] && [ $SUDO != 'Y' ] && 
         [ $SUDO != 'n' ] && [ $SUDO != 'N' ] ; do
      read -p "Run (sudo) as web server user (doing so is the most accurate test)? (Y/n): " SUDO
      if test -z $SUDO; then 
         SUDO=y
      fi
   done;
   echo
fi;


# now run the test
#
if [ $SUDO = 'y' ] || [ $SUDO = 'Y' ] ; then
   echo Executing as \"$WEBSERVER\" user.  If your web server runs as a different
   echo user, please change the WEBSERVER variable at the top of this script.
   echo
   echo ---------------------------------------------------------------------
   echo
   cat $SCRIPTDIR/test_creds | sudo -u $WEBSERVER $SCRIPTDIR/vadmin_auth -fakearg fakearg2 -fakearg3 fakearg4
   echo
   echo ---------------------------------------------------------------------
   echo
else
   echo ---------------------------------------------------------------------
   echo
   cat $SCRIPTDIR/test_creds | $SCRIPTDIR/vadmin_auth fakearg fakearg2; 
   echo
   echo ---------------------------------------------------------------------
   echo
fi


# check return code
#
xx=$?
if [ $xx -eq '0' ] ; then
   echo "SUCCESS (return code 0), but that doesn't always mean that the"
   echo "test itself succeeded; check for errors immediately above"
else
   echo "POSSIBLE ERROR.  RESULT WAS:" $xx
fi;


echo


# remove the test_creds file
#
echo If you do not need to perform additional tests, remove the file 'test_creds'
REMOVE=x
while [ $REMOVE != 'y' ] && [ $REMOVE != 'Y' ] && 
      [ $REMOVE != 'n' ] && [ $REMOVE != 'N' ] ; do
   read -p "Remove now? (Y/n): " REMOVE
   if test -z $REMOVE; then 
      REMOVE=y
   fi
done;

if [ $REMOVE = 'y' ] || [ $REMOVE = 'Y' ] ; then
   rm $SCRIPTDIR/test_creds
fi;

echo
echo
