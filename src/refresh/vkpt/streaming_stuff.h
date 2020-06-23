#ifndef __STREAMING_STUFF_H__
#define __STREAMING_STUFF_H__

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

void streaming_stuff_dump_bsp_mesh(float const *verts, int num_verts, char const *map_name);

void streaming_stuff_init();
void streaming_stuff_send_frame(void *vk_image, int rtx, unsigned width, unsigned height, void *fence);
void streaming_stuff_shutdown();

void streaming_stuff_set_matrices(float const *vieworg, float const *viewangles, float const *proj);

void streaming_stuff_send_text(char const *text);

int streaming_stuff_check_fence(void *fence);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __STREAMING_STUFF_H__