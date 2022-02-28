#include "yuv_converter.hpp"
#include <string>

static const char* to_gl_check_framebuffer_status(GLenum e)
{
    switch (e) {
        case GL_FRAMEBUFFER_UNDEFINED:
            return "GL_FRAMEBUFFER_UNDEFINED";

        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
            return "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";

        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
            return "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";

        case GL_FRAMEBUFFER_UNSUPPORTED:
            return "GL_FRAMEBUFFER_UNSUPPORTED";

        case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
            return "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE";
    }
    return "UNKNOWN GLenum";
}

static void put_error_message(const char* where, const char* what)
{
    std::string msg;
    if (where) {
        msg.append(where);
        msg.append("\n");
    }
    if (what) {
        msg.append(what);
        msg.append("\n");
    }
    throw std::runtime_error(msg);
}

namespace bnb::oep::converter
{

    const int drawing_plane_vert_count = 4;

    const char* shader_vec_prog =
        "layout(location = 0) in vec3 in_vertex;\n"
        "layout(location = 1) in vec2 in_uv;\n"
        "out vec2 uv_coord;\n"
        "void main() {\n"
        "    uv_coord = in_uv;\n"
        "    gl_Position = vec4(in_vertex, 1.0);\n"
        "}\n";

    const char* shader_frag_prog =
        "layout (location = 0) out vec4 out_color;\n"
        "uniform vec2 pixel_step;\n"
        "uniform vec4 plane_coef;\n"
        "uniform sampler2D in_texture;\n"
        "in vec2 uv_coord;\n"
        "void main() {\n"
        "    float a = plane_coef.a;\n"
        "    vec3 rgb = plane_coef.rgb;\n"
        "    out_color.r = a + dot(rgb, texture(in_texture, uv_coord - 1.5 * pixel_step).rgb);\n"
        "    out_color.g = a + dot(rgb, texture(in_texture, uv_coord - 0.5 * pixel_step).rgb);\n"
        "    out_color.b = a + dot(rgb, texture(in_texture, uv_coord + 0.5 * pixel_step).rgb);\n"
        "    out_color.a = a + dot(rgb, texture(in_texture, uv_coord + 1.5 * pixel_step).rgb);\n"
        "}\n";


    /* yuv_converter::yuv_converter */
    yuv_converter::yuv_converter(standard st, range rng, rotation rot, bool vertical_flip, yuv_data_layout data_layout)
        : m_data_layout(data_layout), m_shader(nullptr, shader_vec_prog, shader_frag_prog)
    {
        constexpr const int drawing_plane_count = 8;
        constexpr const int drawing_plane_coords_per_vert = 5;
        // clang-format off
        static const float drawing_plane_coords[drawing_plane_coords_per_vert * drawing_plane_vert_count * drawing_plane_count] = {
            /* verical flip 0 rotation 0deg */
            1.0f,  1.0f, 0.0f, 1.0f, 0.0f,  /* top right */
            1.0f, -1.0f, 0.0f, 1.0f, 1.0f,  /* bottom right */
            -1.0f,  1.0f, 0.0f, 0.0f, 0.0f, /* top left */
            -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, /* bottom left */
            /* verical flip 0 rotation 90deg */
            1.0f,  1.0f, 0.0f, 0.0f, 0.0f,  /* top right */
            1.0f, -1.0f, 0.0f, 1.0f, 0.0f,  /* bottom right */
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f, /* top left */
            -1.0f, -1.0f, 0.0f, 1.0f, 1.0f, /* bottom left */
            /* verical flip 0 rotation 180deg */
            1.0f,  1.0f, 0.0f, 0.0f, 1.0f,  /* top right */
            1.0f, -1.0f, 0.0f, 0.0f, 0.0f,  /* bottom right */
            -1.0f,  1.0f, 0.0f, 1.0f, 1.0f, /* top left */
            -1.0f, -1.0f, 0.0f, 1.0f, 0.0f, /* bottom left */
            /* verical flip 0 rotation 270deg */
            1.0f,  1.0f, 0.0f, 1.0f, 1.0f,  /* top right */
            1.0f, -1.0f, 0.0f, 0.0f, 1.0f,  /* bottom right */
            -1.0f,  1.0f, 0.0f, 1.0f, 0.0f, /* top left */
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, /* bottom left */
            /* verical flip 1 rotation 0deg */
            1.0f, -1.0f, 0.0f, 1.0f, 0.0f,  /* top right */
            1.0f,  1.0f, 0.0f, 1.0f, 1.0f,  /* bottom right */
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, /* top left */
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f, /* bottom left */
            /* verical flip 1 rotation 90deg */
            1.0f, -1.0f, 0.0f, 1.0f, 1.0f,  /* top right */
            1.0f,  1.0f, 0.0f, 0.0f, 1.0f,  /* bottom right */
            -1.0f, -1.0f, 0.0f, 1.0f, 0.0f, /* top left */
            -1.0f,  1.0f, 0.0f, 0.0f, 0.0f, /* bottom left */
            /* verical flip 1 rotation 180deg */
            1.0f, -1.0f, 0.0f, 0.0f, 1.0f,  /* top right */
            1.0f,  1.0f, 0.0f, 0.0f, 0.0f,  /* bottom right */
            -1.0f, -1.0f, 0.0f, 1.0f, 1.0f, /* top left */
            -1.0f,  1.0f, 0.0f, 1.0f, 0.0f, /* bottom left */
            /* verical flip 1 rotation 270deg */
            1.0f, -1.0f, 0.0f, 0.0f, 0.0f,  /* top right */
            1.0f,  1.0f, 0.0f, 1.0f, 0.0f,  /* bottom right */
            -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, /* top left */
            -1.0f,  1.0f, 0.0f, 1.0f, 1.0f, /* bottom left */
        };
        // clang-format on

        set_convert_standard(st, rng);
        set_drawing_orientation(rot, vertical_flip);

        /* create and bind drawing geometry */
        glGenVertexArrays(1, &m_vao);
        glBindVertexArray(m_vao);
        glBindBuffer(GL_ARRAY_BUFFER, m_vao);
        glGenBuffers(1, &m_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(drawing_plane_coords), drawing_plane_coords, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * drawing_plane_coords_per_vert, nullptr);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * drawing_plane_coords_per_vert, reinterpret_cast<void*>(sizeof(float) * 3));
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    /* yuv_converter::~yuv_converter */
    yuv_converter::~yuv_converter()
    {
        delete_framebuffer(m_fbo);
        glDeleteBuffers(1, &m_vbo);
        glDeleteVertexArrays(1, &m_vao);
    }

    /* yuv_converter::set_convert_standard */
    void yuv_converter::set_convert_standard(standard st, range rng)
    {
        // clang-format off
        static const float mat_cvt_to_bt601_video_range[] {
            0.256788f, 0.504129f, 0.0979059f, 0.0627451f,
            -0.148223f, -0.290993f, 0.439216f, 0.501961f,
            0.439216f, -0.367788f, -0.0714274f, 0.501961f
        };
        static const float mat_cvt_to_bt601_full_range[] {
            0.299f,  0.587f,  0.114f, 0.0f,
            -0.1687358916f, -0.3312641084f,  0.5f,  0.5f,
            0.5f, -0.4186875892f, -0.08131241084f, 0.5f
        };
        static const float mat_cvt_to_bt709_video_range[] {
            0.1825858824f, 0.6142305882f, 0.06200705882f, 0.06274509804f,
            -0.1006437324f, -0.3385719539f, 0.4392156863f, 0.5019607843f,
            0.4392156863f, -0.3989421626f, -0.04027352368f, 0.5019607843f
        };
        static const float mat_cvt_to_bt709_full_range[] {
            0.2126f, 0.7152f, 0.0722f, 0.0f,
            -0.1145721061f, -0.3854278939f, 0.5f, 0.5019607843f,
            0.5f, -0.4541529083f, -0.04584709169f, 0.5019607843f
        };
        // clang-format on

        const float* cvt_mat{nullptr};
        switch (st) {
            case standard::bt601:
                switch (rng) {
                    case range::full_range:
                        cvt_mat = mat_cvt_to_bt601_full_range;
                        break;
                    case range::video_range:
                        cvt_mat = mat_cvt_to_bt601_video_range;
                        break;
                }
                break;
            case standard::bt709:
                switch (rng) {
                    case range::full_range:
                        cvt_mat = mat_cvt_to_bt709_full_range;
                        break;
                    case range::video_range:
                        cvt_mat = mat_cvt_to_bt709_video_range;
                        break;
                }
                break;
        }

        m_y_plane_coefs = cvt_mat;
        m_u_plane_coefs = cvt_mat + 4;
        m_v_plane_coefs = cvt_mat + 8;
    }

    /* yuv_converter::set_drawing_orientation */
    void yuv_converter::set_drawing_orientation(rotation rot, bool vertical_flip)
    {
        m_rotation = rot;
        m_vertical_flip = vertical_flip;
        m_draw_indent = ((vertical_flip ? 0x4 : 0) | static_cast<int32_t>(rot)) * drawing_plane_vert_count;
        update_pixel_steps();
    }

    /* yuv_converter::convert */
    void yuv_converter::convert(uint32_t gl_texture, int width, int height, yuv_converter::yuv_data& output)
    {
        /* create/recreate the framebuffer if necessary */
        int stride = (width + 7) & ~7;
        int half_height = (height + 1) / 2;
        int half_viewport_width = stride / 8;
        if (m_width != width || m_height != height) {
            if (width <= 0 || height <= 0) {
                return;
            }
            m_width = width;
            m_height = height;
            delete_framebuffer(m_fbo);
            switch (m_data_layout) {
                case yuv_data_layout::semi_planar_row_interleaved:
                    m_fbo = create_framebuffer(stride / 4, m_height + half_height);
                    break;
                case yuv_data_layout::planar_layout:
                    m_fbo = create_framebuffer(stride / 4, m_height + half_height * 2);
                    break;
            }
            update_pixel_steps();
        }

        /* allocate/reallocate memory if necessary */
        if (output.data == nullptr || output.size < calc_min_yuv_data_size(width, height)) {
            output.size = calc_min_yuv_data_size(width, height);
            output.data = std::shared_ptr<uint8_t>(new uint8_t[output.size], std::default_delete<uint8_t>());
        }

        /* just in case, disable dropping geometry */
        glDisable(GL_CULL_FACE);

        /* bind drawing geometry */
        glBindVertexArray(m_vao);

        /* use shader and send texture matrix to shader program */
        m_shader.use();

        /* In cases where blending was not turned off at the end of the effect */
        glDisable(GL_BLEND);

        /* bind input texture */
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gl_texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        m_shader.set_uniform("in_texture", 0);

        /* pixel step used in the shader to access neighboring pixels */
        m_shader.set_uniform("pixel_step", m_pixel_step_y[0], m_pixel_step_y[1]);
        /* render Y plane to the framebuffer*/
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo.fbo);
        m_shader.set_uniform("plane_coef", m_y_plane_coefs[0], m_y_plane_coefs[1], m_y_plane_coefs[2], m_y_plane_coefs[3]);
        glViewport(0, 0, m_fbo.width, m_height);
        glDrawArrays(GL_TRIANGLE_STRIP, m_draw_indent, drawing_plane_vert_count);

        /* pixel step used in the shader to access neighboring pixels */
        m_shader.set_uniform("pixel_step", m_pixel_step_uv[0], m_pixel_step_uv[1]);

        /* render U and V planes to the framebuffer */
        m_shader.set_uniform("plane_coef", m_u_plane_coefs[0], m_u_plane_coefs[1], m_u_plane_coefs[2], m_u_plane_coefs[3]);
        glViewport(0, m_height, half_viewport_width, half_height);
        glDrawArrays(GL_TRIANGLE_STRIP, m_draw_indent, drawing_plane_vert_count);
        m_shader.set_uniform("plane_coef", m_v_plane_coefs[0], m_v_plane_coefs[1], m_v_plane_coefs[2], m_v_plane_coefs[3]);
        switch (m_data_layout) {
            case yuv_data_layout::semi_planar_row_interleaved:
                glViewport(half_viewport_width, m_height, half_viewport_width, half_height);
                break;
            case yuv_data_layout::planar_layout:
                glViewport(0, m_height + half_height, half_viewport_width, half_height);
                break;
        }
        glDrawArrays(GL_TRIANGLE_STRIP, m_draw_indent, drawing_plane_vert_count);
        /* and read all YUV planes data */
        glReadPixels(0, 0, m_fbo.width, m_fbo.height, GL_RGBA, GL_UNSIGNED_BYTE, output.data.get());

        /* unbind all */
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
        m_shader.unuse();

        output.y_plane_data = output.data.get();
        output.u_plane_data = output.data.get() + stride * m_height;
        switch (m_data_layout) {
            case yuv_data_layout::semi_planar_row_interleaved:
                output.v_plane_data = output.data.get() + stride * m_height + half_viewport_width * 4;
                break;
            case yuv_data_layout::planar_layout:
                output.v_plane_data = output.data.get() + stride * m_height + stride * half_height;
                break;
        }
        output.y_plane_stride = stride;
        output.u_plane_stride = stride;
        output.v_plane_stride = stride;
    }

    /* yuv_converter::calc_min_yuv_data_size */
    size_t yuv_converter::calc_min_yuv_data_size(int width, int height)
    {
        auto stride = (width + 7) & ~7;
        switch (m_data_layout) {
            case yuv_data_layout::semi_planar_row_interleaved:
                return stride * (height + (height + 1) / 2);
            case yuv_data_layout::planar_layout:
                return stride * ((height + 1) & ~1) * 2;
        }
    }

    /* yuv_converter::update_pixel_steps */
    void yuv_converter::update_pixel_steps()
    {
        if (m_width == 0) {
            return;
        }
        float x_step{0.0f};
        float y_step{0.0f};
        switch (m_rotation) {
            case rotation::deg_0:
                x_step = 1.0f / m_width;
                break;
            case rotation::deg_90:
                y_step = (m_vertical_flip ? 1.0f : -1.0f) / m_width;
                break;
            case rotation::deg_180:
                x_step = -1.0f / m_width;
                break;
            case rotation::deg_270:
                y_step = (m_vertical_flip ? -1.0f : 1.0f) / m_width;
                break;
        }
        m_pixel_step_y[0] = x_step;
        m_pixel_step_y[1] = y_step;
        m_pixel_step_uv[0] = x_step * 2.0f;
        m_pixel_step_uv[1] = y_step * 2.0f;
    }

    /* yuv_converter::create_framebuffer */
    yuv_converter::framebuffer yuv_converter::create_framebuffer(int width, int height)
    {
        uint32_t fbo;
        uint32_t tex;
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
        uint32_t attach[]{GL_COLOR_ATTACHMENT0};
        glDrawBuffers(1, attach);

        glBindTexture(GL_TEXTURE_2D, 0);
        if (GLenum e = glCheckFramebufferStatus(GL_FRAMEBUFFER); e != GL_FRAMEBUFFER_COMPLETE) {
            put_error_message(
                "create_framebuffer() error: glCheckFramebufferStatus(GL_FRAMEBUFFER)"
                " != GL_FRAMEBUFFER_COMPLETE",
                to_gl_check_framebuffer_status(e));
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return {fbo, tex, width, height};
    }

    /* yuv_converter::delete_framebuffer */
    void yuv_converter::delete_framebuffer(yuv_converter::framebuffer& fbo)
    {
        if (fbo.texture) {
            glDeleteTextures(1, &fbo.texture);
        }
        if (fbo.fbo) {
            glDeleteFramebuffers(1, &fbo.fbo);
        }
        fbo = {0, 0, 0, 0};
    }

} /* namespace bnb::oep::converter */
