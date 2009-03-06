#!/bin/sh

# Structure of the test cvs repository
#
# Message   File:Content         Commit Time
# Rev 1     a: 1.1               2009-02-21 19:11:43 +0100
# Rev 2     a: 1.2    b: 1.1     2009-02-21 19:11:14 +0100
# Rev 3               b: 1.2     2009-02-21 19:11:43 +0100
#
# As you can see the commit of Rev 3 has the same time as
# Rev 1 this leads to a broken import because of a cvsps
# bug.

test_description='git cvsimport testing for correct patchset estimation'
. ./lib-cvs.sh

CVSROOT="$TEST_DIRECTORY"/t9603/cvsroot
export CVSROOT

test_expect_failure 'import with criss cross times on revisions' '

    git cvsimport -p"-x" -C module-git module &&
    cd module-git &&
        git log --pretty=format:%s > ../actual &&
        echo "" >> ../actual &&
    cd .. &&
    echo "Rev 3
Rev 2
Rev 1" > expect &&
    test_cmp actual expect
'

test_done
