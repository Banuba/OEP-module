#include "yuv_converter.hpp"
#include <string>
#include <opengl.hpp>

static const char* to_gl_check_framebuffer_status(GLenum e)
{
    switch(e) {
        case GL_FRAMEBUFFER_UNDEFINED:
            return "GL_FRAMEBUFFER_UNDEFINED";

        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
            return "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";

        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
            return "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";

        case GL_FRAMEBUFFER_UNSUPPORTED:
            return "GL_FRAMEBUFFER_UNSUPPORTED";

        case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
            return "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE";

    }
    return "UNKNOWN GLenum";
}

static void put_error_message(const char* where, const char* what)
{
    std::string msg;
    if (where) {
        msg.append(where);
        msg.append("\n");
    }
    if (what) {
        msg.append(what);
        msg.append("\n");
    }
    throw std::runtime_error(msg);
}

namespace bnb::converter
{

const int drawing_plane_vert_count = 4;
const int drawing_plane_coords_per_vert = 5;

#define N "\n"
const char *shader_vec_prog =
    "layout(location = 0) in vec3 in_vertex;" N
    "layout(location = 1) in vec2 in_uv;" N
    "uniform mat4 rotate_mat;"
    "out vec2 uv_coord;" N
    "void main() {" N
    "    uv_coord = in_uv;" N
    "    gl_Position = rotate_mat * vec4(in_vertex, 1.0);" N
    "}" N;

const char *shader_frag_prog =
    "layout (location = 0) out vec4 out_color;" N
    "uniform vec2 pixel_step;" N
    "uniform vec4 plane_coef;" N
    "uniform sampler2D in_texture;" N
    "in vec2 uv_coord;" N
    "void main() {" N
    "    float a = plane_coef.a;" N
    "    vec3 rgb = plane_coef.rgb;" N
    "    out_color.r = a + dot(rgb, texture(in_texture, uv_coord - 1.5 * pixel_step).rgb);" N
    "    out_color.g = a + dot(rgb, texture(in_texture, uv_coord - 0.5 * pixel_step).rgb);" N
    "    out_color.b = a + dot(rgb, texture(in_texture, uv_coord + 0.5 * pixel_step).rgb);" N
    "    out_color.a = a + dot(rgb, texture(in_texture, uv_coord + 1.5 * pixel_step).rgb);" N
    "}" N;
#undef N

/* yuv_converter::yuv_converter */
yuv_converter::yuv_converter(standard st, range rng, rotation rot) : m_shader(nullptr, shader_vec_prog, shader_frag_prog)
{
    static const float drawing_plane_coords[drawing_plane_coords_per_vert * drawing_plane_vert_count] = {
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, // 0 bottom left
        1.0f, -1.0f, 0.0f, 1.0f, 0.0f, // 1 bottom right
        -1.0f,  1.0f, 0.0f, 0.0f, 1.0f, // 2 top left
        1.0f,  1.0f, 0.0f, 1.0f, 1.0f  // 3 top right
    };

    set_standard(st, rng);
    set_rotation(rot);
    m_pixel_step = m_shader.get_uniform("pixel_step");
    m_plane_coef = m_shader.get_uniform("plane_coef");
    m_in_texture = m_shader.get_uniform("in_texture");
    m_rotate_mat = m_shader.get_uniform("rotate_mat");

    /* create and bind drawing geometry */
    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vao);
    glGenBuffers(1, &m_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(drawing_plane_coords), drawing_plane_coords, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * drawing_plane_coords_per_vert, nullptr);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * drawing_plane_coords_per_vert,
            reinterpret_cast<void*>(sizeof(float) * 3));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

/* yuv_converter::~yuv_converter */
yuv_converter::~yuv_converter()
{
    delete_framebuffer(m_fbo_y);
    delete_framebuffer(m_fbo_uv);
    glDeleteBuffers(1, &m_vbo);
    glDeleteVertexArrays(1, &m_vao);
}

/* yuv_converter::set_standard */
void yuv_converter::set_standard(standard st, range rng)
{
    static const float mat_cvt_to_bt601_video_range[] {
        0.256788f, 0.504129f, 0.0979059f, 0.0627451f,
        -0.148223f, -0.290993f, 0.439216f, 0.501961f,
        0.439216f, -0.367788f, -0.0714274f, 0.501961f
    };
    static const float mat_cvt_to_bt601_full_range[] {
        0.299f,  0.587f,  0.114f, 0.0f,
        -0.1687358916f, -0.3312641084f,  0.5f,  0.5f,
        0.5f, -0.4186875892f, -0.08131241084f, 0.5f
    };
    static const float mat_cvt_to_bt709_video_range[] {
        0.1825858824, 0.6142305882, 0.06200705882, 0.06274509804,
        -0.1006437324, -0.3385719539, 0.4392156863, 0.5019607843,
        0.4392156863, -0.3989421626, -0.04027352368, 0.5019607843
    };
    static const float mat_cvt_to_bt709_full_range[] {
        0.2126, 0.7152, 0.0722, 0,
        -0.1145721061, -0.3854278939, 0.5, 0.5019607843,
        0.5, -0.4541529083, -0.04584709169, 0.5019607843
    };

    const float *cvt_mat {nullptr};
    switch (st) {
        case standard::bt601:
            switch (rng) {
                case range::full_range:
                    cvt_mat = mat_cvt_to_bt601_full_range;
                    break;
                case range::video_range:
                    cvt_mat = mat_cvt_to_bt601_video_range;
                    break;
            }
            break;
        case standard::bt709:
            switch (rng) {
                case range::full_range:
                    cvt_mat = mat_cvt_to_bt709_full_range;
                    break;
                case range::video_range:
                    cvt_mat = mat_cvt_to_bt709_video_range;
                    break;
            }
            break;
    }

    m_y_plane_coefs = cvt_mat;
    m_u_plane_coefs = cvt_mat + 4;
    m_v_plane_coefs = cvt_mat + 8;
}

/* yuv_converter::set_rotation */
void yuv_converter::set_rotation(rotation rot)
{
    static const float mat_rot_0[] {
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 1.0
    };
    static const float mat_rot_90[] {
        0.0, -1.0, 0.0, 0.0,
        1.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 1.0
    };
    static const float mat_rot_180[] {
        -1.0, 0.0, 0.0, 0.0,
        0.0, -1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 1.0
    };
    static const float mat_rot_270[] {
        0.0, 1.0, 0.0, 0.0,
        -1.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 1.0
    };

    switch (rot) {
        case rotation::deg_0:
            m_rot_mat = mat_rot_0;
            break;
        case rotation::deg_90:
            m_rot_mat = mat_rot_90;
            break;
        case rotation::deg_180:
            m_rot_mat = mat_rot_180;
            break;
        case rotation::deg_270:
            m_rot_mat = mat_rot_270;
            break;
    }
}

/* yuv_converter::convert */
void yuv_converter::convert(uint32_t gl_texture, int width, int height, data_t& output)
{
    if (width <= 0 || height <= 0) {
        return;
    }

    /* create/recreate the framebuffer if necessary */
    if (m_width != width || m_height != height) {
        m_width = width;
        m_height = height;
        delete_framebuffer(m_fbo_y);
        delete_framebuffer(m_fbo_uv);
        m_fbo_y = create_framebuffer(m_width / 4, m_height);
        m_fbo_uv = create_framebuffer(m_width / (4 * 2), m_height);
    }

    /* allocate/reallocate memory if necessary */
    if (output.data.get() == nullptr || output.size < m_width * m_height * 3 / 2) {
        output.size = m_width * m_height * 3 / 2;
        output.data = std::make_unique<uint8_t[]>(output.size);
    }

    /* just in case, disable dropping geometry */
    GL_CALL(glDisable(GL_CULL_FACE));

    /* bind drawing geometry */
    GL_CALL(glBindVertexArray(m_vao));
    GL_CALL(glEnableVertexAttribArray(0));
    GL_CALL(glEnableVertexAttribArray(1));

    /* use shader and send texture matrix to shader program */
    m_shader.use();
    m_rotate_mat.send_mat4f(m_rot_mat);

    /* bind input texture */
    GL_CALL(glActiveTexture(GL_TEXTURE0));
    GL_CALL(glBindTexture(GL_TEXTURE_2D, gl_texture));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    m_in_texture.send_1i(0);

    /* pixel step used in the shader to access neighboring pixels */
    float pixel_step[2]{m_rot_mat[0] / m_width, m_rot_mat[4] / m_width};
    m_pixel_step.send_vec2f(pixel_step);

    /* render Y plane to the Y framebuffer and read Y plane data */
    GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, m_fbo_y.fbo));
    m_plane_coef.send_vec4f(m_y_plane_coefs);
    GL_CALL(glViewport(0, 0, m_fbo_y.width, m_fbo_y.height));
    GL_CALL(glDrawArrays(GL_TRIANGLE_STRIP, 0, drawing_plane_vert_count));
    GL_CALL(glReadPixels(0, 0, m_fbo_y.width, m_fbo_y.height, GL_RGBA, GL_UNSIGNED_BYTE, output.data.get()));

    /* pixel step used in the shader to access neighboring pixels */
    pixel_step[0] *= 2.0;
    pixel_step[1] *= 2.0;
    m_pixel_step.send_vec2f(pixel_step);

    /* render U and V planes to the UV framebuffer and read UV planes data */
    GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, m_fbo_uv.fbo));
    m_plane_coef.send_vec4f(m_u_plane_coefs);
    GL_CALL(glViewport(0, 0, m_fbo_uv.width, m_fbo_uv.height / 2));
    GL_CALL(glDrawArrays(GL_TRIANGLE_STRIP, 0, drawing_plane_vert_count));
    m_plane_coef.send_vec4f(m_v_plane_coefs);
    GL_CALL(glViewport(0, m_fbo_uv.height / 2, m_fbo_uv.width, m_fbo_uv.height / 2));
    GL_CALL(glDrawArrays(GL_TRIANGLE_STRIP, 0, drawing_plane_vert_count));
    GL_CALL(glReadPixels(0, 0, m_fbo_uv.width, m_fbo_uv.height, GL_RGBA, GL_UNSIGNED_BYTE, output.data.get() + m_width * m_height ));

    /* unbind all */
    GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    GL_CALL(glBindVertexArray(0));
    GL_CALL(glBindTexture(GL_TEXTURE_2D, 0));
    GL_CALL(glDisableVertexAttribArray(0));
    GL_CALL(glDisableVertexAttribArray(1));
    m_shader.unuse();
}

/* yuv_converter::create_framebuffer */
yuv_converter::framebuffer yuv_converter::create_framebuffer(int width, int height)
{
    uint32_t fbo;
    uint32_t tex;
    GL_CALL(glGenFramebuffers(1, &fbo));
    GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, fbo));

    GL_CALL(glGenTextures(1, &tex));
    GL_CALL(glBindTexture(GL_TEXTURE_2D, tex));
    GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

    GL_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0));
    uint32_t attach[] {GL_COLOR_ATTACHMENT0};
    GL_CALL(glDrawBuffers(1, attach));

    GL_CALL(glBindTexture(GL_TEXTURE_2D, 0));
    if( GLenum e = glCheckFramebufferStatus(GL_FRAMEBUFFER); e != GL_FRAMEBUFFER_COMPLETE ) {
        put_error_message( "create_framebuffer() error: glCheckFramebufferStatus(GL_FRAMEBUFFER)"
                " != GL_FRAMEBUFFER_COMPLETE",
                to_gl_check_framebuffer_status(e));
    }
    GL_CALL(glBindFramebuffer( GL_FRAMEBUFFER, 0 ));
    return {fbo, tex, width, height};
}

/* yuv_converter::delete_framebuffer */
void yuv_converter::delete_framebuffer(yuv_converter::framebuffer& fbo)
{
    if (fbo.texture) {
        GL_CALL(glDeleteTextures(1, &fbo.texture));
    }
    if (fbo.fbo) {
        GL_CALL(glDeleteFramebuffers(1, &fbo.fbo));
    }
    fbo = {0, 0, 0, 0};
}

} /* namespace bnb::converter */
