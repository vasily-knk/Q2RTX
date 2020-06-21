#include <iostream>
#include <fstream>
#include <memory>

#include <SDL.h>
#include <SDL_syswm.h>

#include "wombat_android_test/wombat_android_test.h"

extern "C"
{

#include "shared/shared.h"
#include "refresh/refresh.h"
#include "refresh/images.h"
#include "refresh/models.h"
#include "client/video.h"



void R_RegisterFunctionsWombat();

    int registration_sequence;

    extern SDL_Window       *sdl_window;

}

__pragma(comment(lib, "wombat_android_test.lib"))

#define LOGME() // log_stream << __FUNCTION__ << std::endl;

namespace
{
    namespace wat = wombat_android_test;

    std::ofstream log_stream("D:\\log.txt");
    std::shared_ptr<wat::iface> g_iface;

    bool g_valid_fbo = false;
    uint32_t g_fbo_width = 0;
    uint32_t g_fbo_height = 0;
} // namespace

qboolean R_Init_Wombat(qboolean total)
{
    LOGME();

	registration_sequence = 1;

	if (!VID_Init(GAPI_VULKAN)) {
		Com_Error(ERR_FATAL, "VID_Init failed\n");
		return qfalse;

	}

    IMG_Init();

    g_iface = std::shared_ptr<wat::iface>(
        wat::create({false}),
        wat::destroy
    );

    SDL_SysWMinfo wmInfo;

    SDL_VERSION(&wmInfo.version);
    SDL_GetWindowWMInfo(sdl_window, &wmInfo);
    HWND const hwnd = wmInfo.info.win.window;

    g_iface->init({
        hwnd,
        nullptr
    });

    g_valid_fbo = false;

    return qtrue;
}

void R_Shutdown_Wombat(qboolean total) { LOGME(); }
void R_BeginRegistration_Wombat(const char *map)
{
    LOGME();
    registration_sequence++;
}

void R_SetSky_Wombat(const char *name, float rotate, vec3_t axis) { LOGME(); }
void R_EndRegistration_Wombat(void) { LOGME(); }
void R_RenderFrame_Wombat(refdef_t *fd)
{
    if (!g_valid_fbo)
    {
        g_iface->invalidate_fbo(g_fbo_width, g_fbo_height);
        g_valid_fbo = true;
    }

    wombat_android_test::update_args_t update_args;

    float matrix[16] = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1,
    };

    {
        vec3_t viewaxis[3];

        AnglesToAxis(fd->viewangles, viewaxis);

        {
            size_t constexpr i = 1;
            matrix[0] = -viewaxis[i][0];
            matrix[4] = -viewaxis[i][1];
            matrix[8] = -viewaxis[i][2];
            matrix[12] = 0;//DotProduct(viewaxis[i], fd->vieworg);
        }

        {
            size_t constexpr i = 2;

            matrix[1] = viewaxis[i][0];
            matrix[5] = viewaxis[i][1];
            matrix[9] = viewaxis[i][2];
            matrix[13] = 0;//-DotProduct(viewaxis[i], fd->vieworg);
        }

        {
            size_t constexpr i = 0;

            matrix[2] = -viewaxis[i][0];
            matrix[6] = -viewaxis[i][1];
            matrix[10] = -viewaxis[i][2];
            matrix[14] = 0;//DotProduct(viewaxis[i], fd->vieworg);
        }

        matrix[3] = 0;
        matrix[7] = 0;
        matrix[11] = 0;
        matrix[15] = 1;
    }
    
    update_args.head_matrix = matrix;
    update_args.time_delta = 0;

    g_iface->update(update_args);

    wombat_android_test::render_pass_args_t pass_args;

    pass_args.vp_rect = {
        0,
        0,
        int32_t(g_fbo_width),
        int32_t(g_fbo_height),
    };

    float const hfov = fd->fov_x * 0.5f;
    float const vfov = fd->fov_y * 0.5f;

    pass_args.fov_rect = {
        hfov, hfov,
        vfov, vfov,
    };

    float eye_matrix[16] = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1,
    };

    pass_args.eye_matrix = eye_matrix;

    wombat_android_test::render_args_t render_args;
    render_args.num_passes = 1;
    render_args.passes = &pass_args;

    g_iface->render(render_args);
    g_iface->swap_buffers();
}

void R_LightPoint_Wombat(vec3_t origin, vec3_t light) { LOGME(); }
void R_ClearColor_Wombat(void) { LOGME(); }
void R_SetAlpha_Wombat(float clpha) { LOGME(); }
void R_SetAlphaScale_Wombat(float alpha) { LOGME(); }
void R_SetColor_Wombat(uint32_t color) { LOGME(); }
void R_SetClipRect_Wombat(const clipRect_t *clip) { LOGME(); }
void R_SetScale_Wombat(float scale) { LOGME(); }
void R_DrawChar_Wombat(int x, int y, int flags, int ch, qhandle_t font) { LOGME(); }
int R_DrawString_Wombat(int x, int y, int flags, size_t maxChars,
	const char *string, qhandle_t font)
{
    LOGME();
    return 0;
}

void R_DrawPic_Wombat(int x, int y, qhandle_t pic) { LOGME(); }
void R_DrawStretchPic_Wombat(int x, int y, int w, int h, qhandle_t pic) { LOGME(); }
void R_TileClear_Wombat(int x, int y, int w, int h, qhandle_t pic) { LOGME(); }
void R_DrawFill8_Wombat(int x, int y, int w, int h, int c) { LOGME(); }
void R_DrawFill32_Wombat(int x, int y, int w, int h, uint32_t color) { LOGME(); }
void R_BeginFrame_Wombat(void)
{

}
void R_EndFrame_Wombat(void) {

}
void R_ModeChanged_Wombat(int width, int height, int flags, int rowbytes, void *pixels)
{ 
    g_valid_fbo = false;
    g_fbo_width = width;
    g_fbo_height = height;
}
void R_AddDecal_Wombat(decal_t *d) { LOGME(); }
qboolean R_InterceptKey_Wombat(unsigned key, qboolean down)
{
    LOGME();
    return qfalse;
}

void IMG_Unload_Wombat(image_t *image) { LOGME(); }
void IMG_Load_Wombat(image_t *image, byte *pic)
{
    log_stream << __FUNCTION__ << ": " << image->name << std::endl;

    image->pix_data = pic;
}
byte*  IMG_ReadPixels_Wombat(int *width, int *height, int *rowbytes)
{
    LOGME();
    *width = 0;
    *height = 0;
    *rowbytes = 0;
    return nullptr;
}

qerror_t MOD_LoadMD2_Wombat(model_t *model, const void *rawdata, size_t length) { return 0;}
qerror_t MOD_LoadMD3_Wombat(model_t *model, const void *rawdata, size_t length) { return 0;}
void MOD_Reference_Wombat(model_t *model) {}



void R_RegisterFunctionsWombat()
{
	R_Init = R_Init_Wombat;
	R_Shutdown = R_Shutdown_Wombat;
	R_BeginRegistration = R_BeginRegistration_Wombat;
	R_EndRegistration = R_EndRegistration_Wombat;
	R_SetSky = R_SetSky_Wombat;
	R_RenderFrame = R_RenderFrame_Wombat;
	R_LightPoint = R_LightPoint_Wombat;
	R_ClearColor = R_ClearColor_Wombat;
	R_SetAlpha = R_SetAlpha_Wombat;
	R_SetAlphaScale = R_SetAlphaScale_Wombat;
	R_SetColor = R_SetColor_Wombat;
	R_SetClipRect = R_SetClipRect_Wombat;
	R_SetScale = R_SetScale_Wombat;
	R_DrawChar = R_DrawChar_Wombat;
	R_DrawString = R_DrawString_Wombat;
	R_DrawPic = R_DrawPic_Wombat;
	R_DrawStretchPic = R_DrawStretchPic_Wombat;
	R_TileClear = R_TileClear_Wombat;
	R_DrawFill8 = R_DrawFill8_Wombat;
	R_DrawFill32 = R_DrawFill32_Wombat;
	R_BeginFrame = R_BeginFrame_Wombat;
	R_EndFrame = R_EndFrame_Wombat;
	R_ModeChanged = R_ModeChanged_Wombat;
	R_AddDecal = R_AddDecal_Wombat;
	R_InterceptKey = R_InterceptKey_Wombat;
	IMG_Load = IMG_Load_Wombat;
	IMG_Unload = IMG_Unload_Wombat;
	IMG_ReadPixels = IMG_ReadPixels_Wombat;
	MOD_LoadMD2 = MOD_LoadMD2_Wombat;
	MOD_LoadMD3 = MOD_LoadMD3_Wombat;
	MOD_Reference = MOD_Reference_Wombat;    
}
