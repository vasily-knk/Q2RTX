#include <sstream>
#include <string>
#include <fstream>
#include <iomanip>

#include "streaming_stuff.h"

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
}
