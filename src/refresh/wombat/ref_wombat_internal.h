#pragma once

#include <utility>
#include <memory>
#include <functional>

#include <wombat_android_test/wombat_android_test_fwd.h>


struct ref_wombat_internal
{
    using vec3_t = std::array<float, 3>;
    using log_f = std::function<void(std::string const&)>;

    virtual ~ref_wombat_internal() = default;

    virtual void dump_bsp_vertices(float const *verts, size_t num_verts) = 0;
    virtual void update_matrices(float const *vieworg, float const *viewangles, float fovx, float fovy) = 0;
    virtual void send_frame(uint32_t tex_id) = 0;

    virtual void update_resolution(unsigned width, unsigned height) = 0;

    virtual void init_streaming_client() = 0;

    static void fill_view_matrix(float const *vieworg, float const *viewangles, float *dst_matrix);

};

using ref_wombat_internal_uptr = std::unique_ptr<ref_wombat_internal> ;

ref_wombat_internal_uptr create_ref_wombat_internal(std::shared_ptr<wombat_android_test::iface> iface, ref_wombat_internal::log_f log);

