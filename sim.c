#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <ctype.h>

#define MAX_LINE 10 // width of the memory and the disk
#define MAX_FILE 4096 // size of the main memory
#define MAX_FILE_D 16384 // size of the disk (128 sectors of 128 words)
Print_To_hwregtrace(FILE *hwregtrace, int count, int *io_rege, int read_flag, int io_reg_number) { // prints to hwregtrace.txt, invoked when reading or writing to an io register
	char hexval[9], decval[8];
	sprintf(hexval, "%08X", io_rege[io_reg_number]);
	sprintf(decval, "%i", count);
	fputs(decval, hwregtrace);
	putc(' ', hwregtrace);
	if (read_flag) {
		fputs("READ ", hwregtrace);
	}
	else
		fputs("WRITE ", hwregtrace);

	if (io_reg_number == 0)
		fputs("irq0enable ", hwregtrace);
	if (io_reg_number == 1)
		fputs("irq1enable ", hwregtrace);
	if (io_reg_number == 2)
		fputs("irq2enable ", hwregtrace);
	if (io_reg_number == 3)
		fputs("irq0status ", hwregtrace);
	if (io_reg_number == 4)
		fputs("irq1status ", hwregtrace);
	if (io_reg_number == 5)
		fputs("irq2status ", hwregtrace);
	if (io_reg_number == 6)
		fputs("irqhandler ", hwregtrace);
	if (io_reg_number == 7)
		fputs("irqreturn ", hwregtrace);
	if (io_reg_number == 8)
		fputs("clks ", hwregtrace);
	if (io_reg_number == 9)
		fputs("leds ", hwregtrace);
	if (io_reg_number == 10)
		fputs("display ", hwregtrace);
	if (io_reg_number == 11)
		fputs("timerenable ", hwregtrace);
	if (io_reg_number == 12)
		fputs("timercurrent ", hwregtrace);
	if (io_reg_number == 13)
		fputs("timermax ", hwregtrace);
	if (io_reg_number == 14)
		fputs("diskcmd ", hwregtrace);
	if (io_reg_number == 15)
		fputs("disksector ", hwregtrace);
	if (io_reg_number == 16)
		fputs("diskbuffer ", hwregtrace);
	if (io_reg_number == 17)
		fputs("diskstatus ", hwregtrace);
	fputs(hexval, hwregtrace);
	putc('\n', hwregtrace);
}

int cyclic_address(int address) { //handling addresses that are not betwenn 0-4095, based on cyclic addressing
	int temp;
	if (address > 4095)
		return (address % 4096);
	if (address < 0) {
		temp = (4096 - ((address*(-1)) % 4096));
		if (temp == 4096)
			return 0;
		else
			return temp;
	}
	else return address;
}
int Opcode_Operation(int opcode, int rd, int rs, int rt, int *rege, int *io_rege,int *disk_f, int *reti_flag, long pc, FILE *mem_in, char output[][9], char output_d[][9], int count, FILE *hwregtrace, FILE *leds, FILE *display, int* finish_t) { // the main script, executes the operation, based on the opcode
	char hexval[9],decval[8];
	char *line2, LINES[MAX_LINE];
	line2 = LINES;
	if (count == finish_t[0]) // checks if 1024 cycles has passed after a write or a read frrom the hard disk, and sets diskcmd and diskstatus to 0, while raising irq1
	{
		io_rege[17] = 0;
		io_rege[14] = 0;
		io_rege[4] = 1;
	}
	if (opcode == 0) { //add instruction
		if (rd != 1)
			rege[rd] = rege[rs] + rege[rt];
		if (rd == 0)
			rege[rd] = 0;
		pc += 1;
	}
	if (opcode == 1) { //sub instruction
		if (rd != 1)
			rege[rd] = rege[rs] - rege[rt];
		if (rd == 0)
			rege[rd] = 0;
		pc += 1;
	}
	if (opcode == 2) {//and instruction
		if (rd != 1)
			rege[rd] = (rege[rs] & rege[rt]);
		if (rd == 0)
			rege[rd] = 0;
		pc += 1;
	}
	if (opcode == 3) { //or instruction
		if (rd != 1)
			rege[rd] = (rege[rs] | rege[rt]);
		if (rd == 0)
			rege[rd] = 0;
		pc += 1;
	}
	if (opcode == 4) { //sll instruction
		if (rd != 1)
			rege[rd] = (rege[rs] << rege[rt]);
		if (rd == 0)
			rege[rd] = 0;
		pc += 1;
	}
	if (opcode == 5) { //sra instruction
		if (rd != 1)
			rege[rd] = (rege[rs] >> rege[rt]);
		if (rd == 0)
			rege[rd] = 0;
		pc += 1;
	}
	if (opcode == 6) { //srl instruction
		if (rd != 1)
			rege[rd] = ((unsigned int)rege[rs] >> rege[rt]);
		if (rd == 0)
			rege[rd] = 0;
		pc += 1;
	}
	if (opcode == 7) {//beq instruction
		if (rege[rs] == rege[rt]) {
			pc = cyclic_address(rege[rd]);
			int pc_help = pc;
			fseek(mem_in, 0, SEEK_SET);
			while (pc_help > 0) {
				fgets(line2, MAX_LINE, mem_in);
				pc_help -= 1;
			}
		}
		else
			pc += 1;
	}
	if (opcode == 8) { //bne instruction
		if (rege[rs] != rege[rt]) {
			pc = cyclic_address(rege[rd]);
			int pc_help = pc;
			fseek(mem_in, 0, SEEK_SET);
			while (pc_help > 0) {
				fgets(line2, MAX_LINE, mem_in);
				pc_help -= 1;
			}
		}
		else
			pc += 1;
	}
	if (opcode == 9) { //blt instruction
		if (rege[rs] < rege[rt]) {
			pc = cyclic_address(rege[rd]);
			int pc_help = pc;
			fseek(mem_in, 0, SEEK_SET);
			while (pc_help >= 0) {
				fgets(line2, MAX_LINE, mem_in);
				pc_help -= 1;
			}
		}
		else
			pc += 1;
	}
	if (opcode == 10) {	//bgt instruction
		if (rege[rs] > rege[rt]) {
			pc = cyclic_address(rege[rd]);
			int pc_help = pc;
			fseek(mem_in, 0, SEEK_SET);
			while (pc_help > 0) {
				fgets(line2, MAX_LINE, mem_in);
				pc_help -= 1;
			}
		}
		else
			pc += 1;
	}
	if (opcode == 11) {	//ble instruction
		if (rege[rs] <= rege[rt]) {
			pc = cyclic_address(rege[rd]);
			int pc_help = pc;
			fseek(mem_in, 0, SEEK_SET);
			while (pc_help >= 0) {
				fgets(line2, MAX_LINE, mem_in);
				pc_help -= 1;
			}
		}
	}
	if (opcode == 12) {	//bge instruction
		if (rege[rs] >= rege[rt]) {
			pc = cyclic_address(rege[rd]);
			int pc_help = pc;
			fseek(mem_in, 0, SEEK_SET);
			while (pc_help >= 0) {
				fgets(line2, MAX_LINE, mem_in);
				pc_help -= 1;
			}
		}
	}
	if (opcode == 13) {	//jal instruction
		rege[15] = pc + 1;
		pc = cyclic_address(rege[rd]);
		int pc_help = pc;
		fseek(mem_in, 0, SEEK_SET);
		while (pc_help > 0) {
			fgets(line2, MAX_LINE, mem_in);
			pc_help -= 1;
		}
	}
	if (opcode == 14) {	//lw instruction
		int lines = rege[rs] + rege[rt];
		lines = cyclic_address(lines);
		strcpy(line2, output[lines]);
		int MEM = (int)strtol(line2, NULL, 16);
		if (rd != 1)
			rege[rd] = MEM;
		if (rd == 0)
			rege[rd] = 0;
		pc += 1;
	}
	if (opcode == 15) { //sw instruction
		char hexval[9];
		int lines = rege[rs] + rege[rt];
		lines = cyclic_address(lines);
		sprintf(hexval, "%08X", rege[rd]);
		output[lines][0] = hexval[0];
		output[lines][1] = hexval[1];
		output[lines][2] = hexval[2];
		output[lines][3] = hexval[3];
		output[lines][4] = hexval[4];
		output[lines][5] = hexval[5];
		output[lines][6] = hexval[6];
		output[lines][7] = hexval[7];
		output[lines][8] = hexval[8];
		pc += 1;
	}
	if (opcode == 16) { //reti instruction
		pc = io_rege[7];
		reti_flag[0] = 1;
	}
	if (opcode == 17) { //in instruction
		int lines = rege[rs] + rege[rt];
		if (rd != 1)
			rege[rd] = io_rege[lines];
		if (rd == 0)
			rege[rd] = 0;
		Print_To_hwregtrace(hwregtrace, count, io_rege,1,lines); //prints to hwregtrace, with a read flag
		pc += 1;
	}
	if (opcode == 18) { //out instruction
		int lines = rege[rs] + rege[rt], temp_lines, sector_address, i=0;
		char hexval[9];
		io_rege[lines] = rege[rd];
		
		if (lines==15) //raises flags for the condition of the disk commands - both disksector and diskbuffer are need to be intialized before the command
			{
				disk_f[0] = 1;
			}
		if (lines==16) // raises flags for the condition of the disk commands - both disksector and diskbuffer are need to be intialized before the command
			{
				disk_f[1] = 1;
			}
		if (lines ==14)
			if (disk_f[0] && disk_f[1]) // if the discsector and diskbuffer have been intialized, proceed
			{
				sector_address = 128 * io_rege[15]; //calcutaes the sector adrees
				if (io_rege[lines] == 1) //reading a sector
				{
					for (i = 0; i < 128; i++) { // copies the words of the sector to the memory, each sector contains 128 words
						int buffer_cur_addr = io_rege[16] + i;
						int sector_cur_addr = sector_address + i;
						strcpy(output[buffer_cur_addr],output_d[sector_cur_addr]);
					}
					finish_t[0] = count + 1024; //sets finish time for the disk operation
				}
				if (io_rege[lines] == 2) //writing a sector
				{
					for (i = 0; i < 128; i++) { // copies the words of the memory to the sector, each sector contains 128 words
						int buffer_cur_addr = io_rege[16] + i;
						int sector_cur_addr = sector_address + i;
						strcpy(output_d[sector_cur_addr], output[buffer_cur_addr]);
						finish_t[0] = count + 1024; //sets finish time for the disk operation
					}
				}

				io_rege[17] = 1; //turns on the disk status
				disk_f[0] = 0; //turns off the flags of disksector and diskbuffer
				disk_f[1] = 0; //turns off the flags of disksector and diskbuffer
			}
		if (lines == 9) // updates led.txt - only when writing (turning on or off)
		{
			sprintf(decval, "%i", count);
			fputs(decval, leds);
			putc(' ', leds);
			sprintf(hexval, "%08X", io_rege[lines]);
			fputs(hexval, leds);
			putc(' ', leds);
			putc('\n', leds);
		}
		if (lines == 10) // updates display.txt - only when writing
		{
			sprintf(decval, "%i", count);
			fputs(decval, display);
			putc(' ', display);
			sprintf(hexval, "%08X", io_rege[lines]);
			fputs(hexval, display);
			putc(' ', display);
			putc('\n', display);
		}
		else
				io_rege[lines] = rege[rd];
		Print_To_hwregtrace(hwregtrace, count, io_rege, 0, lines); //prints to hwregtrace, with a write flag
		pc += 1;
	}
	if (opcode < 0 || opcode >19) //handling wrong opcodes, not in the range of 0-19 -> skipping to next pc
		pc += 1;
	return pc;
}
	


	timer(int *io_rege) { //handles timer register, invoked once each cycle
		if (io_rege[12] == io_rege[13]) {
			io_rege[12] = 0;
			io_rege[3];
		}
		else if (io_rege[11])
			io_rege[12] += 1;
	}

	int signExtension(int instr) { //sign extention script (for imm)
		int value = (0x0000FFF & instr);
		int mask = 0x0000800;
		if (mask & instr) {
			value += 0xFFFFF000;
		}
		return value;
	}


	int StrTol(char c) { //string to int script
		if (c >= '0'&&c <= '9')
			return (int)c - 48;
		if (c == 'a' || c == 'A')
			return 10;
		if (c == 'b' || c == 'B')
			return 11;
		if (c == 'c' || c == 'C')
			return 12;
		if (c == 'd' || c == 'D')
			return 13;
		if (c == 'e' || c == 'E')
			return 14;
		if (c == 'f' || c == 'F')
			return 15;
	}

	

	Print_To_Trace(FILE *trace, int pc, char *line, int Reg_Array[]) { //prints to trace.txt
		char hexval[9], instruction[8], *inst = instruction;
		sprintf(hexval, "%08X", pc);
		fputs(hexval, trace);
		putc(' ', trace);
		putc(' ', trace);
		inst = strtok(line, "\n");
		fputs(inst, trace);
		putc(' ', trace);
		for (int i = 0; i <= 15; i++) {
			sprintf(hexval, "%08X", Reg_Array[i]);
			fputs(hexval, trace);
			putc(' ', trace);
		}
		putc('\n', trace);
	}
	
	Print_To_Files(FILE *mem_out, FILE *disk_out, FILE *regout, FILE *trace, FILE *cycles, char output[][9], char output_d[][9], int count, int Reg_Array[]) { //prints to mem_out, diskout, cycles and regout. invoked once at the end of the simulator run.
		int i = MAX_FILE - 2, j = 0, i_disk =MAX_FILE_D -2, j_disk =0;
		char regoutchar[9];
		while (strcmp(output[i], "00000000") == 0)
			i -= 1;
		while (j <= i) {
			fputs(output[j], mem_out);
			putc('\n', mem_out);
			j += 1;
		}
		
		while (strcmp(output_d[i_disk], "00000000") == 0)
			i_disk -= 1;
		while (j_disk <= i_disk) {
			fputs(output_d[j_disk], disk_out);
			putc('\n', disk_out);
			j_disk += 1;
		}

		fprintf(cycles, "%d", count);

		for (i = 2; i <= 15; i++) {
			sprintf(regoutchar, "%08X", Reg_Array[i]);
			fputs(regoutchar, regout);
			putc('\n', regout);
		}
	}

	int main(int argc, char *argv[]) {
		int Reg_Array[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, IO_Reg_Array[17] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 , 0 }, reti_flag_val[1] = { 0 }, disk_flags[3] = { 0,0,0 }, i = 0, i_irq2 = 0, irq2_next_int = 0, irq2_len, finish_time[1] = { -1 };
		int *rege = Reg_Array, *io_rege = IO_Reg_Array, *disk_f=disk_flags, *reti_flag = reti_flag_val, *finish_t=finish_time, rd, rs, rt, imm, pc_help, irq, irq_busy = 0, pc = 0, count = 0;
		FILE *mem_in, *disk_in, *irq2_in, *mem_out, *cycles, *regout, *trace, *hwregtrace, *leds, *display, *diskout;
		char Lines[MAX_LINE], *line=Lines, Lines_d[MAX_LINE], *line_d = Lines_d, opcode, output[MAX_FILE][9], (*out)[9] = output, output_d[MAX_FILE_D][9], (*out_d)[9] = output_d, irq2_next[MAX_LINE],*irq2_n= irq2_next;
		mem_in = fopen(argv[1], "r"); //input mem_in file opening
		disk_in = fopen(argv[2], "r"); //input diskin file opening
		irq2_in = fopen(argv[3], "r"); //input irq2in file opening
		trace = fopen(argv[6], "w"); //trace file opening
		hwregtrace = fopen(argv[7], "w"); //hwregtrace file opening
		leds = fopen(argv[9], "w"); //leds file opening
		display = fopen(argv[10], "w"); //display file opening
		if (mem_in == NULL)
			exit(1);
		while (!feof(mem_in)) { //place input mem_in file lines into array
			if (fgets(line, MAX_LINE, mem_in)) {
				strcpy(output[i], line);
				output[i][8] = '\0';
				i += 1;
			}
		}
		i += 1;
		while (i < MAX_FILE) { // continuing placing lines until limit
			strcpy(output[i - 1], "00000000\0");
			i += 1;
		}
		i = 0;
		if (disk_in == NULL)
			exit(1);
		while (!feof(disk_in)) { //place input disk_in file lines into array
			if (fgets(line_d, MAX_LINE, disk_in)) {
				strcpy(output_d[i], line_d);
				output_d[i][8] = '\0';
				i += 1;
			}
		}
		i += 1;
		while (i < MAX_FILE_D) { // continuing placing lines until limit
			strcpy(output_d[i - 1], "00000000\0");
			i += 1;
		}

		fgets(irq2_n, MAX_LINE, irq2_in); //gets first irq2 time
		irq2_len = strlen(irq2_n)-1;
		for (i_irq2=0; i_irq2 < irq2_len; i_irq2++) {
			irq2_next_int += pow(10,(irq2_len-1-i_irq2))*(StrTol(irq2_n[i_irq2]));
		}
		fseek(mem_in, 0, SEEK_SET);
		while (!feof(mem_in)) { // start reading input file line by line and process it 
			if (count == irq2_next_int+1) { //checks if irq2 time has been reached and updates the nest irq2 time if needed
				io_rege[5] = 1;
				fgets(irq2_n, MAX_LINE, irq2_in);
				irq2_len = strlen(irq2_n) - 1;
				irq2_next_int = 0;
				for (i_irq2 = 0; i_irq2 < irq2_len; i_irq2++) {
					irq2_next_int += pow(10, (irq2_len - 1 - i_irq2))*(StrTol(irq2_n[i_irq2]));
				}
			}
			if (reti_flag[0]) { // checks if the irq was handled, and sets the busy signal to 0
				irq_busy = 0;
				reti_flag[0] = 0;
			}
			irq = (io_rege[0] && io_rege[3]) || (io_rege[1] && io_rege[4]) || (io_rege[2] && io_rege[5]); //checks if any irq have been recived
			if (irq && !irq_busy) { //irq handling
				io_rege[7] = pc;
				pc = io_rege[6];
				irq_busy = 1;
				fseek(mem_in, 0, SEEK_SET); //next five lines made for reading the correct line for next instruction
				pc_help = pc;
				while (pc_help > 0) {
					fgets(line, MAX_LINE, mem_in);
					pc_help -= 1;
				}
			}
			timer(io_rege); //runs timer handling script
			fgets(line, MAX_LINE, mem_in); //decoding of the operarion 
			opcode = StrTol(line[0])*16 + StrTol(line[1]);
			rd = StrTol(line[2]);
			rs = StrTol(line[3]);
			rt = StrTol(line[4]);
			imm = StrTol(line[5])*256 + StrTol(line[6])*16 + StrTol(line[7]);
			rege[1] = signExtension(imm);
			if (opcode != 19) {
				Print_To_Trace(trace, pc, line, Reg_Array);//print to trace

				pc = Opcode_Operation(opcode, rd, rs, rt, rege,io_rege,disk_f, reti_flag, pc, mem_in, out, out_d, count, hwregtrace, leds, display, finish_t); //instruction execution function
				pc = cyclic_address(pc); //updates pc based on cyclic adressing
				fseek(mem_in, 0, SEEK_SET); //next five lines made for reading the correct line for next instruction
				pc_help = pc;
				while (pc_help > 0) {
					fgets(line, MAX_LINE, mem_in);
					pc_help -= 1;
				}
				count += 1; //instructions count to be printed into "count.txt"
			}
			else {
				Print_To_Trace(trace, pc, line, Reg_Array);
				count += 1;
				break;
			}
		}
		//opening output files and printing to them
		mem_out = fopen(argv[4], "w");
		regout = fopen(argv[5], "w");
		cycles = fopen(argv[8], "w");
		diskout = fopen(argv[11], "w");
		Print_To_Files(mem_out,diskout, regout, trace, cycles, output, output_d, count, Reg_Array); //print to files cycles, regout, diskout, memout
		fclose(mem_in);
		fclose(leds);
		fclose(display);
		fclose(diskout);
		fclose(mem_out);
		fclose(regout);
		fclose(trace);
		fclose(cycles);
	}
