#ifndef __RKNN_RGA_FUNC_H__
#define __RKNN_RGA_FUNC_H__

#include <dlfcn.h>
#include "rga/RgaApi.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef int (*FUNC_RGA_INIT)();
    typedef void (*FUNC_RGA_DEINIT)();
    typedef int (*FUNC_RGA_BLIT)(rga_info_t *, rga_info_t *, rga_info_t *);

    typedef struct _rga_context
    {
        void *rga_handle;
        FUNC_RGA_INIT init_func;
        FUNC_RGA_DEINIT deinit_func;
        FUNC_RGA_BLIT blit_func;
    } rga_context;

    int rknn_rga_init(rga_context *rga_ctx);

    int rknn_img_resize_phy_to_phy(rga_context *rga_ctx, int src_fd, int src_w, int src_h, int src_fmt, uint64_t dst_fd, int dst_w, int dst_h, int dst_fmt);

    int rknn_img_resize_phy_to_virt(rga_context *rga_ctx, int src_fd, int src_w, int src_h, int src_fmt, void *dst_virt, int dst_w, int dst_h, int dst_fmt);

    int rknn_img_resize_virt_to_phy(rga_context *rga_ctx, void *src_virt, int src_w, int src_h, int src_fmt, uint64_t dst_fd, int dst_w, int dst_h, int dst_fmt);

    int rknn_img_resize_virt_to_virt(rga_context *rga_ctx, void *src_virt, int src_w, int src_h, int src_fmt, void *dst_virt, int dst_w, int dst_h, int dst_fmt);

    int rknn_rga_deinit(rga_context *rga_ctx);

#ifdef __cplusplus
}
#endif
#endif /*__RKNN_RGA_FUNC_H__*/
