#include "offscreen_render_target.hpp"

namespace bnb::oep
{
    const int drawing_plane_vert_count = 4;
    const int drawing_plane_count = 4;
    const int drawing_plane_coords_per_vert = 5;
    // clang-format off
        static const float drawing_plane_coords[drawing_plane_coords_per_vert * drawing_plane_vert_count * drawing_plane_count] = {
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

    const char* shader_vec_prog =
        " precision highp float; \n "
        " layout (location = 0) in vec3 aPos; \n"
        " layout (location = 1) in vec2 aTexCoord; \n"
        "out vec2 vTexCoord;\n"
        "void main()\n"
        "{\n"
        " gl_Position = vec4(aPos, 1.0); \n"
        " vTexCoord = aTexCoord; \n"
        "}\n";

    const char* shader_frag_prog =
        "precision highp float;\n"
        "in vec2 vTexCoord;\n"
        "out vec4 FragColor;\n"
        "uniform sampler2D uTexture;\n"
        "void main()\n"
        "{\n"
        "FragColor = texture(uTexture, vTexCoord);\n"
        "}\n";

    /* interfaces::offscreen_render_target::create */
    offscreen_render_target_sptr bnb::oep::interfaces::offscreen_render_target::create(render_context_sptr rc)
    {
        return offscreen_render_target_sptr(new bnb::oep::offscreen_render_target(rc));
    }

    /* offscreen_render_target::offscreen_render_target */
    offscreen_render_target::offscreen_render_target(render_context_sptr rc)
        : m_rc(rc)
    {
    }

    /* offscreen_render_target::~offscreen_render_target */
    offscreen_render_target::~offscreen_render_target()
    {
        deinit();
    }

    /* offscreen_render_target::init */
    void offscreen_render_target::init(int32_t width, int32_t height)
    {
        m_width = width;
        m_height = height;

        std::call_once(m_init_flag, [this]() {
            m_rc->create_context();
            activate_context();
            m_shader = std::make_unique<program>(nullptr, shader_vec_prog, shader_frag_prog);

            /* create and bind drawing geometry */
            GL_CALL(glGenVertexArrays(1, &m_vao));
            GL_CALL(glBindVertexArray(m_vao));
            GL_CALL(glGenBuffers(1, &m_vbo));
            GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, m_vbo));
            GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(drawing_plane_coords), drawing_plane_coords, GL_STATIC_DRAW));
            GL_CALL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * drawing_plane_coords_per_vert, nullptr));
            GL_CALL(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * drawing_plane_coords_per_vert, reinterpret_cast<void*>(sizeof(float) * 3)));
            GL_CALL(glEnableVertexAttribArray(0));
            GL_CALL(glEnableVertexAttribArray(1));
            GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
            GL_CALL(glBindVertexArray(0));

            GL_CALL(glGenFramebuffers(1, &m_framebuffer));
            GL_CALL(glGenFramebuffers(1, &m_post_processing_framebuffer));
            deactivate_context();
        });
    }

    /* offscreen_render_target::deinit */
    void offscreen_render_target::deinit()
    {
        std::call_once(m_deinit_flag, [this]() {
            activate_context();
            if (glIsBuffer(m_vbo)) {
                glDeleteBuffers(1, &m_vbo);
                m_vbo = 0;
            }
            if (glIsVertexArray(m_vao)) {
                glDeleteVertexArrays(1, &m_vao);
                m_vao = 0;
            }
            if (glIsFramebuffer(m_framebuffer)) {
                GL_CALL(glDeleteFramebuffers(1, &m_framebuffer));
                m_framebuffer = 0;
            }
            if (glIsFramebuffer(m_post_processing_framebuffer)) {
                GL_CALL(glDeleteFramebuffers(1, &m_post_processing_framebuffer));
                m_post_processing_framebuffer = 0;
            }
            delete_textures();
            deactivate_context();
        });
    }

    /* offscreen_render_target::surface_changed */
    void offscreen_render_target::surface_changed(int32_t width, int32_t height)
    {
        m_width = width;
        m_height = height;
        activate_context();
        delete_textures();
        deactivate_context();
    }

    /* offscreen_render_target::activate_context */
    void offscreen_render_target::activate_context()
    {
        m_rc->activate();
    }

    /* offscreen_render_target::deactivate_context */
    void offscreen_render_target::deactivate_context()
    {
        m_rc->deactivate();
    }

    /* offscreen_render_target::prepare_rendering */
    void offscreen_render_target::prepare_rendering()
    {
        if (m_offscreen_render_texture == 0) {
            generate_texture(m_offscreen_render_texture);
        }

        GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer));
        GL_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_offscreen_render_texture, 0));

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
            std::cout << "[ERROR] Failed to make complete framebuffer object " << status << std::endl;
            return;
        }
        m_active_texture = m_offscreen_render_texture;
        m_last_framebuffer = m_offscreen_render_texture;
    }

    /* offscreen_render_target::orient_image */
    void offscreen_render_target::orient_image(bnb::oep::interfaces::rotation orient)
    {
        GL_CALL(glFlush());

        if (orient == bnb::oep::interfaces::rotation::deg0) {
            return;
        }

        using ns = bnb::oep::interfaces::rotation;
        int32_t draw_indent{0};
        switch (orient) {
            case ns::deg0:
                draw_indent = 0 * drawing_plane_vert_count;
                break;
            case ns::deg90:
                draw_indent = 1 * drawing_plane_vert_count;
                break;
            case ns::deg180:
                draw_indent = 2 * drawing_plane_vert_count;
                break;
            case ns::deg270:
                draw_indent = 3 * drawing_plane_vert_count;
                break;
            default:
                break;
        }

        prepare_post_processing_rendering();
        m_shader->use();
        /* bind drawing geometry */
        glBindVertexArray(m_vao);
        glDrawArrays(GL_TRIANGLE_STRIP, draw_indent, drawing_plane_vert_count);
        m_shader->unuse();

        GL_CALL(glFlush());
    }

    /* offscreen_render_target::read_current_buffer */
    pixel_buffer_sptr offscreen_render_target::read_current_buffer(bnb::oep::interfaces::image_format format)
    {
        activate_context();

        using ns = bnb::oep::interfaces::image_format;
        switch (format) {
            case ns::bpc8_rgb:
            case ns::bpc8_bgr:
            case ns::bpc8_rgba:
            case ns::bpc8_bgra:
            case ns::bpc8_argb:
                return read_current_buffer_bpc8(format);
                break;
            case ns::i420_bt601_full:
            case ns::i420_bt601_video:
            case ns::i420_bt709_full:
            case ns::i420_bt709_video:
                return read_current_buffer_i420(format);
                break;
            default:
                return nullptr;
        }
    }

    /* offscreen_render_target::get_current_buffer_texture */
    rendered_texture_t offscreen_render_target::get_current_buffer_texture()
    {
        return reinterpret_cast<rendered_texture_t>(m_active_texture);
    }

    /* offscreen_render_target::generate_texture */
    void offscreen_render_target::generate_texture(GLuint& texture)
    {
        GL_CALL(glGenTextures(1, &texture));
        GL_CALL(glBindTexture(GL_TEXTURE_2D, texture));
        GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL));

        GL_CALL(glTexParameteri(GLenum(GL_TEXTURE_2D), GLenum(GL_TEXTURE_MIN_FILTER), GL_NEAREST));
        GL_CALL(glTexParameteri(GLenum(GL_TEXTURE_2D), GLenum(GL_TEXTURE_MAG_FILTER), GL_NEAREST));
        GL_CALL(glTexParameterf(GLenum(GL_TEXTURE_2D), GLenum(GL_TEXTURE_WRAP_S), GLfloat(GL_CLAMP_TO_EDGE)));
        GL_CALL(glTexParameterf(GLenum(GL_TEXTURE_2D), GLenum(GL_TEXTURE_WRAP_T), GLfloat(GL_CLAMP_TO_EDGE)));
    }

    /* offscreen_render_target::delete_textures */
    void offscreen_render_target::delete_textures()
    {
        if (m_offscreen_render_texture != 0) {
            GL_CALL(glDeleteTextures(1, &m_offscreen_render_texture));
            m_offscreen_render_texture = 0;
        }
        if (m_offscreen_post_processuing_render_texture != 0) {
            GL_CALL(glDeleteTextures(1, &m_offscreen_post_processuing_render_texture));
            m_offscreen_post_processuing_render_texture = 0;
        }
    }

    /* offscreen_render_target::prepare_post_processing_rendering */
    void offscreen_render_target::prepare_post_processing_rendering()
    {
        if (m_offscreen_post_processuing_render_texture == 0) {
            generate_texture(m_offscreen_post_processuing_render_texture);
        }
        GL_CALL(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_post_processing_framebuffer));
        GL_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_offscreen_post_processuing_render_texture, 0));

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
            std::cout << "[ERROR] Failed to make complete post processing framebuffer object " << status << std::endl;
            return;
        }

        GL_CALL(glViewport(0, 0, GLsizei(m_width), GLsizei(m_height)));

        GL_CALL(glActiveTexture(GLenum(GL_TEXTURE0)));
        GL_CALL(glBindTexture(GL_TEXTURE_2D, m_offscreen_render_texture));
        m_active_texture = m_offscreen_post_processuing_render_texture;
        m_last_framebuffer = m_post_processing_framebuffer;
    }

    /* offscreen_render_target::read_current_buffer_bpc8 */
    pixel_buffer_sptr offscreen_render_target::read_current_buffer_bpc8(bnb::oep::interfaces::image_format format_hint)
    {
        using ns = bnb::oep::interfaces::image_format;
        int32_t pixel_size{0};
        GLenum gl_format{0};
        switch (format_hint) {
            case ns::bpc8_rgb:
                pixel_size = 3;
                gl_format = GL_RGB;
                break;

#if defined(GL_BGR)
            case ns::bpc8_bgr:
                pixel_size = 3;
                gl_format = GL_BGR;
                break;
#endif /* defined(GL_BGR) */

            case ns::bpc8_rgba:
                pixel_size = 4;
                gl_format = GL_RGBA;
                break;

#if defined(GL_BGRA)
            case ns::bpc8_bgra:
                pixel_size = 4;
                gl_format = GL_BGRA;
                break;
#endif /* defined(GL_BGRA) */

            default:
                return nullptr;
        }

        size_t size = m_width * m_height * 4;
        auto plane_storage = std::shared_ptr<uint8_t>(new uint8_t[size]);
        bnb::oep::interfaces::pixel_buffer::plane_data bpc8_plane{plane_storage, size, m_width * 4};

        glBindFramebuffer(GL_FRAMEBUFFER, m_last_framebuffer);
        GL_CALL(glReadPixels(0, 0, m_width, m_height, gl_format, GL_UNSIGNED_BYTE, plane_storage.get()));
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        std::vector<bnb::oep::interfaces::pixel_buffer::plane_data> planes{bpc8_plane};
        return bnb::oep::interfaces::pixel_buffer::create(planes, format_hint, m_width, m_height);
    }

    /* offscreen_render_target::read_current_buffer_i420 */
    pixel_buffer_sptr offscreen_render_target::read_current_buffer_i420(bnb::oep::interfaces::image_format format_hint)
    {
        using ns = bnb::oep::interfaces::image_format;
        using ns_cvt = bnb::oep::converter::yuv_converter;
        ns_cvt::standard std{ns_cvt::standard::bt601};
        ns_cvt::range rng{ns_cvt::range::full_range};
        switch (format_hint) {
            case ns::i420_bt601_full:
                break;
            case ns::i420_bt601_video:
                rng = ns_cvt::range::video_range;
                break;
            case ns::i420_bt709_full:
                std = ns_cvt::standard::bt709;
                break;
            case ns::i420_bt709_video:
                std = ns_cvt::standard::bt709;
                rng = ns_cvt::range::video_range;
                break;
            default:
                return nullptr;
        }

        if (m_yuv_i420_converter == nullptr) {
            m_yuv_i420_converter = std::make_unique<bnb::oep::converter::yuv_converter>();
            m_yuv_i420_converter->set_drawing_orientation(ns_cvt::rotation::deg_0, true);
        }

        m_yuv_i420_converter->set_convert_standard(std, rng);

        auto do_nothing_deleter_uint8 = [](uint8_t*) { /* DO NOTHING */ };
        auto default_deleter_uint8 = std::default_delete<uint8_t>();

        ns_cvt::yuv_data i420_planes_data;
        /* allocate needed memory for store */
        int32_t clamped_width = (m_width + 7) & ~7; /* alhoritm specific */
        i420_planes_data.size = m_yuv_i420_converter->calc_min_yuv_data_size(m_width, m_height);
        i420_planes_data.data = std::shared_ptr<uint8_t>(new uint8_t[i420_planes_data.size], do_nothing_deleter_uint8);

        /* convert to i420 */
        uint32_t gl_texture = static_cast<uint32_t>(reinterpret_cast<uint64_t>(get_current_buffer_texture()));
        m_yuv_i420_converter->convert(gl_texture, m_width, m_height, i420_planes_data);

        /* save data */
        using ns_pb = bnb::oep::interfaces::pixel_buffer;
        ns_pb::plane_sptr y_plane_data(i420_planes_data.y_plane_data, do_nothing_deleter_uint8);
        ns_pb::plane_sptr u_plane_data(i420_planes_data.u_plane_data, do_nothing_deleter_uint8);
        ns_pb::plane_sptr v_plane_data(i420_planes_data.v_plane_data, do_nothing_deleter_uint8);
        size_t y_plane_size(static_cast<size_t>(i420_planes_data.u_plane_data - i420_planes_data.y_plane_data));
        size_t v_u_planes_diff(static_cast<size_t>(i420_planes_data.v_plane_data - i420_planes_data.u_plane_data));
        size_t u_plane_size(i420_planes_data.size - y_plane_size - v_u_planes_diff);
        size_t v_plane_size(u_plane_size);
        ns_pb::plane_data y_plane{y_plane_data, y_plane_size, i420_planes_data.y_plane_stride};
        ns_pb::plane_data u_plane{u_plane_data, u_plane_size, i420_planes_data.u_plane_stride};
        ns_pb::plane_data v_plane{v_plane_data, v_plane_size, i420_planes_data.v_plane_stride};

        std::vector<ns_pb::plane_data> planes{y_plane, u_plane, v_plane};

        return ns_pb::create(planes, format_hint, clamped_width, m_height, [default_deleter_uint8](auto* pb) { default_deleter_uint8(pb->get_base_sptr().get()); });
    }

} /* namespace bnb::oep */
