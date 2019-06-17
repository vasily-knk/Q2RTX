#include <sstream>
#include <string>
#include <fstream>
#include <iomanip>

#include "streaming_stuff.h"

#include <unordered_map>
#include <boost/optional.hpp>

#include <memory>
#include <array>

#define Assert assert
#define Verify assert
#define CG_PRIMITIVES
#define NO_BOOST_ANY_VARIANT

#include "geometry/primitives_fwd.h"
#include "geometry/primitives/point.h"
#include "geometry/primitives/quaternion.h"
#include "geometry/primitives/transform.h"
#include "serialization/io_streams_ops.h"
#include "reflection/reflection.h"
#include "reflection/proc/io_streams_refl.h"
#include <thread>

namespace geom
{
    template<class S, class processor>
    REFL_STRUCT_BODY(point_t<S, 3>)
        REFL_ENTRY(x)
        REFL_ENTRY(y)
        REFL_ENTRY(z)
    REFL_END()
    
    template<class S, class processor>
    REFL_STRUCT_BODY(quaternion_t<S>)
        REFL_ENTRY(w)
        REFL_ENTRY(v)
    REFL_END()
}

#include "vr_streaming/user_data.h"
#include "vr_streaming/cmds.h"

#include "webstream/std_stream_policy.h"
#include "webstream/stream_server.h"

__pragma(comment(lib, "ws_server64.lib"))

namespace
{

struct streaming_stuff
    : webstream::stream_server_callback
{
    streaming_stuff()
    {
        webstream::ws_init_logging();

        webstream::stream_quality_t quality;
        quality.max_resolution = {1920, 1280};
        quality.bitrate = 8 * 1024 * 1024;
        quality.frame_rate = 20;

        webstream::std_stream_policy::stream_policy_t policy;
        for (auto &s : policy)
            s = {quality};


        stream_policy_ = std::make_unique<webstream::std_stream_policy>(policy);
        stream_server_ = webstream::create_websocket_stream_server(webstream::stream_server_settings(), this);
        stream_server_->set_stream_policy(stream_policy_.get());
        encoder_  = webstream::create_nvenc_encoder();

	    stream_server_->set_active_encoder(encoder_.get());        

        stream_server_->start();
    }
    
    
    void dump_bsp_mesh(float const *verts, int num_verts, char const * /*map_name*/)
    {
        vr_streaming::cmd_mesh_t cmd;

        static_assert(sizeof(decltype(cmd.points)::value_type) == sizeof(geom::point_3f), "Invalid vertex size");
        cmd.points.resize(num_verts);
        memcpy(cmd.points.data(), verts, sizeof(geom::point_3f) * num_verts);

        append_cmd(cmd);
    }

    void send_frame(uint64_t vk_image, unsigned width, unsigned height)
    {
        user_data_.vk_image = vk_image;
        
        binary::output_stream os;
        binary::write(os, user_data_);

        binary::write(os, cmd_bytes_);
        cmd_bytes_.clear();

        if (encoder_)
        {
            bool const ok = encoder_->enqueue_frame(
                webstream::input_video_frame(
                width, 
                height, 
                webstream::video_format(webstream::video_format::pixel_format_t::Format_RGBA32), 
                -1,
                true), reinterpret_cast<uint8_t*>(os.data()), os.size());

            assert(ok);
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    void set_matrices(float const *vieworg, float const *viewangles, float const *proj)
    {
        auto &sp = user_data_.scene_params;

        //sp.state.orien = !tr.rotation().quaternion();
        
        geom::cprf const orien(90.f - viewangles[1], -viewangles[0], viewangles[2]);

        sp.state.orien = geom::quaternionf(orien);
        sp.state.global_pos = geom::point_3(vieworg[0], vieworg[1], vieworg[2]);

        memcpy(sp.proj.data(), proj, sizeof(float) * 16);
    }

    void send_text(char const *text)
    {
        vr_streaming::cmd_test_t cmd;

        cmd.text = text;

        append_cmd(cmd);
    }

public:
    void on_client_connected() override
    {
        int aaa = 5;
    }

    void on_server_exiting(const char* reason) override
    {
        
    }

    void on_resolution_requested(webstream::video_resolution const& target_resolution) override
    {
        
    }

private:

    template<typename cmd_t>
    void append_cmd(cmd_t const &cmd)
    {
        binary::bytes_t bytes;

        binary::write(bytes, cmd.id());
        binary::write(bytes, cmd);

        cmd_bytes_.push_back(std::move(bytes));
    }

private:
    webstream::stream_server_ptr stream_server_;
    std::unique_ptr<webstream::stream_policy> stream_policy_;
    webstream::encoder_ptr encoder_;

    vr_streaming::user_data_t user_data_;

    std::vector<binary::bytes_t> cmd_bytes_;
};

std::unique_ptr<streaming_stuff> g_streaming_stuff;

} // namespace

void streaming_stuff_dump_bsp_mesh(float const *verts, int num_verts, char const *map_name)
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

    g_streaming_stuff->dump_bsp_mesh(verts, num_verts, map_name);
}

void streaming_stuff_init()
{
    g_streaming_stuff = std::make_unique<streaming_stuff>();
}

void streaming_stuff_send_frame(uint64_t vk_image, unsigned width, unsigned height)
{
    g_streaming_stuff->send_frame(vk_image, width, height);
}

void streaming_stuff_shutdown()
{
    g_streaming_stuff.reset();
}

void streaming_stuff_set_matrices(float const *vieworg, float const *viewangles, float const *proj)
{
    g_streaming_stuff->set_matrices(vieworg, viewangles, proj);
}

void streaming_stuff_send_text(char const *text)
{
    g_streaming_stuff->send_text(text);
}


