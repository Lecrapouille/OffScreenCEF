#ifndef GLWINDOW_HPP
#  define GLWINDOW_HPP

#include <GL/glew.h>
#include <GLFW/glfw3.h>

class GLWindow
{
public:

    GLWindow(uint32_t const width, uint32_t const height, const char *title);
    virtual ~GLWindow();
    bool start();

private:

    virtual bool setup() = 0;
    virtual bool update() = 0;

protected:

    GLFWwindow *m_window = nullptr;
    uint32_t m_width;
    uint32_t m_height;
};

#endif
