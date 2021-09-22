#include "camera_win.hpp"

#include "camera_device.hpp"
#include <iostream>

struct bnb::camera_win::impl
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
};


bnb::camera_win::camera_win(const camera_base::push_frame_cb_t& cb)
    : camera_base(cb)
    , m_impl(std::make_unique<impl>())
{
    constexpr auto camera_width = 1280;
    constexpr auto camera_hight = 720;
    constexpr auto frames_per_second = 30;
    m_impl->wrapped->Init();

    VideoPixelFormat pixel_format = PIXEL_FORMAT_NV12;
    m_impl->wrapped->AllocateAndStart(camera_width, camera_hight, frames_per_second, pixel_format);

    m_impl->wrapped->SetCallback([this, camera_width, camera_hight, pixel_format](std::shared_ptr<ScopedBufferLock> lock) {
        bnb_image_format_t format;
        format.orientation = BNB_DEG_0;
        format.face_orientation = 0;
        format.require_mirroring = true;
        format.width = camera_width;
        format.height = camera_hight;

        auto data = static_cast<color_plane_data_t*>(lock->data());
        auto surface_stride = lock->pitch();
        auto y_plane_size = format.width * format.height;

        color_plane y_plane = color_plane(data, [lock](color_plane_data_t*) { /* DO NOTHING */ });
        color_plane uv_plane;

        if (pixel_format == PIXEL_FORMAT_NV12) {
            uv_plane = color_plane(data + y_plane_size, [lock](color_plane_data_t*) { /* DO NOTHING */ });
        } else if (pixel_format == PIXEL_FORMAT_I420) {
            std::vector<color_plane_data_t> uv_plane_vector;
            uv_plane_vector.reserve(y_plane_size / 2);
            for (size_t i = 0; i < y_plane_size / 4; ++i) {
                uv_plane_vector.emplace_back(data[i + y_plane_size]);
                uv_plane_vector.emplace_back(data[i + y_plane_size + y_plane_size / 4]);
            }

            auto* ptr = new std::vector<color_plane_data_t>(std::move(uv_plane_vector));
            uv_plane = color_plane(ptr->data(), [ptr](color_plane_data_t*) { delete ptr; });
        } else {
            throw std::runtime_error("Unsupported format from camera");
        }

        // see nv12_image.hpp
        auto img = std::make_shared<nv12_image>(y_plane, surface_stride, uv_plane, surface_stride, format);

        if (m_push_frame_cb) {
            m_push_frame_cb(std::move(img));
        }
    });
}

bnb::camera_win::~camera_win()
{
    m_impl->wrapped->Stop();
}

void bnb::camera_win::set_device_by_index(uint32_t index)
{
    std::cout << "[Camera Win]: "
        << "Only default camera device supported";
}
void bnb::camera_win::set_device_by_id(const std::string& device_id)
{
    std::cout << "[Camera Win]: " <<
        "Only default camera device supported";
}

void bnb::camera_win::start()
{
    std::cout << "[Camera Win]: "
        << "Camera starts in constructor";
}

bnb::camera_sptr bnb::create_camera_device(camera_base::push_frame_cb_t cb, size_t index)
{
    return std::make_shared<bnb::camera_win>(cb);
}
