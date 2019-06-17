#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <png.h>

#define PNG_SIGNATURE_SIZE 8

//void gray_to_art(int y, int x, unsigned char pixels[][x]){
//	printf("some bullshit\n");
//}

void process_png(char *file_name){
	FILE *fp = fopen(file_name, "rb");
	if(!fp){
		perror("error opening image");
		exit(1);
	}

	/* verify png signature */
	png_byte signature[PNG_SIGNATURE_SIZE];
	fread(signature, 1, PNG_SIGNATURE_SIZE, fp);
	if(png_sig_cmp(signature, 0, PNG_SIGNATURE_SIZE)){
		fprintf(stderr, "error: png file is corrupt\n");
		fclose(fp);
		exit(1);
	}

	/* png data struct */
	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL,
			NULL, NULL);
	if(!png_ptr){
		fprintf(stderr, "error: the png struct could not be created\n");
		fclose(fp);
		exit(1);
	}
	/* png info struct */
	png_infop info_ptr = png_create_info_struct(png_ptr);
	if(!info_ptr){
		fprintf(stderr, "error: the png info struct could not be created\n");
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		fclose(fp);
		exit(1);
	}

	/* long jump error routine */
	if(setjmp(png_jmpbuf(png_ptr))){
		fprintf(stderr, "error: the png failed to be read\n");
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		fclose(fp);
		exit(1);
	}
	
	png_init_io(png_ptr, fp);
	png_set_sig_bytes(png_ptr, PNG_SIGNATURE_SIZE); //already read the signature

	/* get basic info */
	png_read_info(png_ptr, info_ptr);

	int width = png_get_image_width(png_ptr, info_ptr);
	int height = png_get_image_height(png_ptr, info_ptr);
	png_byte color_type = png_get_color_type(png_ptr, info_ptr);
	png_byte bit_depth = png_get_bit_depth(png_ptr, info_ptr);

	/* convert image to grayscale if not already */
	if(color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_RGB_ALPHA)
		png_set_rgb_to_gray_fixed(png_ptr, 1, -1, -1);
	/* ensure grayscale images are 8 bits */
	if(color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
		png_set_expand_gray_1_2_4_to_8(png_ptr);

	png_set_interlace_handling(png_ptr);
	png_read_update_info(png_ptr, info_ptr);

	/* read png */
	png_bytepp row_pointers = malloc(sizeof(png_bytep) * height);
	int rowbytes = png_get_rowbytes(png_ptr, info_ptr);
	for (int y = 0; y < height; y++)
		row_pointers[y] = malloc(rowbytes);

	png_read_image(png_ptr, row_pointers);

	/* generate art */
	//gray_to_art(width, height, row_pointers);

	for(int y = 0; y < height; y++)
		free(row_pointers[y]);
	free(row_pointers);
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	fclose(fp);
}


int main(int argc, char **argv){
	if(argc == 2){
		/* file extension must be present */
		char *ext = strrchr(argv[1], '.');
		if(!ext){
			fprintf(stderr, "error: file has no extension\n");
			return 1;
		} 
		/* process filetype if supported */
		if(strcmp(".png", ext) == 0){
			process_png(argv[1]);
		} else {
			fprintf(stderr, "error: filetype \"%s\" is not supported\n", ext);
			return 1;
		}
	} else {
		fprintf(stderr, "usage: asciiart image\n");
		return 1;
	}
	return 0;
}
