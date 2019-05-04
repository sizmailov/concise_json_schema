#!/bin/bash

if [ -z ${CI+x} ];
then
  CI=false
fi;

if $CI
then
  gcovr build --root=./ --xml -o coverage.xml
  bash <(curl -s https://codecov.io/bash)
else
  gcovr cmake-build-coverage --root=./ --xml -o coverage.xml
  bash <(curl -s https://codecov.io/bash) -t @.cc_token
fi
