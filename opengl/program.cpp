#include "program.hpp"

#include <sstream>

namespace bnb::oep
{

    program::program(const char* name, const char* vertex_shader_code, const char* fragmant_shader_code)
        : m_handle(0)
    {
        std::ostringstream vsc;
        vsc << BNB_GLSL_VERSION << std::endl;
        vsc << vertex_shader_code << std::endl;
        vsc.flush();

        std::ostringstream fsc;
        fsc << BNB_GLSL_VERSION << std::endl;
        fsc << fragmant_shader_code << std::endl;
        fsc.flush();

        std::string vsc_str = vsc.str();
        std::string fsc_str = fsc.str();

        const char* vsc_str_c = vsc_str.c_str();
        const char* fsc_str_c = fsc_str.c_str();

        int vertexShader = glCreateShader(GL_VERTEX_SHADER);
        GL_CALL(glShaderSource(vertexShader, 1, &vsc_str_c, NULL));
        GL_CALL(glCompileShader(vertexShader));

        // check for shader compile errors
        int success;
        char infoLog[512];
        GL_CALL(glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success));
        if (!success) {
            GL_CALL(glGetShaderInfoLog(vertexShader, 512, NULL, infoLog));
            throw std::runtime_error(infoLog);
        }

        // fragment shader
        int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        GL_CALL(glShaderSource(fragmentShader, 1, &fsc_str_c, NULL));
        GL_CALL(glCompileShader(fragmentShader));
        // check for shader compile errors
        GL_CALL(glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success));
        if (!success) {
            GL_CALL(glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog));
            throw std::runtime_error(infoLog);
        }

        // link shaders
        uint32_t shaderProgram = glCreateProgram();
        GL_CALL(glAttachShader(shaderProgram, vertexShader));
        GL_CALL(glAttachShader(shaderProgram, fragmentShader));
        GL_CALL(glLinkProgram(shaderProgram));
        // check for linking errors
        GL_CALL(glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success));
        if (!success) {
            glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
            throw std::runtime_error(infoLog);
        }
        GL_CALL(glDeleteShader(vertexShader));
        GL_CALL(glDeleteShader(fragmentShader));

        m_handle = shaderProgram;
    }

    program::~program()
    {
        GL_CALL(glDeleteProgram(m_handle));
    }

    void program::use() const
    {
        GL_CALL(glUseProgram(m_handle));
    }

    void program::unuse() const
    {
        GL_CALL(glUseProgram(0));
    }

    void program::set_uniform(const char* name, int32_t value) const
    {
        GL_CALL(glUniform1i(get_uniform_location(name), value));
    }

    void program::set_uniform(const char* name, float v1, float v2) const
    {
        GL_CALL(glUniform2f(get_uniform_location(name), v1, v2));
    }

    void program::set_uniform(const char* name, float v1, float v2, float v3, float v4) const
    {
        GL_CALL(glUniform4f(get_uniform_location(name), v1, v2, v3, v4));
    }

    uint32_t program::get_uniform_location(const char* name) const
    {
        GLint loc;
        auto it = m_uniforms.find(name);
        if (it != m_uniforms.end()) {
            loc = it->second;
        } else {
            GL_CALL(loc = glGetUniformLocation(m_handle, name));
            m_uniforms.emplace(name, loc);
        }

        return static_cast<uint32_t>(loc);
    }

    uint32_t program::handle() const
    {
        return m_handle;
    }

} /* namespace bnb::oep */
