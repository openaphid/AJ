#!/bin/sh

export SRCROOT=$PWD
export WebCore=$PWD
export CREATE_HASH_TABLE="$SRCROOT/create_hash_table"
export CREATE_REGEXP_TABLES="$SRCROOT/create_regex_tables"

mkdir -p DerivedSources/AJCore
cd DerivedSources/AJCore

make -f ../../DerivedSources.make AJCore=../.. BUILT_PRODUCTS_DIR=../..
cd ../..
