#pragma once

#include "camera_base.hpp"

namespace bnb
{
    class camera_win : public camera_base
    {
    public:
        explicit camera_win(const camera_base::push_frame_cb_t& cb);
        ~camera_win() override;

        void set_device_by_index(uint32_t index) override;
        void set_device_by_id(const std::string& device_id) override;
        void start() override;

    private:
        struct impl;
        std::unique_ptr<impl> m_impl;
    };
} // namespace bnb
