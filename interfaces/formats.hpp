#pragma once

#include <interfaces/api.hpp>

namespace bnb::interfaces {
    struct orient_format
    {
        bnb_image_orientation_alias orientation;
        bool is_y_flip;
    };

} // bnb::interfaces
