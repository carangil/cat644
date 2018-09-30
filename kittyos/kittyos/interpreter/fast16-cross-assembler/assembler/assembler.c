#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <malloc.h>

typedef struct instr_s{
	char* name;
	unsigned int opcode; //the instruction opcode
	int databytes;  //0, 1 or 2
	int relative;  //1 if databyte is relative address
	int pseudo;  //no real opcode
} instr_t;

#define MAX_INST 255
instr_t instructions[255];
int num_instructions = 0;


instr_t* find_instruction(char* name, int create) {

	int i;

	

	
	for (i = 0;i < num_instructions;i++) {

		if (!_stricmp(name, instructions[i].name))
			return instructions + i;
	}
	if (!create)
		return NULL;
	printf(" adding instr %s\n", name);
	instructions[num_instructions].name = _strdup(name);
	
	return instructions + (num_instructions++);
}

/* Reads the hexfile map to determine instruction encodings*/
void readmap(char* fname, char* out_h) {
	int iaddrlow = 0;
	char* iaddrs;
	char* iname;
	char linebuf[1000];
	int smallest = 0xffffff;
	int largest = 0;

	int pos = 0;
	char c;
	instr_t* in;
	FILE* hf = NULL;
	FILE* mapfile = fopen(fname, "rb");
	if (!mapfile)
		exit(1);

	if (out_h)
		hf = fopen(out_h, "wb");


	while (1) {
		c = getc(mapfile);
		if (feof(mapfile))
			break;

		if (c == '\n' || c == '\r') {			
			linebuf[pos] = '\0';
			
			if (iname = strstr(linebuf, "vm_")) {

				if (hf) {
					char* s = iname;
					fprintf(hf,"extern ");
					while (*s && !isspace(*s))
						putc(*(s++), hf);

					fprintf(hf,";\n");


				}

				iname += 3;
				iaddrs = strstr(linebuf, "0x");

				if (iaddrs) {
					iaddrlow = (strtol(iaddrs, NULL, 16) / 2);

					if (iaddrlow < smallest)
						smallest = iaddrlow;

					if (iaddrlow > largest)
						largest = iaddrlow;

					printf(" %s  %x\n", iname, iaddrlow);
					in = find_instruction(iname,1);
					if (in) {

						in->opcode = iaddrlow;

					}

				}
			}



			pos = 0;
			continue;
		}

		if (pos < sizeof(linebuf))
			linebuf[pos++] = c;

	}

	printf(" smallest %x  largest %x\n", smallest, largest);
	if ((largest - smallest)>=256) {
		printf("Critical error: jumptable too big to fit in one flash page\n");
		exit(1);
	}

}




int scanline(FILE* f, char* linebuf, int bufsiz){
	int pos = 0;

	char c=0;
	char last = 0;
	int comment = 0;
	int instring = 0;

	while (1) {
		last = c;
		c = getc(f);
		if (feof(f))
			return 0; //noline



		if (c == '\n' || c == '\r') {
			comment = 0;
			linebuf[pos] = '\0';
			if (pos == 0)
				continue;  //zero length line, try another
			
			return 1;
		}

		//if (c == '#') //begin comment 
			//comment = 1;
			
		

		if (comment) 
			continue;
		

		//skip consecutive space
		if (isspace(c) && isspace(last))
			continue;

		//skip leading space
		if (isspace(c) && pos == 0)
			continue;


		if (pos > 0 && isspace(last) && !(isdigit(c) || c == '@' || c == '$' || c=='#' || c == '\'')) {
			if (!instring)
				linebuf[pos - 1] = '_';
		}

		if (c == '\'')
			instring = 1;

		if (pos < bufsiz)
			linebuf[pos++] = c;

	}
	//unreachable

}



typedef struct label_s {
	char* name;
	int offset;
	int relative;  //only for forward labels
	int databytes; //only for forward labels
	struct label_s* next;
} label_t;

label_t* labels = NULL;		//offset is the position of the label
label_t* forwardlabels = NULL; //offset is the position that needs a label looked up

label_t* findlabel(label_t*  labs, char* name) {

	while (labs) {
		if (!strcmp(name, labs->name))
			return labs;  //return the found label

		labs = labs->next;
	}
	return NULL;
}

label_t*  addlabel(label_t* labs, char* name, int offset) {

	label_t* lab = malloc(sizeof(label_t));

	if (!lab)
	{
		printf("Somehow out of memory\n");
		exit(1);
	}

	memset(lab, 0, sizeof(*lab));

	lab->name = _strdup(name);
	lab->offset = offset;
	lab->next = labs;
	return lab;  //return the new head of the list
}

instr_t* in_string = NULL;

//returns the program buffer
void assemble(char* input_file, char* output_file, int format)
{

	unsigned char* progbuf;

	char linebuf[1000];
	char linebufcopy[1000];
	int pos = 0;
	
	char c;
	FILE* f;
	char* p;
	instr_t* in;
	label_t* lab = 0;
	unsigned int num;
	
	progbuf = malloc(65536); //maximum size for 16-bit programs
	if (!progbuf) {
		printf(" Somehow out of memory allocating progbuf\n");
		exit(1);
	}

	
	//pass 1 : read and compute label positions and program length

	f = fopen(input_file, "rb");
	if (!f) {
		printf(" Can't read %s\n", input_file);
		exit(1);
	}

	while (scanline(f, linebuf, sizeof(linebuf))) {


		if (linebuf[0] == '#')  //ignore start of comment
			continue;

		memcpy(linebufcopy, linebuf, sizeof(linebuf));
		
		


		p = strstr(linebuf, ":");
		if (p) {
			//this is a label
			p[0] = 0;  //kill the colon

			lab = findlabel(labels, linebuf);
			if (lab) {
				printf(" Duplicate label %s\n", linebuf);
				exit(1);
			}
			labels = addlabel(labels, linebuf, pos);

			printf(" Label %s at %x\n", linebuf, pos);
			continue;

		}

		p = strstr(linebuf, " ");

		if (p) {
			*p = 0;
			p++;
		}
		
	

		in = find_instruction(linebuf, 0);
		if (in) {

			printf(" {%s}\n ", in->name);

			if (!in->pseudo) {
				progbuf[pos] = in->opcode;
				pos++;
			}

			if (in == in_string) {

				p++;//skip over '
				while(*p)
					progbuf[pos++] = (*(p++)) & 0xff;

			} 
			else if (in->databytes) {
				if (!p || !*p) {
					printf(" Operand required for %s\n", linebuf);
					exit(1);
				}

				if (p[0] == '$') {

					num = strtol(p + 1, NULL, 16);
				}
				else if (p[0] == '@') {
					lab = findlabel(labels, p + 1);
					if (lab) {
						printf(" Found back label %s %x\n", lab->name, lab->offset);
						if (in->relative)
							num = -pos - in->databytes + lab->offset;

					}
					else {
						forwardlabels = addlabel(forwardlabels, p + 1, pos);
						forwardlabels->relative = in->relative;
						forwardlabels->databytes = in->databytes;
						printf(" Add forward label: fix %x to point at @%s\n", pos, p + 1);
						num = 0xeeee;  //signal to fix
					}

				}
				else {
					num = atoi(p);
				}


				if (in->databytes == 1) {
					printf(" byte %02x\n", num & 0xff);
					progbuf[pos++] = num & 0xff;

				}
				else if (in->databytes == 2) {
					printf("word %04x  %d\n", num & 0xffff, num);
					progbuf[pos++] = num & 0xff;
					progbuf[pos++] = ((unsigned int)(num) >> 8) & 0xff;
				}



			}
			else if (p && *p && !isspace(*p) && (*p!='#')) { /* If there is a 'p', and it isn't just a null terminator, and isn't a space and isn't a comment*/
				printf("unexpected operand for %s %s\n", linebuf, p);
				exit(1);

			}


		}
		else {
			printf("Unknown Instruction '%s'\n", linebuf);
			exit(1);
		}


		//printf(" GOT LINE [%s]\n", linebuf);
		

	}

	//pass 2: todo: fix forward labels
	while (forwardlabels) {

		//find a label that matches the forward label name
		lab = findlabel(labels, forwardlabels->name);
		if (!lab) {
			printf(" Forward label %s never resolved\n", forwardlabels->name);
			exit(1);
		}

		if (forwardlabels->databytes != 2) {
			printf(" Forward label 1-byte not implemented");
			exit(1);
		}

		if (forwardlabels->relative) {
			num = lab->offset - forwardlabels->offset - forwardlabels->databytes;
			printf(" to %d from %d is %d %x", lab->offset, forwardlabels->offset, num, num);
		}
		else
			num = lab->offset;

		progbuf[forwardlabels->offset] = num & 0xff;
		progbuf[forwardlabels->offset+1] = (((unsigned int)num)>>8) & 0xff;


		forwardlabels = forwardlabels->next;

	}

	
	//pass 3:output
	{ int i;
		FILE* of;
		printf("Finish %d bytes\n", pos);
		for (i = 0;i < pos;i++) {
			printf("0x%02x%c\n", progbuf[i], (i == (pos - 1)) ? ' ' : ',');
		}



		of = fopen(output_file, "wb");
		if (!of) {
			printf("Can't write output file\n");
			exit(1);
		}


		fprintf(of,"unsigned char  program[]={\n");
		for (i = 0;i < pos;i++) {
			fprintf(of,"0x%02x%c\n", progbuf[i], (i==(pos-1))?' ':',');
		}
		fprintf(of,"};\n\n");
		fclose(of);

	}
}



int main(int argc, char ** args)
{
	char* input_file=NULL;
	char* output_file = NULL;
	char* map_file = NULL;
	char* output_h = NULL;
	int output_format = 0;
	instr_t* instr;
	int size = 0;
	char* prog = NULL;
	int i;
	for (i = 1;i < argc; i++)
	{

		if (args[i][0] == '@') {
			input_file = args[i] + 1;
			continue;
		}

		if (args[i][0] == '-') {

			if (args[i][1] == 'c')
				output_format = 1;
			continue;
		}

		if (args[i][0] == '^') {

			output_file = args[i] + 1;
			continue;
		}


		if (args[i][0] == 'h') {

			output_h = args[i] + 1;
			continue;
		}

		if (args[i][0] == 'm') {

			map_file = args[i] + 1;
			continue;
		}



	}

	printf(" input:%s\noutput:%s\nmap:%s\nformat:%d\n", input_file, output_file, map_file, output_format);

	//init instruction list
	memset(instructions, 0, sizeof(instructions));


	//fill out some instruction data;
#if 1
	instr = find_instruction("ldi_a", 1);
	instr->databytes = 2;

	instr = find_instruction("ldi_b", 1);
	instr->databytes = 2;

	instr = find_instruction("ldi_c", 1);
	instr->databytes = 2;

	instr = find_instruction("ldi_d", 1);
	instr->databytes = 2;

	instr = find_instruction("pop", 1);
	instr->databytes = 1;


	instr = find_instruction("pick_a", 1);
	instr->databytes = 1;

	instr = find_instruction("put_a", 1);
	instr->databytes = 1;


	instr = find_instruction("jmpr", 1);
	instr->databytes = 2;
	instr->relative = 1;
	
	instr = find_instruction("jz", 1);
	instr->databytes = 2;
	instr->relative = 1;


	instr = find_instruction("jnz", 1);
	instr->databytes = 2;
	instr->relative = 1;

	instr = find_instruction("callr", 1);
	instr->databytes = 2;
	instr->relative = 1;
	instr = find_instruction("jae", 1);
	instr->databytes = 2;
	instr->relative = 1;
	instr = find_instruction("jb", 1);
	instr->databytes = 2;
	instr->relative = 1;
	instr = find_instruction("jneg", 1);
	instr->databytes = 2;
	instr->relative = 1;
	instr = find_instruction("jnz", 1);
	instr->databytes = 2;
	instr->relative = 1;
	instr = find_instruction("jz", 1);
	instr->databytes = 2;
	instr->relative = 1;

	instr = find_instruction("syscall", 1);
	instr->databytes = 1;


	instr = find_instruction("byte", 1);
	instr->databytes = 1;
	instr->pseudo = 1;

	instr = find_instruction("word", 1);
	instr->databytes = 2;
	instr->pseudo = 1;

	in_string = find_instruction("string", 1);
	in_string->pseudo = 1;


#endif 


	//read the map file
	readmap(map_file, output_h);


	for (i = 0;i < num_instructions;i++) {

		printf("%s\t$%02x\tdatabytes:%d\trelative:%d\n",
			instructions[i].name,
			instructions[i].opcode,
			instructions[i].databytes,
			instructions[i].relative);

		if ((instructions[i].opcode == 0) && (instructions[i].pseudo ==0)) {
			printf("Critical error instruction with no jumpcode entry\n");
			exit(1);

		}
	}


	assemble(input_file, output_file, output_format);

	

}