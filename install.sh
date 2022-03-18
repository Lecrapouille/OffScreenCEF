#!/bin/bash -e
### Download CEF, compile it, install locally assets and compile examples.

### Green color message
function msg
{
    echo -e "\033[32m*** $*\033[00m"
}

### Red color message
function err
{
    echo -e "\033[31m*** $*\033[00m"
}

### CMake version
msg "Check if cmake version is >= 3.19"
CMAKE_CURRENT_VERSION=`cmake --version | head -n1 | cut -d" " -f3`
CMAKE_MIN_VERSION="3.19"
if [ "$CMAKE_MIN_VERSION" = "`echo -e "$CMAKE_CURRENT_VERSION\n$CMAKE_MIN_VERSION" | sort -r -V | head -n1`" ]; then
    err "Your CMake version is $CMAKE_CURRENT_VERSION but shall be >= $CMAKE_MIN_VERSION"
    exit 1
fi

### Canonical pathes
THIRDPARTY_PATH=thirdparty
mkdir -p $THIRDPARTY_PATH
cd $THIRDPARTY_PATH
THIRDPARTY_PATH=`pwd`
cd ..

CEF_PATH=$THIRDPARTY_PATH/cef_binary
BUILD_PATH="build"
mkdir -p $BUILD_PATH
cd $BUILD_PATH
BUILD_PATH=`pwd`
cd ..

### Archi
UNAMEM=`uname -m`
ARCHI=""
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    if [[ "$UNAMEM" == "x86_64" ]]; then
        ARCHI="linux64"
    else
        ARCHI="linuxarm"
    fi
elif [[ "$OSTYPE" == "freebsd"* ]]; then
    if [[ "$UNAMEM" == "x86_64" ]]; then
        ARCHI="linux64"
    else
        ARCHI="linuxarm"
    fi
elif [[ "$OSTYPE" == "darwin"* ]]; then
    if [[ "$UNAMEM" == "x86_64" ]]; then
        ARCHI="macosx64"
    else
        ARCHI="macosarm64"
    fi
elif [[ "$OSTYPE" == "msys"* ]]; then
    if [[ "$UNAMEM" == "x86_64" ]]; then
        ARCHI="windows64"
    else
        ARCHI="windowsarm64"
    fi
else
    err "Unknown archi $$OSTYPE: Cannot download Chromium Embedded Framework"
    exit 1
fi

### Number of CPU cores
NPROC=
if [[ "$OSTYPE" == "darwin"* ]]; then
    NPROC=`sysctl -n hw.logicalcpu`
else
    NPROC=`nproc`
fi

### Compile CEF in debug or release mode ... up to you !
#CEF_TARGET=Release
CEF_TARGET=Debug

###
WEBSITE=https://cef-builds.spotifycdn.com
CEF_TARBALL=cef_binary_99.2.12%2Bg2977b3a%2Bchromium-99.0.4844.74_$ARCHI.tar.bz2
if [ ! -d "$THIRDPARTY_PATH/cef_binary" ]; then
    msg "Downloading Chromium Embedded Framework v96 for archi $ARCHI to $CEF_PATH ..."
    mkdir -p $THIRDPARTY_PATH
    (cd $THIRDPARTY_PATH
     wget -c $WEBSITE/$CEF_TARBALL -O- | tar -xj
     mv cef_binary* cef_binary
    )
fi

###
if [ ! -e "$BUILD_PATH/libcef.so" ]; then
    msg "Compiling Chromium Embedded Framework in $CEF_TARGET mode (inside $CEF_PATH) ..."
    mkdir -p $CEF_PATH/build
    (cd $CEF_PATH/build
     # Compile with ninja or make
     if [ -x "$(which ninja)" ]; then
         cmake -G "Ninja" -DCMAKE_BUILD_TYPE=$CEF_TARGET ..
         VERBOSE=1 ninja -j$NPROC cefsimple
     else
         cmake -DCMAKE_BUILD_TYPE=$CEF_TARGET ..
         VERBOSE=1 make -j$NPROC cefsimple
     fi
    )

    msg "Installing Chromium Embedded Framework to $BUILD_PATH ..."
    S="$CEF_PATH/build/tests/cefsimple/$CEF_TARGET"
    cp --verbose "$S/v8_context_snapshot.bin" "$S/icudtl.dat" $BUILD_PATH
    cp --verbose -R "$S/"*.pak "$S/"*.so* "$S/locales" $BUILD_PATH
fi

###
if  [ ! -e "$BUILD_PATH/cefsimple_opengl" ]; then
    msg "Compile OpenGL example"
    (cd cefsimple_opengl
     g++ --std=c++14 -W -Wall -Wextra -Wno-unused-parameter \
         -DCHECK_OPENGL -DCEF_USE_SANDBOX -DNDEBUG \
         -D_FILE_OFFSET_BITS=64 -D__STDC_CONSTANT_MACROS \
         -D__STDC_FORMAT_MACROS -I$CEF_PATH -I$CEF_PATH/include *.cpp \
         -o $BUILD_PATH/cefsimple_opengl $BUILD_PATH/libcef.so \
         $CEF_PATH/build/libcef_dll_wrapper/libcef_dll_wrapper.a \
         `pkg-config --cflags --libs glew --static glfw3`
     cp --verbose -R shaders $BUILD_PATH
    )
fi

###
if  [ ! -e "$BUILD_PATH/cefsimple_sdl" ]; then
    msg "Compile SDL2 example"
    (cd cefsimple_sdl
     g++ --std=c++14 -W -Wall -Wextra -Wno-unused-parameter \
         -DCEF_USE_SANDBOX -DNDEBUG -D_FILE_OFFSET_BITS=64 \
         -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS \
         -I$CEF_PATH -I$CEF_PATH/include sdl_cef_events.cpp main.cpp \
         -o $BUILD_PATH/cefsimple_sdl $BUILD_PATH/libcef.so \
         $CEF_PATH/build/libcef_dll_wrapper/libcef_dll_wrapper.a \
         `pkg-config --cflags --libs sdl2 SDL2_image`
    )
fi

###
if  [ ! -e "$BUILD_PATH/primary_process" ]; then
    msg "Compile Primary process example"
    (cd cefsimple_separate/primary/
     g++ --std=c++14 -W -Wall -Wextra -Wno-unused-parameter \
         -DCEF_USE_SANDBOX -DNDEBUG -D_FILE_OFFSET_BITS=64 \
         -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS \
         -DSECONDARY_PATH=\"$BUILD_PATH/secondary_process\" \
         -I$CEF_PATH -I$CEF_PATH/include main.cpp \
         -o $BUILD_PATH/primary_process $BUILD_PATH/libcef.so \
         $CEF_PATH/build/libcef_dll_wrapper/libcef_dll_wrapper.a \
         `pkg-config --cflags --libs sdl2 SDL2_image`
    )
fi

###
if  [ ! -e "$BUILD_PATH/secondary_process" ]; then
    msg "Compile Secondary process example"
    (cd cefsimple_separate/secondary
     g++ --std=c++14 -W -Wall -Wextra -Wno-unused-parameter -DCEF_USE_SANDBOX \
         -DNDEBUG -D_FILE_OFFSET_BITS=64 -D__STDC_CONSTANT_MACROS \
         -D__STDC_FORMAT_MACROS -I$CEF_PATH -I$CEF_PATH/include \
         *.cpp -o $BUILD_PATH/secondary_process $BUILD_PATH/libcef.so \
         $CEF_PATH/build/libcef_dll_wrapper/libcef_dll_wrapper.a
    )
fi

msg "Compilation done with success! You can go to $BUILD_PATH and run one of the following applications:"
msg "  ./secondary_process"
msg "  ./primary_process"
msg "  ./cefsimple_opengl"
msg "  ./cefsimple_sdl"
