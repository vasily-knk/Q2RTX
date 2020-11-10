#include <sstream>
#include <string>
#include <fstream>
#include <iomanip>

#include "streaming_stuff.h"


#include <unordered_map>
#include <boost/optional.hpp>

#include <memory>
#include <array>

#include <thread>

#include "vk2gl_converter.h"
#include "vr_streaming/vr_streaming_server.h"

__pragma(comment(lib, "vr_streaming_server.lib"))

namespace
{

int check_fence(void * fence)
{
    return streaming_stuff_check_fence(fence);
}



struct streaming_stuff
{
    explicit streaming_stuff(bool rtx)
        : rtx_(rtx)
		, streaming_server_(vr_streaming::create_streaming_server())
    {

        if (rtx)
        {
            //vk2gl_ = create_vk2gl_converter();
            int aaa = 5;
        }
    }
    
    
    void dump_bsp_mesh(float const *verts, int num_verts, char const * /*map_name*/)
    {
        streaming_server_->dump_bsp_mesh(0, verts, num_verts);
    }

    void send_frame(void *vk_image, unsigned width, unsigned height, unsigned full_width, unsigned full_height, void *fence)
    {
        //user_data_.check_fence = check_fence;
        //user_data_.fence = fence;
        //
        //

        vr_streaming::frame_texture_t tex;

        if (rtx_)
        {
            if (vk2gl_)
            {
                vk2gl_->update(vk_image, width, height, full_width, full_height, fence);
                tex = vr_streaming::make_opengl_texture(vk2gl_->get_gl_texture());

                vk2gl_->set_context();
            }
            else
            {
                tex = vr_streaming::make_vulkan_texture(vk_image);
            }
        }
        else
        {
            tex = vr_streaming::make_dummy_texture();
        }

        vr_streaming::video_frame_t video_frame;
        video_frame.tex = tex;

        video_frame.width = width;
        video_frame.height = height;
        video_frame.full_width = full_width;
        video_frame.full_height = full_height;
        video_frame.check_fence = check_fence;
        video_frame.fence = fence;

        streaming_server_->enqueue_frame(video_frame);
        if (rtx_ && vk2gl_)
        {
            vk2gl_->restore_context();
        }
        //std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    void set_matrices(float const *vieworg, float const *viewangles, float const *proj)
    {
        float viewangles_transformed[] = { 90.f - viewangles[1], -viewangles[0], viewangles[2] };
        float proj_transformed[16];
        memcpy(proj_transformed, proj, sizeof(proj_transformed));

        // todo: fact hack, may break any minute!
        //
        if (rtx_)
        {
            for (float &v : proj_transformed)
                v *= -1.f;
            
            proj_transformed[0] *= -1.f;
        }

        streaming_server_->set_matrices_quake2(vieworg, viewangles_transformed, proj_transformed);
    }

    void send_text(char const *text)
    {
        streaming_server_->send_text(text);
    }

private:
    bool rtx_;
    vr_streaming::streaming_server_uptr streaming_server_;

    vk2gl_converter_uptr vk2gl_;
};

std::unique_ptr<streaming_stuff> g_streaming_stuff;

} // namespace

void streaming_stuff_dump_bsp_mesh(float const *verts, int num_verts, char const *map_name)
{
    if (false)
    {
        std::string const obj_name = std::string(map_name) + ".obj";
        
        std::ofstream s(obj_name);

        s << std::fixed << std::setprecision(6);
        for (size_t i = 0; i < num_verts; ++i)
        {
            auto const *p = verts + i * 3;
            s << "v " << p[0] << " " << p[1] << " " << p[2] << "\n";
        }

        s << std::endl;

        for (size_t i = 0; i < num_verts / 3; ++i)
            s << "f " << i * 3 + 1 << " " << i * 3 + 2 << " " << i * 3 + 3 << "\n";

    }

    if (g_streaming_stuff)
        g_streaming_stuff->dump_bsp_mesh(verts, num_verts, map_name);
}

void streaming_stuff_init(int enabled, int rtx)
{
    if (enabled)
        g_streaming_stuff = std::make_unique<streaming_stuff>(rtx != 0);
}

void streaming_stuff_send_frame(void *vk_image, unsigned width, unsigned height, unsigned full_width, unsigned full_height, void *fence)
{
    if (g_streaming_stuff)
        g_streaming_stuff->send_frame(vk_image, width, height, full_width, full_height, fence);
}

void streaming_stuff_shutdown()
{
    g_streaming_stuff.reset();
}

void streaming_stuff_set_matrices(float const *vieworg, float const *viewangles, float const *proj)
{
    if (g_streaming_stuff)
        g_streaming_stuff->set_matrices(vieworg, viewangles, proj);
}

void streaming_stuff_send_text(char const *text)
{
    if (g_streaming_stuff)
        g_streaming_stuff->send_text(text);
}


