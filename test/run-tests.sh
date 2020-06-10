#!/bin/sh

cd $(dirname $0)

tests=$(find . -name '*.in' -not -name '.*' \
            | sed -e 's|\./\(.*\).in$|\1|' \
            | sort)

for t in $tests; do
    echo -n "${t}: " \
        && (../atto < "${t}.in" \
                | diff "${t}.expect" - \
                &&  echo PASS) \
        || echo FAIL;
done
