#include <stdio.h>
#include <string.h>
#include "types.h"
#include "encode.h"
#include "decode.h"
#include "common.h"

// Function to check operation type
OperationType check_operation_type(char *argv)
{
    if (strcmp(argv, "-e") == 0)
        return e_encode;
    else if (strcmp(argv, "-d") == 0)
        return e_decode;
    else
        return e_unsupported;
}

int main(int argc, char *argv[])
{
    // Check minimum arguments
    if (argc < 2)
    {
        printf("INFO: Please pass valid arguments.\n");
        printf("INFO: Encoding - minimum 4 arguments.\n");
        printf("INFO: Decoding - minimum 3 arguments.\n");
        return 1;
    }

    OperationType opType = check_operation_type(argv[1]);

    if (opType == e_encode)
    {
        if (argc < 4)
        {
            printf("INFO: Encoding - minimum 4 arguments.\n");
            return 1;
        }

        EncodeInfo encInfo;
        if (read_and_validate_encode_args(argv, &encInfo) == e_failure)
            return 1;

        // Start encoding
        if (do_encoding(&encInfo) == e_failure)
        {
            printf("ERROR: Encoding failed\n");
            return 1;
        }
    }
    else if (opType == e_decode)
    {
        if (argc < 3)
        {
            printf("INFO: Decoding - minimum 3 arguments.\n");
            return 1;
        }

        DecodeInfo decInfo;
        if (read_and_validate_decode_args(argv, &decInfo) == e_failure)
            return 1;

        decInfo.fptr_stego_image = fopen(decInfo.stego_image_fname, "rb");
        if (!decInfo.fptr_stego_image)
        {
            printf("ERROR: Cannot open stego image file %s\n", decInfo.stego_image_fname);
            return 1;
        }

        printf("INFO: Decoding started...\n");

        if (do_decoding(&decInfo) == e_failure)
        {
            printf("ERROR: Decoding failed\n");
            return 1;
        }
    }
    else
    {
        printf("INFO: Please pass valid arguments.\n");
        printf("INFO: Encoding - minimum 4 arguments.\n");
        printf("INFO: Decoding - minimum 3 arguments.\n");
        return 1;
    }
    return 0;
}
