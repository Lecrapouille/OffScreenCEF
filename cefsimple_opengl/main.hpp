#ifndef MAIN_HPP
#  define MAIN_HPP

#  include "GLWindow.hpp"
#  include "RenderHandler.hpp"
#  include "WebCore.hpp"

class CEFGLWindow: public GLWindow
{
public:

    CEFGLWindow(uint32_t const width, uint32_t const height, const char *title);
    ~CEFGLWindow();

private:

    virtual bool setup() override;
    virtual bool update() override;
    bool setupOpenGL();
    bool updateOpenGL();
    bool setupCEF();
    bool updateCEF();
    std::weak_ptr<WebCore> createBrowser(const std::string &url);
    void removeBrowser(std::weak_ptr<WebCore> web_core);
    
private:

    // Shader variable locations
    GLint m_pos_loc = -1;
    GLint m_texcoord_loc = -1;
    GLint m_tex_loc = -1;
    GLint m_mvp_loc = -1;
    GLuint m_prog = 0;

    // CEF
    std::weak_ptr<WebCore> m_web_core;
    std::weak_ptr<WebCore> m_web_core_other;
    std::vector<std::shared_ptr<WebCore>> m_browsers;
};

#endif
