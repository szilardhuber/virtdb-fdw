#!/bin/sh
git pull --recurse-submodules
git checkout master
git remote add upstream https://github.com/starschema/virtdb-fdw.git
git fetch origin -v
git fetch upstream -v
git merge upstream/master
git status
