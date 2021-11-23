#include <GL/glew.h>

class GLCore
{
public:

    static GLuint compileShaderFromCode(GLenum shader_type, const char *src);
    static GLuint compileShaderFromFile(GLenum shader_type, const char *filepath);
    static GLuint createShaderProgram(const char *vert, const char *frag);
    static GLuint createShaderProgram(GLuint vert, GLuint frag);
    static bool deleteShader(GLuint shader);
    static bool deleteProgram(GLuint program);
};
