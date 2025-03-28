#!/bin/sh

git ls-files --cached --others --exclude-standard | grep -E '\.c$|\.cpp$|\.h$' | grep -v '^third_party/'
