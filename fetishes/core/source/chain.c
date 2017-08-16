#include "chain.h"
#include "error.h"
#include "fraction_math.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <tgmath.h>

void init_chain(Chain * chain)
{
	chain->length = 0;
	chain->start = NULL;
	chain->end = NULL;
}

void clear_chain(Chain * chain)
{
	/* Make pointer to first link */
	Link *temp = chain->start;

	/* If it's null, it's already cleared(hopefully) */
	if (chain->start == NULL)
		return;

	/* Free all except the last link */
	while (temp->next != NULL) {
		temp = temp->next;
		free(temp->prev);
	}

	/* Free last link */
	free(temp);

	/* Reset chain */
	chain->length = 0;
	chain->start = NULL;
	chain->end = NULL;
}

void append_cstr_to_chain(Chain * chain, const char *text)
{
	unsigned int k;
	for (k = 0; text[k] != '\0'; k++) {
		append_flink_to_chain(chain,
				      construct_fraction((FractionInt)
							 text[k], 1));
	}

}

void append_flink_to_chain(Chain * chain, Fraction fraction)
{
	/* Create new link */
	Link *new_link = (Link *) malloc(sizeof(Link));
	new_link->value = fraction;

	/* Insert at end */
	new_link->prev = chain->end;
	new_link->next = NULL;
	if (chain->start == NULL) {
		chain->start = new_link;
	} else {
		chain->end->next = new_link;
	}
	chain->end = new_link;

	/* Increment length */
	chain->length++;
}

void append_chain_to_chain(Chain * chain1, Chain chain2)
{
	Link *it = chain2.start;
	if (it == NULL) {
		return;
	}
	if (chain1->start == NULL) {
		/* Set up iterator as first node */
		chain1->start = (Link *) malloc(sizeof(Link));
		chain1->start->value = it->value;
		chain1->start->prev = NULL;
		chain1->start->next = NULL;
		chain1->end = chain1->start;
	} else {
		/* Set up iterator at end */
		chain1->end->next = (Link *) malloc(sizeof(Link));
		chain1->end->next->prev = chain1->end;
		chain1->end->next->next = NULL;
		chain1->end = chain1->end->next;
		chain1->end->value = it->value;
	}
	while (it->next != NULL) {
		/* Forward iterator */
		it = it->next;
		/* Add node at end */
		chain1->end->next = (Link *) malloc(sizeof(Link));
		chain1->end->next->prev = chain1->end;
		chain1->end->next->next = NULL;
		chain1->end = chain1->end->next;
		chain1->end->value = it->value;
	}

	/* Increment length */
	chain1->length += chain2.length;

}

void chain_to_stream(Chain chain, FILE * stream)
{
	/* Iterator */
	Link *it = chain.start;

	/* Print out chain */
	while (it != NULL) {
		/* Print character */
		fprintf(stream, "%c",
			(char) (it->value.num / it->value.den));

		/* Forward iterator */
		it = it->next;
	}
}

int chain_to_cstr(Chain chain, char * buffer)
{
	/* Iterator */
	Link *it = chain.start;
	int count = 0;

	/* Print out chain */
	while (it != NULL) {
		/* Print character */
		buffer[count] = (char)(it->value.num / it->value.den);

		/* Forward iterator */
		it = it->next;

		count++;
	}
	if(count != chain.length)
	{
		runtime_error("Chain length does not represent true chain length in chain_to_cstr (you should not see this)"); 
	}
	buffer[chain.length] = 0x00;
	return count;
}
	

void append_stream_to_chain(Chain * chain, FILE * stream)
{
	char character;

	/* While not end of file or end of line */
	do {

		/* Get character from stream */
		character = (char) fgetc(stream);

		/* Append to chain */
		append_flink_to_chain(chain,
				      construct_fraction((FractionInt)
							 character, 1));
	} while (character != '\n' && character != EOF);
}

static void num_to_cstr(char *str, FractionInt num)
{
	/* List numeric literals */
	const char *zero_to_nineteen[] = {
		"zero", "one", "two", "three", "four", "five", "six",
		"seven", "eight", "nine", "ten", "eleven",
		"twelve", "thirteen", "fourteen", "fifteen", "sixteen",
		"seventeen", "eighteen", "nineteen"
	};
	const char *twenty_to_ninety[] =
	    { "twenty", "thirty", "forty", "fifty", "sixty", "seventy",
		"eighty", "ninety"
	};
	const char *big_numbers[] =
	    { "thousand", "million", "billion", "quadrillion",
		"quintillion", "sextillion"
	};

	/* Check for less-than-zero error */
	if (num < 0) {
		runtime_error
		    ("num is less than zero in num_to_cstr(this shouldn't happen)");
	} else
		/* Singles and teens */
	if (num < 20) {
		strcpy(str, zero_to_nineteen[num]);
	} else
		/* Adults */
	if (num % 10 == 0 && num < 100) {
		strcpy(str, twenty_to_ninety[num / 10 - 2]);
	} else
		/* Mature */
	if (num % 1000 == 0) {
		strcpy(str, big_numbers[(FractionInt) log10(num) / 3 - 1]);
		/* Anything else returns an error */
	} else {
		runtime_error
		    ("num(%ji) not valid for num_to_cstr(this shouldn't happen)",
		     num);
	}
}

static void append_triple_to_chain(Chain * chain, FractionInt triple)
{
	char temp[50];
	/* Error if triple is too big or too small */
	if (triple > 999 || triple < 0) {
		runtime_error("invalid triple (you should not see this)");
	}

	/* 100's place */
	if (triple / 100 != 0) {
		num_to_cstr(temp, triple / 100);
		append_cstr_to_chain(chain, temp);
		append_cstr_to_chain(chain, " hundred");
		triple %= 100;
		if (triple != 0) {
			append_cstr_to_chain(chain, " and ");
		} else {
			return;
		}

	}


	/* Tens place */
	if (triple / 10 != 0 && triple > 20) {
		num_to_cstr(temp, (triple / 10) * 10);
		append_cstr_to_chain(chain, temp);
		triple %= 10;
		if (triple != 0) {
			append_cstr_to_chain(chain, " ");
		} else {
			return;
		}
	}

	/* Ones place */
	if (triple) {
		num_to_cstr(temp, triple);
		append_cstr_to_chain(chain, temp);
	}

}

static void append_fraction_int_to_chain(Chain * chain, FractionInt num)
{
	FractionInt magnitude = 0;
	int started = 0;
	char temp[50];

	/* Find magnitude */
	/* Magnitude overflows when num is 10^18 */
	for (magnitude = 1; (num / magnitude) != 0; magnitude *= 1000);

	/* Write to chain */
	while (magnitude > 0) {
		/* If applicable... */
		if (num / magnitude != 0) {
			/* Add space */
			if (started == 1) {
				append_cstr_to_chain(chain, ", ");
			} else {
				started = 1;
			}

			/* Add triple to chain */
			append_triple_to_chain(chain, num / magnitude);

			/* If in 1000's, million's, etc's place ... */
			if (magnitude != 1) {
				/* Append a space, and an appropriate power-of-thousand modifier */
				append_cstr_to_chain(chain, " ");
				num_to_cstr(temp, magnitude);
				append_cstr_to_chain(chain, temp);
			}
		}

		/* Reduce number and magnitude */
		num -= (num / magnitude) * magnitude;
		magnitude /= 1000;
	}
}

void append_fraction_to_chain(Chain * chain, Fraction fraction)
{
	/* Reduce fraction */
	reduce_fraction(&fraction);

	/* Append "negative" if necessary */
	if (fraction.num < 0) {
		append_cstr_to_chain(chain, "negative ");
		/* Make numerator positive */
		fraction.num = -fraction.num;
		/* Check if zero */
	} else if (fraction.num == 0) {
		append_cstr_to_chain(chain, "zero");
		return;
	}
	/* Check if infinity */
	if (fraction.den == 0) {
		append_cstr_to_chain(chain, "a lot");
		return;
	}


	/* Write numerator to chain */
	append_fraction_int_to_chain(chain, fraction.num);

	/* Write denominator to chain */
	if (fraction.den != 1) {
		append_cstr_to_chain(chain, " over ");
		append_fraction_int_to_chain(chain, fraction.den);
	}
}

Fraction chain_to_fraction(Chain chain)
{
	/* Define starting fraction */
	Fraction frac = construct_fraction(0, 0);

	char value;
	Link *it = chain.start;

	int place = 0;
	int negative = 0;

	/* Check for negatives */
	if (it != NULL) {
		if ('-' == (char) (it->value.num / it->value.den)) {
			negative = 1;
			it = it->next;
		}

		/* Parse chain */
		while (it != NULL) {
			/* convert value to character */
			value = (char) (it->value.num / it->value.den);

			/* parse digit */
			if (value >= '0' && value <= '9') {
				if (!place) {
					/* Add to numerator */
					frac.num *= 10;
					frac.num += value - '0';
				} else {
					/* Add integer */
					frac.den *= 10;
					frac.den += value - '0';
				}
			} else if (value == '/' && place == 0) {
				place = 1;
			} else {
				/* Break if coming across an unexpected character */
				break;
			}
			/* Forward iterator */
			it = it->next;
		}
	}

	/* Construct, */
	if (place == 0) {
		frac.den = 1;
	}
	if (negative) {
		frac.num = -frac.num;
	}

	/* 0/0 is 1 in this case */
	if (frac.num == 0 && frac.den == 0) {
		frac.num = frac.den = 1;
	}

	/* Reduce and return */
	reduce_fraction(&frac);
	return frac;
}


void write_chain_to_file(Chain chain, Chain filename_as_chain)
{
	// Convert chain to char*
	const int max_filename_size = filename_as_chain.length;
	char * filename = (char*)malloc(sizeof(char*) * (max_filename_size + 1));
	if(filename == NULL)
	{
		runtime_error("Couldn't allocate memory for filename buffer in write_chain_to_file");
	}
	memset(filename, 0x00, max_filename_size);
	const int size = chain_to_cstr(filename_as_chain, filename);
	if(size > max_filename_size)
	{
		runtime_error("Size is ridiculous is write_chain_to_file");
	}

	// Open the file for writing
	FILE* fp = fopen(filename, "wb");
	if(fp == NULL)
	{
		runtime_error("Could not open file %s for writing", filename);
	}

	// Write
	Link * it = chain.start;
	while(it != NULL)
	{
		fputc((char)(it->value.num/it->value.den), fp);
		it = it->next;
	}

	fclose(fp);
}
void read_file_to_chain(Chain * chain, Chain filename_as_chain)
{
	// Convert chain to char*
	const int max_filename_size = filename_as_chain.length;
	char * filename = (char*)malloc(sizeof(char*) * (max_filename_size + 1));
	if(filename == NULL)
	{
		runtime_error("Couldn't allocate memory for filename buffer in read_chain_to_file");
	}
	memset(filename, 0x00, max_filename_size);
	const int size = chain_to_cstr(filename_as_chain, filename);
	if(size > max_filename_size)
	{
		runtime_error("Size is ridiculous is read_file_to_chain");
	}

	// Open the file for reading
	FILE* fp = fopen(filename, "rb");
	if(fp == NULL)
	{
		runtime_error("Could not open file %s for reading", filename);
	}

	// Read
	while(1)
	{
		char c = (char)fgetc(fp);
		if(c == EOF) break;
		append_flink_to_chain(chain, construct_fraction(c, 1));
	}

	fclose(fp);
	return;
}

int compare_chains(Chain a, Chain b)
{
	Link *a_iterator = a.start;
	Link *b_iterator = b.start;

	/* Compare until at least one node is NULL */
	while (a_iterator != NULL && b_iterator != NULL) {
		/* Compare the numerator and denominator */
		if (a_iterator->value.num != b_iterator->value.num
		    || a_iterator->value.den != b_iterator->value.den) {
			return 1;
		}

		/* Foward iterators */
		a_iterator = a_iterator->next;
		b_iterator = b_iterator->next;
	}

	/* Check if one XOR the other is NULL */
	if ((a_iterator == NULL || b_iterator == NULL)
	    && a_iterator != b_iterator) {
		return 1;
	}

	return 0;
}


/*
void print_chain_numerically(Chain chain)
{
	Link *it = chain.start;
	printf("(");
	while (it != NULL) {
		// Display item numerator as integer
		printf("%s%" FRACTION_INT_FORMATTER,
		       it != chain.start ? ", " : "", it->value.num);

		// Display denominator if not 1
		if (it->value.den != 1) {
			printf("/%" FRACTION_INT_FORMATTER, it->value.den);
		}

		// Forward iterator
		it = it->next;
	}
	printf(")");
}
*/
/*
FILE* open_file_as_stream(Chain filename, const char* mode)
{
	int max_size = sizeof(char)*(filename.length+1);
	char* buffer = (char*)malloc(max_size);
	if(buffer == NULL)
	{
		runtime_error("Could not allocate buffer for file");
	}

	int actual_size = chain_to_cstr(filename, buffer);
	if(actual_size > filename.length)
	{
		runtime_error("Something's terribly wrong in open_file_as_stream");
	}

	FILE* fp = fopen(buffer, mode);
	if(fp == NULL){
		perror("perror result: ");
		runtime_error("Could not open file %s", buffer);
	}
	return fp;
}
*/
