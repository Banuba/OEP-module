#pragma once

#include <interfaces/camera.hpp>

namespace bnb::oep
{

    class camera : public bnb::oep::interfaces::camera
    {
    public:
        explicit camera(bnb::oep::interfaces::camera::push_frame_cb_t cb);
        ~camera() override;

        void set_device_by_index(uint32_t index) override;

        void set_device_by_id(const std::string& device_id) override;

        void start() override;

        const std::vector<camera::camera_device_description> get_connected_devices() const override;

        size_t get_current_device_index() const override;

    private:
        push_frame_cb_t m_push_frame_cb;
        std::vector<camera_device_description> m_connected_devices;
        size_t m_device_index{0};
        struct impl;
        std::unique_ptr<impl> m_impl;
    }; /* class camera */

} /* namespace bnb::oep */
