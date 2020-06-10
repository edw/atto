#!/bin/sh

cd $(dirname $0)

for t in $(find . -name '*.in' -not -name '.*' \
               | sed -e 's|\./\(.*\).in$|\1|' \
               | sort)

do
    echo -n "${t}: " \
        && (../atto < "${t}.in" \
                | diff - "${t}.expect" \
                &&  echo PASS) \
        || echo FAIL;
done
