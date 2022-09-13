// Copyright (c) 2021 by Rockchip Electronics Co., Ltd. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "rga_func.h"

int rknn_rga_init(rga_context *rga_ctx)
{
    rga_ctx->rga_handle = dlopen("/usr/lib/aarch64-linux-gnu/librga.so", RTLD_LAZY);
    if (!rga_ctx->rga_handle)
    {
        printf("dlopen /usr/lib/aarch64-linux-gnu/librga.so failed\n");
        return -1;
    }
    rga_ctx->init_func = (FUNC_RGA_INIT)dlsym(rga_ctx->rga_handle, "c_RkRgaInit");
    rga_ctx->deinit_func = (FUNC_RGA_DEINIT)dlsym(rga_ctx->rga_handle, "c_RkRgaDeInit");
    rga_ctx->blit_func = (FUNC_RGA_BLIT)dlsym(rga_ctx->rga_handle, "c_RkRgaBlit");
    rga_ctx->init_func();
    return 0;
}

int rknn_img_resize_phy_to_phy(rga_context *rga_ctx, int src_fd, int src_w, int src_h, int src_fmt, uint64_t dst_fd, int dst_w, int dst_h, int dst_fmt)
{
    int ret = 0;

    if (rga_ctx->rga_handle)
    {
        rga_info_t src, dst;

        memset(&src, 0, sizeof(rga_info_t));
        src.fd = src_fd;
        src.mmuFlag = 1;
        // src.rotation = rotation;

        memset(&dst, 0, sizeof(rga_info_t));
        dst.fd = dst_fd;
        dst.mmuFlag = 0;
        dst.nn.nn_flag = 0;

        rga_set_rect(&src.rect, 0, 0, src_w, src_h, src_w, src_h, src_fmt);
        rga_set_rect(&dst.rect, 0, 0, dst_w, dst_h, dst_w, dst_h, dst_fmt);

        return rga_ctx->blit_func(&src, &dst, NULL);
    }
    return ret;
}

int rknn_img_resize_phy_to_virt(rga_context *rga_ctx, int src_fd, int src_w, int src_h, int src_fmt, void *dst_virt, int dst_w, int dst_h, int dst_fmt)
{
    int ret = 0;

    if (rga_ctx->rga_handle)
    {
        rga_info_t src, dst;

        memset(&src, 0, sizeof(rga_info_t));
        src.fd = src_fd;
        src.mmuFlag = 1;
        // src.rotation = rotation;

        memset(&dst, 0, sizeof(rga_info_t));
        dst.fd = -1;
        dst.mmuFlag = 1;
        dst.virAddr = dst_virt;
        dst.nn.nn_flag = 0;

        rga_set_rect(&src.rect, 0, 0, src_w, src_h, src_w, src_h, src_fmt);
        rga_set_rect(&dst.rect, 0, 0, dst_w, dst_h, dst_w, dst_h, dst_fmt);

        return rga_ctx->blit_func(&src, &dst, NULL);
    }
    return ret;
}

int rknn_img_resize_virt_to_phy(rga_context *rga_ctx, void *src_virt, int src_w, int src_h, int src_fmt, uint64_t dst_fd, int dst_w, int dst_h, int dst_fmt)
{
    int ret = 0;

    if (rga_ctx->rga_handle)
    {
        rga_info_t src, dst;

        memset(&src, 0, sizeof(rga_info_t));
        src.fd = -1;
        src.mmuFlag = 1;
        src.virAddr = (void *)src_virt;
        // src.rotation = rotation;

        memset(&dst, 0, sizeof(rga_info_t));
        dst.fd = dst_fd;
        dst.mmuFlag = 0;
        dst.nn.nn_flag = 0;

        rga_set_rect(&src.rect, 0, 0, src_w, src_h, src_w, src_h, src_fmt);
        rga_set_rect(&dst.rect, 0, 0, dst_w, dst_h, dst_w, dst_h, dst_fmt);

        return rga_ctx->blit_func(&src, &dst, NULL);
    }
    return ret;
}

int rknn_img_resize_virt_to_virt(rga_context *rga_ctx, void *src_virt, int src_w, int src_h, int src_fmt, void *dst_virt, int dst_w, int dst_h, int dst_fmt)
{
    int ret = 0;

    if (rga_ctx->rga_handle)
    {
        rga_info_t src, dst;

        memset(&src, 0, sizeof(rga_info_t));
        src.fd = -1;
        src.mmuFlag = 1;
        src.virAddr = (void *)src_virt;
        // src.rotation = rotation;

        memset(&dst, 0, sizeof(rga_info_t));
        dst.fd = -1;
        dst.mmuFlag = 1;
        dst.virAddr = dst_virt;
        dst.nn.nn_flag = 0;

        rga_set_rect(&src.rect, 0, 0, src_w, src_h, src_w, src_h, src_fmt);
        rga_set_rect(&dst.rect, 0, 0, dst_w, dst_h, dst_w, dst_h, dst_fmt);

        return rga_ctx->blit_func(&src, &dst, NULL);
    }
    return ret;
}

int rknn_rga_deinit(rga_context *rga_ctx)
{
    if (rga_ctx->rga_handle)
    {
        dlclose(rga_ctx->rga_handle);
        rga_ctx->rga_handle = NULL;
    }
    return 0;
}
