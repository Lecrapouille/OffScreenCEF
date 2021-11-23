#include <GL/glew.h>
#include <include/cef_render_handler.h>

class RenderHandler : public CefRenderHandler
{
public:

    RenderHandler();

    void init();
    void resize(int w, int h);

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

private:

    int m_width;
    int m_height;
    GLuint m_tex;
};
