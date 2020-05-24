#!/bin/bash
node-gyp --debug build && valgrind --leak-check=full -s --suppressions=libmtp.supp --suppressions=node.suppressions --show-leak-kinds=all --gen-suppressions=all -s node --expose-gc test.js 2>&1|less
