#ifndef __STREAMING_STUFF_H__
#define __STREAMING_STUFF_H__

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#include "shared/shared.h"
#include "refresh/refresh.h"



void streaming_stuff_dump_bsp_mesh(float const *verts, int num_verts, char const *map_name);

void streaming_stuff_init(int enabled, int rtx);
void streaming_stuff_send_frame(void *vk_image, unsigned width, unsigned height, unsigned full_width, unsigned full_height, void *fence);
void streaming_stuff_shutdown();

void streaming_stuff_set_matrices(refdef_t const *rd);

void streaming_stuff_send_text(char const *text);

int streaming_stuff_check_fence(void *fence);

void streaming_stuff_override_view(refdef_t *rd);
void streaming_stuff_restore_view(refdef_t *rd);

void streaming_stuff_dump_csv(char const *filename);



#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __STREAMING_STUFF_H__