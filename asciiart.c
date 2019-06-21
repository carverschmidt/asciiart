#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <png.h>
#include <jpeglib.h>

#define PNG_SIGNATURE_SIZE 8

void gray_to_art(int y, int x, unsigned char **pixels){
	for(int row = 0; row < y; row++){
		for(int col = 0; col < x; col++){
			if(pixels[row][col] >= 0 && pixels[row][col] < 25)
				putchar(' ');
			else if(pixels[row][col] >= 25 && pixels[row][col] < 50)
				putchar('.');
			else if(pixels[row][col] >= 50 && pixels[row][col] < 75)
				putchar(':');
			else if(pixels[row][col] >= 75 && pixels[row][col] < 100)
				putchar('-');
			else if(pixels[row][col] >= 100 && pixels[row][col] < 125)
				putchar('=');
			else if(pixels[row][col] >= 125 && pixels[row][col] < 150)
				putchar('+');
			else if(pixels[row][col] >= 150 && pixels[row][col] < 175)
				putchar('*');
			else if(pixels[row][col] >= 175 && pixels[row][col] < 200)
				putchar('#');
			else if(pixels[row][col] >= 200 && pixels[row][col] < 225)
				putchar('%');
			else
				putchar('@');
		}
		putchar('\n');
	}
}

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

	/* can only handle 8 bit channels */
	if(bit_depth == 16)
		png_set_strip_16(png_ptr);
	/* convert image to grayscale if not already */
	if(color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_RGB_ALPHA)
		png_set_rgb_to_gray(png_ptr, 1, -1, -1);
	/* ensure grayscale images are 8 bits */
	if(color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
		png_set_expand_gray_1_2_4_to_8(png_ptr);
	/* no need for alpha channel */
	if(color_type & PNG_COLOR_MASK_ALPHA)
		png_set_strip_alpha(png_ptr);

	png_set_interlace_handling(png_ptr);
	png_read_update_info(png_ptr, info_ptr);

	/* read png */
	png_bytepp row_pointers = malloc(sizeof(png_bytep) * height);
	int rowbytes = png_get_rowbytes(png_ptr, info_ptr);
	for (int y = 0; y < height; y++)
		row_pointers[y] = malloc(rowbytes);

	png_read_image(png_ptr, row_pointers);

	/* generate art */
	gray_to_art(width, height, row_pointers);

	for(int y = 0; y < height; y++)
		free(row_pointers[y]);
	free(row_pointers);
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	fclose(fp);
}

void process_jpeg(char *file_name){
	FILE *fp;
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr error;

	if((fp = fopen(file_name, "rb")) == NULL){
		perror("error opening image");
		exit(1);
	}

	/* init error handling */
	cinfo.err = jpeg_std_error(&error);

	/* init JPEG decompression struct */	
	jpeg_create_decompress(&cinfo);
	/* specify data source */
	jpeg_stdio_src(&cinfo, fp);
	/* read file parameters */
	jpeg_read_header(&cinfo, TRUE);

	/* insure output is grayscale */
	if(cinfo.jpeg_color_space != JCS_GRAYSCALE)
		cinfo.out_color_space = JCS_GRAYSCALE;

	jpeg_start_decompress(&cinfo);

	/* there can only be one color component for ascii conversion */
	if(cinfo.out_color_components != 1){
		fprintf(stderr, "error: there was a problem converting the JPEG to grayscale");
		jpeg_destroy_decompress(&cinfo);
		fclose(fp);
		exit(1);
	}

	/* read the image in one pass */
	JSAMPARRAY row_pointers = (*cinfo.mem->alloc_sarray)((j_common_ptr) &cinfo,
		   JPOOL_IMAGE, cinfo.output_width, cinfo.output_height);	

	int i = 0;
	while(cinfo.output_scanline < cinfo.output_height){
		jpeg_read_scanlines(&cinfo, &row_pointers[i], 1);
		i++;
	}
	jpeg_finish_decompress(&cinfo);

	/* generate art */
	gray_to_art(cinfo.output_height, cinfo.output_width, row_pointers);

	jpeg_destroy_decompress(&cinfo);
	//free(row_pointers);
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
		if(strcasecmp(".png", ext) == 0){
			process_png(argv[1]);
		} else if(strcasecmp(".jpg", ext) == 0 || strcasecmp(".jpeg", ext) == 0){
			process_jpeg(argv[1]);
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
