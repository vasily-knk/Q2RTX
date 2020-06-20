#include <iostream>
#include <fstream>

extern "C"
{

#include "shared/shared.h"
#include "refresh/refresh.h"
#include "refresh/images.h"
#include "refresh/models.h"

void R_RegisterFunctionsWombat();

}

std::ofstream log_stream("D:\\log.txt");


#define LOGME() log_stream << __FUNCTION__ << std::endl;

qboolean R_Init_Wombat(qboolean total)
{
    LOGME();

    IMG_Init();

    return qtrue;
}

void R_Shutdown_Wombat(qboolean total) { LOGME(); }
void R_BeginRegistration_Wombat(const char *map) { LOGME(); }
void R_SetSky_Wombat(const char *name, float rotate, vec3_t axis) { LOGME(); }
void R_EndRegistration_Wombat(void) { LOGME(); }
void R_RenderFrame_Wombat(refdef_t *fd) { LOGME(); }
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
void R_BeginFrame_Wombat(void) { LOGME(); }
void R_EndFrame_Wombat(void) { LOGME(); }
void R_ModeChanged_Wombat(int width, int height, int flags, int rowbytes, void *pixels) { LOGME(); }
void R_AddDecal_Wombat(decal_t *d) { LOGME(); }
qboolean R_InterceptKey_Wombat(unsigned key, qboolean down)
{
    LOGME();
    return qfalse;
}

void IMG_Unload_Wombat(image_t *image) { LOGME(); }
void IMG_Load_Wombat(image_t *image, byte *pic)
{
    LOGME();
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
