#include "rknn_utils.h"

void dump_tensor_attr(rknn_tensor_attr *attr)
{
	printf("  index=%d, name=%s, n_dims=%d, dims=[%d, %d, %d, %d], n_elems=%d, size=%d, fmt=%s, type=%s, qnt_type=%s, "
	       "zp=%d, scale=%f\n",
	       attr->index, attr->name, attr->n_dims, attr->dims[0], attr->dims[1], attr->dims[2], attr->dims[3], attr->n_elems, attr->size, get_format_string(attr->fmt), get_type_string(attr->type),
	       get_qnt_type_string(attr->qnt_type), attr->zp, attr->scale);
}

unsigned char *load_data(FILE *fp, size_t ofst, size_t sz)
{
	unsigned char *data;
	int ret;

	data = NULL;

	if (NULL == fp) {
		return NULL;
	}

	ret = fseek(fp, ofst, SEEK_SET);
	if (ret != 0) {
		printf("blob seek failure.\n");
		return NULL;
	}

	data = (unsigned char *)malloc(sz);
	if (data == NULL) {
		printf("buffer malloc failure.\n");
		return NULL;
	}
	ret = fread(data, 1, sz, fp);
	return data;
}

unsigned char *load_model(const char *filename, int *model_size)
{
	FILE *fp;
	unsigned char *data;

	fp = fopen(filename, "rb");
	if (NULL == fp) {
		printf("Open file %s failed.\n", filename);
		return NULL;
	}

	fseek(fp, 0, SEEK_END);
	int size = ftell(fp);

	data = load_data(fp, 0, size);

	fclose(fp);

	*model_size = size;
	return data;
}