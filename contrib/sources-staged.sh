#!/bin/sh

git diff --cached --name-only --diff-filter=ACM | grep -E '\.c$|\.cpp$|\.h$' | grep -v '^third_party/'
