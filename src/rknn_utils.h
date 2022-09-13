#ifndef __UTILS_H__
#define __UTILS_H__

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

#include "rknn_api.h"

void dump_tensor_attr(rknn_tensor_attr *attr);
unsigned char *load_data(FILE *fp, size_t ofst, size_t sz);
unsigned char *load_model(const char *filename, int *model_size);

#endif