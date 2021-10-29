#pragma once 

namespace bnb
{

class uniform
{
public:
    uniform() = default;
    uniform(unsigned int program_handle, const char* unifrom_name);
    ~uniform();

    void send_1i(const int val) const;
    void send_vec2f(const float* val) const;
    void send_vec4f(const float* val) const;
    void send_mat4f(const float* val) const;

    int handle() const;
    
private:
    int m_uniform{-1};
}; /* class uniform */



/* uniform::handle */
inline int uniform::handle() const
{
    return m_uniform;
}

} /* namespace bnb */
