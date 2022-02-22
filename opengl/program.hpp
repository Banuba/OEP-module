#pragma once

#include <iostream>
#include <unordered_map>

#include "opengl.hpp"

namespace bnb::oep
{

    class program
    {
    public:
        program(const char* name, const char* vertex_shader_code, const char* fragmant_shader_code);
        ~program();

        void use() const;
        void unuse() const;

        void set_uniform(const char* name, int32_t value) const;
        void set_uniform(const char* name, float v1, float v2) const;
        void set_uniform(const char* name, float v1, float v2, float v3, float v4) const;

        uint32_t get_uniform_location(const char* name) const;
        uint32_t handle() const;

    private:
        uint32_t m_handle;
        mutable std::unordered_map<const void*, uint32_t> m_uniforms;
    }; /* class bnb::oep::program */

} /* namespace bnb::oep */
