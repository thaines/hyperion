#!/bin/bash
make lin_debug 1> out.txt 2>&1
make lin_release 1>> out.txt 2>&1

gedit out.txt &
