#pragma once
#include <memory>
#include <interfaces/api.hpp>
#include <program.hpp>

namespace bnb::converter
{

class yuv_converter
{
public:
    enum class standard
    {
        bt601,
        bt709
    };

    enum class range
    {
        full_range,
        video_range
    };

    enum class rotation
    {
        deg_0,
        deg_90,
        deg_180,
        deg_270
    };

public:
    yuv_converter(standard st = standard::bt601, range rng = range::video_range, rotation rot = rotation::deg_0);
    ~yuv_converter();

    void set_standard(standard st, range rng);
    void set_rotation(rotation rot);
    void convert(uint32_t gl_texture, int width, int height, data_t& output);
    int get_width();
    int get_height();

private:
    struct framebuffer
    {
        uint32_t fbo{0};
        uint32_t texture{0};
        int width{0};
        int height{0};
    };

private:
    framebuffer create_framebuffer(int width, int height);
    void delete_framebuffer(framebuffer& fbo);

private:
    uint32_t m_vbo {0};
    uint32_t m_vao {0};
    int m_width {0};
    int m_height {0};
    const float *m_y_plane_coefs {nullptr};
    const float *m_u_plane_coefs {nullptr};
    const float *m_v_plane_coefs {nullptr};
    const float *m_rot_mat {nullptr};
    framebuffer m_fbo_y;
    framebuffer m_fbo_uv;
    program m_shader;
    uniform m_pixel_step;
    uniform m_plane_coef;
    uniform m_in_texture;
    uniform m_rotate_mat;
};



/* yuv_converter::get_width */
inline int yuv_converter::get_width() {
    return m_width;
}

/* yuv_converter::get_height */
inline int yuv_converter::get_height() {
    return m_height;
}

} /* namespace bnb::converter */
