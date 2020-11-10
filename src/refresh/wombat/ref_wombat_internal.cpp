#include "ref_wombat_internal.h"

#include <sstream>
#include <string>
#include <fstream>
#include <iomanip>

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

#include "wombat_android_test/wombat_android_test.h"

#include "streaming_client/streaming_client.h"

#include "vr_streaming/network_cmd_server.h"

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

__pragma(comment(lib, "network_cmd_server.lib"))

namespace
{

} // namespace


struct ref_wombat_internal_impl
    : ref_wombat_internal
    , vr_streaming::network_cmd_client_callbacks
{
    using iface_ptr = std::shared_ptr<wombat_android_test::iface>;

    explicit ref_wombat_internal_impl(iface_ptr iface, ref_wombat_internal::log_f log)
        : iface_(iface)
        , log_(log)
        , scene_params_()
    {
        
    }


    void dump_bsp_vertices(float const* verts, size_t num_verts) override
    {
        // auto os = begin_frame_data();
        //
        // binary::write(os, uint32_t(1));
        //
        // binary::bytes_t cmd_bytes;
        // {
        //     vr_streaming::cmd_mesh_t cmd;
        //
        //     auto const *src = reinterpret_cast<geom::point_3f const*>(verts);
        //     cmd.points.assign(src, src + num_verts);
        //
        //     binary::write(cmd_bytes, cmd.id());
        //     binary::write(cmd_bytes, cmd);
        // }
        //
        // binary::write(os, cmd_bytes);
        //
        // end_frame_data(os);
    }

    void update_matrices(float const* vieworg, float const* viewangles, float fovx, float fovy) override
    {
        auto &sp = scene_params_;

        geom::cprf const orien(90.f - viewangles[1], -viewangles[0], viewangles[2]);

        sp.state.orien = geom::quaternionf(orien);
        sp.state.global_pos = geom::point_3(vieworg[0], vieworg[1], vieworg[2]);
        sp.desired_orien = sp.state.orien;

        {
            auto &matrix = sp.proj;

            float const znear = 2.;

            float const zfar = 2048;

            float const ymax = znear * tan(fovy * geom::pi / 360.0);
            float const ymin = -ymax;

            float const xmax = znear * tan(fovx * geom::pi / 360.0);
            float const xmin = -xmax;

            float const width = xmax - xmin;
            float const height = ymax - ymin;
            float const depth = zfar - znear;

            matrix[0] = 2 * znear / width;
            matrix[4] = 0;
            matrix[8] = (xmax + xmin) / width;
            matrix[12] = 0;

            matrix[1] = 0;
            matrix[5] = 2 * znear / height;
            matrix[9] = (ymax + ymin) / height;
            matrix[13] = 0;

            matrix[2] = 0;
            matrix[6] = 0;
            matrix[10] = -(zfar + znear) / depth;
            matrix[14] = -2 * zfar * znear / depth;

            matrix[3] = 0;
            matrix[7] = 0;
            matrix[11] = -1;
            matrix[15] = 0;
        }
    }

    void send_frame(uint32_t tex_id) override
    {
        // tex_id_ = tex_id;
        // auto os = begin_frame_data();
        //
        // end_frame_data(os);
    }

    void update_resolution(unsigned width, unsigned height) override
    {
        resolution_ = geom::point_2ui(width, height);
    }

    void init_streaming_client() override
    {
        cmd_client_.reset();
        cmd_client_ = vr_streaming::create_network_cmd_client(this);
    }


    void send_cmd(char const* data, uint32_t size) override
    {
        iface_->get_cmd_server().send_cmd(data, size);
    }



    /* void on_frame(vr_streaming::frame_t const& frame, uint8_t const* user_data_ptr, uint32_t user_data_size) override
    {
        wombat_android_test::frame_data_t const data = {
            frame,
            user_data_ptr,
            user_data_size
        };

        iface_->enqueue_frame(data);
    }*/

    void on_connected() override
    {
        log_("Streaming client connected");
    }

    void on_refused() override
    {
        cmd_client_.reset();
        log_("Streaming client connection refused");
    }

    void on_disconnected() override
    {
        cmd_client_.reset();
        log_("Streaming client disconnected");
    }
private:

    // binary::output_stream begin_frame_data()
    // {
    //     binary::output_stream os;
    //
    //     vr_streaming::user_data_t user_data;
    //     user_data.scene_params = scene_params_;
    //     binary::write(os, user_data);
    //
    //     return os;
    //     
    // }
    //
    // void end_frame_data(binary::output_stream const &os)
    // {
    //     vr_streaming::frame_t frame;
    //
    //     frame.tex_id = tex_id_;
    //     frame.width = resolution_.x;
    //     frame.height = resolution_.y;
    //
    //     wombat_android_test::frame_data_t const data = {
    //         frame,
    //         reinterpret_cast<uint8_t const *>(os.data()),
    //         uint32_t(os.size())
    //     };
    //
    //     iface_->enqueue_frame(data);
    // }
    
private:
    iface_ptr iface_;
    log_f log_;
    vr_streaming::scene_params_t scene_params_;
    geom::point_2ui resolution_;

    vr_streaming::network_cmd_client_uptr cmd_client_;

    uint32_t tex_id_ = 0;
};


void ref_wombat_internal::fill_view_matrix(float const *vieworg, float const *viewangles, float *dst_matrix)
{
    geom::cprf const orien(90.f - viewangles[1], -viewangles[0], viewangles[2]);
    geom::point_3f const pos(vieworg[0], vieworg[1], vieworg[2]);

    geom::transform_4f const tr(geom::as_translation(pos), orien);

    auto const &m = tr.inverse_matrix();

    memcpy(dst_matrix, m.rawdata(), 16 * sizeof(float));
}


ref_wombat_internal_uptr create_ref_wombat_internal(std::shared_ptr<wombat_android_test::iface> iface, ref_wombat_internal::log_f log)
{
    return std::make_unique<ref_wombat_internal_impl>(iface, log);
}
