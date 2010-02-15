#!/bin/bash
make lin_relbug 1> out.txt 2>&1

gedit out.txt &
