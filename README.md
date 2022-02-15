# Chromium Embedded Framework's cefsimple Off-Screen Rendering using SDL2 or Core OpenGL

CEF = Chromium Embedded Framework

I needed to use a modifed version of the C++ CEF example named [cefsimple](https://bitbucket.org/chromiumembedded/cef/wiki/Tutorial) instead of X11 I needed using either SDL2 or OpenGL Core (glew, glfw3). I tried these three GitHub repos without success:
- SDL2: https://github.com/gotnospirit/cef3-sdl2
- SDL2: https://github.com/jamethy/sdl2-cef
- OpenGL Core: https://github.com/if1live/cef-gl-example
- OpenGL Core: https://github.com/andmcgregor/cefgui

They are outdated (more than > 4 years), the CEF API changed and when I compiled they crash by forking indefinitively the application until my Linux fell down. So they are not safe to be used! I tried to update fixes, so here is the repo. They are inside folders:
- cefsimple_sdl
- cefsimple_opengl
- cefsimple_separate

[![OpenGL version](doc/screenshot.png)](https://youtu.be/8xhxiDI4D5o)

*Fig 1 - The OpenGL version. Click on the image to see the youtube video of the demo.*

**Note 1:** the keyboard/mouse event conversion from SDL2/glfw3 to CEF has to be fixed.

**Note 2:** I find the system not very reactive compared to the official cefsimple example.

I'm also currently trying separated executables based on https://github.com/oivio/BLUI calling https://github.com/ashea-code/BluBrowser. They are inside folder cefsimple_separate. Similar project: https://github.com/cztomczak/cefpython (C++ code).

## Help wanted

Any pull requests and help are welcome to improve these examples :)
You can complete the API by adding more methods such as previous/next page, reload, zoom ... See https://github.com/oivio/BLUI

## Tested on:

It's working on my Linux 64-bits Debian 11 and CEF 96.0.14 downloaded at https://cef-builds.spotifycdn.com/index.html
I know the Windows version can work quite easily but I did not have batch files.
The MacOS X is more complex and is not working yet.

## Some Differences

- `cefsimple_sdl` is a single main file while cefsimple_opengl has been splited into several files.
- `cefsimple_sdl` is using more handler such as `CefLifeSpanHandler`. cefsimple_opengl not.
- `cefsimple_opengl` is using several viewports. cefsimple_sdl not.
- `CefExecuteProcess` shall be called before OpenGL/SDL2 context and using the command line
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
- In `cefsimple_sdl` and `cefsimple_opengl`, both programs can access to the `main(int argc, char* argv[])` function and therefore access to the command line. CEF; when forking; modifies the command lines to give behaviors to forked child. Sometimes, accessing to the main function is not possible (or when your application is also using the command line, while possible to save and restore it before and after calling CEF). Therefore you shall launch a second process which can access its main function and therefore call chromium. As consequence `cefsimple_separate` has been added when you cannot modify the main application to access to the `int main(int argc, char* argv[])` function.

## /!\ Installation /!\

Run the bash script for Linux:
```
./install.sh
```

Once done with success, you can go the `build` folder and run one of the following applications:
- `./secondary_process`
- `./primary_process`
- `./cefsimple_opengl`
- `./cefsimple_sdl`

I will download CEF, compile it and compile examples. Here what the script is doing:

Before compiling any example you will need some libs and packages compiled from Chromium Embedded Framework example and copy these assets inside my folders to make my cefsimple compiling and working. You will need to adapt build.sh scripts to refer to the CEF folder.

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

Do the same thing for `$CEFSIMPLE_GL`. See the screenshot for needed assets:

![Needed assets](doc/assets.png)

*Fig 2 - Needed assets from CEF.*

## Compilation

You need to adapt the path to CEF main folder by mofdifying the line `CEF=...` in each `build.sh` files.

```bash
cd $CEFSIMPLE_SDL && ./build.sh
cd $CEFSIMPLE_GL && ./build.sh
```

You have two binaries:
- `$CEFSIMPLE_SDL/cefsimple_sdl`
- `$CEFSIMPLE_GL/cefsimple_opengl`

These applications do not have command lines, you have to edit the code to give your desired page.
