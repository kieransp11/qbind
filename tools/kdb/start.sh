#!/usr/bin/env bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
QINIT=$SCRIPT_DIR/src/q.q rlwrap -r q -c 40 200 $@
