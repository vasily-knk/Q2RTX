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
#include "serialization/io_streams_ops.h"
#include "reflection/reflection.h"
#include "reflection/proc/io_streams_refl.h"

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


namespace
{

struct streaming_stuff
{
    void dump_bsp_mesh(float const *verts, int num_verts, char const * /*map_name*/)
    {
        
        static_assert(sizeof(decltype(bsp_mesh_)::value_type) == sizeof(geom::point_3f), "Invalid vertex size");
        bsp_mesh_.resize(num_verts);
        memcpy(bsp_mesh_.data(), verts, sizeof(geom::point_3f) * num_verts);
    }

    void send_frame()
    {
        
    }
    
private:
    std::vector<geom::point_3f> bsp_mesh_;
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

void streaming_stuff_send_frame()
{
    g_streaming_stuff->send_frame();
}

void streaming_stuff_shutdown()
{
    g_streaming_stuff.reset();
}
