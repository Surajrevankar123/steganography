#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "decode.h"
#include "common.h"
#include "types.h"
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    if (!argv[2]) 
    {
        printf("Error: please provide stego image\n"); 
        return e_failure; 
    }
    if (strstr(argv[2], ".bmp")) decInfo->stego_image_fname = argv[2];
    else 
    {
        printf("Error: Stego image must be .bmp file\n"); 
        return e_failure; 
    }
    if (argv[3]) decInfo->output_fname = argv[3];
    else 
    {
        decInfo->output_fname = "decoded"; 
        printf("INFO : Output file not mentioned, using default name\n"); 
    }
    return e_success;
}
static char decode_byte_from_lsb(FILE *fptr)
{
    char ch = 0;
    unsigned char buffer;
    for (int i = 0; i < 8; i++)
    {
        fread(&buffer, 1, 1, fptr);
        ch = (ch << 1) | (buffer & 1);
    }
    return ch;
}
static int decode_int_from_lsb(FILE *fptr)
{
    int value = 0;
    unsigned char buffer;
    for (int i = 0; i < 32; i++)
    {
        fread(&buffer, 1, 1, fptr);
        value = (value << 1) | (buffer & 1);
    }
    return value;
}
Status decode_magic_string(DecodeInfo *dec)
{
    fseek(dec->fptr_stego_image, 54, SEEK_SET);
    for (int i = 0; i < strlen(MAGIC_STRING); i++)
        dec->magic_string[i] = decode_byte_from_lsb(dec->fptr_stego_image);
    dec->magic_string[strlen(MAGIC_STRING)] = '\0';
    if (strcmp(dec->magic_string, MAGIC_STRING) == 0)
    {
        printf("INFO: Magic string verified\n");
        return e_success;
    }
    else
    {
        printf("ERROR: Magic string mismatch\n");
        return e_failure;
    }
}
Status decode_secret_extn(DecodeInfo *dec)
{
    dec->extn_size = decode_int_from_lsb(dec->fptr_stego_image);
    for (int i = 0; i < dec->extn_size; i++)
        dec->extn_secret_file[i] = decode_byte_from_lsb(dec->fptr_stego_image);
    dec->extn_secret_file[dec->extn_size] = '\0';
    char new_name[50];
    sprintf(new_name, "%s%s", dec->output_fname, dec->extn_secret_file);
    dec->fptr_output = fopen(new_name, "wb");
    if (!dec->fptr_output) 
    {
        printf("ERROR: Cannot create output file\n"); 
        return e_failure; 
    }

    printf("INFO: Secret file extension decoded successfully\n");
    return e_success;
}
Status decode_secret_size(DecodeInfo *dec)
{
    dec->secret_file_size = decode_int_from_lsb(dec->fptr_stego_image);
    printf("INFO: Secret file size = %d bytes\n", dec->secret_file_size);
    return e_success;
}
Status decode_secret_data(DecodeInfo *dec)
{
    for (int i = 0; i < dec->secret_file_size; i++)
    {
        char ch = decode_byte_from_lsb(dec->fptr_stego_image);
        fputc(ch, dec->fptr_output);
    }
    printf("INFO: Secret file data decoded successfully\n");
    return e_success;
}
Status do_decoding(DecodeInfo *dec)
{
    if (decode_magic_string(dec) == e_failure) 
    return e_failure;
    if (decode_secret_extn(dec) == e_failure) 
    return e_failure;
    if (decode_secret_size(dec) == e_failure) 
    return e_failure;
    if (decode_secret_data(dec) == e_failure) 
    return e_failure;

    fclose(dec->fptr_stego_image);
    fclose(dec->fptr_output);

    printf("INFO: Decoding completed successfully 🎉\n");
    return e_success;
}

