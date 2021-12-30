#pragma once

#include <interfaces/pixel_buffer.hpp>
#include <vector>

namespace bnb::oep
{

    class pixel_buffer : public bnb::oep::interfaces::pixel_buffer
    {
    public:
        pixel_buffer(const std::vector<plane_data>& planes, bnb::oep::interfaces::image_format fmt, int32_t width, int32_t height);
        ~pixel_buffer();

        bnb::oep::interfaces::image_format get_image_format() override;
        int32_t get_plane_count() override;

        plane_sptr get_base_sptr() override;
        plane_sptr get_base_sptr_of_plane(int32_t plane_num) override;

        int32_t get_bytes_per_pixel() override;
        int32_t get_bytes_per_pixel_of_plane(int32_t plane_num) override;
        int32_t get_bytes_per_row() override;
        int32_t get_bytes_per_row_of_plane(int32_t plane_num) override;
        int32_t get_width() override;
        int32_t get_width_of_plane(int32_t plane_num) override;
        int32_t get_height() override;
        int32_t get_height_of_plane(int32_t plane_num) override;

    private:
        struct image_format_info
        {
            int32_t planes_num{0}; /* [1..3] - number of planes */
            int32_t pixel_sizes[3]{0, 0, 0};
        }; /* image_format_info */

        struct plane_data_extended : public plane_data
        {
            int32_t width{0};
            int32_t height{0};
            int32_t pixel_size{0};
        }; /* struct plane_data_extended */

        image_format_info get_image_format_info(bnb::oep::interfaces::image_format fmt);
        void validate_plane_number(int32_t plane_num);

    private:
        bnb::oep::interfaces::image_format m_image_format;
        int32_t m_plane_count{0};
        plane_data_extended m_planes[3];
    }; /* class image_data  */

} /* namespace bnb::oep */
