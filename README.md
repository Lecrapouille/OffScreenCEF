# Chromium Embedded Framework's cefsimple Off-Screen Rendering

Updating these two GitHub repos:
- SDL: https://github.com/gotnospirit/cef3-sdl2
- OpenGL Core: https://github.com/if1live/cef-gl-example

Not yet safe to be used !

Download https://cef-builds.spotifycdn.com/index.html for Linux:

CEF=xxxx
cp cefsimple_opengl in $CEF/tests
cd $CEF/build
cmake -DCMAKE_BUILD_TYPE=Debug ..
copy all stuffs needed in $CEF/tests/cefsimple_opengl

Build:
g++ --std=c++14 -W -Wall -Wextra -Wno-unused-parameter -DCEF_USE_SANDBOX -DNDEBUG -D_FILE_OFFSET_BITS=64 -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -I$CEF -I$CEF/include *.cpp -o cefopengl ./libcef.so ./libcef_dll_wrapper.a `pkg-config --cflags --libs glew --static glfw3`

