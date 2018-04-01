#pragma warning(disable:4996)

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <math.h>

struct instruction_data {
	unsigned char instruction;
	unsigned char next_byte;
	unsigned char n4, n3, n2, n1;
};

struct ram_bank {
	unsigned char register_status[32];
	unsigned char memory[128];
	unsigned char *r[16];
	unsigned char chip_output_port[4];
};

int delay = 0;

//Arithmetics
unsigned char addition(unsigned char variable, unsigned char value, char affect_carry);
unsigned char subtraction(unsigned char variable, unsigned char value);

//load ROM
unsigned char *load_rom(const char *filename);

//imprimir instrução
void print_instruction(const char *opcode, struct instruction_data data);

//iniciar instruções
void init();

//imprimir valores
void print_registers();

//formatar instrução
struct instruction_data instruction_format(unsigned char instruction, unsigned char next_byte);

//executar instrução
void execute(unsigned char instruction, unsigned char next_byte);

//funções de instruções
void nop(struct instruction_data data);
void jcn(struct instruction_data data);
void fim(struct instruction_data data);
void fin(struct instruction_data data);
void jin(struct instruction_data data);
void jun(struct instruction_data data);
void jms(struct instruction_data data);
void inc(struct instruction_data data);
void isz(struct instruction_data data);
void add(struct instruction_data data);
void sub(struct instruction_data data);
void ld(struct instruction_data data);
void xch(struct instruction_data data);
void bbl(struct instruction_data data);
void ldm(struct instruction_data data);
void clb(struct instruction_data data);
void clc(struct instruction_data data);
void iac(struct instruction_data data);
void cmc(struct instruction_data data);
void cma(struct instruction_data data);
void ral(struct instruction_data data);
void rar(struct instruction_data data);
void tcc(struct instruction_data data);
void dac(struct instruction_data data);
void tcs(struct instruction_data data);
void stc(struct instruction_data data);
void daa(struct instruction_data data);
void kbp(struct instruction_data data);
void dcl(struct instruction_data data);
void src(struct instruction_data data);
void wrm(struct instruction_data data);
void wmp(struct instruction_data data);
void wrr(struct instruction_data data);
void wr0(struct instruction_data data);
void wr1(struct instruction_data data);
void wr2(struct instruction_data data);
void wr3(struct instruction_data data);
void sbm(struct instruction_data data);
void rdm(struct instruction_data data);
void rdr(struct instruction_data data);
void adm(struct instruction_data data);
void rd0(struct instruction_data data);
void rd1(struct instruction_data data);
void rd2(struct instruction_data data);
void rd3(struct instruction_data data);

//gerência de registradores
char register_fetch(int index);
char register_insert(int index, unsigned char value);

//gerência de stack
char stack_push(short val);
short stack_pop();

//gerencia de RAM
void ram_print();
void ram_insert(unsigned char index, unsigned char value);
void ram_insert_chip_output_port(unsigned char val);
unsigned char ram_fetch_chip_output_port();
unsigned char ram_fetch(unsigned char index);
void ram_insert_register_status(unsigned char register_index, unsigned char nibble_n, unsigned char status);
unsigned char ram_get_register_status(unsigned char ram_bank, unsigned char index);

//ROM
unsigned char *ROM;
unsigned char rom_output_port = 0;
unsigned char rom_fetch_output();
void rom_insert_output(unsigned char val);

//RAM
ram_bank RAM[4];
unsigned char current_ram_bank = 0;
unsigned char current_ram_chip = 0;
unsigned char current_ram_register = 0;
unsigned char current_ram_nibble = 0;
void ram_print();

//array de instruções
void(*instruction_set[255])(struct instruction_data data);

//registradores
unsigned short stack[3];
unsigned char registers[8];
int ip = 0, sp = -1;
unsigned char accumulator;
int temp_reg;

//flags
unsigned char zf = 0, cf = 0;
unsigned char test_pin = 0;

int cycles = 0;

int main() {

	init();

	ROM = load_rom("ROM.txt");
	int i;

	unsigned char instruction = 0x00, next_byte = 0x00;

	printf("\n-----------------INSTRUCOES-----------------\n");
	while (1) {

		instruction = *(ROM + ip);
		next_byte = *(ROM + ip + 1);

		if (instruction == 0xff)
			break;

		execute(instruction, next_byte);
		Sleep(delay);
	}

	system("cls");
	printf("%d\n", cycles);
	print_registers();
	ram_print();

	system("PAUSE");
	return 0;
}

void print_registers() {
	int i;

	printf("\n-----------------REGISTRADORES-----------------\n");
	for (i = 0; i < 16; i+=2) {
		printf("register %d: %x --- register %d: %x\n", i, register_fetch(i), i+1, register_fetch(i+1));
	}
	printf("accumulator: %x\n", accumulator);
	printf("ip: %x", ip);

	printf("\n-----------------FLAGS-----------------\n");
	printf("zero flag: %x\n", zf);
	printf("carry flag: %x\n", cf);
	printf("test pin: %x", test_pin);

	printf("\n-----------------STACK-----------------\n");
	for (i = 0; i < 3; i++) {
		printf("stack level %d: %x\n", i, stack[i]);
	}
}

unsigned char addition(unsigned char variable, unsigned char value, char affect_carry) {
	unsigned char result = variable + value;

	if (affect_carry) {
		cf = (result & 0x10) >> 4;
	}
	result = result & 0x0f;
	zf = !result && !cf;

	return result;
}

unsigned char subtraction(unsigned char variable, unsigned char value) {
	cf = (variable > value);
	value = ~value;
	value++;
	variable += value;
	zf = !variable;

	return variable & 0x0f;
}

unsigned char *load_rom(const char *filename) {
	FILE *f = fopen(filename, "r");
	unsigned char *ROM = (unsigned char*)malloc(2048);


	int i = 0;
	char ch;
	char c0, c1;
	char *hex_byte = (char *)malloc(3);
	unsigned char hex_val;

	while ((ch = fgetc(f)) != EOF) {
		if (ch == ' ' || ch == '\n') {
			continue;
		}
		c0 = (char)ch;
		c1 = (char)fgetc(f);
		*(hex_byte + 0) = c0;
		*(hex_byte + 1) = c1;
		*(hex_byte + 2) = '\0';
		hex_val = (unsigned char)strtol(hex_byte, NULL, 16);
		*(ROM + i) = hex_val;
		i++;
	}

	return ROM;
}

void print_instruction(const char *opcode, struct instruction_data data) {

	system("cls");
	printf("cycles: %d\n", cycles);
	if (data.n4 == 0x01 || data.n4 == 0x02 || data.n4 == 0x04 || data.n4 == 0x05 || data.n4 == 0x07) {
		if (data.n4 == 0x02 && (data.n3 % 2) == 1) {
			printf("%s: %x%x\n", opcode, data.n4, data.n3);
			cycles++;
		}
		else {
			printf("%s: %x%x %x%x\n", opcode, data.n4, data.n3, data.n2, data.n1);
			cycles+=2;
		}
	}
	else {
		printf("%s: %x%x\n", opcode, data.n4, data.n3);
		cycles++;
	}
	print_registers();
	printf("ROM output port: %x\n", rom_fetch_output());
	ram_print();
	//getchar();
}

void execute(unsigned char instruction, unsigned char next_byte) {

	struct instruction_data data = instruction_format(instruction, next_byte);
	unsigned char high = (instruction & 0xF0) >> 4;
	unsigned char low = instruction & 0x0F;

	if (high != 0xf && high != 0xe) {
		instruction_set[high](data);
	}
	else {
		if (!instruction_set[instruction]) {
			print_instruction("??", data);
			ip++;
		}
		else {
			instruction_set[instruction](data);
		}
	}
}

void ram_insert(unsigned char index, unsigned char value) {
	
	if ((index % 2) == 0) {
		index = (index / 2);
		value = value << 4;
		RAM[current_ram_bank].memory[index] = RAM[current_ram_bank].memory[index] & 0x0f;
		RAM[current_ram_bank].memory[index] += value;
	}
	else {
		index = floor(((float)index / 2));
		RAM[current_ram_bank].memory[index] = RAM[current_ram_bank].memory[index] & 0xf0;
		RAM[current_ram_bank].memory[index] += value;
	}
}

unsigned char ram_fetch_chip_output_port() {
	return RAM[current_ram_bank].chip_output_port[current_ram_chip];
}

void ram_insert_chip_output_port(unsigned char val) {
	RAM[current_ram_bank].chip_output_port[current_ram_chip] = val & 0x0f;
}

unsigned char ram_fetch(unsigned char index) {
	
	if ((index % 2) == 0) {
		index = (index / 2);
		return (RAM[current_ram_bank].memory[index] & 0xf0) >> 4;
	}
	else {
		index = floor(((float)index / 2));
		return (RAM[current_ram_bank].memory[index] & 0x0f);
	}
}

unsigned char ram_get_register_status(unsigned char ram_bank, unsigned char register_index, unsigned char nibble_n) {
	unsigned char *status_register = &RAM[ram_bank].register_status[register_index * 2];
	unsigned char val;
	
	if (nibble_n > 1) {
		status_register += 1;
	}

	if (nibble_n % 2 != 0) {
		val = *status_register & 0x0f;
	}
	else {
		val = *status_register & 0xf0;
		val = val >> 4;
	}
	return val;
}

unsigned char rom_fetch_output() {
	return rom_output_port;
}

void rom_insert_output(unsigned char val) {
	rom_output_port = val & 0x0f;
}

void ram_insert_register_status(unsigned char register_index, unsigned char nibble_n, unsigned char status) {
	unsigned char *status_register = &RAM[current_ram_bank].register_status[register_index * 2];
	if (nibble_n > 1) {
		status_register += 1;
	}

	if (nibble_n % 2 != 0) {
		*status_register = *status_register & 0xf0;
	}
	else {
		*status_register = *status_register & 0x0f;
		status = status << 4;
	}
	*status_register += status;
}

void ram_print() {
	int i, j, k, l;

	for (i = 0; i < 1; i++) {
		printf("------------------BANK #%d------------------\n", i);
		for (j = 0; j < 4; j++) {
			printf("\t------CHIP #%d------\n", j);
			for (k = 0; k < 4; k++) {
				printf("\tREGISTER #%d: ", k);
				for (l = 0; l < 8; l++) {
					printf("%02x ", *(RAM[i].r[(j*4)+k]+l) );
				}
				printf("\tSTATUS: %02x%02x\n", (unsigned short)RAM[i].register_status[((j * 4) + k)*2], (unsigned short)RAM[i].register_status[(((j * 4) + k) * 2)+1]);
			}
			printf("\tOUTPUT: %x\n", RAM[i].chip_output_port[j]);
		}
	}
}

char register_fetch(int index) {
	char value;

	if ((index % 2) == 0) {
		index = (index / 2);
		value = registers[index] >> 4;
	}
	else {
		index = floor(((float)index / 2));
		value = registers[index] & 0x0f;
	}

	return value;
};

char register_insert(int index, unsigned char value) {

	if (value > 15) {
		return 0;
	}


	if ((index % 2) == 0) {
		index = (index / 2);
		value = value << 4;
		registers[index] = registers[index] & 0x0f;
		registers[index] += value;
	}
	else {
		index = floor(((float)index / 2));
		registers[index] = registers[index] & 0xf0;
		registers[index] += value;
	}

	return 1;
}

char stack_push(short val) {

	if (val > 4096) {
		return 0;
	}

	sp++;
	stack[sp] = val;
	return 1;
}

short stack_pop() {
	if (sp == -1) {
		return 0;
	}

	short aux = stack[sp];
	stack[sp] = 0;
	sp--;
	return aux;
}

void init() {

	int i, j;
	
	for (i = 0; i < 4; i++) {
		for (j = 0; j < 16; j++) {
			RAM[i].r[j] = RAM[i].memory + (j * 8);
		}
	}

	instruction_set[0x00] = &nop;
	instruction_set[0x01] = &jcn;
	instruction_set[0x02] = &fim;
	instruction_set[0x03] = &fin;
	instruction_set[0x04] = &jun;
	instruction_set[0x05] = &jms;
	instruction_set[0x06] = &inc;
	instruction_set[0x07] = &isz;
	instruction_set[0x08] = &add;
	instruction_set[0x09] = &sub;
	instruction_set[0x0a] = &ld;
	instruction_set[0x0b] = &xch;
	instruction_set[0x0c] = &bbl;
	instruction_set[0x0d] = &ldm;
	instruction_set[0xf0] = &clb;
	instruction_set[0xf1] = &clc;
	instruction_set[0xf2] = &iac;
	instruction_set[0xf3] = &cmc;
	instruction_set[0xf4] = &cma;
	instruction_set[0xf5] = &ral;
	instruction_set[0xf6] = &rar;
	instruction_set[0xf7] = &tcc;
	instruction_set[0xf8] = &dac;
	instruction_set[0xf9] = &tcs;
	instruction_set[0xfa] = &stc;
	instruction_set[0xfb] = &daa;
	instruction_set[0xfc] = &kbp;
	instruction_set[0xfd] = &dcl;
	instruction_set[0xe0] = &wrm;
	instruction_set[0xe1] = &wmp;
	instruction_set[0xe2] = &wrr;
	instruction_set[0xe4] = &wr0;
	instruction_set[0xe5] = &wr1;
	instruction_set[0xe6] = &wr2;
	instruction_set[0xe7] = &wr3;
	instruction_set[0xe8] = &sbm;
	instruction_set[0xe9] = &rdm;
	instruction_set[0xea] = &rdr;
	instruction_set[0xeb] = &adm;
	instruction_set[0xec] = &rd0;
	instruction_set[0xed] = &rd1;
	instruction_set[0xee] = &rd2;
	instruction_set[0xef] = &rd3;
}

struct instruction_data instruction_format(unsigned char instruction, unsigned char next_byte) {

	struct instruction_data data;
	data.n4 = (instruction & 0xF0) >> 4;
	data.n3 = instruction & 0x0F;
	data.n2 = (next_byte & 0xF0) >> 4;
	data.n1 = next_byte & 0x0F;
	data.instruction = instruction;
	data.next_byte = next_byte;

	return data;
}

void nop(struct instruction_data data) {
	print_instruction("nop", data);
	ip += 1;
	if (ip == 0x100) {
		ip = 0; //revisit
	}
}

void jcn(struct instruction_data data) {
	print_instruction("jcn", data);

	char jump = 0;
	unsigned char c1, c2, c3, c4;
	c1 = data.n3 & 0x08;
	c2 = data.n3 & 0x04;
	c3 = data.n3 & 0x02;
	c4 = data.n3 & 0x01;

	if (c2 && c3 && c4) {
		jump = (accumulator == 0) && (cf == 1) && (test_pin == 0);
	}

	else if (c2 && c3 && !c4) {
		jump = (accumulator == 0) && (cf == 1);
	}

	else if (c2 && !c3 && c4) {
		jump = (accumulator == 0) && (test_pin == 0);
	}

	else if (!c2 && c3 && c4) {
		jump = (cf == 1) && (test_pin == 0);
	}

	else if (c2 && !c3 && !c4) {
		jump = (accumulator == 0);
	}

	else if (!c2 && c3 && !c4) {
		jump = (cf == 1);
	}

	else if (!c2 && !c3 && c4) {
		jump = (test_pin == 0);
	}

	if (c1) {
		jump = !jump;
	}

	if (jump) {
		ip = (int)data.next_byte;
	}
	else {
		ip += 2;
	}
}

void fim(struct instruction_data data) {

	if ((data.n3 % 2) == 1) {
		src(data);
		return;
	}

	print_instruction("fim", data);

	registers[data.n3 / 2] = data.next_byte;
	ip += 2;
}

void fin(struct instruction_data data) {

	if ((data.n3 % 2) == 1) {
		jin(data);
		return;
	}

	print_instruction("fin", data);

	unsigned char value = *(ROM + registers[0]);
	registers[data.n3 / 2] = value;

	ip += 1;
}

void jin(struct instruction_data data) {
	print_instruction("jin", data);

	ip += 1;
}

void jun(struct instruction_data data) {
	print_instruction("jun", data);

	//int address = (data.n1 * 1) + (data.n2 * 16) + (data.n3 * 256);
	int address = (data.n1 * 1) + (data.n2 * 16);//revisit
	ip = address;
}

void jms(struct instruction_data data) {
	print_instruction("jms", data);

	//int address = (data.n1 * 1) + (data.n2 * 16) + (data.n3 * 256);
	int address = (data.n1 * 1) + (data.n2 * 16); //revisit
	stack_push((short)ip + 1);

	ip = address;
}

void inc(struct instruction_data data) {
	print_instruction("inc", data);

	unsigned char result = addition(register_fetch(data.n3), 1, 1);
	register_insert(data.n3, result);
	ip += 1;
}

void isz(struct instruction_data data) {
	print_instruction("isz", data);

	unsigned char result = addition(register_fetch(data.n3), 1, 0);
	register_insert(data.n3, result);

	if (result) {
		ip = (int)data.next_byte;
	}
	else {
		ip += 2;
	}
}

void add(struct instruction_data data) {
	print_instruction("add", data);

	accumulator = addition(accumulator, register_fetch(data.n3), 1);
	ip += 1;
}

void sub(struct instruction_data data) {
	print_instruction("sub", data);
	accumulator = subtraction(accumulator, register_fetch(data.n3));
	ip += 1;
}

void ld(struct instruction_data data) {
	print_instruction("ld", data);

	accumulator = register_fetch(data.n3);
	ip += 1;
}

void xch(struct instruction_data data) {
	print_instruction("xch", data);

	int aux = accumulator;
	accumulator = register_fetch(data.n3);
	register_insert(data.n3, (unsigned char)aux);

	ip += 1;
}

void bbl(struct instruction_data data) {
	print_instruction("bbl", data);

	accumulator = data.n3;
	int address = (int)stack_pop();

	if (!address) {
		ip++;
		return;
	}

	ip = address + 1;
}

void ldm(struct instruction_data data) {
	print_instruction("ldm", data);

	accumulator = data.n3;
	ip += 1;
}

void clb(struct instruction_data data) {
	print_instruction("clb", data);

	accumulator = cf = 0;
	ip += 1;
}

void clc(struct instruction_data data) {
	print_instruction("clc", data);
	cf = 0;
	ip += 1;
}

void iac(struct instruction_data data) {
	print_instruction("iac", data);
	accumulator = addition(accumulator, 1, 1);
	ip += 1;
}

void cmc(struct instruction_data data) {
	print_instruction("cmc", data);
	cf = !cf;
	ip += 1;
}

void cma(struct instruction_data data) {
	print_instruction("cma", data);
	accumulator = ~accumulator;
	ip += 1;
}

void ral(struct instruction_data data) {
	print_instruction("ral", data);

	unsigned char prev_carry, prev_least, prev_most;
	prev_carry = (unsigned char)cf;
	prev_least = ((unsigned char)accumulator & 0x01) != 0;
	prev_most = ((unsigned char)accumulator & 0x08) != 0;

	accumulator = (accumulator << 1) & 0x0f;
	cf = prev_most;
	if (prev_carry == 1) {
		accumulator += 0x01;
	}

	ip += 1;
}

void rar(struct instruction_data data) {
	print_instruction("rar", data);

	unsigned char prev_carry, prev_least, prev_most;
	prev_carry = (unsigned char)cf;
	prev_least = ((unsigned char)accumulator & 0x01) != 0;
	prev_most = ((unsigned char)accumulator & 0x08) != 0;

	accumulator = accumulator >> 1;
	cf = prev_least;
	if (prev_carry == 1) {
		accumulator += 0x08;
	}

	ip += 1;
}

void tcc(struct instruction_data data) {
	print_instruction("tcc", data);
	accumulator = cf;
	cf = 0;
	ip += 1;
}

void dac(struct instruction_data data) {
	print_instruction("dac", data);
	accumulator = subtraction(accumulator, 1);
	ip += 1;
}

void tcs(struct instruction_data data) {
	print_instruction("tcs", data);

	if (cf) {
		accumulator = 10;
	}
	else {
		accumulator = 9;
	}
	cf = 0;

	ip += 1;
}

void stc(struct instruction_data data) {
	print_instruction("stc", data);
	cf = 1;
	ip += 1;
}

void daa(struct instruction_data data) {
	print_instruction("daa", data);

	unsigned char aux;

	if (cf == 1 || accumulator > 9) {
		if ((accumulator + 6) > 15) {
			accumulator = addition(accumulator, 6, 1);
		}
		else {
			accumulator = addition(accumulator, 6, 0);
		}
	}

	ip += 1;
}

void kbp(struct instruction_data data) {
	print_instruction("kbp", data);
	
	if (accumulator == 0 || accumulator == 1 || accumulator == 2) {
	}
	else if (accumulator == 4) {
		accumulator = 3;
	}
	else if (accumulator == 8) {
		accumulator = 4;
	}
	else {
		accumulator = 15;
	}
	
	ip += 1;
}

void dcl(struct instruction_data data) {
	print_instruction("*dcl", data);
	ip += 1;
}

void src(struct instruction_data data) {
	print_instruction("src", data);

	current_ram_nibble = registers[(data.n3 - 1) / 2];
	current_ram_chip = floor(current_ram_nibble / 64);
	current_ram_register = floor(current_ram_nibble / 16);

	ip += 1;
}

void wrm(struct instruction_data data) {
	print_instruction("wrm", data);
	ram_insert(current_ram_nibble, accumulator);
	ip += 1;
}

void wmp(struct instruction_data data) {
	print_instruction("wmp", data);
	ram_insert_chip_output_port(accumulator);
	ip += 1;
}

void wrr(struct instruction_data data) {
	print_instruction("wrr", data);
	rom_insert_output(accumulator);
	ip += 1;
}

void wr0(struct instruction_data data) {
	print_instruction("wr0", data);

	ram_insert_register_status(current_ram_register, 0, accumulator);
	ip += 1;
}

void wr1(struct instruction_data data) {
	print_instruction("wr1", data);
	ram_insert_register_status(current_ram_register, 1, accumulator);
	ip += 1;
}

void wr2(struct instruction_data data) {
	print_instruction("wr2", data);
	ram_insert_register_status(current_ram_register, 2, accumulator);
	ip += 1;
}

void wr3(struct instruction_data data) {
	print_instruction("wr3", data);
	ram_insert_register_status(current_ram_register, 3, accumulator);
	ip += 1;
}

void sbm(struct instruction_data data) {
	print_instruction("sbm", data);
	accumulator = subtraction(accumulator, ram_fetch(current_ram_nibble));
	ip += 1;
}

void rdm(struct instruction_data data) {
	print_instruction("rdm", data);
	accumulator = ram_fetch(current_ram_nibble);
	ip += 1;
}

void rdr(struct instruction_data data) {
	print_instruction("rdr", data);
	accumulator = rom_fetch_output();
	ip += 1;
}

void adm(struct instruction_data data) {
	print_instruction("adm", data);
	accumulator = addition(accumulator, ram_fetch(current_ram_nibble), 1);
	ip += 1;
}

void rd0(struct instruction_data data) {
	print_instruction("rd0", data);
	accumulator = ram_get_register_status(current_ram_bank, current_ram_register, 0);
	ip += 1;
}

void rd1(struct instruction_data data) {
	print_instruction("rd1", data);
	accumulator = ram_get_register_status(current_ram_bank, current_ram_register, 1);
	ip += 1;
}

void rd2(struct instruction_data data) {
	print_instruction("rd2", data);
	accumulator = ram_get_register_status(current_ram_bank, current_ram_register, 2);
	ip += 1;
}

void rd3(struct instruction_data data) {
	print_instruction("rd3", data);
	accumulator = ram_get_register_status(current_ram_bank, current_ram_register, 3);
	ip += 1;
}
