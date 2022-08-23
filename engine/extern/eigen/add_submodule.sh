#!/bin/bash

# NOTE:
# To clone a main project with submodules: git clone --recurse-submodules https://github.com/chaconinc/MainProject
# Or, clone the repo and call git submodule update --init --recursive
# If the remote were to change: 
# https://stackoverflow.com/questions/913701/how-to-change-the-remote-repository-for-a-git-submodule

REMOTE_URL=https://gitlab.com/libeigen/eigen.git
DESTINATION_FOLDER=eigen.submodule
BRANCH_NAME=3.4

# Add submodule to track branch_name branch
# Will fail if already added
git submodule add -b $BRANCH_NAME $REMOTE_URL $DESTINATION_FOLDER
