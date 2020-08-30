#include <SDL.h>
#include <stdexcept>
#include <vector>
#include <algorithm>
#include <sstream>

#define NOMINMAX


extern "C"
{
typedef enum { qfalse, qtrue } qboolean;
#include "refresh/gl/qgl/dynamic.h"

extern qboolean QGL_Init(void);

}



#include "vk2gl_converter.h"


#define INIT_GL_FUNCTION(name) init_gl_function(#name, name##_)

struct vk2gl_converter_impl
    : vk2gl_converter
{
    vk2gl_converter_impl()
    {
        if (SDL_GL_LoadLibrary("opengl32") != 0) 
            throw error("SDL_GL_LoadLibrary failed");

        SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);

        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);

        uint32_t window_flags = SDL_WINDOW_HIDDEN | SDL_WINDOW_OPENGL;

        context_lock_t context_lock;
        window_ = SDL_CreateWindow("vk2gl_converter", 0, 0, 256, 256, window_flags);
        if (!window_)
            throw error("SDL_CreateWindow failed");

	    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

        context_ = SDL_GL_CreateContext(window_);
        if (!context_)
            throw error("SDL_GL_CreateContext failed");

        if (!QGL_Init())
            throw error("QGL_Init failed");


        INIT_GL_FUNCTION(glGenFramebuffers);
        INIT_GL_FUNCTION(glFramebufferTexture);
        INIT_GL_FUNCTION(glBindFramebuffer);
        INIT_GL_FUNCTION(glDrawVkImageNV);

        qglGenTextures(1, &gl_texture_);
        if (!gl_texture_)
            throw error("glGenTextures failed");

        qglBindTexture(GL_TEXTURE_2D, gl_texture_);

        {
            tex_width_ = tex_height_ = 512;

            std::vector<uint32_t> data(tex_width_ * tex_height_);
            for (size_t y = 0; y < tex_height_; ++y)
            {
                for (size_t x = 0; x < tex_width_; ++x)
                {
                    uint32_t const v = std::min<uint32_t>((x + y) / 2, 255);
                    data.at(y * tex_width_ + x) = ~0;//v | v << 8 | v << 16 | v << 24;
                }
            }

            qglTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_width_, tex_height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.data());
        }

        qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        check_gl_error();

        glGenFramebuffers_(1, &gl_fb_);
        if (!gl_fb_)
            throw error("glGenFramebuffers failed");

        glBindFramebuffer_(GL_DRAW_FRAMEBUFFER, gl_fb_);
        check_gl_error();
        glFramebufferTexture_(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, gl_texture_, 0);
        check_gl_error();

        qglViewport(0, 0, tex_width_, tex_height_);
        check_gl_error();
    }

    uint32_t get_gl_texture() const override
    {
        return gl_texture_;
    }

    void update(void* vk_image, unsigned width, unsigned height, unsigned full_width, unsigned full_height, void* fence) override
    {
        float const w_ratio = float(width) / float(full_width);
        float const h_ratio = float(height) / float(full_height);

        context_lock_t context_lock;

        SDL_GL_MakeCurrent(window_, context_);

        //check_gl_error();
        //glDrawVkImageNV_(reinterpret_cast<uint64_t>(vk_image), 0, 0.f, 0.f, tex_width_, tex_height_, 0.f, 0.f, 0.f, w_ratio, h_ratio);
        //check_gl_error();
    }


    void set_context() override
    {
        if (external_context_lock_)
            throw error("Context already set");

        external_context_lock_ = std::make_unique<context_lock_t>();
        SDL_GL_MakeCurrent(window_, context_);
    }

    void restore_context() override
    {
        if (!external_context_lock_)
            throw error("Context already not");

        external_context_lock_.reset();
    }

    struct error
        : std::runtime_error
    {
        explicit error(const std::string& _Message)
            : runtime_error(_Message)
        {
        }

        explicit error(const char* _Message)
            : runtime_error(_Message)
        {
        }
    };

private:
    struct context_lock_t
    {
        context_lock_t()
            : window_(SDL_GL_GetCurrentWindow())
            , context_(SDL_GL_GetCurrentContext())
        {
            
        }

        ~context_lock_t()
        {
            SDL_GL_MakeCurrent(window_, context_);
        }
    private:
        SDL_Window* window_ = nullptr;
        SDL_GLContext context_ = nullptr;
    };


private:
    template<typename T>
    void init_gl_function(char const *name, T &dst)
    {
        auto f = reinterpret_cast<T>(SDL_GL_GetProcAddress(name));
        if (!f)
        {
            std::stringstream ss;
            ss << "Gl function '" << name << "' not found";
            throw error(ss.str());
        }
        dst = f;
    }

    void check_gl_error()
    {
        if (auto err = qglGetError())
        {
            int aaa = 5;
        }        
    }

private:
    SDL_Window* window_ = nullptr;
    SDL_GLContext context_ = nullptr;

    uint32_t gl_texture_ = 0;
    uint32_t gl_fb_ = 0;

    uint32_t tex_width_ = 0, tex_height_ = 0;


    using  context_lock_uptr = std::unique_ptr<context_lock_t>;
    context_lock_uptr external_context_lock_;


    typedef void (APIENTRY * PFNGLGENFRAMEBUFFERSPROC) (GLsizei n, GLuint* framebuffers);
    typedef void (APIENTRY * PFNGLFRAMEBUFFERTEXTUREPROC) (GLenum target, GLenum attachment, GLuint texture, GLint level);
    typedef void (APIENTRY * PFNGLBINDFRAMEBUFFEREXTPROC) (GLenum target, GLuint framebuffer);

    typedef void (APIENTRY * PFNGLDRAWVKIMAGENVEXTPROC) (GLuint64 vkImage, GLuint sampler,
                   GLfloat x0, GLfloat y0, 
                   GLfloat x1, GLfloat y1,
                   GLfloat z,
                   GLfloat s0, GLfloat t0, 
                   GLfloat s1, GLfloat t1);

    PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers_ = nullptr;
    PFNGLFRAMEBUFFERTEXTUREPROC glFramebufferTexture_ = nullptr;
    PFNGLBINDFRAMEBUFFEREXTPROC glBindFramebuffer_ = nullptr;
    PFNGLDRAWVKIMAGENVEXTPROC glDrawVkImageNV_ = nullptr;


};

vk2gl_converter_uptr create_vk2gl_converter()
{
    try
    {
        return std::make_unique<vk2gl_converter_impl>();
    }
    catch (vk2gl_converter_impl::error const &e)
    {
        return nullptr;
    }
}