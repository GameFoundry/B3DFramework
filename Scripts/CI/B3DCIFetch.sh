#!/bin/bash
set -e

echo "=== Fetching Banshee 3D ==="
echo "Repository: $GIT_URL"
echo "Branch: $GIT_BRANCH"
echo "Commit: $GIT_COMMIT"
echo "Workspace: $WORKSPACE"
echo "Clean Build: ${CLEAN_BUILD:-0}"

# Initialize git repo if needed
if [ ! -d ".git" ]; then
    echo "Initializing git repository..."
    git init
    git remote add origin "$GIT_URL"
else
    echo "Using existing git repository (incremental fetch)"
fi

echo "Fetching from remote..."
git fetch origin "$GIT_BRANCH" --depth=1

# Only reset if there are local changes (avoids touching timestamps unnecessarily)
# Ignore submodule changes since we always checkout latest master, not parent-recorded commit
if [ -n "$(git status --porcelain --ignore-submodules)" ]; then
    echo "Resetting local changes..."
    git reset --hard HEAD
    git clean -fd -e Build/ -e build/
else
    echo "Working directory clean, skipping reset"
fi

# Checkout specific commit or branch
TARGET_COMMIT=""
if [ -n "$GIT_COMMIT" ]; then
    TARGET_COMMIT="$GIT_COMMIT"
else
    TARGET_COMMIT=$(git rev-parse "origin/$GIT_BRANCH")
fi

CURRENT_COMMIT=$(git rev-parse HEAD 2>/dev/null || echo "none")
if [ "$CURRENT_COMMIT" != "$TARGET_COMMIT" ]; then
    echo "Checking out $TARGET_COMMIT..."
    git checkout -f "$TARGET_COMMIT"
else
    echo "Already at target commit $TARGET_COMMIT"
fi

# Initialize submodules only if not already present
# Note: Use -e not -d because .git in submodules is a file, not a directory
if [ ! -e "Framework/.git" ]; then
    echo "Initializing submodules..."
    git submodule update --init --recursive
else
    echo "Submodules already initialized"
fi

# Update each submodule to latest master (only if needed)
echo "Checking submodules..."

update_submodule() {
    local path="$1"
    cd "$path"

    git fetch origin master
    LOCAL=$(git rev-parse HEAD)
    REMOTE=$(git rev-parse origin/master)

    if [ "$LOCAL" != "$REMOTE" ]; then
        echo "Updating $path to latest master..."
        git checkout master
        git pull origin master
    else
        echo "$path already at latest master"
    fi

    cd "$WORKSPACE"
}

update_submodule "Framework"
update_submodule "Framework/Examples"
update_submodule "Framework/Tools/BansheeCodeGenerator"
update_submodule "Framework/Tools/BansheeForge"

echo "=== Fetch complete ==="
git log -1 --oneline
