#include "main.hpp"
#include "GLCore.hpp"

static void reshape_callback(GLFWwindow* ptr, int w, int h)
{
    assert(nullptr != ptr);
    CEFGLWindow* window = static_cast<CEFGLWindow*>(glfwGetWindowUserPointer(ptr));

    window->m_web_core.lock()->reshape(w, h);
    window->m_web_core_other.lock()->reshape(w, h);
    GLCHECK(glViewport(0, 0, (GLsizei)w, (GLsizei)h));
}

static void mouse_callback(GLFWwindow* ptr, int btn, int state, int mods)
{
    assert(nullptr != ptr);
    CEFGLWindow* window = static_cast<CEFGLWindow*>(glfwGetWindowUserPointer(ptr));

    // send mouse click to browser
    CefBrowserHost::MouseButtonType b = CefBrowserHost::MouseButtonType(btn);
    window->m_web_core.lock()->mouseClick(b, GLFW_PRESS);
    window->m_web_core.lock()->mouseClick(b, GLFW_RELEASE);
    window->m_web_core_other.lock()->mouseClick(b, GLFW_PRESS);
    window->m_web_core_other.lock()->mouseClick(b, GLFW_RELEASE);
}

static void motion_callback(GLFWwindow* ptr, double x, double y)
{
    assert(nullptr != ptr);
    CEFGLWindow* window = static_cast<CEFGLWindow*>(glfwGetWindowUserPointer(ptr));

    // send mouse movement to browser
    window->m_web_core.lock()->mouseMove((int) x, (int) y);
    window->m_web_core_other.lock()->mouseMove((int) x, (int) y);
}

static void keyboard_callback(GLFWwindow* ptr, int key, int scancode, int action, int mods)
{
    assert(nullptr != ptr);
    CEFGLWindow* window = static_cast<CEFGLWindow*>(glfwGetWindowUserPointer(ptr));

    // send key press to browser
    window->m_web_core.lock()->keyPress(key, true);
    window->m_web_core_other.lock()->keyPress(key, true);
}

RenderHandler::~RenderHandler()
{
    GLCore::deleteProgram(m_prog);
}

bool RenderHandler::init()
{
    // Dummy texture data - for debugging
    const unsigned char data[] = {
        255, 0, 0, 255,
        0, 255, 0, 255,
        0, 0, 255, 255,
        255, 255, 255, 255,
    };

    // Compile shader
    m_prog = GLCore::createShaderProgram("shaders/tex.vert", "shaders/tex.frag");
    if (m_prog == 0)
    {
        std::cerr << "shader compile failed" << std::endl;
        return false;
    }

    // Location of shader variables
    m_pos_loc = GLCHECK(glGetAttribLocation(m_prog, "position"));
    m_tex_loc = GLCHECK(glGetUniformLocation(m_prog, "tex"));
    //m_mvp_loc = GLCHECK(glGetUniformLocation(m_prog, "mvp");

    float coords[] = {-1.0,-1.0,-1.0,1.0,1.0,-1.0,1.0,-1.0,-1.0,1.0,1.0,1.0};

    GLCHECK(glGenVertexArrays(1, &m_vao));
    GLCHECK(glBindVertexArray(m_vao));
    GLCHECK(glGenBuffers(1, &m_vbo));
    GLCHECK(glBindBuffer(GL_ARRAY_BUFFER, m_vbo));
    GLCHECK(glBufferData(GL_ARRAY_BUFFER, sizeof(coords), coords, GL_STATIC_DRAW));
    GLCHECK(glEnableVertexAttribArray(m_pos_loc));
    GLCHECK(glVertexAttribPointer(m_pos_loc, 2, GL_FLOAT, GL_FALSE, 0, 0));

    GLCHECK(glGenTextures(1, &m_tex));
    GLCHECK(glBindTexture(GL_TEXTURE_2D, m_tex));
    GLCHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    GLCHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GLCHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    GLCHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    GLCHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, data));
    GLCHECK(glBindTexture(GL_TEXTURE_2D, 0));

    GLCHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));
    GLCHECK(glBindVertexArray(0));
    m_initialized = true;

    return false;
}

void RenderHandler::draw()
{
    GLCHECK(glUseProgram(m_prog));
    GLCHECK(glBindVertexArray(m_vao));

    GLCHECK(glBindBuffer(GL_ARRAY_BUFFER, m_vbo));
    GLCHECK(glActiveTexture(GL_TEXTURE0));
    GLCHECK(glBindTexture(GL_TEXTURE_2D, m_tex));
    GLCHECK(glDrawArrays(GL_TRIANGLES, 0, 6));
    GLCHECK(glBindTexture(GL_TEXTURE_2D, 0));
    GLCHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));

    GLCHECK(glBindVertexArray(0));
    GLCHECK(glUseProgram(0));
}

void RenderHandler::reshape(int w, int h)
{
    m_width = w;
    m_height = h;
}

void RenderHandler::GetViewRect(CefRefPtr<CefBrowser> browser, CefRect &rect)
{
    rect = CefRect(0, 0, m_width, m_height);
}

void RenderHandler::OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type,
                            const RectList &dirtyRects, const void *buffer,
                            int width, int height)
{
    GLCHECK(glActiveTexture(GL_TEXTURE0));
    GLCHECK(glBindTexture(GL_TEXTURE_2D, m_tex));
    GLCHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA_EXT,
                         GL_UNSIGNED_BYTE, (unsigned char*)buffer));
    GLCHECK(glBindTexture(GL_TEXTURE_2D, 0));
}

WebCore::WebCore(const std::string &url)
    : m_mouse_x(0), m_mouse_y(0)
{
    CefWindowInfo window_info;
    window_info.SetAsWindowless(0);

    m_render_handler = new RenderHandler();
    m_render_handler->init();
    m_render_handler->reshape(128, 128); // initial size

    CefBrowserSettings browserSettings;
    browserSettings.windowless_frame_rate = 60; // 30 is default

    m_client = new BrowserClient(m_render_handler);
    m_browser = CefBrowserHost::CreateBrowserSync(window_info, m_client.get(),
                                                  url, browserSettings,
                                                  nullptr, nullptr);
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
    if ((m_render_handler == nullptr) ||
        (m_render_handler->m_initialized == false))
    {
        m_render_handler->init();
    }

    m_browser->GetMainFrame()->LoadURL(url);
}

void WebCore::draw()
{
    CefDoMessageLoopWork();
    m_render_handler->draw();
}

void WebCore::reshape(int w, int h)
{
    m_render_handler->reshape(w, h);
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

CEFGLWindow::CEFGLWindow(uint32_t const width, uint32_t const height, const char *title)
    : GLWindow(width, height, title)
{
//std::cout << __PRETTY_FUNCTION__ << std::endl;
}

CEFGLWindow::~CEFGLWindow()
{
    m_browsers.clear();
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
    // CEF
    std::string url = "http://google.com";
    m_web_core = createBrowser(url);
    m_web_core.lock()->reshape(m_width, m_height);

    std::string other_url = "https://github.com/Lecrapouille?tab=repositories";
    m_web_core_other = createBrowser(other_url);
    m_web_core_other.lock()->reshape(m_width, m_height);

    // Windows events
    GLCHECK(glfwSetFramebufferSizeCallback(m_window, reshape_callback));
    GLCHECK(glfwSetKeyCallback(m_window, keyboard_callback));
    GLCHECK(glfwSetCursorPosCallback(m_window, motion_callback));
    GLCHECK(glfwSetMouseButtonCallback(m_window, mouse_callback));

    // OpenGL states
    GLCHECK(glViewport(0, 0, m_width, m_height));
    GLCHECK(glClearColor(0.0, 0.0, 0.0, 0.0));
    //GLCHECK(glEnable(GL_CULL_FACE));
    GLCHECK(glEnable(GL_DEPTH_TEST));
    GLCHECK(glDepthFunc(GL_LESS));
    GLCHECK(glDisable(GL_BLEND));
    GLCHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

    return true;
}

bool CEFGLWindow::update()
{
    GLCHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    m_web_core.lock()->draw();
    m_web_core_other.lock()->draw();

    // CEF
    CefDoMessageLoopWork();
    return true;
}

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

    // When generating projects with CMake the CEF_USE_SANDBOX value will be
    // defined automatically. Pass -DUSE_SANDBOX=OFF to the CMake command-line
    // to disable use of the sandbox.
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

int main(int argc, char *argv[])
{
    CEFsetUp(argc, argv);

    CEFGLWindow win(640, 480, "CEF OpenGL");
    int res = win.start();

    CefShutdown();
    return res;
}
