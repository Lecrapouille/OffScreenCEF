#ifndef MAIN_HPP
#  define MAIN_HPP

// Main application
#  include "GLWindow.hpp"

// Matrices manipulation for OpenGL
#  include <glm/glm.hpp>
#  include <glm/ext.hpp>

// Chromium Embedded Framework
#  include <cef_render_handler.h>
#  include <cef_client.h>
#  include <cef_app.h>

#  include <string>
#  include <vector>
#  include <memory>
#  include <iostream>
#  include <algorithm>

// ****************************************************************************
//! \brief Interface class rendering a single web page.
// ****************************************************************************
class BrowserView
{
public:

    //! \brief Default Constructor using a given URL.
    BrowserView(const std::string &url);

    //! \brief
    ~BrowserView();

    //! \brief Load the given web page
    void load(const std::string &url);

    //! \brief Render the web page
    void draw();

    //! \brief Set the windows size
    void reshape(int w, int h);

    //! \brief Set the viewport
    bool viewport(float x, float y, float w, float h);

    //! \brief Get the viewport
    inline glm::vec4 const& viewport() const
    {
        return m_viewport;
    }

    //! \brief TODO
    // void executeJS(const std::string &cmd);

    //! \brief Set the new mouse position
    void mouseMove(int x, int y);

    //! \brief Set the new mouse state (clicked ...)
    void mouseClick(CefBrowserHost::MouseButtonType btn, bool mouse_up);

    //! \brief Set the new keyboard state (char typed ...)
    void keyPress(int key, bool pressed);

private:

    // *************************************************************************
    //! \brief Private implementation to handle CEF events to draw the web page.
    // *************************************************************************
    class RenderHandler: public CefRenderHandler
    {
    public:

        RenderHandler(glm::vec4 const& viewport);

        //! \brief
        ~RenderHandler();

        //! \brief Compile OpenGL shaders and create OpenGL objects (VAO,
        //! VBO, texture, locations ...)
        bool init();

        //! \brief Render OpenGL VAO (rotating a textured square)
        void draw(glm::vec4 const& viewport, bool fixed);

        //! \brief Resize the view
        void reshape(int w, int h);

        //! \brief Return the OpenGL texture handle
        GLuint texture() const
        {
            return m_tex;
        }

        //! \brief CefRenderHandler interface
        virtual void GetViewRect(CefRefPtr<CefBrowser> browser, CefRect &rect) override;

        //! \brief CefRenderHandler interface
        //! Update the OpenGL texture.
        virtual void OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type,
                             const RectList &dirtyRects, const void *buffer,
                             int width, int height) override;

        //! \brief CefBase interface
        IMPLEMENT_REFCOUNTING(RenderHandler);

    private:

        //! \brief Dimension
        int m_width;
        int m_height;
        
        //! \brief Where to draw on the OpenGL window
        glm::vec4 const& m_viewport;

        //! \brief OpenGL shader program handle
        GLuint m_prog = 0;
        //! \brief OpenGL texture handle
        GLuint m_tex = 0;
        //! \brief OpenGL vertex array object handle
        GLuint m_vao = 0;
        //! \brief OpenGL vertex buffer obejct handle
        GLuint m_vbo = 0;

        //! \brief OpenGL shader variable locations for vertices of the
        //! rectangle
        GLint m_pos_loc = -1;
        //! \brief OpenGL shader variable locations for the texture
        GLint m_tex_loc = -1;
        //! \brief OpenGL shader variable locations for the Model View
        //! Projection matrix.
        GLint m_mvp_loc = -1;
    };

    // *************************************************************************
    //! \brief Provide access to browser-instance-specific callbacks. A single
    //! CefClient instance can be shared among any number of browsers.
    // *************************************************************************
    class BrowserClient: public CefClient
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

private:

    //! \brief Mouse cursor position on the OpenGL window
    int m_mouse_x;
    int m_mouse_y;

    //! \brief Where to draw on the OpenGL window
    glm::vec4 m_viewport;

    //! \brief Chromium Embedded framework elements
    CefRefPtr<CefBrowser> m_browser;
    CefRefPtr<BrowserClient> m_client;
    RenderHandler* m_render_handler = nullptr;

    //! \brief OpenGL has created GPU elements with success
    bool m_initialized = false;

public:

    //! \brief If set to false then the web page is turning.
    bool m_fixed = true;
};

// ****************************************************************************
//! \brief Extend the OpenGL base window and add Chromium Embedded Framework
//! browser views.
// ****************************************************************************
class CEFGLWindow: public GLWindow
{
public:

    //! \brief Default construction: define the window dimension and title
    CEFGLWindow(uint32_t const width, uint32_t const height, const char *title);

    //! \brief Destructor
    ~CEFGLWindow();

    //! \brief Non const getter of the list of browsers
    inline std::vector<std::shared_ptr<BrowserView>>& browsers()
    {
        return m_browsers;
    }

private: // Concrete implementation from GLWindow

    virtual bool setup() override;
    virtual bool update() override;

private:

    //! \brief Create a new browser view from a given URL
    std::weak_ptr<BrowserView> createBrowser(const std::string &url);

    //! \brief Destroy the given browser view.
    void removeBrowser(std::weak_ptr<BrowserView> web_core);

private:

    //! \brief List of BrowserView managed by createBrowser() and
    //! removeBrowser() methods.
    std::vector<std::shared_ptr<BrowserView>> m_browsers;
};

#endif
