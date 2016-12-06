#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define true 1
#define false 0

int *parsefile(FILE *f);
void printoutput(int *array);
void writeoutput(int *array, char *output_filename);

/* Parsing function that given a file stream outputs an array of 15 numbers, with '-' entered as 45*/
int *parsefile(FILE *f){
	int *array;
	array = (int *)malloc(sizeof(int) * 16);
	int c, stored, counter = 0;
	int is_last_space = true;
	while ((c=fgetc(f)) != EOF){
			// printf("current char is %c\n", c);
			if ((c == '\n') || (c == '\t')){
				// printf("space char\n");
				is_last_space = true;
				counter++;

			}
			/* Note the hyphen character writes in as 45 */
			else if (c == '-'){
				array[counter] = '-';
				is_last_space = false;
				// printf("hyphen character\n");
			} else{
				array[counter] = c;
				if (!is_last_space){
					array[counter] = c - '0' + 10;
				} else{
					array[counter] = c - '0';
					is_last_space = false;
				}
			}
	}

	//Debugging sanity check
	// int i;
	// for (i = 0; i < 16; i++){
	// 	printf("array[%d]: %d\n", i, array[i]);
	// }
	fclose(f);
	return array;
}
/* Debugging function to just print the array */
void printoutput(int *array){
	int i;
	for (i = 0; i < 16; i++){
		printf("array[%d]: %d\n", i, array[i]);
	}
}
/* Output function to write output*/
void writeoutput(int *array, char *output_filename){
	int i;
	FILE *f = fopen(output_filename, "w+");
	for (i = 0; i < 16; i++){
		if (array[i] == 45) fprintf(f, "-");
		else fprintf(f, "%d", array[i]);
		//print space character
		if (i % 4 == 3){
			fprintf(f, "\n");
		} else fprintf(f, "\t");
	}
	fclose(f);
}



int main(int argc, char **argv){
	char *filename = argv[1];
	FILE *f = fopen(filename, "r");
	if (f == NULL){
		printf("No file with that name exists\n");
		return 0;
	}
	int *array;
	array = parsefile(f);
	/*Do Stuff here with the array after its parsed*/
	char *outputName = "test.txt";
	writeoutput(array, outputName);


	return 0;
}