#!/bin/bash
# set -x
# set -euo pipefail
# IFS=$'\n\t'

assert() {
  expected="${1}"
  input="${2}"

  ./9cc "${input}" > tmp.s
  cc -o tmp tmp.s
  ./tmp
  actual="${?}"

  if [ "${actual}" = "${expected}" ]; then
    echo "${input} => ${actual}"
  else
    echo "${input} => ${expected} expected, but got ${actual}"
    exit 1
  fi
}

assert 0 0
assert 22 22

echo OK
