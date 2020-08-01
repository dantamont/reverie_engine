PREFIX=engine
REMOTE=grand_blue_remote
BRANCH=develop

FLAGS="--prefix=$PREFIX --squash $REMOTE $BRANCH"

git remote remove $REMOTE
git remote add --no-tags $REMOTE https://daintybubainty@bitbucket.org/daintybubainty/grand-blue-engine.git

# Will fail if already added
git subtree add $FLAGS

# May fail with message "Working tree has modifications". If so, just re-checkout the branch
git subtree pull $FLAGS