#pragma once

#include <memory>


struct vk2gl_converter
{
    virtual ~vk2gl_converter() = default;

    virtual uint32_t get_gl_texture() const = 0;
    virtual void update(void *vk_image, unsigned width, unsigned height, unsigned full_width, unsigned full_height, void *fence) = 0;

    virtual void set_context() = 0;
    virtual void restore_context() = 0;
};

using vk2gl_converter_uptr = std::unique_ptr<vk2gl_converter>;

vk2gl_converter_uptr create_vk2gl_converter();