#ifndef DECODE_H
#define DECODE_H

#include "types.h"

typedef struct _DecodeInfo
{
    char *stego_image_fname;  
    FILE *fptr_stego_image;    
    char *output_fname;        
    FILE *fptr_output;        
    char magic_string[8];     
    char extn_secret_file[8]; 
    int extn_size;            
    int secret_file_size;     
} DecodeInfo;

Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo);
Status do_decoding(DecodeInfo *decInfo);
Status open_decode_files(DecodeInfo *decInfo);
Status decode_magic_string(DecodeInfo *decInfo);

Status decode_secret_file_extn_size(DecodeInfo *decInfo);
Status decode_secret_file_extn(DecodeInfo *decInfo);
Status decode_secret_file_size(DecodeInfo *decInfo);
Status decode_secret_file_data(DecodeInfo *decInfo);

#endif
