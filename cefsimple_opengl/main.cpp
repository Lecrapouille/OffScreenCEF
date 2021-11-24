#include "main.hpp"
#include "GLCore.hpp"

//------------------------------------------------------------------------------
//! \brief Callback when the OpenGL base window has been resized. Dispatch this
//! event to all BrowserView.
//------------------------------------------------------------------------------
static void reshape_callback(GLFWwindow* ptr, int w, int h)
{
    assert(nullptr != ptr);
    CEFGLWindow* window = static_cast<CEFGLWindow*>(glfwGetWindowUserPointer(ptr));

    // Send screen size to browsers
    for (auto it: window->browsers())
    {
        it->reshape(w, h);
    }
}

//------------------------------------------------------------------------------
//! \brief Callback when the mouse has clicked inside the OpenGL base window.
//! Dispatch this event to all BrowserView.
//------------------------------------------------------------------------------
static void mouse_callback(GLFWwindow* ptr, int btn, int state, int /*mods*/)
{
    assert(nullptr != ptr);
    CEFGLWindow* window = static_cast<CEFGLWindow*>(glfwGetWindowUserPointer(ptr));

    // Send mouse click to browsers
    for (auto it: window->browsers())
    {
        it->mouseClick(CefBrowserHost::MouseButtonType(btn), state == GLFW_PRESS);
    }
}

//------------------------------------------------------------------------------
//! \brief Callback when the mouse has been displaced inside the OpenGL base
//! window. Dispatch this event to all BrowserView.
//------------------------------------------------------------------------------
static void motion_callback(GLFWwindow* ptr, double x, double y)
{
    assert(nullptr != ptr);
    CEFGLWindow* window = static_cast<CEFGLWindow*>(glfwGetWindowUserPointer(ptr));

    // Send mouse movement to browsers
    for (auto it: window->browsers())
    {
        it->mouseMove((int) x, (int) y);
    }
}

//------------------------------------------------------------------------------
//! \brief Callback when the keybaord has been pressed inside the OpenGL base
//! window. Dispatch this event to all BrowserView.
//------------------------------------------------------------------------------
static void keyboard_callback(GLFWwindow* ptr, int key, int /*scancode*/,
                              int action, int /*mods*/)
{
    assert(nullptr != ptr);
    CEFGLWindow* window = static_cast<CEFGLWindow*>(glfwGetWindowUserPointer(ptr));

    // Send key press to browsers
    for (auto it: window->browsers())
    {
        it->keyPress(key, (action == GLFW_PRESS));
    }
}

//------------------------------------------------------------------------------
BrowserView::RenderHandler::RenderHandler(glm::vec4 const& viewport)
    : m_viewport(viewport)
{}

//------------------------------------------------------------------------------
BrowserView::RenderHandler::~RenderHandler()
{
    // Free GPU memory
    GLCore::deleteProgram(m_prog);
    glDeleteBuffers(1, &m_vbo);
    glDeleteVertexArrays(1, &m_vao);
}

//------------------------------------------------------------------------------
bool BrowserView::RenderHandler::init()
{
    // Dummy texture data
    const unsigned char data[] = {
        255, 0, 0, 255,
        0, 255, 0, 255,
        0, 0, 255, 255,
        255, 255, 255, 255,
    };

    // Compile vertex and fragment shaders
    m_prog = GLCore::createShaderProgram("shaders/tex.vert", "shaders/tex.frag");
    if (m_prog == 0)
    {
        std::cerr << "shader compile failed" << std::endl;
        return false;
    }

    // Get locations of shader variables (attributes and uniforms)
    m_pos_loc = GLCHECK(glGetAttribLocation(m_prog, "position"));
    m_tex_loc = GLCHECK(glGetUniformLocation(m_prog, "tex"));
    m_mvp_loc = GLCHECK(glGetUniformLocation(m_prog, "mvp"))

    // Square vertices (texture positions are computed directly inside the shader)
    float coords[] = {-1.0,-1.0,-1.0,1.0,1.0,-1.0,1.0,-1.0,-1.0,1.0,1.0,1.0};

    // See https://learnopengl.com/Getting-started/Textures
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

    return true;
}

//------------------------------------------------------------------------------
void BrowserView::RenderHandler::draw(glm::vec4 const& viewport, bool fixed)
{
    // Where to paint on the OpenGL window
    GLCHECK(glViewport(viewport[0],
                       viewport[1],
                       GLsizei(viewport[2] * m_width),
                       GLsizei(viewport[3] * m_height)));

    // Apply a rotation
    glm::mat4 trans = glm::mat4(1.0f); // Identity matrix
    if (!fixed)
    {
        trans = glm::translate(trans, glm::vec3(0.5f, -0.5f, 0.0f));
        trans = glm::rotate(trans, (float)glfwGetTime() / 5.0f, glm::vec3(0.0f, 0.0f, 1.0f));
    }

    // See https://learnopengl.com/Getting-started/Textures
    GLCHECK(glUseProgram(m_prog));
    GLCHECK(glBindVertexArray(m_vao));

    GLCHECK(glUniformMatrix4fv(m_mvp_loc, 1, GL_FALSE, glm::value_ptr(trans)));
    GLCHECK(glBindBuffer(GL_ARRAY_BUFFER, m_vbo));
    GLCHECK(glActiveTexture(GL_TEXTURE0));
    GLCHECK(glBindTexture(GL_TEXTURE_2D, m_tex));
    GLCHECK(glDrawArrays(GL_TRIANGLES, 0, 6));
    GLCHECK(glBindTexture(GL_TEXTURE_2D, 0));
    GLCHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));

    GLCHECK(glBindVertexArray(0));
    GLCHECK(glUseProgram(0));
}

//------------------------------------------------------------------------------
void BrowserView::RenderHandler::reshape(int w, int h)
{
    m_width = w;
    m_height = h;
}

bool BrowserView::viewport(float x, float y, float w, float h)
{
    if (!(x >= 0.0f) && (x < 1.0f))
        return false;

    if (!(x >= 0.0f) && (y < 1.0f))
        return false;

    if (!(w > 0.0f) && (w <= 1.0f))
        return false;

    if (!(h > 0.0f) && (h <= 1.0f))
        return false;

    if (x + w > 1.0f)
        return false;

    if (y + h > 1.0f)
        return false;

    m_viewport[0] = x;
    m_viewport[1] = y;
    m_viewport[2] = w;
    m_viewport[3] = h;

    return true;
}

//------------------------------------------------------------------------------
void BrowserView::RenderHandler::GetViewRect(CefRefPtr<CefBrowser> browser, CefRect &rect)
{
    rect = CefRect(m_viewport[0], m_viewport[1], m_viewport[2] * m_width, m_viewport[3] * m_height);
}

//------------------------------------------------------------------------------
void BrowserView::RenderHandler::OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type,
                            const RectList &dirtyRects, const void *buffer,
                            int width, int height)
{
    GLCHECK(glActiveTexture(GL_TEXTURE0));
    GLCHECK(glBindTexture(GL_TEXTURE_2D, m_tex));
    GLCHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA_EXT,
                         GL_UNSIGNED_BYTE, (unsigned char*)buffer));
    GLCHECK(glBindTexture(GL_TEXTURE_2D, 0));
}

//------------------------------------------------------------------------------
BrowserView::BrowserView(const std::string &url)
    : m_mouse_x(0), m_mouse_y(0), m_viewport(0.0f, 0.0f, 1.0f, 1.0f)
{
    CefWindowInfo window_info;
    window_info.SetAsWindowless(0);

    m_render_handler = new RenderHandler(m_viewport);
    m_initialized = m_render_handler->init();
    m_render_handler->reshape(128, 128); // initial size

    CefBrowserSettings browserSettings;
    browserSettings.windowless_frame_rate = 60; // 30 is default

    m_client = new BrowserClient(m_render_handler);
    m_browser = CefBrowserHost::CreateBrowserSync(window_info, m_client.get(),
                                                  url, browserSettings,
                                                  nullptr, nullptr);
}

//------------------------------------------------------------------------------
BrowserView::~BrowserView()
{
    CefDoMessageLoopWork();
    m_browser->GetHost()->CloseBrowser(true);

    m_browser = nullptr;
    m_client = nullptr;
}

//------------------------------------------------------------------------------
void BrowserView::load(const std::string &url)
{
    assert(m_initialized);
    m_browser->GetMainFrame()->LoadURL(url);
}

//------------------------------------------------------------------------------
void BrowserView::draw()
{
    CefDoMessageLoopWork();
    m_render_handler->draw(m_viewport, m_fixed);
}

//------------------------------------------------------------------------------
void BrowserView::reshape(int w, int h)
{
    m_render_handler->reshape(w, h);
    GLCHECK(glViewport(m_viewport[0],
                       m_viewport[1],
                       GLsizei(m_viewport[2] * w),
                       GLsizei(m_viewport[3] * h)));
    m_browser->GetHost()->WasResized();
}

//------------------------------------------------------------------------------
void BrowserView::mouseMove(int x, int y)
{
    m_mouse_x = x;
    m_mouse_y = y;

    CefMouseEvent evt;
    evt.x = x;
    evt.y = y;

    bool mouse_leave = false; // TODO
    m_browser->GetHost()->SendMouseMoveEvent(evt, mouse_leave);
}

//------------------------------------------------------------------------------
void BrowserView::mouseClick(CefBrowserHost::MouseButtonType btn, bool mouse_up)
{
    CefMouseEvent evt;
    evt.x = m_mouse_x;
    evt.y = m_mouse_y;

    int click_count = 1; // TODO
    m_browser->GetHost()->SendMouseClickEvent(evt, btn, mouse_up, click_count);
}

//------------------------------------------------------------------------------
void BrowserView::keyPress(int key, bool pressed)
{
    CefKeyEvent evt;
    evt.character = key;
    evt.native_key_code = key;
    evt.type = pressed ? KEYEVENT_CHAR : KEYEVENT_KEYUP;

    m_browser->GetHost()->SendKeyEvent(evt);
}

//------------------------------------------------------------------------------
CEFGLWindow::CEFGLWindow(uint32_t const width, uint32_t const height, const char *title)
    : GLWindow(width, height, title)
{
    std::cout << __PRETTY_FUNCTION__ << std::endl;
}

//------------------------------------------------------------------------------
CEFGLWindow::~CEFGLWindow()
{
    m_browsers.clear();
    CefShutdown();
}

//------------------------------------------------------------------------------
std::weak_ptr<BrowserView> CEFGLWindow::createBrowser(const std::string &url)
{
    auto web_core = std::make_shared<BrowserView>(url);
    m_browsers.push_back(web_core);
    return web_core;
}

//------------------------------------------------------------------------------
void CEFGLWindow::removeBrowser(std::weak_ptr<BrowserView> web_core)
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

//------------------------------------------------------------------------------
bool CEFGLWindow::setup()
{
    std::vector<std::string> urls = {
        "https://youtu.be/rRr19a08mHs",
        "https://www.researchgate.net/profile/Philip-Polack/publication/318810853_The_kinematic_bicycle_model_A_consistent_model_for_planning_feasible_trajectories_for_autonomous_vehicles/links/5addcbc2a6fdcc29358b9c01/The-kinematic-bicycle-model-A-consistent-model-for-planning-feasible-trajectories-for-autonomous-vehicles.pdf",
        //"https://www.youtube.com/"
    };

    // Create BrowserView
    for (auto const& url: urls)
    {
        auto browser = createBrowser(url);
        browser.lock()->reshape(m_width, m_height);
    }

    // Change viewports (vertical split)
    m_browsers[0]->viewport(0.0f, 0.0f, 0.5f, 1.0f);
    m_browsers[1]->viewport(0.5f, 0.0f, 1.0f, 1.0f);

    // Do rotation animation
    m_browsers[1]->m_fixed = false;

    // Windows events
    GLCHECK(glfwSetFramebufferSizeCallback(m_window, reshape_callback));
    GLCHECK(glfwSetKeyCallback(m_window, keyboard_callback));
    GLCHECK(glfwSetCursorPosCallback(m_window, motion_callback));
    GLCHECK(glfwSetMouseButtonCallback(m_window, mouse_callback));

    // Set OpenGL states
    //GLCHECK(glViewport(0, 0, m_width, m_height));
    GLCHECK(glClearColor(0.0, 0.0, 0.0, 0.0));
    //GLCHECK(glEnable(GL_CULL_FACE));
    GLCHECK(glEnable(GL_DEPTH_TEST));
    GLCHECK(glDepthFunc(GL_LESS));
    GLCHECK(glDisable(GL_BLEND));
    GLCHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

    return true;
}

//------------------------------------------------------------------------------
bool CEFGLWindow::update()
{
    GLCHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    for (auto it: m_browsers)
    {
        it->draw();
    }

    CefDoMessageLoopWork();
    return true;
}

//------------------------------------------------------------------------------
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

    // Configurate Chromium
    CefSettings settings;
    // TODO CefString(&settings.locales_dir_path) = "cef/linux/lib/locales";
    settings.windowless_rendering_enabled = true;
#if !defined(CEF_USE_SANDBOX)
    settings.no_sandbox = true;
#endif

    bool result = CefInitialize(args, settings, nullptr, nullptr);
    if (!result)
    {
        std::cerr << "CefInitialize: failed" << std::endl;
        exit(-2);
    }
}

//------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    CEFsetUp(argc, argv);

    CEFGLWindow win(800, 600, "CEF OpenGL");
    int res = win.start();

    return res;
}
