
#ifndef STDIO_H
#include <stdio.h>
#define STDIO_H
#endif

#ifndef HUFFMAN_H
#include "huffman.h"
#define HUFFMAN_H
#endif


typedef struct bit_buffer {
    int seek;
    int size;
    int bit[8];
} bit_buffer_t;


/*******************************************************************************
 * Creates a new buffer from a character
 * 
 * Author:
 * - Tom
 *
 * Inputs:
 * - c : The character to make into a buffer
 *
 * Outputs:
 * - A new buffer of the bits that represent the character
 *
*******************************************************************************/
bit_buffer_t char_to_buffer(char c)
{
    bit_buffer_t b;

    /* seek is used for knowing what bit we are up to checking */
    b.seek = 0;
    b.size = 8;

    int i;
    for (i = 0; i < 8; i++)
    {
        b.bit[7-i] = (c >> i) & 1;
    }

    return b;
}


/*******************************************************************************
 * Gets a char from a buffer
 *
 * Author:
 * - Tom
 *
 * Inputs:
 * - b : The bit buffer
 *
 * Outputs:
 * - A character that represents the bits
 *
*******************************************************************************/
char buffer_to_char(bit_buffer_t b)
{
    unsigned char val = 0;

    int i;
    for (i = 0; i < 8; i++)
    {
        val += b.bit[i] << (7-i);
    }

    return val;
}


/*******************************************************************************
 * Clears a bit buffer to zeroes
 *
 * Author:
 * - Tom
 *
 * Inputs:
 * - b : Bit buffer to clear
 *
 * Outputs:
 * - None
 *
*******************************************************************************/
void clear_buffer(bit_buffer_t *b)
{
    b->seek = 0;
    b->size = 0;

    int i;
    for (i = 0; i < 8; ++i)
    {
        b->bit[i] = 0;
    }
}


/*******************************************************************************
 * Displays a buffer in its entirety
 *
 * Author:
 * - Tom
 *
 * Inputs:
 * - b : The buffer to display
 *
 * Outputs:
 * - None
 *
*******************************************************************************/
void display_buffer(bit_buffer_t b)
{
    printf("Seek: %d | Bits: ", b.seek);

    int i;
    for (i = 0; i < 8; i++)
    {
        printf("%d", b.bit[i]);
    }

    printf("\n");
}


/*******************************************************************************
 * Gets the next bit in a bit buffer. If we have run out of bits, returns -1.
 *
 * Author:
 * - Tom
 *
 * Inputs:
 * - b : Bit buffer to read from
 *
 * Outputs:
 * - 0 or 1 depending on the bit. If out of bits, returns -1
 *
*******************************************************************************/
int get_next_bit(bit_buffer_t *b)
{
    if (b->seek == 8)
    {
        return -1;
    }

    return b->bit[b->seek++];
}


/*******************************************************************************
 * Adds one bit to a bit buffer. Returns 1 if the buffer is full after this
 *
 * Author:
 * - Tom
 *
 * Inputs:
 * - b : Bit buffer to add to
 * - c : Value to add as the next bit to the bit buffer ('0' or '1')
 *
 * Outputs:
 * - 1 if bit buffer is full, else 0
 *
*******************************************************************************/
int add_bit_char(bit_buffer_t *b, char c)
{
    int val;

    if (c == '0')
        val = 0;
    else if (c == '1')
        val = 1;
    else
    {
        fprintf(stderr, "Error in add_bit_char (Character wasn't a bit)\n");
        exit(4);
    }

    b->bit[b->size++] = val;

    if (b->size == 8)
        return 1;
    return 0;
}


/*******************************************************************************
 * Compresses a file such that it takes up less space on disc.
 *
 * Author: 
 * - Tom
 *
 * Inputs:
 * - codes       : Huffman code file
 * - target_file : The file to compress
 *
 * Output:
 * - 0 if successful, otherwise 1
 *
 ******************************************************************************/
int compress_file(huffman_code_t *codes, char *target_file)
{
    FILE *target_p;

    /* open the file */
    target_p = fopen(target_file, "rb");
    if (target_p == NULL)
    {
        fprintf(stderr, "Error opening target file\n");
        return 1;
    }

    /* string and string length */
    int compressed_text_size = 2;
    char *compressed_text = (char *)malloc(compressed_text_size);

    int bytes_written = 0;

    /* string of bits */
    char *bit_string = "";
    char write_byte;

    /* bit buffer to use */
    bit_buffer_t buffer;
    clear_buffer(&buffer);

    /* get the filesize of target */
    long file_size;
    fseek(target_p, 0L, SEEK_END);
    file_size = ftell(target_p);
    rewind(target_p);
    printf("  Target filesize is %lu\n", file_size);

    /* read the file char by char*/
    unsigned char c;
    int count;
    int char_count;
    for (char_count = 0; char_count < file_size; ++char_count)
    {
        c = getc(target_p);

        /* for each character, look up what the compressed bit string is */
        char_to_code(codes, c, &bit_string);

        /* have a count. for each bit we read, increment it by one */
        for (count = 0; count < strlen(bit_string); ++count)
        {
            /* move the bits into a bit buffer */
            if (add_bit_char(&buffer, bit_string[count]) == 1)
            {
                /* if the buffer needs to be resized */
                if (bytes_written >= compressed_text_size)
                {
                    int new_size = compressed_text_size * 2;

                    char *new_p = realloc(compressed_text, new_size);
                    if (new_p == NULL)
                    {
                        printf("Ran out of memory\n");
                        return 1;
                    }
                    else
                        compressed_text = new_p;

                    compressed_text_size = new_size;
                }

                /* when the size of the bit buffer is 8, write that byte to the buffer */
                write_byte = buffer_to_char(buffer);
                compressed_text[bytes_written++] = write_byte;

                /* empty the bit buffer */
                clear_buffer(&buffer);
            }
        }
    }

    /* if the buffer needs to be resized */
    if (bytes_written >= compressed_text_size)
    {
        int new_size = compressed_text_size * 2;

        char *new_p = realloc(compressed_text, new_size);
        if (new_p == NULL)
        {
            printf("Ran out of memory\n");
            return 1;
        }
        else
            compressed_text = new_p;

        compressed_text_size = new_size;
    }
    
    /* write the left over bits to the buffer */
    write_byte = buffer_to_char(buffer);
    compressed_text[bytes_written++] = write_byte;

    printf("  Compressed file size is %d bytes\n", bytes_written);

    /* close the file being read */
    fclose(target_p);

    /* reopen for write */
    target_p = fopen(target_file, "wb");
    if (target_p == NULL)
    {
        fprintf(stderr, "Error opening target file\n");
        return 1;
    }

    /* write the bytes to target file */
    fwrite(compressed_text, bytes_written, 1, target_p);

    /* close the file again */
    fclose(target_p);

    free(compressed_text);

    return 0;
}


/*******************************************************************************
 * Decompresses a file.
 *
 * Author: 
 * - Tom
 *
 * Inputs:
 * - codes       : Huffman code file
 * - target_file : The file to decompress
 *
 * Output:
 * - 0 if successful, otherwise 1
 *
 ******************************************************************************/
int decompress_file(huffman_code_t *codes, char *target_file)
{
    FILE *target_p;

    /* open the files */
    target_p = fopen(target_file, "rb");
    if (target_p == NULL)
    {
        fprintf(stderr, "Error opening target file\n");
        return 1;
    }

    /* string and string length */
    int decompressed_text_size = 8;
    char *decompressed_text = (char *)malloc(decompressed_text_size);

    int bytes_written = 0;

    /* string of bits */
    char bit_string[256] = "";
    char write_byte;

    /* bit buffer to use */
    bit_buffer_t buffer;
    clear_buffer(&buffer);

    /* get the filesize of target */
    long file_size;
    fseek(target_p, 0L, SEEK_END);
    file_size = ftell(target_p);
    rewind(target_p);
    printf("  Target filesize is %lu\n", file_size);

    /* read the file char by char */
    unsigned char c;
    int count;
    int char_count;
    for (char_count = 0; char_count < file_size; ++char_count)
    {
        c = getc(target_p);

        /* for each character, get the bits */
        buffer = char_to_buffer(c);

        /* for each bit in the buffer */
        for (count = 0; count < 8; ++count)
        {

            /* add it to the bit string and check it */
            strcat(bit_string, 
                get_next_bit(&buffer) == 0 ? "0" : "1"
            );

            /* if we found a code */
            if (code_to_char(codes, &write_byte, bit_string) == 0)
            {
                /* if the buffer needs to be resized */
                if (bytes_written + 1 >= decompressed_text_size)
                {
                    int new_size = decompressed_text_size * 2;

                    char *new_p = realloc(decompressed_text, new_size);
                    if (new_p == NULL)
                    {
                        printf("Ran out of memory\n");
                        return 1;
                    }
                    else
                    {
                        decompressed_text = new_p;
                    }

                    decompressed_text_size = new_size;
                }

                /* write to the buffer */
                decompressed_text[bytes_written++] = write_byte;

                /* clear the bitstring */
                strcpy(bit_string, "");
            }
        }
    }

    printf("  Decompressed file size is %d bytes\n", bytes_written);

    /* close the files */
    fclose(target_p);

    /* reopen for write */
    target_p = fopen(target_file, "wb");
    if (target_p == NULL)
    {
        fprintf(stderr, "Error opening target file\n");
        return 1;
    }

    /* write the bytes to target file */
    fwrite(decompressed_text, bytes_written, 1, target_p);

    /* close the file again */
    fclose(target_p);

    free(decompressed_text);

    return 0;
}
