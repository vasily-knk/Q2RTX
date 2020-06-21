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


namespace ref_wombat_internal
{
void fill_view_matrix(float const *vieworg, float const *viewangles, float *dst_matrix)
{
    geom::cprf const orien(90.f - viewangles[1], -viewangles[0], viewangles[2]);
    geom::point_3f const pos(vieworg[0], vieworg[1], vieworg[2]);

    geom::transform_4f const tr(geom::as_translation(pos), orien);

    auto const &m = tr.inverse_matrix();

    memcpy(dst_matrix, m.rawdata(), 16 * sizeof(float));
}


void dump_bsp_vertices(float const *verts, size_t num_verts, wombat_android_test::iface *iface)
{

    binary::output_stream os;

    {
        vr_streaming::user_data_t user_data;
        binary::write(os, user_data);
    }


    binary::write(os, uint32_t(1));

    {
        binary::bytes_t cmd_bytes;
        {
            vr_streaming::cmd_mesh_t cmd;

            auto const *src = reinterpret_cast<geom::point_3f const*>(verts);
            cmd.points.assign(src, src + num_verts);

            binary::write(cmd_bytes, cmd.id());
            binary::write(cmd_bytes, cmd);
        }

        binary::write(os, cmd_bytes);
    }



    wombat_android_test::frame_data_t data = {
        vr_streaming::frame_t(),
        reinterpret_cast<uint8_t*>(os.data()),
        uint32_t(os.size())
    };

    iface->enqueue_frame(data);
}

    
}
