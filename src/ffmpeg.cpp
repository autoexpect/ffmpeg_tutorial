#include "ffmpeg.h"

int FFmpegStreamChannel::init_rga_drm()
{
	/* init drm */
	memset(&drm_ctx, 0, sizeof(drm_context));
	int drm_fd = rknn_drm_init(&drm_ctx);

	/* drm mem1 */
	drm_buf_for_rga1.drm_buf_ptr = rknn_drm_buf_alloc(&drm_ctx, drm_fd, 2560, 1440,
							  4 * 8, // 4 channel x 8bit
							  &drm_buf_for_rga1.drm_buf_fd, &drm_buf_for_rga1.drm_buf_handle, &drm_buf_for_rga1.drm_buf_size);

	/* drm mem2 */
	drm_buf_for_rga2.drm_buf_ptr = rknn_drm_buf_alloc(&drm_ctx, drm_fd, 2560, 1440,
							  4 * 8, // 4 channel x 8bit
							  &drm_buf_for_rga2.drm_buf_fd, &drm_buf_for_rga2.drm_buf_handle, &drm_buf_for_rga2.drm_buf_size);

	/* init rga */
	memset(&rga_ctx, 0, sizeof(rga_context));
	rknn_rga_init(&rga_ctx);

	return 0;
}

int FFmpegStreamChannel::init_window()
{
	window_name = "RK3588";
	// cv::namedWindow(window_name, cv::WINDOW_OPENGL);
	// cv::setOpenGlContext(window_name);
	return 0;
}

void FFmpegStreamChannel::bind_cv_mat_to_gl_texture(cv::Mat &image, GLuint &imageTexture)
{
	//glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glGenTextures(1, &imageTexture);
	glBindTexture(GL_TEXTURE_2D, imageTexture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Set texture clamping method
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	// cv::cvtColor(image, image, CV_RGB2BGR);

	glTexImage2D(GL_TEXTURE_2D, // Type of texture
		     0, // Pyramid level (for mip-mapping) - 0 is the top level
		     GL_RGB, // Internal colour format to convert to
		     image.cols, // Image width  i.e. 640 for Kinect in standard mode
		     image.rows, // Image height i.e. 480 for Kinect in standard mode
		     0, // Border width in pixels (can either be 1 or 0)
		     GL_RGB, // Input image format (i.e. GL_RGB, GL_RGBA, GL_BGR etc.)
		     GL_UNSIGNED_BYTE, // Image data type
		     image.ptr()); // The actual image data itself
}

int FFmpegStreamChannel::init_rknn2()
{
	printf("Loading mode...\n");
	int model_data_size = 0;
	unsigned char *model_data = load_model(MODEL_PATH, &model_data_size);
	int ret = rknn_init(&rknn_ctx, model_data, model_data_size, 0, NULL);
	if (ret < 0) {
		printf("rknn_init error ret=%d\n", ret);
		return -1;
	}

	rknn_sdk_version version;
	ret = rknn_query(rknn_ctx, RKNN_QUERY_SDK_VERSION, &version, sizeof(rknn_sdk_version));
	if (ret < 0) {
		printf("rknn_init error ret=%d\n", ret);
		return -1;
	}
	printf("sdk version: %s driver version: %s\n", version.api_version, version.drv_version);

	ret = rknn_query(rknn_ctx, RKNN_QUERY_IN_OUT_NUM, &io_num, sizeof(io_num));
	if (ret < 0) {
		printf("rknn_init error ret=%d\n", ret);
		return -1;
	}
	printf("model input num: %d, output num: %d\n", io_num.n_input, io_num.n_output);

	rknn_tensor_attr input_attrs[io_num.n_input];
	memset(input_attrs, 0, sizeof(input_attrs));
	for (int i = 0; i < io_num.n_input; i++) {
		input_attrs[i].index = i;
		ret = rknn_query(rknn_ctx, RKNN_QUERY_INPUT_ATTR, &(input_attrs[i]), sizeof(rknn_tensor_attr));
		if (ret < 0) {
			printf("rknn_init error ret=%d\n", ret);
			return -1;
		}
		dump_tensor_attr(&(input_attrs[i]));
	}

	output_attrs = (rknn_tensor_attr *)malloc(io_num.n_output * sizeof(rknn_tensor_attr));
	memset(output_attrs, 0, sizeof(output_attrs));
	for (int i = 0; i < io_num.n_output; i++) {
		output_attrs[i].index = i;
		ret = rknn_query(rknn_ctx, RKNN_QUERY_OUTPUT_ATTR, &(output_attrs[i]), sizeof(rknn_tensor_attr));
		dump_tensor_attr(&(output_attrs[i]));
	}

	if (input_attrs[0].fmt == RKNN_TENSOR_NCHW) {
		printf("model is NCHW input fmt\n");
		rknn_input_channel = input_attrs[0].dims[1];
		rknn_input_width = input_attrs[0].dims[2];
		rknn_input_height = input_attrs[0].dims[3];
	} else {
		printf("model is NHWC input fmt\n");
		rknn_input_width = input_attrs[0].dims[1];
		rknn_input_height = input_attrs[0].dims[2];
		rknn_input_channel = input_attrs[0].dims[3];
	}
	printf("model input height=%d, width=%d, channel=%d\n", rknn_input_height, rknn_input_width, rknn_input_channel);

	memset(inputs, 0, sizeof(inputs));
	inputs[0].index = 0;
	inputs[0].type = RKNN_TENSOR_UINT8;
	inputs[0].size = rknn_input_width * rknn_input_height * rknn_input_channel;
	inputs[0].fmt = RKNN_TENSOR_NHWC;
	inputs[0].pass_through = 0;

	return 0;
}

bool FFmpegStreamChannel::decode(const char *input_stream_url)
{
	int ret;
	long long ts_mark = 0;

	av_register_all();
	avformat_network_init();

	av_log_set_level(AV_LOG_INFO);

	AVDictionary *opts = NULL;
	av_dict_set(&opts, "rtsp_transport", "+udp+tcp", 0);
	av_dict_set(&opts, "rtsp_flags", "+prefer_tcp", 0);
	av_dict_set(&opts, "threads", "auto", 0);

	format_context_input = avformat_alloc_context();
	ret = avformat_open_input(&format_context_input, input_stream_url, NULL, &opts);
	if (ret < 0) {
		printf("avformat_open_input filed: %d\n", ret);
		return false;
	}

	ret = avformat_find_stream_info(format_context_input, NULL);
	if (ret < 0) {
		printf("avformat_find_stream_info filed: %d\n", ret);
		return false;
	}

	av_dump_format(format_context_input, 0, input_stream_url, 0);

	{
		video_stream_index_input = av_find_best_stream(format_context_input, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
		AVStream *stream_input = format_context_input->streams[video_stream_index_input];

		// codec_input_video = avcodec_find_decoder_by_name("h264_rkmpp");
		// if (codec_input_video == NULL) {
		// 	printf("avcodec_find_decoder_by_name filed...\n");
		// 	return false;
		// }

		codec_input_video = avcodec_find_decoder(stream_input->codec->codec_id);
		if (codec_input_video == NULL) {
			printf("avcodec_find_decoder_by_name failed...\n");
			return false;
		}

		codec_ctx_input_video = avcodec_alloc_context3(codec_input_video);
		if (codec_input_video == NULL) {
			printf("avcodec_alloc_context3 failed...\n");
			return false;
		}

		avcodec_copy_context(codec_ctx_input_video, stream_input->codec);
		codec_ctx_input_video->pix_fmt = AV_PIX_FMT_DRM_PRIME;
		codec_ctx_input_video->lowres = codec_input_video->max_lowres;
		codec_ctx_input_video->flags2 |= AV_CODEC_FLAG2_FAST;
		printf("-->: avcodec_get_hw_config: %s\n", codec_input_video->name);

		AVDictionary *opts = NULL;
		av_dict_set(&opts, "strict", "1", 0);
		ret = avcodec_open2(codec_ctx_input_video, codec_input_video, &opts);
		if (ret < 0) {
			printf("avcodec_open2 filed: %d\n", ret);
			return false;
		}
	}

	{
		audio_stream_index_input = av_find_best_stream(format_context_input, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
		AVStream *stream_input = format_context_input->streams[video_stream_index_input];
		codec_input_audio = avcodec_find_decoder(stream_input->codec->codec_id);
		codec_ctx_input_audio = avcodec_alloc_context3(codec_input_audio);
		avcodec_copy_context(codec_ctx_input_audio, stream_input->codec);
	}

	/* Init Mat */
	cv::Mat *mat4show = new cv::Mat(cv::Size(WIDTH_P, HEIGHT_P), CV_8UC3, drm_buf_for_rga2.drm_buf_ptr);

	AVPacket *packet_input_tmp = av_packet_alloc();
	AVFrame *frame_input_tmp = av_frame_alloc();
	while (true) {
		ret = av_read_frame(format_context_input, packet_input_tmp);
		if (ret < 0) {
			break;
		}

		AVStream *stream_input = format_context_input->streams[packet_input_tmp->stream_index];

		/* video */
		if (packet_input_tmp->stream_index == video_stream_index_input) {
			video_frame_size += packet_input_tmp->size;
			video_frame_count++;

			ret = avcodec_send_packet(codec_ctx_input_video, packet_input_tmp);
			if (ret < 0) {
				printf("avcodec_send_packet filed: %d\n", ret);
				return false;
			}

			while (ret >= 0) {
				ret = avcodec_receive_frame(codec_ctx_input_video, frame_input_tmp);
				if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
					break;
				} else if (ret < 0) {
					printf("avcodec_receive_frame filed: %d\n", ret);
					return false;
				}

				auto av_drm_frame = reinterpret_cast<const AVDRMFrameDescriptor *>(frame_input_tmp->data[0]);
				auto layer = &reinterpret_cast<AVDRMFrameDescriptor *>(frame_input_tmp->data[0])->layers[0];
				void *ptr = av_drm_frame->objects[0].ptr;
				int fd = av_drm_frame->objects[0].fd;
				int w = layer->planes[0].pitch;
				int h = layer->planes[1].offset / w;

				ts_mark = current_timestamp();

				/* YUV 4 RKNN2 Compute */
				rknn_img_resize_phy_to_phy(&rga_ctx, //
							   fd, w, h, RK_FORMAT_YCbCr_420_SP, //
							   drm_buf_for_rga1.drm_buf_fd, rknn_input_width, rknn_input_height, RK_FORMAT_RGB_888);

				/* YUV 4 Show */
				rknn_img_resize_phy_to_phy(&rga_ctx, //
							   fd, w, h, RK_FORMAT_YCbCr_420_SP, //
							   drm_buf_for_rga2.drm_buf_fd, WIDTH_P, HEIGHT_P, RK_FORMAT_BGR_888);

				/* rknn2 compute */
				inputs[0].buf = drm_buf_for_rga1.drm_buf_ptr;
				rknn_inputs_set(rknn_ctx, io_num.n_input, inputs);

				rknn_output outputs[io_num.n_output];
				memset(outputs, 0, sizeof(outputs));
				for (int i = 0; i < io_num.n_output; i++) {
					outputs[i].want_float = 0;
				}

				ret = rknn_run(rknn_ctx, NULL);
				ret = rknn_outputs_get(rknn_ctx, io_num.n_output, outputs, NULL);
				printf("DETECT OK---->[%fms]\n", ((double)(current_timestamp() - ts_mark)) / 1000);

				/* post process */
				float scale_w = (float)rknn_input_width / WIDTH_P;
				float scale_h = (float)rknn_input_height / HEIGHT_P;

				detect_result_group_t detect_result_group;
				std::vector<float> out_scales;
				std::vector<int32_t> out_zps;
				for (int i = 0; i < io_num.n_output; ++i) {
					out_scales.push_back(output_attrs[i].scale);
					out_zps.push_back(output_attrs[i].zp);
				}
				post_process((int8_t *)outputs[0].buf, (int8_t *)outputs[1].buf, (int8_t *)outputs[2].buf, rknn_input_height, rknn_input_width,
					     box_conf_threshold, nms_threshold, scale_w, scale_h, out_zps, out_scales, &detect_result_group);
				printf("POST PROCESS OK---->[%fms]\n", ((double)(current_timestamp() - ts_mark)) / 1000);

				/* Draw Objects */
				char text[256];
				for (int i = 0; i < detect_result_group.count; i++) {
					detect_result_t *det_result = &(detect_result_group.results[i]);
					sprintf(text, "%s %.1f%%", det_result->name, det_result->prop * 100);

					printf("---->%s @ (%d %d %d %d) %f\n", det_result->name, det_result->box.left, det_result->box.top,
					       det_result->box.right, det_result->box.bottom, det_result->prop);

					int x1 = det_result->box.left;
					int y1 = det_result->box.top;
					int x2 = det_result->box.right;
					int y2 = det_result->box.bottom;
					rectangle(*mat4show, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(255, 0, 0, 255), 2);
					putText(*mat4show, text, cv::Point(x1, y1 + 12), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0));
				}

				printf("DRAW BOX OK---->[%fms]\n", ((double)(current_timestamp() - ts_mark)) / 1000);

				/* Free Outputs */
				rknn_outputs_release(rknn_ctx, io_num.n_output, outputs);

				/* OpenGL */
				// if (image_texture != 0) {
				// 	glDeleteTextures(1, &image_texture);
				// 	image_texture = 0;
				// }
				// bind_cv_mat_to_gl_texture(*mat4show, image_texture);

				/* Opencv */
				cv::imshow(window_name, *mat4show);
				cv::waitKey(1);

				if (frame_input_tmp->pkt_pts > 0) {
					cv::imwrite(std::to_string(frame_input_tmp->pkt_pts) + ".jpg", *mat4show);
				}

				printf("SHOW OK---->[%fms]\n", ((double)(current_timestamp() - ts_mark)) / 1000);
			}
		}

		/* audio */
		if (packet_input_tmp->stream_index == audio_stream_index_input) {
			audio_frame_size += packet_input_tmp->size;
			audio_frame_count++;
		}

		av_packet_unref(packet_input_tmp);
		av_frame_unref(frame_input_tmp);
	}

	av_packet_free(&packet_input_tmp);
	avformat_close_input(&format_context_input);
	return true;
};
