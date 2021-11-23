#include "WebCore.hpp"
#include "BrowserClient.hpp"
#include "RenderHandler.hpp"
#include <include/cef_app.h>
#include <cassert>
#include <iostream>
#include <algorithm>

bool WebCoreManager::setUp(/*int argc, char** argv*/)
{
    CefMainArgs args/*(argc, argv)*/;
    int exit_code = CefExecuteProcess(args, nullptr, nullptr);
    if (exit_code >= 0)
    {
        std::cerr << "CefExecuteProcess: failed" << std::endl;
        return false;
    }

    CefSettings settings;
    // TODO CefString(&settings.locales_dir_path) = "cef/linux/lib/locales";
    bool result = CefInitialize(args, settings, nullptr, nullptr);
    if (!result)
    {
        std::cerr << "CefInitialize: failed" << std::endl;
        return false;
    }
    return true;
}

void WebCoreManager::shutDown()
{
    m_browsers.clear();
    CefShutdown();
}

void WebCoreManager::update()
{
    CefDoMessageLoopWork();
}

std::weak_ptr<WebCore> WebCoreManager::createBrowser(const std::string &url)
{
    auto web_core = std::make_shared<WebCore>(url);
    m_browsers.push_back(web_core);
    return web_core;
}

void WebCoreManager::removeBrowser(std::weak_ptr<WebCore> web_core)
{
    auto elem = web_core.lock();
    if (elem)
    {
        auto found = std::find(m_browsers.begin(), m_browsers.end(), elem);
        if (found != m_browsers.end())
        {
            m_browsers.erase(found);
        }
    }
}

WebCore::WebCore(const std::string &url)
    : m_mouse_x(0), m_mouse_y(0)
{
    CefWindowInfo window_info;
    window_info.SetAsWindowless(0);
    //window_info.SetAsWindowless(nullptr);
    //window_info.SetTransparentPainting(true);

    m_render_handler = new RenderHandler();
    m_render_handler->init();
    m_render_handler->resize(128, 128); // initial size

    CefBrowserSettings browserSettings;
    browserSettings.windowless_frame_rate = 60; // 30 is default

    m_client = new BrowserClient(m_render_handler);
    m_browser = CefBrowserHost::CreateBrowserSync(window_info, m_client.get(), url, browserSettings, nullptr, nullptr);
}

WebCore::~WebCore()
{
    m_browser->GetHost()->CloseBrowser(true);
    CefDoMessageLoopWork();

    m_browser = nullptr;
    m_client = nullptr;
}

void WebCore::load(const std::string &url)
{
  //if (!m_render_handler->initialized)
  //  m_render_handler->init();

  m_browser->GetMainFrame()->LoadURL(url);
}

void WebCore::reshape(int w, int h)
{
    m_render_handler->resize(w, h);
    m_browser->GetHost()->WasResized();
}

void WebCore::mouseMove(int x, int y)
{
    m_mouse_x = x;
    m_mouse_y = y;

    CefMouseEvent evt;
    evt.x = x;
    evt.y = y;

    bool mouse_leave = false; // TODO
    m_browser->GetHost()->SendMouseMoveEvent(evt, mouse_leave);
}

void WebCore::mouseClick(CefBrowserHost::MouseButtonType btn, bool mouse_up)
{
    CefMouseEvent evt;
    evt.x = m_mouse_x;
    evt.y = m_mouse_y;

    int click_count = 1; // TODO
    m_browser->GetHost()->SendMouseClickEvent(evt, btn, mouse_up, click_count);
}

void WebCore::keyPress(int key, bool pressed)
{
    //TODO ???
    // test page http://javascript.info/tutorial/keyboard-events
    CefKeyEvent evt;
    //event.native_key_code = key;
    //event.type = pressed ? KEYEVENT_KEYDOWN : KEYEVENT_KEYUP;
    evt.character = key;
    evt.native_key_code = key;
    evt.type = KEYEVENT_CHAR;

    m_browser->GetHost()->SendKeyEvent(evt);
}
