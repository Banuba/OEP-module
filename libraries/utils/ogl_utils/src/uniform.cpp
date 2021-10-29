#include "opengl.hpp"
#include "uniform.hpp"
#include <string>
#include <cassert>

namespace bnb
{

/* uniform::uniform */
uniform::uniform(unsigned int program_handle, const char* unifrom_name)
{
    GL_CALL((m_uniform = glGetUniformLocation(program_handle, unifrom_name)));
    if (m_uniform == -1) {
        std::string s("uniform::uniform() error: Uniform name is incorrect:\n    ");
        s.append(unifrom_name);
        s.append("\n");
        throw std::runtime_error(s.c_str());
    }
}

/* uniform::~uniform */
uniform::~uniform()
{
    m_uniform = -1;
}

/* uniform::send_1i */
void uniform::send_1i(const int val) const
{
    assert(m_uniform != -1);
    GL_CALL(glUniform1i(m_uniform, val));
}

/* uniform::send_vec2f */
void uniform::send_vec2f(const float* val) const
{
    assert(m_uniform != -1);
    GL_CALL(glUniform2fv(m_uniform, 1, val));
}

/* uniform::send_vec4f */
void uniform::send_vec4f(const float* val) const
{
    assert(m_uniform != -1);
    GL_CALL(glUniform4fv(m_uniform, 1, val));
}

/* uniform::send_mat4f */
void uniform::send_mat4f(const float* val) const
{
    assert(m_uniform != -1);
    GL_CALL(glUniformMatrix4fv(m_uniform, 1, GL_FALSE, val));
}

} /* namespace bnb */
