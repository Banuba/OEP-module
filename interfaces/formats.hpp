#pragma once

#include <interfaces/c_api/c_api_only.hpp>

namespace bnb::interfaces {
    struct orient_format
    {
        bnb_image_orientation_alias orientation;
        bool is_y_flip;
    };

} // bnb::interfaces
