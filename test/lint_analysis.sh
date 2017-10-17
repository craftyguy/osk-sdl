#!/bin/bash

exe=nsiqcppstyle
which $exe 2>/dev/null 1>&2 || exe=$HOME/nsiqcppstyle/nsiqcppstyle
conf=test/filefilter.txt

$exe -f $conf . || exit 1
