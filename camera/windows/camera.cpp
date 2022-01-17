#include "camera.hpp"

#include "camera_device.hpp"
#include <iostream>

namespace bnb::oep
{

    struct camera::impl
    {
        impl()
        {
            ComPtr<IMFMediaSource> ptrMediaSource;
            HRESULT hr = CreateVideoDeviceSource(&ptrMediaSource);
            if (FAILED(hr)) {
                throw std::runtime_error("Failed to crate media source");
            }
            wrapped = std::make_unique<VideoCaptureDeviceMFWin>(ptrMediaSource);
        }
        std::unique_ptr<VideoCaptureDeviceMFWin> wrapped = nullptr;
    }; /* struct camera::impl */

    /* camera::camera */
    camera::camera(bnb::oep::interfaces::camera::push_frame_cb_t cb)
        : m_push_frame_cb(cb)
        , m_impl(std::make_unique<impl>())
    {
        constexpr auto camera_width = 1280;
        constexpr auto camera_hight = 720;
        constexpr auto frames_per_second = 30;
        m_impl->wrapped->Init();

        VideoPixelFormat pixel_format = PIXEL_FORMAT_NV12;
        m_impl->wrapped->AllocateAndStart(camera_width, camera_hight, frames_per_second, pixel_format);

        m_impl->wrapped->SetCallback([this, camera_width, camera_hight, pixel_format](std::shared_ptr<ScopedBufferLock> lock) {
            pixel_buffer_sptr img;

            auto data = static_cast<uint8_t*>(lock->data());
            auto surface_stride = lock->pitch();
            auto y_plane_size = camera_width * camera_hight;
            auto size = camera_width * camera_hight * 3 / 2;
            uint8_t* y_plane_data{data};

            if (pixel_format == PIXEL_FORMAT_NV12) {
                uint8_t* uv_plane_data = y_plane_data + y_plane_size;

                using ns = bnb::oep::interfaces::pixel_buffer;
                ns::plane_data y_plane{std::shared_ptr<uint8_t>(y_plane_data, [lock](uint8_t*) { /* do nothing */ }), 0, static_cast<int32_t>(surface_stride)};
                ns::plane_data uv_plane{std::shared_ptr<uint8_t>(uv_plane_data, [lock](uint8_t*) { /* do nothing */ }), 0, static_cast<int32_t>(surface_stride)};
                std::vector<ns::plane_data> planes{y_plane, uv_plane};

                img = ns::create(planes, bnb::oep::interfaces::image_format::nv12_bt709_full, camera_width, camera_hight);
            } else if (pixel_format == PIXEL_FORMAT_I420) {
                uint8_t* u_plane_data = y_plane_data + y_plane_size;
                uint8_t* v_plane_data = u_plane_data + camera_width * camera_hight / 4;

                using ns = bnb::oep::interfaces::pixel_buffer;
                ns::plane_data y_plane{std::shared_ptr<uint8_t>(y_plane_data, [lock](uint8_t*) { /* do nothing */ }), 0, static_cast<int32_t>(surface_stride)};
                ns::plane_data u_plane{std::shared_ptr<uint8_t>(u_plane_data, [lock](uint8_t*) { /* do nothing */ }), 0, static_cast<int32_t>(surface_stride / 2)};
                ns::plane_data v_plane{std::shared_ptr<uint8_t>(v_plane_data, [lock](uint8_t*) { /* do nothing */ }), 0, static_cast<int32_t>(surface_stride / 2)};
                std::vector<ns::plane_data> planes{y_plane, u_plane, v_plane};

                img = ns::create(planes, bnb::oep::interfaces::image_format::i420_bt709_full, camera_width, camera_hight);
            } else {
                throw std::runtime_error("Unsupported format from camera");
            }

            if (m_push_frame_cb) {
                m_push_frame_cb(img);
            }
        });
    }

    /* camera::~camera */
    camera::~camera()
    {
        m_impl->wrapped->Stop();
    }

    /* camera::set_device_by_index */
    void camera::set_device_by_index(uint32_t index)
    {
        std::cout << "[Camera Win]: Only default camera device supported" << std::endl;
    }

    /* camera::set_device_by_id */
    void camera::set_device_by_id(const std::string& device_id)
    {
        std::cout << "[Camera Win]: Only default camera device supported" << std::endl;
    }

    /* camera::start */
    void camera::start()
    {
        std::cout << "[Camera Win]: Camera starts in constructor" << std::endl;
    }

    /* interfaces::camera::create */
    camera_sptr bnb::oep::interfaces::camera::create(bnb::oep::interfaces::camera::push_frame_cb_t cb, size_t index)
    {
        return std::make_shared<bnb::oep::camera>(cb);
    }

    /* camera::get_connected_devices */
    const std::vector<bnb::oep::interfaces::camera::camera_device_description> camera::get_connected_devices() const
    {
        return m_connected_devices;
    }

    /* camera::get_current_device_index */
    size_t camera::get_current_device_index() const
    {
        return m_device_index;
    }

} /* namespace bnb::oep */
