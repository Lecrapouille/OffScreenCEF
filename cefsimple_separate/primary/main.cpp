// This code is a modification of the original projects that can be found at
// https://github.com/ashea-code/BluBrowser here quick and dirty modified to
// make things worked with SDL.

#include <stdio.h>
#include <iostream>
#include <sstream>
#include <mutex>

#include <cef_app.h>
#include <cef_browser.h>
#include <cef_client.h>
#include <cef_render_handler.h>
#include <cef_life_span_handler.h>
#include <cef_load_handler.h>
#include <wrapper/cef_helpers.h>

#include <SDL2/SDL.h>
#include <SDL_image.h>
//#include "sdl_keyboard_utils.h"

//=============================================================================
//
//=============================================================================
class BluManager : public CefApp
{
public:

    static void DoBluMessageLoop()
    {
        CefDoMessageLoopWork();
    }

    virtual void OnBeforeCommandLineProcessing(
        const CefString& ProcessType,
        CefRefPtr<CefCommandLine> CommandLine) override
    { // FIXME LeCrapouille: does not this way!

        /**
         * Used to pick command line switches
         * If set to "true": CEF will use less CPU, but rendering performance will be lower. CSS3 and WebGL are not be usable
         * If set to "false": CEF will use more CPU, but rendering will be better, CSS3 and WebGL will also be usable
         */
        BluManager::CPURenderSettings = false;

        CommandLine->AppendSwitch("off-screen-rendering-enabled");
        CommandLine->AppendSwitchWithValue("off-screen-frame-rate", "60");
        CommandLine->AppendSwitch("enable-font-antialiasing");
        CommandLine->AppendSwitch("enable-media-stream");

        // Should we use the render settings that use less CPU?
        if (CPURenderSettings)
        {
            CommandLine->AppendSwitch("disable-gpu");
            CommandLine->AppendSwitch("disable-gpu-compositing");
            CommandLine->AppendSwitch("enable-begin-frame-scheduling");
        }
        else
        {
            // Enables things like CSS3 and WebGL
            CommandLine->AppendSwitch("enable-gpu-rasterization");
            CommandLine->AppendSwitch("enable-webgl");
            CommandLine->AppendSwitch("disable-web-security");
        }

        CommandLine->AppendSwitchWithValue("enable-blink-features", "HTMLImports");

        if (AutoPlay)
        {
            CommandLine->AppendSwitchWithValue("autoplay-policy", "no-user-gesture-required");
        }

        // Append more command line options here if you want
        // Visit Peter Beverloo's site: http://peter.sh/experiments/chromium-command-line-switches/ for more info on the switches
    }

    static CefSettings Settings;
    static CefMainArgs MainArgs;
    static bool CPURenderSettings;
    static bool AutoPlay;

    IMPLEMENT_REFCOUNTING(BluManager);
};

CefSettings BluManager::Settings;
CefMainArgs BluManager::MainArgs;
bool BluManager::CPURenderSettings = false;
bool BluManager::AutoPlay = true;

//=============================================================================
//
//=============================================================================
class RenderHandler: public CefRenderHandler
{
public:

    RenderHandler(SDL_Renderer& renderer, int w, int h)
        : m_renderer(renderer)
    {
        resize(w, h);
    }

    ~RenderHandler()
    {
        if (m_texture != nullptr)
        {
            SDL_DestroyTexture(m_texture);
        }
    }

    virtual void GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) override
    {
        rect = CefRect(0, 0, m_width, m_height);
    }

    virtual void OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type,
                         const RectList& dirtyRects, const void* buffer,
                         int w, int h) override
    {
        resize(w, h); // FIXME: I dunno why this avoid segfault
        std::lock_guard<std::mutex> locker(m_mutex_texture);

        if ((m_texture == nullptr) || (buffer == nullptr) || (w <= 0) || (h <= 0)) {
            std::cerr << "OnPaint: bad texture or bad size" << std::endl;
            return ;
        }

        unsigned char* texture_data = nullptr;
        int texture_pitch = 0;

        SDL_LockTexture(m_texture, nullptr, (void**) &texture_data, &texture_pitch);
        memcpy(texture_data, buffer, static_cast<size_t>(w * h * 4));
        SDL_UnlockTexture(m_texture);
    }

    void resize(int w, int h)
    {
        std::lock_guard<std::mutex> locker(m_mutex_texture);

        if ((w == m_width) && (h == m_height)) {
            return ;
        }

        if (m_texture != nullptr) {
            SDL_DestroyTexture(m_texture);
        }

        m_texture = SDL_CreateTexture(&m_renderer, SDL_PIXELFORMAT_UNKNOWN,
                                      SDL_TEXTUREACCESS_STREAMING, w, h);
        assert(m_texture != nullptr);
        m_width = w;
        m_height = h;
    }

    void render()
    {
        std::lock_guard<std::mutex> locker(m_mutex_texture);

        if (m_texture != nullptr)
        {
            SDL_RenderCopy(&m_renderer, m_texture, nullptr, nullptr);
        }
    }

private:

    SDL_Renderer& m_renderer;
    SDL_Texture* m_texture = nullptr;
    std::mutex m_mutex_texture;
    int m_width;
    int m_height;

    IMPLEMENT_REFCOUNTING(RenderHandler);
};

//=============================================================================
//
//=============================================================================
class BrowserClient: public CefClient,
                     public CefLifeSpanHandler,
                     public CefDownloadHandler,
                     public CefLoadHandler,
                     public CefDisplayHandler
{
public:

    BrowserClient(CefRefPtr<CefRenderHandler> ptr)
        : m_handler(ptr)
    {}

    virtual bool OnProcessMessageReceived(CefRefPtr<CefBrowser> Browser,
                                          CefRefPtr<CefFrame> Frame,
                                          CefProcessId SourceProcess,
                                          CefRefPtr<CefProcessMessage> Message) override
    {
        return true;
    }

    /*virtual void OnUncaughtException(CefRefPtr<CefBrowser> Browser,
      CefRefPtr<CefFrame> Frame,
      CefRefPtr<CefV8Context> Context,
      CefRefPtr<CefV8Exception> Exception,
      CefRefPtr<CefV8StackTrace> StackTrace) override
      {
      std::cout << "OnConsoleMessage: " << Exception->GetMessage().c_str() << std::endl;
      }

      // FIXME Getter for renderer
      virtual CefRefPtr<RenderHandler> GetRenderHandlerCustom()
      {
      return m_handler;
      };

    */

    // FIXME required or pdf download won't work
    virtual CefRefPtr<CefDownloadHandler> GetDownloadHandler() override
    {
        return this;
    }

    // CefLifeSpanHandler
    virtual bool OnBeforePopup(CefRefPtr<CefBrowser> Browser,
                               CefRefPtr<CefFrame> Frame,
                               const CefString& TargetUrl,
                               const CefString& TargetFrameName,
                               WindowOpenDisposition TargetDisposition,
                               bool UserGesture,
                               const CefPopupFeatures& PopupFeatures,
                               CefWindowInfo& WindowInfo,
                               CefRefPtr<CefClient>& Client,
                               CefBrowserSettings& Settings,
                               CefRefPtr<CefDictionaryValue>& ExtraInfo,
                               bool* NoJavascriptAccess)
    {
        return false;
    }

    // CefDownloadHandler
    virtual void OnBeforeDownload(
        CefRefPtr<CefBrowser> Browser,
        CefRefPtr<CefDownloadItem> DownloadItem,
        const CefString& SuggestedName,
        CefRefPtr<CefBeforeDownloadCallback> Callback) override
    {
        // TODO
    }

    virtual void OnDownloadUpdated(
        CefRefPtr<CefBrowser> Browser,
        CefRefPtr<CefDownloadItem> DownloadItem,
        CefRefPtr<CefDownloadItemCallback> Callback) override
    {
        // TODO
    }

    // Getter for lifespan
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
        // CEF_REQUIRE_UI_THREAD();

        if (!m_browser.get())
        {
            // Keep a reference to the main browser.
            m_browser = browser;
            m_browser_id = browser->GetIdentifier();
        }
    }

    virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) override
    {
        // CEF_REQUIRE_UI_THREAD();
        if (m_browser_id == browser->GetIdentifier())
        {
            m_browser = nullptr;
        }
    }

    virtual bool OnConsoleMessage(CefRefPtr<CefBrowser> Browser,
                                  cef_log_severity_t Level,
                                  const CefString& Message,
                                  const CefString& Source,
                                  int Line) override
    {
        std::cout << "OnConsoleMessage: " << Message << std::endl;
        return true;
    }

    virtual void OnFullscreenModeChange(CefRefPtr< CefBrowser > Browser, bool Fullscreen) override
    {}

    virtual void OnTitleChange(CefRefPtr< CefBrowser > Browser, const CefString& Title) override
    {}

    CefRefPtr<CefBrowser> GetCEFBrowser()
    {
        return m_browser;
    }

private:

    int m_browser_id;
    CefRefPtr<CefRenderHandler> m_handler;
    CefRefPtr<CefBrowser> m_browser;
    bool m_is_closing;

    IMPLEMENT_REFCOUNTING(BrowserClient);
};

//=============================================================================
//
//=============================================================================
void StartupModule()
{
    std::cout << "StartupModule begin" << std::endl;

    //CefString GameDirCef = *FPaths::ConvertRelativePathToFull(FPaths::ProjectDir() + "BluCache");
    //FString ExecutablePath = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir() + "Plugins/BLUI/ThirdParty/cef/");

    // Setup the default settings for BluManager
    BluManager::Settings.windowless_rendering_enabled = true;
    BluManager::Settings.no_sandbox = true;
    BluManager::Settings.remote_debugging_port = 7777;
    BluManager::Settings.uncaught_exception_stack_size = 5;

#ifndef SECONDARY_PATH
#  error "SECONDARY_PATH is not defined and its path shall be canonical"
#endif

    CefString realExePath = SECONDARY_PATH;

    // Set the sub-process path
    CefString(&BluManager::Settings.browser_subprocess_path).FromString(realExePath);

    // Set the cache path
    //CefString(&BluManager::Settings.cache_path).FromString(GameDirCef);

    // Make a new manager instance
    CefRefPtr<BluManager> BluApp = new BluManager();

    //CefExecuteProcess(BluManager::main_args, BluApp, nullptr);
    CefInitialize(BluManager::MainArgs, BluManager::Settings, BluApp, nullptr);

    std::cout << "StartupModule end" << std::endl;
}

//=============================================================================
//
//=============================================================================
class BrowserView
{
public:

    struct BluEyeSettings
    {
        float FrameRate = 60.f;
        int32_t Width = 200;
        int32_t Height = 200;
        bool bIsTransparent = false;
        bool bEnableWebGL = true;
        bool bAudioMuted = false;
        bool bAutoPlayEnabled = true;
    };

    void init(SDL_Renderer *sdl, int w, int h)
    {
// FIXME Lecrapouille: this does not work like this !

        // We don't want this running in editor unless it's PIE
        // If we don't check this, CEF will spawn infinite processes with widget components

        if ((w <= 0) || (h <= 0))
        {
            std::cerr << "Can't initialize when Width or Height are <= 0" << std::endl;
            return;
        }
        Settings.Width = w;
        Settings.Height = h;

        //BrowserSettings.universal_access_from_file_urls = STATE_ENABLED;
        //BrowserSettings.file_access_from_file_urls = STATE_ENABLED;

        //BrowserSettings.web_security = STATE_DISABLED;
        //BrowserSettings.fullscreen_enabled = true;

        //Info.width = Settings.Width;
        //Info.height = Settings.Height;

        // Set transparant option
        Info.SetAsWindowless(0); //bIsTransparent

        // Figure out if we want to turn on WebGL support
        if (Settings.bEnableWebGL)
        {
            if (BluManager::CPURenderSettings)
            {
                std::cerr << "You have enabled WebGL for this browser, but CPU Saver is enabled in BluManager.cpp - WebGL will not work!" << std::endl;
            }
            BrowserSettings.webgl = STATE_ENABLED;
        }

        //NB: this setting will change it globally for all new instances
        BluManager::AutoPlay = Settings.bAutoPlayEnabled;

        Renderer = new RenderHandler(*sdl, Settings.Width, Settings.Height);
        ClientHandler = new BrowserClient(Renderer);
        Browser = CefBrowserHost::CreateBrowserSync(Info,
                                                    ClientHandler.get(),
                                                    "https://github.com/Lecrapouille/gdcef",
                                                    BrowserSettings,
                                                    nullptr,
                                                    nullptr);

        Browser->GetHost()->SetWindowlessFrameRate(Settings.FrameRate);
        Browser->GetHost()->SetAudioMuted(Settings.bAudioMuted);

        // Setup JS event emitter
        //ClientHandler->SetEventEmitter(&ScriptEventEmitter);
        //ClientHandler->SetLogEmitter(&LogEventEmitter);;

        // Load the default URL
        // LoadURL("https://github.com/");
        //ResetTexture();

        //Instead of manually ticking, we now tick whenever one blu eye is created
        //FIXME SpawnTickEventLoopIfNeeded();
    }

    void WasResized()
    {
        if (Browser) {
           Browser->GetHost()->WasResized();
        }
    }

    void SetFocus(bool const v)
    {
        if (Browser) {
           Browser->GetHost()->SetFocus(v);
        }
    }

    void WasHidden(bool const v)
    {
        if (Browser) {
           Browser->GetHost()->WasHidden(v);
        }
    }

    void CloseBrowser()
    {
        m_closing = true;
        if (Browser)
        {
            // Close up the browser
            Browser->GetHost()->SetAudioMuted(true);
            Browser->GetMainFrame()->LoadURL("about:blank");
            //browser->GetMainFrame()->Delete();
            Browser->GetHost()->CloseDevTools();
            Browser->GetHost()->CloseBrowser(true);
            Browser = nullptr;


            std::cout << "Browser Closing" << std::endl;
        }

#if 0
        DestroyTexture();
        SetFlags(RF_BeginDestroyed);

        //Remove our auto-ticking setup
        EventLoopData.EyeCount--;
        if (EventLoopData.EyeCount <= 0)
        {
            FTicker::GetCoreTicker().RemoveTicker(EventLoopData.DelegateHandle);
            EventLoopData.DelegateHandle = FDelegateHandle();
        }
        Super::BeginDestroy();
#endif
    }

    void SetShouldTickEventLoop(bool ShouldTick /*= true*/)
    {
        //FIXME EventLoopData.bShouldTickEventLoop = ShouldTick;
    }

    void LoadURL(std::string const& url)
    {
        assert(Browser != nullptr);

        Browser->GetMainFrame()->LoadURL(url);
    }

    std::string GetCurrentURL()
    {
        assert(Browser != nullptr);

        return Browser->GetMainFrame()->GetURL();
    }

    void SetZoom(const float Scale /*= 1*/)
    {
        assert(Browser != nullptr);

        Browser->GetHost()->SetZoomLevel(Scale);
    }

    float GetZoom()
    {
        assert(Browser != nullptr);

        return Browser->GetHost()->GetZoomLevel();
    }

    void DownloadFile(std::string const& FileUrl)
    {
        assert(Browser != nullptr);

        Browser->GetHost()->StartDownload(FileUrl);
        //Todo: ensure downloading works in some way, shape or form?
    }

    bool IsBrowserLoading()
    {
        assert(Browser != nullptr);

        return Browser->IsLoading();
    }

    void ReloadBrowser(bool IgnoreCache)
    {
        assert(Browser != nullptr);

        if (IgnoreCache)
        {
            return Browser->ReloadIgnoreCache();
        }

        Browser->Reload();
    }

    void NavBack()
    {
        assert(Browser != nullptr);

        if (Browser->CanGoBack())
        {
            Browser->GoBack();
        }
    }

    void NavForward()
    {
        assert(Browser != nullptr);

        if (Browser->CanGoForward())
        {
            Browser->GoForward();
        }
    }

    void ResizeBrowser(const int32_t NewWidth, const int32_t NewHeight)
    {
        assert(Renderer != nullptr);
        assert(Browser != nullptr);

        if (NewWidth <= 0 || NewHeight <= 0)
            return ;

        // Disable the web view while we resize
        //bEnabled = false;

        // Set our new Width and Height
        Settings.Width = NewWidth;
        Settings.Height = NewHeight;

        // Update our render handler
        Renderer->resize(NewWidth, NewHeight);

        //bValidTexture = false;

        //Texture = UTexture2D::CreateTransient(Settings.Width, Settings.Height, PF_B8G8R8A8);
        //Texture->AddToRoot();
        //Texture->UpdateResource();

        //bValidTexture = true;

        // Let the browser's host know we resized it
        Browser->GetHost()->WasResized();

        // Now we can keep going
        //bEnabled = true;

        //return Texture;
    }

    bool closeAllowed() const
    {
        return m_closing;
    }

    void render()
    {
        assert(Renderer != nullptr);
        Renderer->render();
    }

private:

    CefWindowInfo Info;
    CefBrowserSettings BrowserSettings;
    BluEyeSettings Settings;
    CefRefPtr<BrowserClient> ClientHandler = nullptr;
    RenderHandler* Renderer = nullptr;
    CefRefPtr<CefBrowser> Browser = nullptr;
    bool m_closing = false;
};

//=============================================================================
//
//=============================================================================
int main(int argc, char * argv[])
{
    StartupModule();

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return EXIT_FAILURE;
    }

    int width = 800;
    int height = 600;

    auto window = SDL_CreateWindow("Render CEF with SDL with secondry process", SDL_WINDOWPOS_UNDEFINED,
                                   SDL_WINDOWPOS_UNDEFINED, width, height,
                                   SDL_WINDOW_RESIZABLE);
    if (window == nullptr)
    {
        std::cerr << "SDL could not create window! SDL_Error: "
                  << SDL_GetError() << std::endl;
        return EXIT_FAILURE;
    }

    SDL_Renderer* sdl_renderer =
            SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED |
                               SDL_RENDERER_PRESENTVSYNC);
    if (sdl_renderer == nullptr)
    {
        std::cerr << "SDL could not create renderer! SDL_Error: "
                  << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return EXIT_FAILURE;
    }

    SDL_Event e;
    BrowserView browser_client;
    browser_client.init(sdl_renderer, width, height);

    bool shutdown = false;
    while (!browser_client.closeAllowed())
    {
        // send events to browser
        while ((!shutdown) && (SDL_PollEvent(&e) != 0))
        {
            switch (e.type)
            {
            case SDL_QUIT:
                shutdown = true;
                browser_client.CloseBrowser();
                break;
            case SDL_WINDOWEVENT:
                switch (e.window.event)
                {
                case SDL_WINDOWEVENT_SIZE_CHANGED:
                    browser_client.ResizeBrowser(e.window.data1, e.window.data2);
                    browser_client.WasResized();
                    break;

                case SDL_WINDOWEVENT_FOCUS_GAINED:
                    browser_client.ResizeBrowser(e.window.data1, e.window.data2);
                    break;

                case SDL_WINDOWEVENT_FOCUS_LOST:
                    browser_client.SetFocus(false);
                    break;

                case SDL_WINDOWEVENT_HIDDEN:
                case SDL_WINDOWEVENT_MINIMIZED:
                    // browser->GetHost()->SetWindowVisibility(false);
                    browser_client.WasHidden(true);
                    break;

                case SDL_WINDOWEVENT_SHOWN:
                case SDL_WINDOWEVENT_RESTORED:
                    browser_client.WasHidden(false);
                    break;

                case SDL_WINDOWEVENT_CLOSE:
                    e.type = SDL_QUIT;
                    SDL_PushEvent(&e);
                    break;
                }
                break;
            default:
                break;
            }
        }

        // let browser process events
        CefDoMessageLoopWork();

        // render
        SDL_RenderClear(sdl_renderer);
        browser_client.render();
        SDL_RenderPresent(sdl_renderer);
    }

    CefShutdown();
    SDL_DestroyRenderer(sdl_renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return EXIT_SUCCESS;
}
