#ifndef WEBCORE_HPP
#  define WEBCORE_HPP

#include <include/cef_client.h>
#include <string>
#include <vector>
#include <memory>

class RenderHandler;
class BrowserClient;

class WebCore
{
public:

    WebCore(const std::string &url);
    ~WebCore();

    void reshape(int w, int h);
    void mouseMove(int x, int y);
    void load(const std::string &url);
    void mouseClick(CefBrowserHost::MouseButtonType btn, bool mouse_up);
    void keyPress(int key, bool pressed);
    RenderHandler* render_handler() const { return m_render_handler; }

private:

    int m_mouse_x;
    int m_mouse_y;

    CefRefPtr<CefBrowser> m_browser;
    CefRefPtr<BrowserClient> m_client;

    RenderHandler* m_render_handler = nullptr;
};

#endif
