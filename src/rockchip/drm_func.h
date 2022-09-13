#ifndef __RKNN_DRM_FUNC_H__
#define __RKNN_DRM_FUNC_H__
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>

#include <linux/input.h>
#include "libdrm/drm_fourcc.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef int (*FUNC_DRM_IOCTL)(int fd, unsigned long request, void *arg);

    typedef struct _drm_context
    {
        void *drm_handle;
        FUNC_DRM_IOCTL io_func;
    } drm_context;

    int rknn_drm_init(drm_context *drm_ctx);

    void *rknn_drm_buf_alloc(drm_context *drm_ctx, int drm_fd, int TexWidth, int TexHeight, int bpp, int *fd, unsigned int *handle, size_t *actual_size);

    int rknn_drm_buf_destroy(drm_context *drm_ctx, int drm_fd, int buf_fd, int handle, void *drm_buf, size_t size);

    void rknn_drm_deinit(drm_context *drm_ctx, int drm_fd);

#ifdef __cplusplus
}
#endif
#endif /*__RKNN_DRM_FUNC_H__*/
