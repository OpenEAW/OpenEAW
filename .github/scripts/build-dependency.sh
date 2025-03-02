#!/bin/bash
if [ "$1" = "" ] || [ "$2" = "" ]; then
  echo "Usage: build-dependency.sh PACKAGE_NAME PACKAGE_GIT_URL"
  echo
  echo "  This will search conan.lock in the working directory for the PACKAGE_NAME dependency,"
  echo "  extract the version string from it (assumed to be of form 'x.y.z+<commit hash>'),"
  echo "  download it from PACKAGE_GIT_URL and build it into the Conan cache."
  exit 1
fi
PACKAGE_NAME=$1
PACKAGE_GIT_URL=$2
PACKAGE_VERSION=$(grep -o "$PACKAGE_NAME/[0-9a-f\.+]\+" conan.lock | head -n 1 | sed "s/$PACKAGE_NAME\///")
if [ "$PACKAGE_VERSION" = "" ]; then echo "error: unexpected version for '$PACKAGE_NAME'"; exit 1; fi
PACKAGE_COMMIT=$(echo "$PACKAGE_VERSION" | cut -s -d '+' -f 2)
if [ "$PACKAGE_COMMIT" = "" ]; then echo "error: missing commit hash in dependency '$PACKAGE_NAME/$PACKAGE_VERSION'"; exit 1; fi
PACKAGE_DIR=/tmp/$PACKAGE_NAME-$PACKAGE_VERSION
echo "Checking out $PACKAGE_NAME/$PACKAGE_VERSION"
git clone --single-branch $PACKAGE_GIT_URL $PACKAGE_DIR
git -C $PACKAGE_DIR checkout $PACKAGE_COMMIT
echo "Building $PACKAGE_NAME/$PACKAGE_VERSION"
conan create $PACKAGE_DIR -s compiler.cppstd=17 -s build_type=Release --build=missing
