#!/bin/sh

cd $(dirname $0)

echo -n 'FACT: ' && (../atto < fact.in | diff - fact.expect && echo PASS) || echo FAIL;
