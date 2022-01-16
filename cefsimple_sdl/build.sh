#!/bin/bash

CEF=/media/data/cef_binary/

g++ --std=c++14 -W -Wall -Wextra -Wno-unused-parameter -DCEF_USE_SANDBOX -DNDEBUG -D_FILE_OFFSET_BITS=64 -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -I$CEF -I$CEF/include sdl_cef_events.cpp main.cpp -o cefsimple_sdl ./libcef.so ./libcef_dll_wrapper.a `pkg-config --cflags --libs sdl2 SDL2_image`
