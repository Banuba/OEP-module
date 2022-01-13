#pragma once

#include <functional>
#include <atomic>
#include <string>
#include <vector>
#include <interfaces/pixel_buffer.hpp>

namespace bnb::oep::interfaces
{
    class camera;
}

using camera_sptr = std::shared_ptr<bnb::oep::interfaces::camera>;

namespace bnb::oep::interfaces
{

    class camera
    {
    public:
        struct camera_device_description
        {
            std::string localized_name;
        }; /* struct camera_device_description */

        using push_frame_cb_t = std::function<void(pixel_buffer_sptr)>;

    public:
        /**
         * Create camera
         *
         * @param cb callback for push frames
         * @param index camera device number
         *
         * @return shared pointer to the camera
         *
         * @example bnb::oep::interfaces::camera::create([](pixel_buffer_sptr image){}, 0)
         */
        static camera_sptr create(push_frame_cb_t cb, size_t index);

        virtual ~camera() = default;

        /**
         * Select camera device by index
         * Must be called from the main thread.
         *
         * @param index - device index
         *
         * @example set_device_by_index(0)
         */
        virtual void set_device_by_index(uint32_t index) = 0;

        /**
         * Select camera device by device identificator
         * Must be called from the main thread.
         *
         * @param device_id ID of the camera device
         *
         * @example set_device_by_id(get_connected_devices()[0].localized_name)
         */
        virtual void set_device_by_id(const std::string& device_id) = 0;

        /**
         * Start camera
         * Must be called from the main thread.
         *
         * @example start()
         */
        virtual void start() = 0;

        /**
         * Returns a list of available camera devices
         * Must be called from the main thread.
         *
         * @example get_connected_devices()
         */
        virtual const std::vector<camera_device_description> get_connected_devices() const = 0;

        /**
         * Returns currently selected device index
         * Must be called from the main thread.
         *
         * @example get_current_device_index()
         */
        virtual size_t get_current_device_index() const = 0;
    }; /* class camera  INTERFACE */

} /* namespace bnb::oep::interfaces */
