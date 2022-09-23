CMAKE_CROSS := 

ROOT_DIR := ${PWD}
INSTALL_DIR	:= ${PWD}/target
BUILD_DIR	:= ${PWD}/build
CMAKE_STRIP := ${CMAKE_CROSS}strip
CMAKE_C_COMPILER := ${CMAKE_CROSS}gcc
CMAKE_CXX_COMPILER := ${CMAKE_CROSS}g++

all: release

clean:
	rm -rf ${INSTALL_DIR}/*
	rm -rf ${BUILD_DIR}/*

release:
	-rm -rf ${BUILD_DIR}/ffmpeg_tutorial && mkdir ${BUILD_DIR}/ffmpeg_tutorial
	cd ${BUILD_DIR}/ffmpeg_tutorial && cmake \
	-DCMAKE_TOOLCHAIN_FILE=${ROOT_DIR}/toolChain.cmake \
	-DCMAKE_C_COMPILER=${CMAKE_C_COMPILER} \
	-DCMAKE_INSTALL_PREFIX=${INSTALL_DIR} \
	${ROOT_DIR}/src && \
	make -j$(nproc) && make install && cd -
	cp ${ROOT_DIR}/src/rockchip/librknn_api.so ${INSTALL_DIR}/lib
	cp ${ROOT_DIR}/src/rockchip/librknnrt.so ${INSTALL_DIR}/lib
	patchelf --set-rpath './target/lib' target/bin/ffmpeg_tutorial

opencv:
	@[ ! -d ${BUILD_DIR}/opencv ] && git clone https://github.com/opencv/opencv.git --depth=1 ${BUILD_DIR}/opencv || echo "opencv source ready..."
	@[ -e ${BUILD_DIR}/opencv/.build_ok ] && echo "opencv compilation completed..." || mkdir -p ${BUILD_DIR}/opencv/build 

	cd ${BUILD_DIR}/opencv/build && LD_LIBRARY_PATH=${INSTALL_DIR}/lib \
	cmake \
		-DWITH_OPENGL=ON -DWITH_QT=OFF -DWITH_GTK=ON -DWITH_GTK_2_X=ON \
		-DBUILD_TESTS=OFF -DBUILD_PERF_TESTS=OFF \
		-DCMAKE_INSTALL_PREFIX=${INSTALL_DIR} \
		-DCMAKE_C_COMPILER=${CMAKE_C_COMPILER} \
		-DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER} \
		-DINSTALL_DIR=${INSTALL_DIR} ${BUILD_DIR}/opencv && \
	make -j4 && make install && cd -
	touch ${BUILD_DIR}/opencv/.build_ok

sdl2:
	@[ ! -d ${BUILD_DIR}/SDL ] && git clone https://github.com/libsdl-org/SDL.git --depth=1 ${BUILD_DIR}/SDL || echo "SDL source ready..."
	@[ -e ${BUILD_DIR}/SDL/.build_ok ] && echo "SDL compilation completed..." || mkdir -p ${BUILD_DIR}/SDL/build

	cd ${BUILD_DIR}/SDL/build && LD_LIBRARY_PATH=${INSTALL_DIR}/lib \
	cmake \
		-DSDL_TEST=OFF -DSDL_OPENGLES=ON -DSDL_OPENGL=ON \
		-DCMAKE_INSTALL_PREFIX=${INSTALL_DIR} \
		-DCMAKE_C_COMPILER=${CMAKE_C_COMPILER} \
		-DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER} \
		-DINSTALL_DIR=${INSTALL_DIR} ${BUILD_DIR}/SDL && \
	make -j4 && make install && cd -
	touch ${BUILD_DIR}/SDL/.build_ok

ffmpeg:
	@[ ! -d ${BUILD_DIR}/ffmpeg-rockchip-4.1.3 ] && git clone https://github.com/autoexpect/ffmpeg-rockchip-4.1.3.git --depth=1 ${BUILD_DIR}/ffmpeg-rockchip-4.1.3 || echo "ffmpeg-rockchip-4.1.3 source ready..."
	@[ -e ${BUILD_DIR}/ffmpeg-rockchip-4.1.3/.build_ok ] && echo "ffmpeg-rockchip-4.1.3 compilation completed..." || mkdir -p ${BUILD_DIR}/ffmpeg-rockchip-4.1.3/build 

	cd ${BUILD_DIR}/ffmpeg-rockchip-4.1.3 && \
	./configure --disable-static --enable-shared --prefix=${INSTALL_DIR} \
		--enable-libfdk-aac --enable-ffplay --enable-libvpx \
		--enable-avfilter --enable-version3 --enable-logging --enable-optimizations --disable-extra-warnings \
		--enable-avdevice --enable-avcodec --enable-avformat --enable-network --disable-gray \
		--enable-swscale-alpha --enable-dct --enable-fft --enable-mdct --enable-rdft --disable-crystalhd \
		--disable-dxva2 --enable-runtime-cpudetect --disable-hardcoded-tables --disable-mipsdsp --disable-mipsdspr2 \
		--disable-msa --enable-hwaccels --disable-cuda --disable-cuvid --disable-nvenc --disable-avisynth \
		--disable-frei0r --disable-libopencore-amrnb --disable-libopencore-amrwb --disable-libdc1394 --disable-libgsm \
		--disable-libilbc --disable-libvo-amrwbenc --disable-symver --disable-doc --enable-gpl --enable-nonfree --disable-debug \
		--disable-small --enable-ffmpeg --enable-ffplay --enable-avresample --enable-ffprobe --enable-postproc --enable-swscale \
		--enable-librga --enable-indevs --enable-alsa --enable-outdevs --enable-pthreads --enable-zlib \
		--disable-libcdio --disable-gnutls --enable-openssl --enable-libdrm --disable-libopenh264 \
		--enable-muxer=ogg --disable-vaapi --disable-vdpau --enable-rkmpp --enable-libdrm \
		--disable-decoder=h264_v4l2m2m --disable-decoder=hevc_v4l2m2m --disable-decoder=vp8_v4l2m2m --disable-decoder=mpeg4_v4l2m2m \
		--disable-encoder=h264_v4l2m2m --disable-encoder=hevc_v4l2m2m --disable-encoder=vp8_v4l2m2m --disable-encoder=mpeg4_v4l2m2m \
		--disable-mmal --disable-omx --disable-omx-rpi --disable-libopencv --disable-libopus --disable-libvpx --disable-libass \
		--disable-libbluray --disable-libmfx --disable-libmp3lame --disable-libmodplug --disable-libspeex \
		--disable-libtheora --disable-libwavpack --disable-iconv --enable-libfreetype --disable-libopenjpeg \
		--enable-libx264 --enable-libx265 --disable-x86asm --disable-mmx --disable-sse --disable-sse2 --disable-sse3 --disable-ssse3 \
		--disable-sse4 --disable-sse42 --disable-avx --disable-avx2 --disable-armv6 --disable-armv6t2 --enable-vfp --enable-neon --disable-altivec \
		--extra-libs=-latomic --enable-pic --enable-openssl && \
	make -j$(nproc) && make install && cd -
	touch ${BUILD_DIR}/ffmpeg-rockchip-4.1.3/.build_ok
