#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "encode.h"
#include "types.h"
#include "common.h"
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width = 0, height = 0;
    fseek(fptr_image, 18, SEEK_SET);
    fread(&width, sizeof(int), 1, fptr_image);
    fread(&height, sizeof(int), 1, fptr_image);
    printf("Width = %u\n", width);
    printf("Height = %u\n", height);
    return width * height * 3;
}
Status open_file(EncodeInfo *encInfo)
{
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "rb");
    if (!encInfo->fptr_src_image) 
    {
        perror("fopen"); 
        return e_failure; 
    }
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "rb");
    if (!encInfo->fptr_secret) 
    {
        perror("fopen"); 
        return e_failure; 
    }
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "wb");
    if (!encInfo->fptr_stego_image) 
    {
        perror("fopen"); 
        return e_failure; 
    }
    printf("INFO : Files are opened successfully\n");
    return e_success;
}
Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    if (strstr(argv[2], ".bmp"))
        encInfo->src_image_fname = argv[2];
    else 
    {
        printf("Error: source must be .bmp\n"); 
        return e_failure; 
    }
    if (argv[3] != NULL)
    {
        encInfo->secret_fname = argv[3];
        char *dot = strrchr(argv[3], '.');
        if (dot)
        {
            strncpy(encInfo->extn_secret_file, dot, sizeof(encInfo->extn_secret_file) - 1);
            encInfo->extn_secret_file[sizeof(encInfo->extn_secret_file) - 1] = '\0';
        }
    }
    else 
    {
        printf("Error: secret file not specified\n"); 
        return e_failure; 
    }
    if (argv[4] != NULL)
    {
        if (strstr(argv[4], ".bmp"))
            encInfo->stego_image_fname = argv[4];
        else return e_failure;
    }
    else
    {
        encInfo->stego_image_fname = "stego.bmp";
        printf("INFO : output file not mentioned, default file name is added\n");
    }
    printf("INFO : Read and validation is Successfully executed\n");
    return e_success;
}
uint get_file_size(FILE *fptr_secret)
{
    fseek(fptr_secret, 0, SEEK_END);
    long size = ftell(fptr_secret);
    rewind(fptr_secret);
    return (uint)size;
}
Status check_capacity(EncodeInfo *encInfo)
{
    int src_file_size = get_image_size_for_bmp(encInfo->fptr_src_image);
    encInfo->size_secret_file = get_file_size(encInfo->fptr_secret);
    long required = 54 + strlen(MAGIC_STRING) * 8 + strlen(encInfo->extn_secret_file) * 8 + 32 + encInfo->size_secret_file * 8;
    if (src_file_size > required)
    {
        printf("INFO : Checking file capacity successfully completed\n");
        return e_success;
    }
    else
    {
        printf("ERROR: Not enough capacity. Required %ld, available %d\n", required, src_file_size);
        return e_failure;
    }
}
Status copy_bmp_header(FILE *src, FILE *dest)
{
    char image_buffer[54];
    rewind(src);
    fread(image_buffer, 1, 54, src);
    fwrite(image_buffer, 1, 54, dest);
    printf("INFO : Copy bmp header is successfull\n");
    return e_success;
}
Status encode_byte_to_lsb(char data, char *image_buffer)
{
    for (int i = 0; i < 8; i++)
    {
        char bit = (data >> (7 - i)) & 1;
        image_buffer[i] = (image_buffer[i] & 0xFE) | bit;
    }
    return e_success;
}
Status encode_size_to_lsb(long size, char *image_buffer)
{
    for (int i = 0; i < 32; i++)
    {
        char bit = (size >> (31 - i)) & 1;
        image_buffer[i] = (image_buffer[i] & 0xFE) | bit;
    }
    return e_success;
}
Status encode_data_to_image(char *data, int size, FILE *src, FILE *dest)
{
    char image_buffer[8];
    for (int i = 0; i < size; i++)
    {
        fread(image_buffer, 1, 8, src);
        encode_byte_to_lsb(data[i], image_buffer);
        fwrite(image_buffer, 1, 8, dest);
    }
    return e_success;
}
Status encode_magic_string(const char *magic_str, EncodeInfo *encInfo)
{
    printf("INFO : Magic_string encoding started\n");
    Status st = encode_data_to_image((char *)magic_str, strlen(magic_str), encInfo->fptr_src_image, encInfo->fptr_stego_image);
    if (st == e_success)
        printf("INFO : Magic_string encoded Successfully\n");
    return st;
}
Status encode_secret_file_extn_size(long extn_size, EncodeInfo *encInfo)
{
    char image_buffer[32];
    fread(image_buffer, 1, 32, encInfo->fptr_src_image);
    encode_size_to_lsb(extn_size, image_buffer);
    fwrite(image_buffer, 1, 32, encInfo->fptr_stego_image);
    printf("INFO : Secret file extension size is Encoded successfully.\n");
    return e_success;
}
Status encode_secret_file_extn(char *extn, EncodeInfo *encInfo)
{
    Status st = encode_data_to_image(extn, strlen(extn), encInfo->fptr_src_image, encInfo->fptr_stego_image);
    if (st == e_success)
        printf("INFO : Secret file extension encoded successfully.\n");
    return st;
}
Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
    char image_buffer[32];
    fread(image_buffer, 1, 32, encInfo->fptr_src_image);
    encode_size_to_lsb(file_size, image_buffer);
    fwrite(image_buffer, 1, 32, encInfo->fptr_stego_image);
    printf("INFO : Secret file size encoded successfully.\n");
    return e_success;
}
Status encode_secret_file_data(EncodeInfo *encInfo)
{
    char *data = malloc(encInfo->size_secret_file);
    rewind(encInfo->fptr_secret);
    fread(data, 1, encInfo->size_secret_file, encInfo->fptr_secret);
    encode_data_to_image(data, encInfo->size_secret_file, encInfo->fptr_src_image, encInfo->fptr_stego_image);
    free(data);
    printf("INFO : Secret file data encoded successfully.\n");
    return e_success;
}
Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    char ch;
    while (fread(&ch, 1, 1, fptr_src) == 1)
        fwrite(&ch, 1, 1, fptr_dest);
    printf("INFO : Copying the remaining data is Successfully completed\n");
    return e_success;
}
Status do_encoding(EncodeInfo *encInfo)
{
    printf("INFO : Selected encoding, Encoding Started.\n");
    if (open_file(encInfo) == e_failure) 
    return e_failure;
    if (check_capacity(encInfo) == e_failure) 
    return e_failure;
    copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image);
    encode_magic_string(MAGIC_STRING, encInfo);
    encode_secret_file_extn_size(strlen(encInfo->extn_secret_file), encInfo);
    encode_secret_file_extn(encInfo->extn_secret_file, encInfo);
    encode_secret_file_size(encInfo->size_secret_file, encInfo);
    encode_secret_file_data(encInfo);
    copy_remaining_img_data(encInfo->fptr_src_image, encInfo->fptr_stego_image);

    fclose(encInfo->fptr_src_image);
    fclose(encInfo->fptr_secret);
    fclose(encInfo->fptr_stego_image);

    printf("------------------------------------------------\n");
    printf("INFO : Encoding Successfully completed\n");
    printf("------------------------------------------------\n");
    return e_success;
}
