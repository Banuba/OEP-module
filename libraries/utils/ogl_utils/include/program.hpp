#pragma once
#include <iostream>
#include "uniform.hpp"

namespace bnb
{
    class program
    {
    public:
        program(const char* name, const char* vertex_shader_code, const char* fragmant_shader_code);
        ~program();

        uniform get_uniform(const char* uniform_name);

        void use() const;
        void unuse() const;

        unsigned int handle() const { return m_handle; }

    private:
        unsigned int m_handle;
    }; /* namespace bnb */
}
