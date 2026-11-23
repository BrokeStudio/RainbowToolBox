#!/bin/bash

pushd ..
Vendor/Binaries/Premake/macOS/premake5 --cc=clang --file=Build.lua gmake
popd
