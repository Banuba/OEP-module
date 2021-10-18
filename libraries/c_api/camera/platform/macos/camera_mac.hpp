#pragma once

#include "camera_base.hpp"

namespace bnb
{
    class camera_mac : public camera_base
    {
    public:
        explicit camera_mac(const camera_base::push_frame_cb_t& cb);
        ~camera_mac() override;

        void set_device_by_index(uint32_t index) override;
        void set_device_by_id(const std::string& device_id) override;
        void start() override;

    private:
        struct impl;
        std::unique_ptr<impl> m_impl;
    };
} // namespace bnb
