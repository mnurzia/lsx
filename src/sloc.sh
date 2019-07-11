#!/bin/sh
ls -1 | grep -E '*\.c|*\.h' | xargs wc 
