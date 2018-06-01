
#ifndef STDLIB_H
#include <stdlib.h> /*atoi*/
#define STDLIB_H
#endif

#ifndef STDIO_H
#include <stdio.h> /*printf sprintf*/
#define STDIO_H
#endif


#ifndef STRING_H
#include <string.h>
#define STRING_H
#endif

#ifndef HUFFMAN_H
#include "huffman.h"
#define HUFFMAN_H
#endif

/*Needs a folder name and number of files as parameters*/
int main(int argc, char *argv[])
{
	/* check if any files were entered */
	if (argc <= 2)
	{
		fprintf(stderr, "Must specify a folder and number of images\n");
		return 1;
	}
	char *folder_name = argv[1];
	int image_count = atoi(argv[2]);

	/* file pointer to use */
	FILE *file_p;

	/* byte count array */
	byte_counts_t counts = new_byte_counts();

	/* count the occurances of each byte in each file */
	int i;
	for (i = 0; i < image_count; ++i)
	{
		/*Making filepath*/
		char filepath[80];
		sprintf(filepath, "%s/%d.bmp", folder_name, i);
		#if DEBUG
			printf("Reading file: %s\n", filepath);
		#endif

		file_p = fopen(filepath, "rb");
		if (file_p == NULL)
		{
			fprintf(stderr, "Failed opening file, ignoring and moving to next\n");
		} else {
			/* count the frequency of each byte */
			get_byte_counts(&counts, file_p);

			/* Very verbose, only print at highest level */
			#if DEBUG & VERBOSE
				printf("  Displaying byte counts:\n");
				display_byte_counts(counts);
				printf("\n");
			#endif			
		}
	}

	/*Don't proceed if no bytes counted*/
	if (!counts.total_byte_count)
	{
		printf("No bytes read, exiting: \n");
		return 1;
	}

	/* create a list of nodes for the tree */
	node_list_t list = list_from_counts(counts);
	sort_nodes(&list);

	/* display the nodes only in debug mode */
	#if DEBUG & LOG_LEVEL >= 3
		printf("Displaying the list of nodes\n");
		display_nodes(list);
		printf("\n");
	#endif

	/* reduce the node list to a tree */
	node_t head = reduce_node_list(list);

	/* generate the huffman code */
	huffman_code_t *codes = new_huffman_codes();
	eval_code(codes, &head, "");

	/* only display the codes when in debug mode */
	#if DEBUG & LOG_LEVEL >= 2
		printf("Displaying the generated huffman codes\n");
		display_codes(codes);
		printf("\n");
	#endif

	if (write_huffman_code_to_file(codes, "huffman.codes"))
		return 1;

	printf("Codes generated\n");

	return 0;
}
