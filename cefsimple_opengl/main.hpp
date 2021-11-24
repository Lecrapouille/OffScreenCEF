#ifndef MAIN_HPP
#  define MAIN_HPP

#  include "GLWindow.hpp"

#  include <glm/glm.hpp>
#  include <glm/ext.hpp>

#  include <cef_render_handler.h>
#  include <cef_client.h>
#  include <cef_app.h>

#  include <string>
#  include <vector>
#  include <memory>
#  include <iostream>
#  include <algorithm>

class RenderHandler : public CefRenderHandler
{
public:

    ~RenderHandler();
    bool init();
    void draw();
    void reshape(int w, int h);

    //! \brief CefBase interface
    IMPLEMENT_REFCOUNTING(RenderHandler);

    //! \brief CefRenderHandler interface
    virtual void GetViewRect(CefRefPtr<CefBrowser> browser, CefRect &rect) override;
    virtual void OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type,
                         const RectList &dirtyRects, const void *buffer,
                         int width, int height) override;

public:

    GLuint texture() const
    {
        return m_tex;
    }

public:

    bool m_initialized = false;

private:

    int m_width;
    int m_height;
    GLuint m_tex = 0;
    GLuint m_vao, m_vbo;

    // Shader variable locations
    GLint m_pos_loc = -1;
    GLint m_texcoord_loc = -1;
    GLint m_tex_loc = -1;
    GLint m_mvp_loc = -1;
    GLuint m_prog = 0;
};

class BrowserClient : public CefClient
{
public:

    BrowserClient(CefRefPtr<CefRenderHandler> ptr)
        : m_renderHandler(ptr)
    {}

    virtual CefRefPtr<CefRenderHandler> GetRenderHandler() override
    {
        return m_renderHandler;
    }

    CefRefPtr<CefRenderHandler> m_renderHandler;

    IMPLEMENT_REFCOUNTING(BrowserClient);
};

class WebCore
{
public:

    WebCore(const std::string &url);
    ~WebCore();

    void draw();
    void reshape(int w, int h);
    void load(const std::string &url);
    // void executeJS(const std::string &cmd);

    void mouseMove(int x, int y);
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

class CEFGLWindow: public GLWindow
{
public:

    CEFGLWindow(uint32_t const width, uint32_t const height, const char *title);
    ~CEFGLWindow();

private:

    virtual bool setup() override;
    virtual bool update() override;
    std::weak_ptr<WebCore> createBrowser(const std::string &url);
    void removeBrowser(std::weak_ptr<WebCore> web_core);

public:

    std::weak_ptr<WebCore> m_web_core;
    std::weak_ptr<WebCore> m_web_core_other;

private:

    // CEF
    std::vector<std::shared_ptr<WebCore>> m_browsers;
};

#endif
