PREFIX=Source/Common/CppPackets.subtree
REMOTE=origin-subtree-name
BRANCH=develop

FLAGS="--prefix=$PREFIX --squash $REMOTE $BRANCH"

git remote remove $REMOTE
git remote add --no-tags $REMOTE ssh://git@thething

# Will fail if already added
git subtree add $FLAGS

# May fail with message "Working tree has modifications". If so, just re-checkout the branch
git subtree pull $FLAGS