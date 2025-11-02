#ifndef ENCODE_H
#define ENCODE_H

#include <stdio.h>
#include "types.h"

typedef struct _EncodeInfo
{
    char *src_image_fname;
    FILE *fptr_src_image;

    char *secret_fname;
    FILE *fptr_secret;

    char *stego_image_fname;
    FILE *fptr_stego_image;

    char extn_secret_file[8];   // fixed size array for extension (.txt, .mp3, etc.)
    long size_secret_file;      // secret file size
} EncodeInfo;

// function declarations
Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo);
Status open_file(EncodeInfo *encInfo);
Status copy_bmp_header(FILE *src, FILE *dest);
Status encode_magic_string(const char *magic_str, EncodeInfo *encInfo);
Status encode_data_to_image(char *data, int size, FILE *src, FILE *dest);
Status encode_secret_file_extn_size(long extn_size, EncodeInfo *encInfo);
Status encode_secret_file_extn(char *extn, EncodeInfo *encInfo);
Status encode_secret_file_size(long file_size, EncodeInfo *encInfo);
Status encode_secret_file_data(EncodeInfo *encInfo);
Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest);
Status do_encoding(EncodeInfo *encInfo);

#endif
