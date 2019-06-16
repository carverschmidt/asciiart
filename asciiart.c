#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <png.h>

#define PNG_SIGNATURE_SIZE 8

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
		fprintf(stderr, "error: the png struct could not be created");
		fclose(fp);
		exit(1);
	}
	/* png info struct */
	png_infop info_ptr = png_create_info_struct(png_ptr);
	if(!info_ptr){
		fprintf(stderr, "error: the info struct could not be created");
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		fclose(fp);
		exit(1);
	}

	png_init_io(png_ptr, fp);
	png_set_sig_bytes(png_ptr, PNG_SIGNATURE_SIZE); //already read the signature
	png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

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
