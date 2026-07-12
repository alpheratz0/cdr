#!/bin/sh

rg -v '^[A-Z][a-z]*\t[A-Z][a-z]*\t[0-9]*\t[0-9]*\t[0-9]*\t' -o ./data/cdr.tsv
