#pragma once

#include <interfaces/api.hpp>

#include "program.hpp"
#include "frame_surface_handler.hpp"

namespace bnb::render
{
    class renderer
    {
    public:
        renderer(int width, int height);

        void surface_change(int32_t width, int32_t height);

        void update_data(int texture_id, bool draw_gray = false);
        bool draw();

    private:
        program m_program;
        program m_program_gray;
        frame_surface_handler m_frame_surface;

        int m_width;
        int m_height;
        int m_texture_id{0};
        bool m_draw_gray{false};

        std::atomic<bool> m_rendering = false;
        std::atomic<bool> m_texture_updated = false;

        std::atomic<bool> m_surface_changed = false;
    };
} // namespace bnb::render
