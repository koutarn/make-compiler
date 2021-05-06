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
assert 21 "5+20-4"
assert 41 "12 + 34 - 5"
assert 13 "2 * 4 + 5 * 1"
assert 10 "-10+20"
assert 10 "+10"

echo OK
