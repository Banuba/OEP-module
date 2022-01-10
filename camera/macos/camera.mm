#include "camera.hpp"

#import <CoreText/CoreText.h>
#import <AVFoundation/AVFoundation.h>
#import <Foundation/Foundation.h>
#import <CoreFoundation/CoreFoundation.h>
#import <AppKit/AppKit.h>
#import <CoreData/CoreData.h>

#include <vector>

using namespace bnb::oep;

@interface VideoCaptureManager : NSObject <AVCaptureVideoDataOutputSampleBufferDelegate>
- (void)start:(bnb::oep::interfaces::camera::push_frame_cb_t)delegate;
- (void)stop;
@end


@interface VideoCaptureManager ()

@property AVCaptureSession* session;
@property AVCaptureDevice* currentDevice;
@property NSArray* videoDevices;
@property bnb::oep::interfaces::camera::push_frame_cb_t callback;


- (void)captureOutput:(AVCaptureOutput*)captureOutput
    didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer
           fromConnection:(AVCaptureConnection*)connection;

- (void)processFrame:(CMSampleBufferRef)sampleBuffer;
- (std::vector<bnb::oep::interfaces::camera::camera_device_description>)getConnectedDevices;
- (void)switchCamera:(size_t)device_id;

@end

@implementation VideoCaptureManager
- (id)init
{
    self = [super init];
    // Get all media devices
    NSMutableArray* devices = (NSMutableArray*) [AVCaptureDevice devicesWithMediaType:AVMediaTypeVideo];

    self.videoDevices = (NSArray*) devices;

    NSLog(@"Found Devices:");
    for (AVCaptureDevice* object in self.videoDevices) {
        NSLog(@"Device: %@", object.localizedName);
    }

    self.session = [[AVCaptureSession alloc] init];
    if (self.session) {
        [self.session beginConfiguration];

        NSError* error;
        self.currentDevice = [AVCaptureDevice defaultDeviceWithMediaType:AVMediaTypeVideo];

        AVCaptureDeviceInput* input = [AVCaptureDeviceInput deviceInputWithDevice:self.currentDevice error:&error];

        if (!input || error) {
            NSLog(@"Error creating capture device input: %@", error.localizedDescription);
        } else {
            [self.session addInput:input];
        }

        AVCaptureVideoDataOutput* output = [[AVCaptureVideoDataOutput alloc] init];

        output.videoSettings = @{(NSString*) kCVPixelBufferPixelFormatTypeKey: @(kCVPixelFormatType_420YpCbCr8BiPlanarFullRange)};
        [output setSampleBufferDelegate:self queue:dispatch_get_main_queue()];
        [self.session addOutput:output];

        [self.session commitConfiguration];
    }

    return self;
}

- (id)init:(int)device_id
{
}

- (void)switchCamera:(size_t)device_id
{
    @synchronized(self) {
        AVCaptureDevice* newCamera = [self.videoDevices objectAtIndex:device_id];

        if (newCamera == nil) {
            NSLog(@"Not compatible name: %zu", device_id);
            newCamera = [self.videoDevices objectAtIndex:device_id];
            // return;
        }
        // Change camera source
        if (self.session) {
            // Indicate that some changes will be made to the session
            [self.session beginConfiguration];

            // //Remove existing input
            AVCaptureInput* currentCameraInput = [self.session.inputs objectAtIndex:0];

            if (currentCameraInput) {
                [self.session removeInput:currentCameraInput];
            }

            // Add input to session
            NSError* err = nil;
            AVCaptureDeviceInput* newVideoInput = [[AVCaptureDeviceInput alloc] initWithDevice:newCamera error:&err];
            if (!newVideoInput || err) {
                NSLog(@"Error creating capture device input: %@", err.localizedDescription);
            } else {
                [self.session addInput:newVideoInput];
            }

            // Commit all the configuration changes at once
            [self.session commitConfiguration];
        }
    }
}

- (void)start:(bnb::oep::interfaces::camera::push_frame_cb_t)delegate
{
    [AVCaptureDevice requestAccessForMediaType:AVMediaTypeVideo
                             completionHandler:^(BOOL granted) {
                               if (granted) {
                                   dispatch_async(dispatch_get_main_queue(), ^{
                                     self.callback = delegate;
                                     [self.session startRunning];
                                   });
                               } else {
                                   dispatch_async(dispatch_get_main_queue(), ^{
                                     NSLog(@"Camera Has No Permission");
                                   });
                               }
                             }];
}

- (void)stop
{
    [self.session stopRunning];
}

- (void)captureOutput:(AVCaptureOutput*)captureOutput
    didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer
           fromConnection:(AVCaptureConnection*)connection
{
    @synchronized(self) {
        [self processFrame:sampleBuffer];
    }
}

- (void)processFrame:(CMSampleBufferRef)sampleBuffer
{
    CVImageBufferRef pixelBuffer = CMSampleBufferGetImageBuffer(sampleBuffer);

    OSType pixelFormat = CVPixelBufferGetPixelFormatType(pixelBuffer);
    pixel_buffer_sptr img;

    switch (pixelFormat) {
        case kCVPixelFormatType_420YpCbCr8BiPlanarFullRange: {
            CVPixelBufferLockBaseAddress(pixelBuffer, kCVPixelBufferLock_ReadOnly);
            uint8_t* lumo = static_cast<uint8_t*>(CVPixelBufferGetBaseAddressOfPlane(pixelBuffer, 0));
            uint8_t* chromo = static_cast<uint8_t*>(CVPixelBufferGetBaseAddressOfPlane(pixelBuffer, 1));
            int bufferWidth = CVPixelBufferGetWidth(pixelBuffer);
            int bufferHeight = CVPixelBufferGetHeight(pixelBuffer);

            // Retain twice. Each plane will release once.
            CVPixelBufferRetain(pixelBuffer);
            CVPixelBufferRetain(pixelBuffer);

            using ns = bnb::oep::interfaces::pixel_buffer;
            ns::plane_data y_plane{std::shared_ptr<uint8_t>(lumo, [pixelBuffer](uint8_t*) {
                                       CVPixelBufferRelease(pixelBuffer);
                                   }),
                                   0,
                                   static_cast<int32_t>(CVPixelBufferGetBytesPerRowOfPlane(pixelBuffer, 0))};
            ns::plane_data uv_plane{std::shared_ptr<uint8_t>(chromo, [pixelBuffer](uint8_t*) {
                                        CVPixelBufferUnlockBaseAddress(pixelBuffer, kCVPixelBufferLock_ReadOnly);
                                        CVPixelBufferRelease(pixelBuffer);
                                    }),
                                    0,
                                    static_cast<int32_t>(CVPixelBufferGetBytesPerRowOfPlane(pixelBuffer, 0))};

            std::vector<ns::plane_data> planes{y_plane, uv_plane};
            img = ns::create(planes, bnb::oep::interfaces::image_format::nv12_bt709_full, bufferWidth, bufferHeight);
        } break;
        default:
            NSLog(@"ERROR TYPE : %d", pixelFormat);
            return;
    }

    if (self.callback) {
        self.callback(std::move(img));
    }
}

- (std::vector<bnb::oep::interfaces::camera::camera_device_description>)getConnectedDevices
{
    std::vector<bnb::oep::interfaces::camera::camera_device_description> devices;
    devices.reserve([self.videoDevices count]);
    for (AVCaptureDevice* obj in self.videoDevices) {
        bnb::oep::interfaces::camera::camera_device_description descr;
        descr.localized_name = std::string([obj.localizedName UTF8String]);
        devices.emplace_back(std::move(descr));
    }

    return devices;
}
@end


struct bnb::oep::camera::impl
{
    impl()
        : wrapped([[VideoCaptureManager alloc] init])
    {
    }
    VideoCaptureManager* wrapped = nullptr;
};


bnb::oep::camera::camera(bnb::oep::interfaces::camera::push_frame_cb_t cb)
    : m_push_frame_cb(cb)
    , m_impl(std::make_unique<impl>())
{
    m_connected_devices = [m_impl->wrapped getConnectedDevices];
}

bnb::oep::camera::~camera()
{
    if (m_impl->wrapped != nullptr) {
        [m_impl->wrapped stop];
        [m_impl->wrapped release];
    }
}

void bnb::oep::camera::set_device_by_index(uint32_t index)
{
    m_device_index = index;
    [m_impl->wrapped switchCamera:index];
}

void bnb::oep::camera::set_device_by_id(const std::string& device_id)
{
    for (size_t i = 0; i < m_connected_devices.size(); ++i) {
        if (device_id == m_connected_devices[i].localized_name) {
            m_device_index = i;
            [m_impl->wrapped switchCamera:i];
            break;
        }
    }
}

void bnb::oep::camera::start()
{
    [m_impl->wrapped start:m_push_frame_cb];
}

camera_sptr bnb::oep::interfaces::camera::create(bnb::oep::interfaces::camera::push_frame_cb_t cb, size_t index)
{
    auto cam_ptr = std::make_shared<bnb::oep::camera>(cb);
    cam_ptr->set_device_by_index(index);
    cam_ptr->start();
    return cam_ptr;
}

/* camera::get_connected_devices */
const std::vector<bnb::oep::interfaces::camera::camera_device_description> bnb::oep::camera::get_connected_devices() const
{
    return m_connected_devices;
}

/* camera::get_current_device_index */
size_t bnb::oep::camera::get_current_device_index() const
{
    return m_device_index;
}
