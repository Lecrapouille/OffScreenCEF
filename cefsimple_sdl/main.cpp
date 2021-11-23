#include <stdio.h>
#include <iostream>
#include <sstream>

#include <cef_app.h>
#include <cef_client.h>
#include <cef_render_handler.h>
#include <cef_life_span_handler.h>
#include <cef_load_handler.h>
#include <wrapper/cef_helpers.h>

#include <SDL2/SDL.h>
#include <SDL_image.h>
#include "sdl_keyboard_utils.h"

class RenderHandler: public CefRenderHandler
{
public:

    RenderHandler(SDL_Renderer* renderer, int w, int h)
        : m_width(w),
          m_height(h),
          m_renderer(renderer)
    {
        m_texture = SDL_CreateTexture(m_renderer, SDL_PIXELFORMAT_UNKNOWN,
                                      SDL_TEXTUREACCESS_STREAMING, w, h);
        assert(m_texture != nullptr);
    }

    ~RenderHandler()
    {
        if (m_texture != nullptr)
        {
            SDL_DestroyTexture(m_texture);
        }
        m_renderer = nullptr;
    }

    virtual void GetViewRect(CefRefPtr<CefBrowser> browser, CefRect &rect) override
    {
        rect = CefRect(0, 0, m_width, m_height);
    }

    virtual void OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type,
                         const RectList &dirtyRects, const void * buffer,
                         int w, int h) override
    {
        if (m_texture != nullptr)
        {
            unsigned char * texture_data = NULL;
            int texture_pitch = 0;

            SDL_LockTexture(m_texture, 0, (void **)&texture_data, &texture_pitch);
            memcpy(texture_data, buffer, w * h * 4);
            SDL_UnlockTexture(m_texture);
        }
    }

    void resize(int w, int h)
    {
        if (m_texture != nullptr)
        {
            SDL_DestroyTexture(m_texture);
        }

        m_texture = SDL_CreateTexture(m_renderer, SDL_PIXELFORMAT_UNKNOWN,
                                      SDL_TEXTUREACCESS_STREAMING, w, h);
        m_width = w;
        m_height = h;
    }

    void render()
    {
        SDL_RenderCopy(m_renderer, m_texture, NULL, NULL);
    }

private:

    int m_width;
    int m_height;
    SDL_Renderer * m_renderer = nullptr;
    SDL_Texture * m_texture = nullptr;

    IMPLEMENT_REFCOUNTING(RenderHandler);
};

// for manual render handler
class BrowserClient: public CefClient,
                     public CefLifeSpanHandler,
                     public CefLoadHandler
{
public:

    BrowserClient(CefRefPtr<CefRenderHandler> ptr)
        : m_handler(ptr)
    {
    }

    virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override
    {
        return this;
    }

    virtual CefRefPtr<CefLoadHandler> GetLoadHandler() override
    {
        return this;
    }

    virtual CefRefPtr<CefRenderHandler> GetRenderHandler() override
    {
        return m_handler;
    }

    // CefLifeSpanHandler methods.
    virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) override
    {
        // Must be executed on the UI thread.
        CEF_REQUIRE_UI_THREAD();

        m_browser_id = browser->GetIdentifier();
    }

    virtual bool DoClose(CefRefPtr<CefBrowser> browser) override
    {
        // Must be executed on the UI thread.
        CEF_REQUIRE_UI_THREAD();

        // Closing the main window requires special handling. See the DoClose()
        // documentation in the CEF header for a detailed description of this
        // process.
        if (browser->GetIdentifier() == m_browser_id)
        {
            // Set a flag to indicate that the window close should be allowed.
            m_closing = true;
        }

        // Allow the close. For windowed browsers this will result in the OS close
        // event being sent.
        return false;
    }

    virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) override
    {
    }

    virtual void OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                           int httpStatusCode) override
    {
        std::cout << "OnLoadEnd(" << httpStatusCode << ")" << std::endl;
        m_loaded = true;
    }

    virtual void OnLoadingStateChange(CefRefPtr<CefBrowser> browser, bool isLoading,
                                      bool canGoBack, bool canGoForward) override
    {
        std::cout << "OnLoadingStateChange()" << std::endl;
    }

    // FIXME virtual override ?
    bool OnLoadError(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                     CefLoadHandler::ErrorCode errorCode,
                     const CefString& failedUrl,
                     CefString & errorText)
    {
        std::cout << "OnLoadError()" << std::endl;
        m_loaded = true; //FIXME
        return true; //FIXME
    }

    void OnLoadStart(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame)
    {
        std::cout << "OnLoadStart()" << std::endl;
    }

    bool closeAllowed() const
    {
        return m_closing;
    }

    bool isLoaded() const
    {
        return m_loaded;
    }

private:

    int m_browser_id;
    bool m_closing = false;
    bool m_loaded = false;
    CefRefPtr<CefRenderHandler> m_handler;

    IMPLEMENT_REFCOUNTING(BrowserClient);
};

CefBrowserHost::MouseButtonType translateMouseButton(SDL_MouseButtonEvent const &e)
{
    CefBrowserHost::MouseButtonType result;
    switch (e.button)
    {
    case SDL_BUTTON_LEFT:
    case SDL_BUTTON_X1:
        result = MBT_LEFT;
        break;

    case SDL_BUTTON_MIDDLE:
        result = MBT_MIDDLE;
        break;

    case SDL_BUTTON_RIGHT:
    case SDL_BUTTON_X2:
        result = MBT_RIGHT;
        break;
    }
    return result;
}

int main(int argc, char * argv[])
{
    CefMainArgs args(argc, argv);

    int result = CefExecuteProcess(args, nullptr, nullptr);
    // checkout CefApp, derive it and set it as second parameter, for more control on
    // command args and resources.
    if (result >= 0) // child proccess has endend, so exit.
    {
        return result;
    }
    else if (result == -1)
    {
        // we are here in the father proccess.
    }

    CefSettings settings;
    settings.windowless_rendering_enabled = true;

    // When generating projects with CMake the CEF_USE_SANDBOX value will be defined
    // automatically. Pass -DUSE_SANDBOX=OFF to the CMake command-line to disable
    // use of the sandbox.
#if !defined(CEF_USE_SANDBOX)
    settings.no_sandbox = true;
#endif

    // CefInitialize creates a sub-proccess and executes the same executeable,
    // as calling CefInitialize, if not set different in
    // settings.browser_subprocess_path if you create an extra program just for
    // the childproccess you only have to call CefExecuteProcess(...) in it.
    if (!CefInitialize(args, settings, nullptr, nullptr))
    {
        // handle error
        return -1;
    }

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    int width = 800;
    int height = 600;

    auto window = SDL_CreateWindow("Render CEF with SDL", SDL_WINDOWPOS_UNDEFINED,
                                   SDL_WINDOWPOS_UNDEFINED, width, height,
                                   SDL_WINDOW_RESIZABLE);
    if (window == nullptr)
    {
        std::cerr << "SDL could not create window! SDL_Error: "
                  << SDL_GetError() << std::endl;
    }
    else
    {
        auto m_renderer = SDL_CreateRenderer(window, -1,
                                             SDL_RENDERER_ACCELERATED |
                                             SDL_RENDERER_PRESENTVSYNC);
        if (m_renderer == nullptr)
        {
            std::cerr << "SDL could not create renderer! SDL_Error: "
                      << SDL_GetError() << std::endl;
        }
        else
        {
            SDL_Event e;
            CefRefPtr<RenderHandler> renderHandler =
                    new RenderHandler(m_renderer, width, height);

            // create browser-window
            CefRefPtr<CefBrowser> browser;
            CefRefPtr<BrowserClient> browserClient;

            CefWindowInfo window_info;
            CefBrowserSettings browserSettings;

            browserSettings.windowless_frame_rate = 60; // 30 is default
            window_info.SetAsWindowless(0);

            browserClient = new BrowserClient(renderHandler);
            browser = CefBrowserHost::CreateBrowserSync(window_info,
                                                        browserClient.get(),
                                                        "http://www.google.com",
                                                        browserSettings,
                                                        nullptr, nullptr);

            // inject user-input by calling - non-trivial for non-windows -
            // checkout the cefclient source and the platform specific cpp, like
            // cefclient_osr_widget_gtk.cpp for linux
            // browser->GetHost()->SendKeyEvent(...);
            // browser->GetHost()->SendMouseMoveEvent(...);
            // browser->GetHost()->SendMouseClickEvent(...);
            // browser->GetHost()->SendMouseWheelEvent(...);

            bool shutdown = false;
            //bool js_executed = false;
            while (!browserClient->closeAllowed())
            {
                // send events to browser
                while (!shutdown && SDL_PollEvent(&e) != 0)
                {
                    switch (e.type)
                    {
                    case SDL_QUIT:
                        shutdown = true;
                        browser->GetHost()->CloseBrowser(false);
                        break;

                    case SDL_KEYDOWN:
                        {
                            CefKeyEvent event;
                            event.modifiers = getKeyboardModifiers(e.key.keysym.mod);
                            event.windows_key_code = getWindowsKeyCode(e.key.keysym);

                            event.type = KEYEVENT_RAWKEYDOWN;
                            browser->GetHost()->SendKeyEvent(event);

                            event.type = KEYEVENT_CHAR;
                            browser->GetHost()->SendKeyEvent(event);
                        }
                        break;

                    case SDL_KEYUP:
                        {
                            CefKeyEvent event;
                            event.modifiers = getKeyboardModifiers(e.key.keysym.mod);
                            event.windows_key_code = getWindowsKeyCode(e.key.keysym);

                            event.type = KEYEVENT_KEYUP;

                            browser->GetHost()->SendKeyEvent(event);
                        }
                        break;

                    case SDL_WINDOWEVENT:
                        switch (e.window.event)
                        {
                        case SDL_WINDOWEVENT_SIZE_CHANGED:
                            renderHandler->resize(e.window.data1, e.window.data2);
                            browser->GetHost()->WasResized();
                            break;

                        case SDL_WINDOWEVENT_FOCUS_GAINED:
                            browser->GetHost()->SetFocus(true);
                            break;

                        case SDL_WINDOWEVENT_FOCUS_LOST:
                            browser->GetHost()->SetFocus(false);
                            break;

                        case SDL_WINDOWEVENT_HIDDEN:
                        case SDL_WINDOWEVENT_MINIMIZED:
                            // browser->GetHost()->SetWindowVisibility(false);
                            browser->GetHost()->WasHidden(true);
                            break;

                        case SDL_WINDOWEVENT_SHOWN:
                        case SDL_WINDOWEVENT_RESTORED:
                            //browser->GetHost()->SetWindowVisibility(true);
                            browser->GetHost()->WasHidden(false);
                            break;

                        case SDL_WINDOWEVENT_CLOSE:
                            e.type = SDL_QUIT;
                            SDL_PushEvent(&e);
                            break;
                        }
                        break;

                    case SDL_MOUSEMOTION:
                        {
                            CefMouseEvent event;
                            event.x = e.motion.x;
                            event.y = e.motion.y;

                            browser->GetHost()->SendMouseMoveEvent(event, false);
                        }
                        break;

                    case SDL_MOUSEBUTTONUP:
                        {
                            CefMouseEvent event;
                            event.x = e.button.x;
                            event.y = e.button.y;

                            browser->GetHost()->SendMouseClickEvent(event,
                                       translateMouseButton(e.button), true, 1);
                        }
                        break;

                    case SDL_MOUSEBUTTONDOWN:
                        {
                            CefMouseEvent event;
                            event.x = e.button.x;
                            event.y = e.button.y;

                            browser->GetHost()->SendMouseClickEvent(event,
                                      translateMouseButton(e.button), false, 1);
                        }
                        break;

                    case SDL_MOUSEWHEEL:
                        {
                            int delta_x = e.wheel.x;
                            int delta_y = e.wheel.y;

                            if (SDL_MOUSEWHEEL_FLIPPED == e.wheel.direction)
                            {
                                delta_y *= -1;
                            }
                            else
                            {
                                delta_x *= -1;
                            }

                            CefMouseEvent event;
                            browser->GetHost()->SendMouseWheelEvent(event, delta_x, delta_y);
                        }
                        break;
                    }
                }

#if 0
                if (!js_executed && browserClient->isLoaded())
                {
                    js_executed = true;

                    CefRefPtr<CefFrame> frame = browser->GetMainFrame();
                    frame->ExecuteJavaScript("alert('ExecuteJavaScript works!');", frame->GetURL(), 0);
                }
#endif

                // let browser process events
                CefDoMessageLoopWork();

                // render
                SDL_RenderClear(m_renderer);

                renderHandler->render();

                // Update screen
                SDL_RenderPresent(m_renderer);
            }

            browser = nullptr;
            browserClient = nullptr;
            renderHandler = nullptr;

            CefShutdown();

            SDL_DestroyRenderer(m_renderer);
        }
    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
