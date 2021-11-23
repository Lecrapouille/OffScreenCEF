#ifndef GLWINDOW_HPP
#  define GLWINDOW_HPP

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>

class GLWindow
{
public:

    GLWindow(uint32_t const width, uint32_t const height, const char *title);
    virtual ~GLWindow();
    bool start();

private:

    void init();
    virtual bool setup() = 0;
    virtual bool update() = 0;

protected:

    GLFWwindow *m_window = nullptr;
    uint32_t m_width;
    uint32_t m_height;
    std::string m_title;
};

#endif
