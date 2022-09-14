## ffmpeg_tutorial for rockchip rk3588 debian 11

ffmpeg->rockchip mpp decoding->rknpu rknn->opencv opengl rendering

```shell

# bootstrap
sudo apt install build-essential autoconf automake libtool cmake pkg-config git libdrm-dev clang-format
sudo apt install libgtkgl2.0-dev libgtkglext1-dev

make opencv # make opencv with opengl
make ffmpeg # make ffmpeg with rockchip mpp
make # make ffmpeg_tutorial

```
![4480](https://user-images.githubusercontent.com/6939585/190074110-ebf579fb-faff-48ab-af40-16578145c53b.jpg)
