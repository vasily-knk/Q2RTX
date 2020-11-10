#include <cassert>
#include <iostream>
#include <fstream>
#include <memory>
#include <vector>
#include <array>

#include <SDL.h>
#include <SDL_syswm.h>


#include "wombat_android_test/wombat_android_test.h"
#include "ref_wombat_internal.h"


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


    typedef struct bsp_s bsp_t;
    bsp_t *GL_GetBSP();

    vec3_t *extract_bsp_mesh_vertices(bsp_t *bsp, const char* map_name, int *out_num_verts);

}

#define LOGME() // log_stream << __FUNCTION__ << std::endl;

namespace
{
    namespace wat = wombat_android_test;


    struct ref_functions_t
    {
        qboolean(*R_Init)(qboolean total) = NULL;
        void(*R_Shutdown)(qboolean total) = NULL;
        void(*R_BeginRegistration)(const char *map) = NULL;
        void(*R_SetSky)(const char *name, float rotate, vec3_t axis) = NULL;
        void(*R_EndRegistration)(void) = NULL;
        void(*R_RenderFrame)(refdef_t *fd) = NULL;
        void(*R_LightPoint)(vec3_t origin, vec3_t light) = NULL;
        void(*R_ClearColor)(void) = NULL;
        void(*R_SetAlpha)(float clpha) = NULL;
        void(*R_SetAlphaScale)(float alpha) = NULL;
        void(*R_SetColor)(uint32_t color) = NULL;
        void(*R_SetClipRect)(const clipRect_t *clip) = NULL;
        void(*R_SetScale)(float scale) = NULL;
        void(*R_DrawChar)(int x, int y, int flags, int ch, qhandle_t font) = NULL;
        int(*R_DrawString)(int x, int y, int flags, size_t maxChars,
	        const char *string, qhandle_t font) = NULL;
        void(*R_DrawPic)(int x, int y, qhandle_t pic) = NULL;
        void(*R_DrawStretchPic)(int x, int y, int w, int h, qhandle_t pic) = NULL;
        void(*R_TileClear)(int x, int y, int w, int h, qhandle_t pic) = NULL;
        void(*R_DrawFill8)(int x, int y, int w, int h, int c) = NULL;
        void(*R_DrawFill32)(int x, int y, int w, int h, uint32_t color) = NULL;
        void(*R_BeginFrame)(void) = NULL;
        void(*R_EndFrame)(void) = NULL;
        void(*R_ModeChanged)(int width, int height, int flags, int rowbytes, void *pixels) = NULL;
        void(*R_AddDecal)(decal_t *d) = NULL;
        qboolean(*R_InterceptKey)(unsigned key, qboolean down) = NULL;

        void(*IMG_Unload)(image_t *image) = NULL;
        void(*IMG_Load)(image_t *image, byte *pic) = NULL;
        byte* (*IMG_ReadPixels)(int *width, int *height, int *rowbytes) = NULL;

        qerror_t(*MOD_LoadMD2)(model_t *model, const void *rawdata, size_t length) = NULL;
        #if USE_MD3
        qerror_t(*MOD_LoadMD3)(model_t *model, const void *rawdata, size_t length) = NULL;
        #endif
        void(*MOD_Reference)(model_t *model) = NULL;        
    };

    enum struct ref_mode_e
    {
        wombat,
        gl,
        wombat_only,


        num_modes,
    };

    HMODULE g_wat_dll = nullptr;


    std::shared_ptr<wat::iface> g_iface;
    ref_wombat_internal_uptr g_internal;

    bool g_valid_fbo = false;
    uint32_t g_fbo_width = 0;
    uint32_t g_fbo_height = 0;

    ref_functions_t g_gl_functions;

    std::vector<ref_wombat_internal::vec3_t> g_bsp_verts;

    ref_mode_e g_ref_mode = ref_mode_e::wombat;

    void switch_ref_mode()
    {
        auto constexpr num = size_t(ref_mode_e::num_modes);

        auto i = size_t(g_ref_mode);

        ++i;
        i %= num;

        g_ref_mode = ref_mode_e(i);
    }


    void send_frame()
    {
        if (g_internal)
            g_internal->send_frame(42);
    }

    void streaming_client()
    {
        if (g_internal)
            g_internal->init_streaming_client();
    }

    void log_to_console(std::string const &)
    {
        
    }


} // namespace

qboolean R_Init_Wombat(qboolean total)
{
    if (!g_gl_functions.R_Init(total))
        return qfalse;

    Cmd_AddCommand("switch_ref_mode", (xcommand_t)&switch_ref_mode);
    Cmd_AddCommand("send_frame", (xcommand_t)&send_frame);
    Cmd_AddCommand("streaming_client", (xcommand_t)&streaming_client);



    auto *win = SDL_GL_GetCurrentWindow();
    auto *context = SDL_GL_GetCurrentContext();

    assert(win == sdl_window);

    {
        char const *dll_name = "wombat_android_test.dll";

        g_wat_dll = LoadLibraryA(dll_name);
        if (!g_wat_dll)
        {
            Com_Error(ERR_FATAL, "Can't load '%s'", dll_name);
            g_gl_functions.R_Shutdown(total);

            return qfalse;
        }

        auto create_f = reinterpret_cast<wat::create_pfn>(GetProcAddress(g_wat_dll, "create"));
        auto destroy_f = reinterpret_cast<wat::destroy_pfn>(GetProcAddress(g_wat_dll, "destroy"));


        g_iface = std::shared_ptr<wat::iface>(
            create_f({wat::head_axis_e::wombat, false}),
            destroy_f
        );

    }

    if (g_iface)
        g_internal = create_ref_wombat_internal(g_iface, log_to_console);

    if (g_internal)
        g_internal->update_resolution(g_fbo_width, g_fbo_height);


    SDL_SysWMinfo wmInfo;

    SDL_VERSION(&wmInfo.version);
    SDL_GetWindowWMInfo(win, &wmInfo);
    HWND const hwnd = wmInfo.info.win.window;

    //SDL_GL_MakeCurrent(nullptr, nullptr);

    g_iface->init({
        hwnd,
        nullptr
    });

    g_valid_fbo = false;

    //g_iface->on_text_message(0, "helicopter 0");
    //g_iface->on_text_message(0, "interpolator 2 3");

    //SDL_GL_MakeCurrent(win, context);

    return qtrue;
}

void R_Shutdown_Wombat(qboolean total)
{
    g_internal.reset();
    g_iface.reset();

    if (g_wat_dll)
        FreeLibrary(g_wat_dll);

    Cmd_RemoveCommand("switch_ref_mode");
    Cmd_RemoveCommand("send_frame");

    Cmd_RemoveCommand("streaming_client");

    g_gl_functions.R_Shutdown(total);
}

void R_BeginRegistration_Wombat(const char *map)
{
    g_gl_functions.R_BeginRegistration(map);

	return;

    auto *bsp = GL_GetBSP();

    int num_verts;
    auto *verts = reinterpret_cast<ref_wombat_internal::vec3_t*>(extract_bsp_mesh_vertices(bsp, map, &num_verts));
    g_internal->dump_bsp_vertices(reinterpret_cast<float*>(verts), num_verts);

    Z_Free(verts);

}

void R_SetSky_Wombat(const char *name, float rotate, vec3_t axis) { LOGME(); }
void R_EndRegistration_Wombat(void) { LOGME(); }

void R_RenderFrame_Wombat(refdef_t *fd)
{
    if (g_ref_mode == ref_mode_e::gl)
    {
        return g_gl_functions.R_RenderFrame(fd);
    }

    if (!g_valid_fbo)
    {
        g_iface->invalidate_fbo(g_fbo_width, g_fbo_height);
        g_valid_fbo = true;
    }

    g_internal->update_matrices(fd->vieworg, fd->viewangles, fd->fov_x, fd->fov_y);

    wombat_android_test::update_args_t update_args;

    float matrix[16] = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1,
    };

    ref_wombat_internal::fill_view_matrix(fd->vieworg, fd->viewangles, matrix);
    
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
    g_iface->swap_buffers(g_ref_mode == ref_mode_e::wombat_only);
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

    if (g_ref_mode != ref_mode_e::wombat_only)
        g_gl_functions.R_EndFrame();

}
void R_ModeChanged_Wombat(int width, int height, int flags, int rowbytes, void *pixels)
{ 
    g_gl_functions.R_ModeChanged(width, height, flags, rowbytes, pixels);

    if (g_internal)
        g_internal->update_resolution(uint32_t(width), uint32_t(height));

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
    R_RegisterFunctionsGL();

    g_gl_functions.R_Init = R_Init;
	g_gl_functions.R_Shutdown = R_Shutdown;
	g_gl_functions.R_BeginRegistration = R_BeginRegistration;
	g_gl_functions.R_EndRegistration = R_EndRegistration;
	g_gl_functions.R_SetSky = R_SetSky;
	g_gl_functions.R_RenderFrame = R_RenderFrame;
	g_gl_functions.R_LightPoint = R_LightPoint;
	g_gl_functions.R_ClearColor = R_ClearColor;
	g_gl_functions.R_SetAlpha = R_SetAlpha;
	g_gl_functions.R_SetAlphaScale = R_SetAlphaScale;
	g_gl_functions.R_SetColor = R_SetColor;
	g_gl_functions.R_SetClipRect = R_SetClipRect;
	g_gl_functions.R_SetScale = R_SetScale;
	g_gl_functions.R_DrawChar = R_DrawChar;
	g_gl_functions.R_DrawString = R_DrawString;
	g_gl_functions.R_DrawPic = R_DrawPic;
	g_gl_functions.R_DrawStretchPic = R_DrawStretchPic;
	g_gl_functions.R_TileClear = R_TileClear;
	g_gl_functions.R_DrawFill8 = R_DrawFill8;
	g_gl_functions.R_DrawFill32 = R_DrawFill32;
	g_gl_functions.R_BeginFrame = R_BeginFrame;
	g_gl_functions.R_EndFrame = R_EndFrame;
	g_gl_functions.R_ModeChanged = R_ModeChanged;
	g_gl_functions.R_AddDecal = R_AddDecal;
	g_gl_functions.R_InterceptKey = R_InterceptKey;
	g_gl_functions.IMG_Load = IMG_Load;
	g_gl_functions.IMG_Unload = IMG_Unload;
	g_gl_functions.IMG_ReadPixels = IMG_ReadPixels;
	g_gl_functions.MOD_LoadMD2 = MOD_LoadMD2;
	g_gl_functions.MOD_LoadMD3 = MOD_LoadMD3;
	g_gl_functions.MOD_Reference = MOD_Reference;    



    R_Init = R_Init_Wombat;
	R_Shutdown = R_Shutdown_Wombat;
	R_BeginRegistration = R_BeginRegistration_Wombat;
	// R_EndRegistration = R_EndRegistration_Wombat;
	// R_SetSky = R_SetSky_Wombat;
	R_RenderFrame = R_RenderFrame_Wombat;
	// R_LightPoint = R_LightPoint_Wombat;
	// R_ClearColor = R_ClearColor_Wombat;
	// R_SetAlpha = R_SetAlpha_Wombat;
	// R_SetAlphaScale = R_SetAlphaScale_Wombat;
	// R_SetColor = R_SetColor_Wombat;
	// R_SetClipRect = R_SetClipRect_Wombat;
	// R_SetScale = R_SetScale_Wombat;
	// R_DrawChar = R_DrawChar_Wombat;
	// R_DrawString = R_DrawString_Wombat;
	// R_DrawPic = R_DrawPic_Wombat;
	// R_DrawStretchPic = R_DrawStretchPic_Wombat;
	// R_TileClear = R_TileClear_Wombat;
	// R_DrawFill8 = R_DrawFill8_Wombat;
	// R_DrawFill32 = R_DrawFill32_Wombat;
	// R_BeginFrame = R_BeginFrame_Wombat;
	R_EndFrame = R_EndFrame_Wombat;
	R_ModeChanged = R_ModeChanged_Wombat;
	// R_AddDecal = R_AddDecal_Wombat;
	// R_InterceptKey = R_InterceptKey_Wombat;
	// IMG_Load = IMG_Load_Wombat;
	// IMG_Unload = IMG_Unload_Wombat;
	// IMG_ReadPixels = IMG_ReadPixels_Wombat;
	// MOD_LoadMD2 = MOD_LoadMD2_Wombat;
	// MOD_LoadMD3 = MOD_LoadMD3_Wombat;
	// MOD_Reference = MOD_Reference_Wombat;    
}
