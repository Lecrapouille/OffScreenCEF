#!/bin/bash

CEF=/media/data/cef_binary/

g++ --std=c++14 -W -Wall -Wextra -Wno-unused-parameter -DCHECK_OPENGL -DCEF_USE_SANDBOX -DNDEBUG -D_FILE_OFFSET_BITS=64 -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -I$CEF -I$CEF/include *.cpp -o cefsimple_opengl ./libcef.so ./libcef_dll_wrapper.a `pkg-config --cflags --libs glew --static glfw3`
