#!/bin/bash

# Update the submodule, all that is needed since tracking branch.
# See: https://stackoverflow.com/questions/1979167/git-submodule-update
# $ git submodule update --recursive --remote

# On switching branches: https://stackoverflow.com/questions/29882960/changing-an-existing-submodules-branch/29882995
git submodule update --remote 