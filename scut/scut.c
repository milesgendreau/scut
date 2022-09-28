/*
 * File: scut.c
 * Author: Miles Gendreau
 * Description: Simple version of the cut bash command. Takes multiple lines from stdin
 * and prints selected columns from each line to stdout.
 *
 * Usage: Run executable with the two manditory arguments:
 * 	$ ./a.out [option] [selections]
 *
 * Options:
 * 	-l : specifies that the columns for the cut selections are 1 character wide.
 * 	-w : specifies that columns will be separated by whitespace.
 * 	-c : specifies that columns will be separated by ','.
 *
 * Selections:
 * 	a     : column a will be cut
 * 	a-b   : columns a, a+1,..., b will be cut
 *	
 *	Combine multiple selections into a list separated by ','. Ex:
 *	a,b-c,d : columns a, b, b+1, ..., c, d will be cut
 *
 * Example 1:
 * 	stuff.csv contains:
 * 	alice,30,532,AZ,S
 * 	bob,25,3411,CA,Z
 * 	jonas,40,8192,AZ,T
 * 	greg,50,400,UT,C
 *	
 *	$ cat stuff.csv | ./a.out -l 1-2,7-15
 *	a l 3 0 , 5 3 2 , A Z
 *	b o , 3 4 1 1 , C A ,
 *	j o 4 0 , 8 1 9 2 , A
 *	g r 0 , 4 0 0 , U T ,
 *
 *	$ cat stuff.csv | ./a.out -c 1,3-5
 *	alice 532 AZ S
 *	bob 3411 CA Z
 *	jonas 8192 AZ T
 *	greg 400 UT C
 *
 * Example 2:
 * 	$ printf "this is some\nsample text" | ./a.out -w 1,3-5
 * 	this some
 * 	sample text
 */

#include<stdio.h>
#include<string.h>
#include<stdlib.h>

/*
 * Returns 0 if the selection is valid, ie, it begins and ends with a digit,
 * and contains only digits, ',', and '-' characters.
 * If invalid, returns 2.
 */
int validateSelection(char *sel) {
	if (!(sel[0] > 47 && sel[0] < 58)) {return 2;}
	for (int i = 1; i < strlen(sel)-1; i++){
		if (!((sel[i] > 47 && sel[i] < 58) || sel[i] == ',' || sel[i] == '-')) {
			return 2;
		}
	}
	if (!(sel[strlen(sel)-1] > 47 && sel[strlen(sel)-1] < 58)){return 2;}

	return 0;
}

/*
 * Inserts the integers from a-b (inclusive) into the provided array cols. Keeps track
 * of the pointer to the first unassigned element, col_ind.
 */
void addRange(int *cols, int *col_ind, int a, int b) {
	int j = *col_ind;
	for (int i = a; i <= b; i++) {
		cols[j] = i;
		j ++;
	}
	*col_ind = j;
}

/*
 * Parses the second command line argument, adding each column that the selection has
 * specified to the cols array. This array will contain every column which the user
 * has specified should be extracted from the lines of standard input.
 */
void getCols(int *cols, char *sel) {
	int col_ind = 0; // index of first unassigned element, or "empty space" in cols

	int curr_ind = 0;
	char curr[3] = "  "; // current sequence of digits being parsed

	int prev = -1; // number before a dash, ex: "3-4", prev would be 3.
	for (int i = 0; i < strlen(sel); i++) {
		if (sel[i] > 47 && sel[i] < 58) {
			curr[curr_ind] = sel[i];
			curr_ind ++;
			if (sel[i+1] == ',') {
				// if this number is the second half of a x-x pair, add
				// every number in that range to cols
				if (prev != -1) {
					addRange(cols, &col_ind, prev, atoi(curr));
					prev = -1;
				}
				else {
					cols[col_ind] = atoi(curr);
					col_ind ++;
				}
				curr[1] = ' ';
				curr_ind = 0;
			}
			else if (sel[i+1] == '-') {
				if (prev == -1) {
					prev = atoi(curr);
				}
				curr[1] = ' ';
				curr_ind = 0;
			}
		}
	}
	if (prev != -1) {
		addRange(cols, &col_ind, prev, atoi(curr));
		prev = -1;		
	}
	else {
		cols[col_ind] = atoi(curr);
	}
}

int main(int argc, char *argv[]){
	if(argc != 3){
		fprintf(stderr, "%s\n", "expected 2 command line arguments.");
		return 2;
	}
	
	int flag;
	if (strcmp(argv[1], "-l") == 0) {flag = 0;}
	else if (strcmp(argv[1], "-w") == 0) {flag = 1;}
	else if (strcmp(argv[1], "-c") == 0) {flag = 2;}
	else {
		fprintf(stderr, "%s\n", "Invalid delimiter type.");
		return 2;
	}

	int cols[128] = {0};
	if (validateSelection(argv[2]) != 0) {
		fprintf(stderr, "%s\n", "Invalid selection.");
		return 2;
	}
	getCols(cols, argv[2]);

	char buffer[129];
	while (fgets(buffer, 128, stdin) != NULL) {
		int i = 0, j = 0;
		int col = 1;
		while (buffer[i] != '\n' && buffer[i] != '\0') {
			if (flag == 0 && i+1 == cols[j]) {
				printf("%c", buffer[i]);
				j++;
				if (cols[j] != 0) {
					printf(" ");
				}
			}
			else if (flag == 1) {
				if (buffer[i] == ' ') {
					if (col == cols[j] && cols[j+1] != 0) {
						printf(" ");
						j++;
					}
					col++;
					i++;
				}
				if (col == cols[j]) {
					printf("%c", buffer[i]);
				}
			}
			else if (flag == 2) {
				if (buffer[i] == ',') {
					if (col == cols[j] && cols[j+1] != 0) {
						printf(" ");
						j++;
					}
					col++;
					i++;
				}
				if (col == cols[j]) {
					printf("%c", buffer[i]);
				}
			}
			i++;
		}
		printf("\n");
	}

	return 0;
}
