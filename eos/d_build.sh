#!/bin/bash
make lin_docs 1> out.txt 2>&1

gedit out.txt &
