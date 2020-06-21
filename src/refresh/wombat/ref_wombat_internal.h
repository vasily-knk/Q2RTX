#pragma once

#include <utility>

#include <wombat_android_test/wombat_android_test_fwd.h>

namespace ref_wombat_internal
{
using vec3_t = std::array<float, 3>;

void fill_view_matrix(float const *vieworg, float const *viewangles, float *dst_matrix);

void dump_bsp_vertices(float const *verts, size_t num_verts, wombat_android_test::iface *iface);

} // namespace ref_wombat_internal