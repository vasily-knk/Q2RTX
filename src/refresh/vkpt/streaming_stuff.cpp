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

#include "vk2gl_converter.h"


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

int check_fence(void * fence)
{
    return streaming_stuff_check_fence(fence);
}


webstream::stream_server_settings const &get_webstream_server_settings()
{
    static auto const val = []()
    {
        webstream::stream_server_settings s;

        s.listen_port = 9002;

        {
            auto &as = s.adj_sett;

            as.max_ping_value = 250;
            as.max_stable_ping_value = 150;
            as.bandwidth_averager_n = 30;
            as.bandwidth_vote_n = 2;
            as.ping_averager_size = 3;
            as.min_bandwidth_data_size = 4194304;
            as.bandwidth_check_interval = 4000;
            as.optimistic_bitrate_portion = 1.2;
            as.bitrate_reduction_portion = 0.7;
            as.bitrate_counter_window = 1000;
            as.encode_counter_window = 1000;
            as.server_stat_sending_interval = 3000;
            as.server_ping_request_interval = 1500;
            as.min_fps_raw_delta = 1.0;
            as.min_fps_mux_delta = 2.0;
            as.frame_rate_adj_counter = 2;
            as.frame_rate_low_portion = 0.85;
            as.resolution_low_portion = 0.9;
            as.high_ping_relax_number = 3;
            as.ping_ratio_to_lower_bitrate = 5.0;
            as.bitrate_lower_ratio = 0.85;
            as.bitrate_raise_ratio = 1.5;
            as.bandwidth_to_bitrate_portion = 0.8;
            as.max_bitrate = 8388608;
            as.min_bitrate = 786432;
        }

        s.muxer_settings.num_frames_in_blob = -1;

        return s;
    }();

    return val;
}

struct streaming_stuff
    : webstream::stream_server_callback
{
    explicit streaming_stuff(bool rtx)
        : rtx_(rtx)
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
        stream_server_ = webstream::create_websocket_stream_server(get_webstream_server_settings(), this);
        stream_server_->set_stream_policy(stream_policy_.get());
        encoder_  = webstream::create_nvenc_encoder();

	    stream_server_->set_active_encoder(encoder_.get());        

        stream_server_->start();

        if (rtx)
        {
            vk2gl_ = create_vk2gl_converter();
            int aaa = 5;
        }
    }
    
    
    void dump_bsp_mesh(float const *verts, int num_verts, char const * /*map_name*/)
    {
        vr_streaming::cmd_mesh_t cmd;

        static_assert(sizeof(decltype(cmd.points)::value_type) == sizeof(geom::point_3f), "Invalid vertex size");
        cmd.points.resize(num_verts);
        memcpy(cmd.points.data(), verts, sizeof(geom::point_3f) * num_verts);

        append_cmd(cmd);
    }

    void send_frame(void *vk_image, unsigned width, unsigned height, unsigned full_width, unsigned full_height, void *fence)
    {
        user_data_.check_fence = check_fence;
        user_data_.fence = fence;

        binary::output_stream os;

        user_data_.w_ratio = 1.f;//float(width) / float(full_width);
        user_data_.h_ratio = 1.f;//float(height) / float(full_height);

        binary::write(os, user_data_);

        binary::write(os, cmd_bytes_);
        cmd_bytes_.clear();

        uint32_t gl_tex = 0;
        if (rtx_)
        {
            if (vk2gl_)
            {
                vk2gl_->update(vk_image, width, height, full_width, full_height, fence);
                gl_tex = vk2gl_->get_gl_texture();

                vk2gl_->set_context();
            }
        }
        else
            gl_tex = uint32_t(vk_image);


        if (encoder_)
        {
            webstream::input_video_frame const frame(
                    width, 
                    height, 
                    webstream::video_format(webstream::video_format::pixel_format_t::Format_RGBA32), 
                    gl_tex,
                    true)
            ;



            bool const ok = encoder_->enqueue_frame(frame, reinterpret_cast<uint8_t*>(os.data()), os.size());

            assert(ok);
        }
        if (rtx_ && vk2gl_)
        {
            vk2gl_->restore_context();
        }
        //std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    void set_matrices(float const *vieworg, float const *viewangles, float const *proj)
    {
        auto &sp = user_data_.scene_params;

        //sp.state.orien = !tr.rotation().quaternion();
        
        geom::cprf const orien(90.f - viewangles[1], -viewangles[0], viewangles[2]);

        sp.state.orien = geom::quaternionf(orien);
        sp.state.global_pos = geom::point_3(vieworg[0], vieworg[1], vieworg[2]);
        sp.desired_orien = sp.state.orien;

        memcpy(sp.proj.data(), proj, sizeof(float) * 16);

        
        // todo: fact hack, may break any minute!
        //
        if (rtx_)
        {
            for (float &v : sp.proj)                        
                v *= -1.f;
            
            sp.proj[0] *= -1.f;
        }
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
    bool rtx_;

    webstream::stream_server_ptr stream_server_;
    std::unique_ptr<webstream::stream_policy> stream_policy_;
    webstream::encoder_ptr encoder_;

    vr_streaming::user_data_t user_data_;

    std::vector<binary::bytes_t> cmd_bytes_;

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


