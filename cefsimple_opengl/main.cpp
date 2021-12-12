#include "CEFGLWindow.hpp"

//------------------------------------------------------------------------------
static void CEFsetUp(int argc, char** argv)
{
    // This function should be called from the application entry point function to
    // execute a secondary process. It can be used to run secondary processes from
    // the browser client executable (default behavior) or from a separate
    // executable specified by the CefSettings.browser_subprocess_path value. If
    // called for the browser process (identified by no "type" command-line value)
    // it will return immediately with a value of -1. If called for a recognized
    // secondary process it will block until the process should exit and then return
    // the process exit code. The |application| parameter may be empty. The
    // |windows_sandbox_info| parameter is only used on Windows and may be NULL (see
    // cef_sandbox_win.h for details).
    CefMainArgs args(argc, argv);
    int exit_code = CefExecuteProcess(args, nullptr, nullptr);
    if (exit_code >= 0)
    {
        // Sub proccess has endend, so exit
        exit(exit_code);
    }
    else if (exit_code == -1)
    {
        // If called for the browser process (identified by no "type" command-line value)
        // it will return immediately with a value of -1
    }

    // Configurate Chromium
    CefSettings settings;
    //CefString(&settings.locales_dir_path) = "/home/qq/MyGitHub/OffScreenCEF/godot/locales";
    //CefString(&settings.resources_dir_path) = "/home/qq/MyGitHub/OffScreenCEF/godot/";
    //CefString(&settings.framework_dir_path) = "/home/qq/MyGitHub/OffScreenCEF/godot/";
    //CefString(&settings.cache_path) = "/home/qq/MyGitHub/OffScreenCEF/godot/";
    settings.windowless_rendering_enabled = true;
#if !defined(CEF_USE_SANDBOX)
    settings.no_sandbox = true;
#endif

    bool result = CefInitialize(args, settings, nullptr, nullptr);
    if (!result)
    {
        std::cerr << "CefInitialize: failed" << std::endl;
        exit(-2);
    }
}

//------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    CEFsetUp(argc, argv);

    CEFGLWindow win(800, 600, "CEF OpenGL");
    int res = win.start();

    return res;
}
