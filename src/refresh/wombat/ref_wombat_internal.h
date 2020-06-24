#pragma once

#include <utility>
#include <memory>

#include <wombat_android_test/wombat_android_test_fwd.h>


struct ref_wombat_internal
{
    using vec3_t = std::array<float, 3>;


    virtual ~ref_wombat_internal() = default;

    virtual void dump_bsp_vertices(float const *verts, size_t num_verts) = 0;
    virtual void update_matrices(float const *vieworg, float const *viewangles, float fovx, float fovy) = 0;
    virtual void send_frame() = 0;

    static void fill_view_matrix(float const *vieworg, float const *viewangles, float *dst_matrix);

};

using ref_wombat_internal_uptr = std::unique_ptr<ref_wombat_internal> ;

ref_wombat_internal_uptr create_ref_wombat_internal(std::shared_ptr<wombat_android_test::iface> iface);

