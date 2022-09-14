#ifndef __FFMPEG_H__
#define __FFMPEG_H__

#include <errno.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <iostream>
#include <sys/ioctl.h>
#include <cstdio>
#include <sys/mman.h>
#include <unistd.h>
#include <dlfcn.h>
#include <sys/time.h>

#include <GL/gl.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgproc/types_c.h>
#include <opencv2/core/opengl.hpp>

#include "rknn_api.h"
#include "yolov5s_postprocess.h"
#include "rknn_utils.h"

#ifdef __cplusplus
extern "C" {
#endif
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avassert.h>
#include <libavutil/channel_layout.h>
#include <libavutil/hwcontext_drm.h>
#include <libavutil/imgutils.h>
#include <libavutil/mathematics.h>
#include <libavutil/opt.h>
#include <libavutil/timestamp.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
#ifdef __cplusplus
}
#endif

#include "config.h"
#include "drm_func.h"
#include "rga_func.h"

class FFmpegStreamChannel {
    public:
	/* ffmpeg */
	AVFormatContext *format_context_input;
	int video_stream_index_input;
	int audio_stream_index_input;

	AVCodec *codec_input_video;
	AVCodec *codec_input_audio;

	AVCodecContext *codec_ctx_input_video;
	AVCodecContext *codec_ctx_input_audio;

	int video_frame_size = 0;
	int audio_frame_size = 0;
	int video_frame_count = 0;
	int audio_frame_count = 0;

	drm_context drm_ctx;
	rga_context rga_ctx;
	struct drm_buf drm_buf_for_rga1;
	struct drm_buf drm_buf_for_rga2;
	bool decode(const char *);

	/* rknn */
	const float nms_threshold = NMS_THRESH;
	const float box_conf_threshold = BOX_THRESH;
	rknn_context rknn_ctx;
	int rknn_input_channel = 3;
	int rknn_input_width = 0;
	int rknn_input_height = 0;
	rknn_input inputs[1];
	rknn_input_output_num io_num;
	rknn_tensor_attr *output_attrs;
	int init_rga_drm();
	int init_rknn2();

	/* opencv */
	std::string window_name;
	GLuint image_texture;
	int init_window();
	void bind_cv_mat_to_gl_texture(cv::Mat& image, GLuint& imageTexture);

	FFmpegStreamChannel()
	{
		init_rga_drm();

		init_rknn2();

		init_window();
	}

	~FFmpegStreamChannel()
	{
		free(output_attrs);
	}
};

#endif