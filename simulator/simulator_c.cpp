#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <tuple>
#include <algorithm>
using namespace std;

#include <getopt.h>
#include "isa_class.h"
#include "isa.h"
#include "fpu.h"
#include "ram.h"

string int2hexstring(unsigned int i){
	stringstream stream;
	stream << hex << i;
	return stream.str();
}

int main(int argc, char* argv[]){

	//Option Handler
	char result;
	long long int breakpoint = -1;
	long long int limit = -1;
	char *filename = nullptr, *io_outputfilename = nullptr, *io_inputfilename = nullptr, *ramfilename = nullptr, *outputramfilename = nullptr;
	bool branchprofile_flag = false;
	while((result=getopt(argc,argv,"f:i:o:r:d:l:b:p"))!=-1){
		switch(result){
			case 'f': // Input Binary File
				filename = optarg;
				break;
			case 'o': // IO-Output
				io_outputfilename = optarg;
				break;
			case 'i': // IO-Input
				io_inputfilename = optarg;
				break;
			case 'r': // RAM input
				ramfilename = optarg;
				break;
			case 'd': // RAM dump
				outputramfilename = optarg;
				break;
			case 'l': // Instruction Limit
				limit = stoul(optarg, nullptr, 0);
				break;
			case 'b': // Break Point
				breakpoint = stoul(optarg, nullptr, 0);
				break;
			case 'p': // Make Profile
				branchprofile_flag = true;
				break;
			case ':':
				cerr << result << " needs value" << endl;
				return 1;
			case '?':
				cerr << "unknown option" << endl;
				return 1;
		}
	}

	//Read Binary File
	ifstream fin;
	if(filename != nullptr){
		fin.open(filename, ios::in|ios::binary);
		if(fin.fail()){
			cerr << "Can't open file" << endl;
			return 1;
		}
	}else{
		cerr << "No input file." << endl;
	}
	vector<uint32_t> instructions;
	uint32_t tmp;
	while(fin.read(reinterpret_cast<char*>(&tmp),sizeof(tmp))){
		instructions.push_back(tmp);
	}

	//Initilaize Profiler
	vector<int> branchprofile(instructions.size());
	vector<int> branchprofile2(instructions.size());
	unsigned int faddfmul=0;
	unsigned int fmulfadd=0;
	unsigned int addadd=0;
	unsigned int addsub=0;
	unsigned int subadd=0;
	unsigned int subsub=0;
	unsigned int mvcounter = 0;
	unsigned int icounter = 0;
	unsigned int ocounter = 0;
	
	//Initilaize RAM
	RAM ram(ramfilename);

	//Initilaize IO
	istream *input;
	ifstream fileinput;
	if(io_inputfilename != nullptr){
		fileinput.open(io_inputfilename, ios::binary);
		if(fileinput.fail()){
			cerr << "Can't open file : " << io_inputfilename << endl;
			return 1;
		}
		input = &fileinput;
	}else{
		cerr << "IO input from stdin." << endl;
		input = &cin;
	}

	ostream *output;
	ofstream fileoutput;
	if(io_outputfilename != nullptr){
		fileoutput.open(io_outputfilename, ios::binary);
		if(fileoutput.fail()){
			cerr << "Can't open file : " << io_outputfilename << endl;
			return 1;
		}
		output = &fileoutput;
	}else{
		cerr << "IO output to stdout." << endl;
		output = &cout;
	}

	//Initilaize REG and PC
	vector<REG> greg(GREGNAMES.size());
	vector<int> gregage(GREGNAMES.size());
	vector<REG> freg(FREGNAMES.size());
	vector<int> fregage(GREGNAMES.size());
	unsigned int pc=0;


	//Main
	long long int counter = 0;
	uint32_t prev = 0;
	long long int clock = 0; 
	vector<unsigned long> profile(INAMES.size(),0);
	
	int breakpointcounter=0;
	while(pc < instructions.size()){
		if(pc == breakpoint){
			//Print Reg
			breakpointcounter++;
			cerr << "== counter : " << counter << endl;
			cerr << "== num : " << breakpointcounter << endl;
			for(unsigned int i=0;i<greg.size();i++){
				cerr << ISA::greg2name(i) << ":" << hex << "0x" << greg[i].r << dec << "(" << greg[i].d  << ")" << " ";
			}
			cerr << endl;
			for(unsigned int i=0;i<freg.size();i++){
				cerr << ISA::freg2name(i) << ":" << hex << "0x" << freg[i].r << dec << "(" << freg[i].f << ")" <<  " ";
			}
			cerr << endl;
		}
		if(counter == limit){
			cerr << "Stop by Instruction Limit." << endl;
			goto END_MAIN;
		}

		int latency = 1;
		int stall = 0;
		counter++;

		FORMAT fm;
		fm.data = instructions[pc];
		profile[fm.J.op]++;
		switch(fm.J.op){
			// no regs and imm
			case RET:
				pc = greg[31].u;
				latency += gregage[31];
				stall = 3;
				break;

				// 1 imm
			case J:
				if(fm.J.cx==0){
					cerr << "Detect Halt." << endl;
					goto END_MAIN;
				}
				pc += fm.J.cx;
				stall = 1;
				break;

			case JEQ:
				if(greg[30].d == 0){
					branchprofile[pc]+=1;
					pc += fm.J.cx;
					if (fm.J.br==1) stall = 1;
					else stall = 3;
				}else{
					branchprofile[pc]-=1;
					pc += 1;
					if (fm.J.br==0) stall = 0;
					else stall = 3;
				}
				branchprofile2[pc]+=1;
				latency += gregage[30];
				break;

			case JLE:
				if(greg[30].d <= 0){
					branchprofile[pc]+=1;
					pc += fm.J.cx;
					if (fm.J.br==1) stall = 1;
					else stall = 3;
				}else{
					branchprofile[pc]-=1;
					pc += 1;
					if (fm.J.br==0) stall = 0;
					else stall = 3;
				}
				branchprofile2[pc]+=1;
				latency += gregage[30];
				break;

			case JLT:
				if(greg[30].d < 0){
					branchprofile[pc]+=1;
					pc += fm.J.cx;
					if (fm.J.br==1) stall = 1;
					else stall = 3;
				}else{
					branchprofile[pc]-=1;
					pc += 1;
					if (fm.J.br==0) stall = 0;
					else stall = 3;
				}
				branchprofile2[pc]+=1;
				latency += gregage[30];
				break;

			case JSUB:
				greg[31].u = pc + 1;
				pc += fm.J.cx;
				latency += gregage[31];
				stall = 1;
				break;

				// 1 greg, 1 imm
			case LDIH:
				greg[fm.L.ra].u = (fm.L.cx << 16) + (greg[fm.L.ra].u & 0xffff);
				pc+=1;
				latency += gregage[fm.L.ra];
				break;

				// 1 freg, 1 greg
			case ITOF:
				freg[fm.L.ra].r = FPU::_itof(greg[fm.L.rb].r);
				pc+=1;
				latency += gregage[fm.L.rb];
				fregage[fm.L.ra] = latency+3;
				break;

				// 1 greg, 1 freg
			case FTOI:
				greg[fm.L.ra].r = FPU::_ftoi(freg[fm.L.rb].r);
				pc+=1;
				latency += fregage[fm.L.rb];
				gregage[fm.L.ra] = latency+3;
				break;

				// 2 freg
			case FINV:
				freg[fm.A.ra].r = FPU::inv(freg[fm.A.rb].r);
				pc+=1;
				latency += fregage[fm.A.rb];
				fregage[fm.L.ra] = latency+3;
				break;

			case FSQRT:
				freg[fm.A.ra].r = FPU::sqrt(freg[fm.A.rb].r);
				pc+=1;
				latency += fregage[fm.A.rb];
				fregage[fm.L.ra] = latency+3;
				break;

			case FABS:
				freg[fm.A.ra].r = FPU::abs(freg[fm.A.rb].r);
				pc+=1;
				latency += fregage[fm.A.rb];
				break;

			case FCMP:
				greg[30].d = FPU::cmp(freg[fm.A.rb].r, freg[fm.A.rc].r);
				pc+=1;
				latency += max(fregage[fm.A.rb], fregage[fm.A.rc]);
				fregage[fm.L.ra] = latency+1;
				break;
			
			case FLR:
				freg[fm.A.ra].r = FPU::floor(freg[fm.A.rb].r);
				pc+=1;
				latency += fregage[fm.A.rb];
				break;

				// 2 gregs, 1 imm
			case LD:
				{
					unsigned int address = (greg[fm.L.rb].d + fm.L.cx) & IOADDR;
					if (address >= RAMSIZE)
					{
						cerr << "Invalid Address : 0x" << hex << address << endl;
						goto END_MAIN;
					}
					if (address == IOADDR)
					{
						icounter++;
						input->read((char*)&(greg[fm.L.ra].r), sizeof(char));
						if (input->eof())
						{
							cerr << "No input any longer" << endl;
							goto END_MAIN;
						}
						stall = 1000;
					}
					else {

						int t = ram.read(address, greg[fm.L.ra].r);
						stall = t;
					}
				}
				pc+=1;
				latency += gregage[fm.A.rb];
				gregage[fm.L.ra] = latency+1;
				break;

			case ST:
				{
					unsigned int address = (greg[fm.L.rb].d + fm.L.cx) & IOADDR;
					if (address>=RAMSIZE)
					{
						cerr << "Invalid Address : 0x" << hex << address << endl;
						goto END_MAIN;
					}
					if (address == IOADDR){
						ocounter++;
						output->write((char*)&(greg[fm.L.ra].r), sizeof(char));
						output->flush();
					}else{
						int t = ram.write(address, greg[fm.L.ra].r);
						stall = t;
					}
				}
				pc+=1;
				latency += max(gregage[fm.A.ra], gregage[fm.A.rb]);
				break;

			case ADDI:
				greg[fm.L.ra].u = greg[fm.L.rb].u + fm.L.cx;
				pc+=1;
				if(fm.L.cx == 0) mvcounter ++;
				latency += gregage[fm.L.rb];
				break;

			case SHLI:
				greg[fm.L.ra].u = greg[fm.L.rb].u << fm.L.cx;
				pc+=1;
				latency += gregage[fm.L.rb];
				break;

			case SHRI:
				greg[fm.L.ra].u = greg[fm.L.rb].u >> fm.L.cx;
				pc+=1;
				latency += gregage[fm.L.rb];
				break;

				// 1 freg, 1greg, 1 imm
			case FLD:
				{
					unsigned int address = (greg[fm.L.rb].d + fm.L.cx) & IOADDR;
					if (address>=RAMSIZE)
					{
						cerr << "Invalid Address : 0x" << hex << address << endl;
						goto END_MAIN;
					}
					if (address == IOADDR)
					{
						icounter++;
						input->read((char*)&(freg[fm.L.ra].r), sizeof(uint32_t));
						if (input->eof())
						{
							cerr << "No input any longer" << endl;
							goto END_MAIN;
						}
						stall = 10;
					}
					else{
						int t = ram.read(address, freg[fm.L.ra].r);
						stall = t;
					}
				}
				pc+=1;
				latency += gregage[fm.L.rb];
				fregage[fm.L.ra] = latency+1;
				break;

			case FST:
				{
					unsigned int address = (greg[fm.L.rb].d + fm.L.cx) & IOADDR;
					if (address>=RAMSIZE)
					{
						cerr << "Invalid Address : 0x" << hex << address << endl;
						goto END_MAIN;
					}
					if (address == IOADDR){
						ocounter++;
						output->write((char*)&(freg[fm.L.ra].r), sizeof(uint32_t));
					}else{
						int t = ram.write(address, freg[fm.L.ra].r);
						stall = t;
					}
				}
				pc+=1;
				latency += max(fregage[fm.L.ra], gregage[fm.L.rb]);
				break;

				// 2 freg, 1 imm
			case FLDI:
				freg[fm.L.ra].u = (fm.L.cx << 16) + (freg[fm.L.rb].u >> 16);
				pc+=1;
				latency += fregage[fm.L.rb];
				break;

				// 3 gregs
			case ADD:
				greg[fm.A.ra].u = greg[fm.A.rb].u + greg[fm.A.rc].u;
				pc+=1;
				latency += max(gregage[fm.A.rb], gregage[fm.A.rb]);
				break;

			case SUB:
				greg[fm.A.ra].u = greg[fm.A.rb].u - greg[fm.A.rc].u;
				pc+=1;
				latency += max(gregage[fm.A.rb], gregage[fm.A.rb]);
				break;

			case SHL:
				greg[fm.A.ra].u = greg[fm.A.rb].u << greg[fm.A.rc].u;
				pc+=1;
				latency += max(gregage[fm.A.rb], gregage[fm.A.rb]);
				break;

			case SHR:
				greg[fm.A.ra].u = greg[fm.A.rb].u >> greg[fm.A.rc].u;
				pc+=1;
				latency += max(gregage[fm.A.rb], gregage[fm.A.rb]);
				break;

				// 3 fregs
			case FADD:
				freg[fm.A.ra].r = FPU::add(freg[fm.A.rb].r, freg[fm.A.rc].r);
				pc+=1;
				latency += max(fregage[fm.A.rb], fregage[fm.A.rb]);
				fregage[fm.L.ra] = latency+3;
				break;

			case FSUB:
				freg[fm.A.ra].r = FPU::sub(freg[fm.A.rb].r, freg[fm.A.rc].r);
				pc+=1;
				latency += max(fregage[fm.A.rb], fregage[fm.A.rb]);
				fregage[fm.L.ra] = latency+3;
				break;

			case FMUL:
				freg[fm.A.ra].r = FPU::mul(freg[fm.A.rb].r, freg[fm.A.rc].r);
				pc+=1;
				latency += max(fregage[fm.A.rb], fregage[fm.A.rb]);
				fregage[fm.L.ra] = latency+3;
				break;

			default:
				cerr << "Error!" << endl;
				exit(1);
		}

		greg[0].r = 0;
		freg[0].f = 0;
		
		for(auto &el : gregage){
			el -= latency;
			if (el < 0 ) el = 0;
		}
		
		for(auto &el : fregage){
			el -= latency;
			if (el < 0 ) el = 0;
		}
		//cerr << "pc: " << pc << " latency: " << latency << " stall " << stall << endl;
		clock += latency;
		clock += stall;
		ram.update(latency+stall);
		// M
		FORMAT fm2;
		fm2.data = prev;
		prev = fm.data;
		if(fm.J.op==FADD && fm2.J.op==FMUL && (fm2.A.ra==fm.A.rb || fm2.A.ra==fm.A.rc)) faddfmul++;
		if(fm.J.op==FMUL && fm2.J.op==FADD && (fm2.A.ra==fm.A.rb || fm2.A.ra==fm.A.rc)) fmulfadd++;
		if(fm.J.op==ADD && fm2.J.op==ADD && (fm2.A.ra==fm.A.rb || fm2.A.ra==fm.A.rc)) addadd++;
		if(fm.J.op==ADD && fm2.J.op==SUB && (fm2.A.ra==fm.A.rb || fm2.A.ra==fm.A.rc)) addsub++;
		if(fm.J.op==SUB && fm2.J.op==ADD && (fm2.A.ra==fm.A.rb || fm2.A.ra==fm.A.rc)) subadd++;
		if(fm.J.op==SUB && fm2.J.op==SUB && (fm2.A.ra==fm.A.rb || fm2.A.ra==fm.A.rc)) subsub++;

	}

END_MAIN:


	//Dump RAM
	ofstream fout;
	if(outputramfilename != nullptr){
		fout.open(outputramfilename, ios::binary);
		if(fout.fail()){
			cerr << "Can't open file : " << outputramfilename << endl;
			return 1;
		}
		for(auto x : ram){
			fout.write((char *)&x,sizeof(x));
		}
	}

	ofstream fout2;
	if(branchprofile_flag){
		fout2.open("branch.profile", ios::binary);
		if(fout2.fail()){
			cerr << "Can't open file : " << "branch.profile" << endl;
			return 1;
		}
		for(unsigned int i=0; i < branchprofile.size(); i++){
			float x = 1.0*branchprofile[i]/branchprofile2[i];
			fout2.write((char *)&x,sizeof(x));
		}
	}

	cerr << "Program Counter = " << pc << endl;
	cerr << "Instructions = " << counter << endl;

	//Print Reg
	for(unsigned int i=0;i<greg.size();i++){
		cerr << ISA::greg2name(i) << ":" << hex << "0x" << greg[i].r << dec << "(" << greg[i].d  << ")" << " ";
	}
	cerr << endl;
	for(unsigned int i=0;i<freg.size();i++){
		cerr << ISA::freg2name(i) << ":" << hex << "0x" << freg[i].r << dec << "(" << freg[i].f << ")" <<  " ";
	}
	cerr << endl;
	for(unsigned int i=0;i<profile.size();i++){
		cerr << ISA::isa2name(i) << ":" << profile[i] << ",";
	}
	cerr << endl;
	
	ram.printramstatus();

	//Print 
	cerr << "fmulfadd:" << fmulfadd << endl; 
	cerr << "faddfmul:" << faddfmul << endl; 
	cerr << "addadd:" << addadd << endl; 
	cerr << "addsub:" << addsub << endl; 
	cerr << "subadd:" << subadd << endl; 
	cerr << "subsub:" << subsub << endl; 
	cerr << "move:" << mvcounter << endl;
	cerr << "io_in:" << icounter << endl;
	cerr << "io_out:" << ocounter << endl;
	cerr << "clock:" << clock + 7 + max(*max_element(gregage.begin(), gregage.end()), *max_element(fregage.begin(), fregage.end()) )<< endl;
	return 0;
}