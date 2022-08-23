#!/bin/bash

# Update the submodule, all that is needed since tracking branch.
# See: https://stackoverflow.com/questions/1979167/git-submodule-update
# $ git submodule update --recursive --remote
# git submodule update --remote 
git submodule update --init --recursive --remote

# cp -rf ./eigen.submodule/Eigen ./include