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

namespace ref_wombat_internal
{
void fill_view_matrix(float const *vieworg, float const *viewangles, float *dst_matrix)
{
    geom::cprf const orien(90.f - viewangles[1], -viewangles[0], viewangles[2]);
    geom::point_3f const pos;//(vieworg[0], vieworg[1], vieworg[2]);

    geom::transform_4f const tr(geom::as_translation(pos), orien);

    auto const &m = tr.direct_matrix();

    memcpy(dst_matrix, m.rawdata(), 16 * sizeof(float));
}
    
}
