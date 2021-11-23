#include "RenderHandler.hpp"

RenderHandler::RenderHandler()
    : m_width(2), m_height(2), m_tex(0)
{}

void RenderHandler::init()
{
    glGenTextures(1, &m_tex);
    glBindTexture(GL_TEXTURE_2D, m_tex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // dummy texture data - for debugging
    const unsigned char data[] = {
        255, 0, 0, 255,
        0, 255, 0, 255,
        0, 0, 255, 255,
        255, 255, 255, 255,
    };

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void RenderHandler::resize(int w, int h)
{
    m_width = w;
    m_height = h;
}

void RenderHandler::GetViewRect(CefRefPtr<CefBrowser> browser, CefRect &rect)
{
    rect = CefRect(0, 0, m_width, m_height);
}

void RenderHandler::OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type, const RectList &dirtyRects, const void *buffer, int width, int height)
{
    glBindTexture(GL_TEXTURE_2D, m_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, (unsigned char*)buffer);
    glBindTexture(GL_TEXTURE_2D, 0);
}
