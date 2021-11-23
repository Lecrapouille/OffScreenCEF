#include "main.hpp"
#include "GLCore.hpp"
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <include/cef_app.h>
#include <iostream>
#include <algorithm>

static void reshape_callback(GLFWwindow* window, int w, int h)
{
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);
}

static void key_callback(GLFWwindow* obj, int key, int scancode, int action, int mods)
{
    assert(nullptr != obj);
    GLWindow* window = static_cast<GLWindow*>(glfwGetWindowUserPointer(obj));

    //bool pressed = (action == GLFW_PRESS);
    //web_core.lock()->keyPress(key, pressed);
}

static void mouse_callback(GLFWwindow* obj, int btn, int state, int mods)
{
    assert(nullptr != obj);
    GLWindow* window = static_cast<GLWindow*>(glfwGetWindowUserPointer(obj));

    //int mouse_up = (GLFW_RELEASE == state);

    //std::map<int, CefBrowserHost::MouseButtonType> btn_type_map;
    //btn_type_map[GLFW_MOUSE_BUTTON_LEFT] = MBT_LEFT;
    //btn_type_map[GLFW_MOUSE_BUTTON_MIDDLE] = MBT_MIDDLE;
    //btn_type_map[GLFW_MOUSE_BUTTON_RIGHT] = MBT_RIGHT;
    //CefBrowserHost::MouseButtonType btn_type = btn_type_map[btn];

    //web_core.lock()->mouseClick(btn_type, mouse_up);
}

static void motion_callback(GLFWwindow* window, double x, double y)
{
    //web_core.lock()->mouseMove(x, y);
}

CEFGLWindow::CEFGLWindow(uint32_t const width, uint32_t const height, const char *title)
    : GLWindow(width, height, title)
{}

CEFGLWindow::~CEFGLWindow()
{
    m_browsers.clear();

    GLCore::deleteProgram(m_prog);
}

std::weak_ptr<WebCore> CEFGLWindow::createBrowser(const std::string &url)
{
    auto web_core = std::make_shared<WebCore>(url);
    m_browsers.push_back(web_core);
    return web_core;
}

void CEFGLWindow::removeBrowser(std::weak_ptr<WebCore> web_core)
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

bool CEFGLWindow::setup()
{
    if (!setupCEF())
    {
        std::cerr << "GLWindow::setupCEF() failed" << std::endl;
        return false;
    }
    if (!setupOpenGL())
    {
        std::cerr << "GLWindow::setupOpenGL() failed" << std::endl;
        return false;
    }
    return true;
}

bool CEFGLWindow::setupCEF()
{
    std::string url = "http://google.com";
    m_web_core = createBrowser(url);
    m_web_core.lock()->reshape(m_width, m_height);

    std::string other_url = "https://github.com/Lecrapouille?tab=repositories";
    m_web_core_other = createBrowser(other_url);
    m_web_core_other.lock()->reshape(m_width, m_height);

    return true;
}

bool CEFGLWindow::setupOpenGL()
{
    glfwSetKeyCallback(m_window, key_callback);
    glfwSetCursorPosCallback(m_window, motion_callback);
    glfwSetMouseButtonCallback(m_window, mouse_callback);
    glfwSetFramebufferSizeCallback(m_window, reshape_callback);

    m_prog = GLCore::createShaderProgram("shaders/tex.vert", "shaders/tex.frag");
    if (m_prog == 0)
    {
        std::cerr << "shader compile failed" << std::endl;
        return false;
    }

    m_pos_loc = glGetAttribLocation(m_prog, "a_position");
    m_texcoord_loc = glGetAttribLocation(m_prog, "a_texcoord");
    m_tex_loc = glGetUniformLocation(m_prog, "s_tex");
    m_mvp_loc = glGetUniformLocation(m_prog, "u_mvp");

    // initial GL state
    glViewport(0, 0, m_width, m_height);
    glClearColor(0.0, 0.0, 0.0, 0.0);

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_CULL_FACE);

    return true;
}

bool CEFGLWindow::update()
{
    if (!updateCEF())
    {
        std::cerr << "GLWindow::updte() failed" << std::endl;
        return false;
    }
    if (!updateOpenGL())
    {
        std::cerr << "GLWindow::updte() failed" << std::endl;
        return false;
    }
    return true;
}

bool CEFGLWindow::updateCEF()
{
    CefDoMessageLoopWork();
    return true;
}

bool CEFGLWindow::updateOpenGL()
{
    // 2 3
    // 0 1
    float vertices[] = {
        -1, -1, 0,
        1, -1, 0,
        -1, 1, 0,
        1, 1, 0,
    };

    float texcoords[] = {
        0, 1,
        1, 1,
        0, 0,
        1, 0,
    };

    unsigned short indices[] = {
        0, 1, 3,
        0, 3, 2,
    };

    glUseProgram(m_prog);

    glm::mat4 mvp = glm::ortho(-1, 1, -1, 1);
    //mvp *= glm::rotate((float)glfwGetTime() * 10.f, glm::vec3(0, 0, 1));
    glUniformMatrix4fv(m_mvp_loc, 1, GL_FALSE, glm::value_ptr(mvp));

    glEnableVertexAttribArray(m_pos_loc);
    glEnableVertexAttribArray(m_texcoord_loc);

    // bind texture
    glBindTexture(GL_TEXTURE_2D, m_web_core.lock()->render_handler()->texture());

    // draw
    glVertexAttribPointer(m_pos_loc, 3, GL_FLOAT, GL_FALSE, 0, vertices);
    glVertexAttribPointer(m_texcoord_loc, 2, GL_FLOAT, GL_FALSE, 0, texcoords);
    glDrawElements(GL_TRIANGLES, sizeof(indices) / sizeof(indices[0]), GL_UNSIGNED_SHORT, indices);

    // draw other web core
    glm::rotate(mvp, (float)glfwGetTime() * 0.1f, glm::vec3(0, 0, 1));
    glm::scale(mvp, glm::vec3(0.5, 0.5, 0.5));
    glUniformMatrix4fv(m_mvp_loc, 1, GL_FALSE, glm::value_ptr(mvp));
    glBindTexture(GL_TEXTURE_2D, m_web_core_other.lock()->render_handler()->texture());
    glDrawElements(GL_TRIANGLES, sizeof(indices) / sizeof(indices[0]), GL_UNSIGNED_SHORT, indices);

    return true;
}

// Séquence d'initialisation CEF / GL
// Lors de l'initialisation dans l'ordre CEF -> GL, les problèmes suivants peuvent survenir.
// * Initialisation du CEF réussie -> Échec de l'initialisation du GL -> Terminer le processus en cours -> Mais le CEF ne s'éteint pas correctement car il revient à un processus séparé
// Si l'initialisation est effectuée dans l'ordre GL -> CEF, les problèmes suivants peuvent survenir.
// * CEF crée un sous-processus à l'intérieur.
// * À moins que CefExecuteProcess ne soit appelé, il est impossible de savoir avec certitude si le processus actuel est un enfant ou un parent.
// Parmi les deux problèmes, je pensais que vérifier l'identité du processus CEF serait plus gênant, je l'initialise donc dans l'ordre CEF->GL
static void CEFsetUp(int argc, char** argv)
{
    CefMainArgs args(argc, argv);
    int exit_code = CefExecuteProcess(args, nullptr, nullptr);
    if (exit_code >= 0)
    {
        std::cerr << "CefExecuteProcess: child proccess has endend, so exit" << std::endl;
        exit(exit_code);
    }
    else if (exit_code == -1)
    {
        // we are here in the father proccess.
    }

    CefSettings settings;

    // When generating projects with CMake the CEF_USE_SANDBOX value will be defined
    // automatically. Pass -DUSE_SANDBOX=OFF to the CMake command-line to disable
    // use of the sandbox.
#if !defined(CEF_USE_SANDBOX)
    settings.no_sandbox = true;
#endif

    settings.windowless_rendering_enabled = true;
    // TODO CefString(&settings.locales_dir_path) = "cef/linux/lib/locales";
    bool result = CefInitialize(args, settings, nullptr, nullptr);
    if (!result)
    {
        std::cerr << "CefInitialize: failed" << std::endl;
        exit(-2);
    }
}

// CEF=/media/data/cef_binary/
// g++ --std=c++14 -W -Wall -Wextra -Wno-unused-parameter -DCEF_USE_SANDBOX -DNDEBUG -D_FILE_OFFSET_BITS=64 -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -I$CEF -I$CEF/include *.cpp -o cefopengl ./libcef.so ./libcef_dll_wrapper.a `pkg-config --cflags --libs glew --static glfw3`
int main(int argc, char *argv[])
{
    CEFsetUp(argc, argv);
    CEFGLWindow win(640, 480, "CEF OpenGL");
    int res = win.start();
    CefShutdown();
    return res;
}
