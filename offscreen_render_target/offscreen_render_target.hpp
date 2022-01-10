#pragma once

#include <interfaces/offscreen_effect_player.hpp>
#include <interfaces/render_context.hpp>
#include <mutex>

#include <opengl/yuv_converter.hpp>

namespace bnb::oep
{

    class offscreen_render_target : public bnb::oep::interfaces::offscreen_render_target
    {
    public:
        offscreen_render_target(render_context_sptr rc);

        ~offscreen_render_target();

        void init(int32_t width, int32_t height) override;

        void deinit() override;

        void surface_changed(int32_t width, int32_t height) override;

        void activate_context() override;

        void deactivate_context() override;

        void prepare_rendering() override;

        void orient_image(bnb::oep::interfaces::rotation orient) override;

        pixel_buffer_sptr read_current_buffer(bnb::oep::interfaces::image_format format) override;

        rendered_texture_t get_current_buffer_texture() override;

    private:
        void generate_texture(GLuint& texture);
        void delete_textures();
        void prepare_post_processing_rendering();
        pixel_buffer_sptr read_current_buffer_bpc8(bnb::oep::interfaces::image_format format_hint);
        pixel_buffer_sptr read_current_buffer_i420(bnb::oep::interfaces::image_format format_hint);

    private:
        render_context_sptr m_rc;
        int32_t m_width{0};
        int32_t m_height{0};

        GLuint m_framebuffer{0};
        GLuint m_post_processing_framebuffer{0};
        GLuint m_last_framebuffer{0};
        GLuint m_offscreen_render_texture{0};
        GLuint m_offscreen_post_processuing_render_texture{0};

        GLuint m_active_texture{0};

        std::unique_ptr<program> m_shader;
        std::once_flag m_init_flag;
        std::once_flag m_deinit_flag;

        std::unique_ptr<bnb::oep::converter::yuv_converter> m_yuv_i420_converter;

        GLuint m_vbo{0};
        GLuint m_vao{0};
    }; /* class offscreen_render_target */

} /* namespace bnb::oep */
