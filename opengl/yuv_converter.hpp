/* for more details see
 * https://chromium.googlesource.com/external/webrtc/+/HEAD/sdk/android/api/org/webrtc/YuvConverter.java
 *
 * NOTE:
 * converter works correctly if the width is a multiple of eight and the height is a multiple of two.
 * In case of a remainder of dividing the width by eight, there will be distortion in the width.
 * The larger the remainder of the division, the greater the distortion of the width.
 */

#pragma once
#include <memory>
#include <opengl/program.hpp>

namespace bnb::oep::converter
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
            /* do not modify these assignments */
            deg_0 = 0,
            deg_90 = 1,
            deg_180 = 2,
            deg_270 = 3
        };

        enum class yuv_data_layout
        {
            /* interlaced layout
            * to get the minimum required memory size call function 'calc_min_yuv_data_size(width, height)'
            * representation in memory:
            * +---------------+
            * |               |
            * |       Y       |
            * |     plane     |
            * |               |
            * +-------+-------+
            * |   U   |   V   |
            * | plane | plane |
            * +-------+-------+
            */
            semi_planar_row_interleaved, /* U and V planes are stored interlaced */

            /* consistent layout
            * to get the minimum required memory size call function 'calc_min_yuv_data_size(width, height)'
            * representation in memory:
            * +---------------+
            * |               |
            * |       Y       |
            * |     plane     |
            * |               |
            * +-------+-------+
            * |   U   |       |
            * | plane | unused|
            * +-------+       |
            * |   V   | memory|
            * | plane |       |
            * +-------+-------+
            */
            planar_layout  /* Y, U and V planes stored sequentially */
        };

        struct yuv_data
        {
            std::shared_ptr<uint8_t> data;
            size_t size{0};
            uint8_t* y_plane_data{nullptr};
            uint8_t* u_plane_data{nullptr};
            uint8_t* v_plane_data{nullptr};
            int y_plane_stride{0};
            int u_plane_stride{0};
            int v_plane_stride{0};
        };

    public:
        yuv_converter(standard st = standard::bt601, range rng = range::video_range, rotation rot = rotation::deg_0, bool vertical_flip = false, yuv_data_layout data_layout = yuv_data_layout::planar_layout);
        ~yuv_converter();

        void set_convert_standard(standard st, range rng);
        void set_drawing_orientation(rotation rot, bool vertical_flip);
        void convert(uint32_t gl_texture, int width, int height, yuv_data& output);
        int get_width();
        int get_height();
        size_t calc_min_yuv_data_size(int width, int height);

    private:
        struct framebuffer
        {
            uint32_t fbo{0};
            uint32_t texture{0};
            int width{0};
            int height{0};
        };

    private:
        void update_pixel_steps();
        framebuffer create_framebuffer(int width, int height);
        void delete_framebuffer(framebuffer& fbo);

    private:
        uint32_t m_vbo{0};
        uint32_t m_vao{0};
        int32_t m_draw_indent{0};
        int m_width{0};
        int m_height{0};
        const float* m_y_plane_coefs{nullptr};
        const float* m_u_plane_coefs{nullptr};
        const float* m_v_plane_coefs{nullptr};
        float m_pixel_step_y[2]{0.0f, 0.0f};
        float m_pixel_step_uv[2]{0.0f, 0.0f};
        rotation m_rotation{rotation::deg_0};
        bool m_vertical_flip{false};
        yuv_data_layout m_data_layout{yuv_data_layout::planar_layout};
        framebuffer m_fbo;
        program m_shader;
    };


    /* yuv_converter::get_width */
    inline int yuv_converter::get_width()
    {
        return m_width;
    }

    /* yuv_converter::get_height */
    inline int yuv_converter::get_height()
    {
        return m_height;
    }

} /* namespace bnb::oep::converter */
