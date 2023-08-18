#!/bin/bash
BUILD_DIR="build-android"
if [ ! -d "$BUILD_DIR" ]; then
    mkdir "$BUILD_DIR"
fi
cd "$BUILD_DIR"
cmake -DBUILD_ANDROID=ON -DANDROID_ABI=armeabi-v7a -GNinja ..
if [ -d "lib" ]; then
    rm -rf lib
fi
ninja
cd ..


BUILD_DIR="build-android64"
if [ ! -d "$BUILD_DIR" ]; then
    mkdir "$BUILD_DIR"
fi
cd "$BUILD_DIR"
cmake -DBUILD_ANDROID=ON -DANDROID_ABI=arm64-v8a -GNinja ..
if [ -d "lib" ]; then
    rm -rf lib
fi
ninja
cd ..

cp "build-android64/bin/org.renderdoc.renderdoccmd.arm64.apk" "build-android/bin/org.renderdoc.renderdoccmd.arm64.apk"