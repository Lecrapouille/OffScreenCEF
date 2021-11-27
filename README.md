# Chromium Embedded Framework's cefsimple Off-Screen Rendering

I needed to use a modifed version of [cefsimple](https://bitbucket.org/chromiumembedded/cef/wiki/Tutorial) using either SDL or OpenGL Core. I tried these three GitHub repos:
- SDL: https://github.com/gotnospirit/cef3-sdl2
- OpenGL Core: https://github.com/if1live/cef-gl-example
- OpenGL Core: https://github.com/andmcgregor/cefgui

But they are outdated (more than > 4 years), the CEF API changed and when I compiled they crash by forking indefinitively the application until my Linux fell down. So they are not safe to be used! I tried to update fixes, so here is the repo.

It's working on my Linux 64-bits Debian 11 and CEF 96.0.14 downloaded at https://cef-builds.spotifycdn.com/index.html

Before compiling cefsimple, you need some libs and packages from CEF inside the two folders `cefsimple_sdl` and `cefsimple_opengl`.

## Setup

Firstly, let name some folders. This will shorter code in this document. Adapt the CEF version to your operating system with the desired version according to https://cef-builds.spotifycdn.com/index.html

```bash
TMP=/tmp
CEF_LINK=https://cef-builds.spotifycdn.com/cef_binary_96.0.14%2Bg28ba5c8%2Bchromium-96.0.4664.55_linux64.tar.bz2
CEF=$TMP/cef_binary_96.0.14+g28ba5c8+chromium-96.0.4664.55_linux64
CEF_TARBALL=$CEF.tar.bz2
CEFSIMPLE_SDL=$CEF/tests/cefsimple_sdl
CEFSIMPLE_GL=$CEF/tests/cefsimple_opengl
```

Download and decompress it inside a temporary folder:

```bash
wget $CEF_LINK
tar jxvf $CEF_TARBALL
```

Git clone this repo inside the cef folder:

```bash
cd $CEF
git clone https://github.com/Lecrapouille/OffScreenCEF.git --depth=1
```

Compile CEF (needs some minutes, not hours):

```bash
mkdir $CEF/build
cd $CEF/build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j$(nproc)
```

You can use `-DCMAKE_BUILD_TYPE=Release` instead but just avoid mixing debug and release symbols. Now we have to copy needed elements for compiling `cefsimple_sdl` and `cefsimple_opengl`:

```bash
cp -v $CEF/Debug/libcef.so $CEF/build/libcef_dll_wrapper/libcef_dll_wrapper.a $CEFSIMPLE_SDL

(cd $CEF/build/tests/cefsimple/Debug/
 cp -v icudtl.dat resources.pak chrome_100_percent.pak chrome_200_percent.pak v8_context_snapshot.bin $CEFSIMPLE_SDL
)

mkdir -p $CEFSIMPLE_SDL/locales
cp -v $CEF/build/tests/cefsimple/Debug/locales/en-US.pak $CEFSIMPLE_SDL/locales
```

Do the same thing for `$CEFSIMPLE_GL`.

## Compilation

```bash
cd $CEFSIMPLE_SDL && ./build.sh
cd $CEFSIMPLE_GL && ./build.sh
```

You have two binaries:
- `$CEFSIMPLE_SDL/cefsimple_sdl`
- `$CEFSIMPLE_GL/cefsimple_opengl`

These applications do not have command lines, you have to edit the code to give your desired page.

## Important note

`CefExecuteProcess` shall be called before OpenGL/SDL2 context and using the command line
of your application (`int argc, char* argv[]`). If not your system can freeze. Here is the
function documentation (`caf_app.h` file):

```
   This function should be called from the application entry point function to
   execute a secondary process. It can be used to run secondary processes from
   the browser client executable (default behavior) or from a separate
   executable specified by the CefSettings.browser_subprocess_path value. If
   called for the browser process (identified by no "type" command-line value)
   it will return immediately with a value of -1. If called for a recognized
   secondary process it will block until the process should exit and then return
   the process exit code. The |application| parameter may be empty. The
   |windows_sandbox_info| parameter is only used on Windows and may be NULL (see
   cef_sandbox_win.h for details).
```

## Demo

https://youtu.be/8xhxiDI4D5o
