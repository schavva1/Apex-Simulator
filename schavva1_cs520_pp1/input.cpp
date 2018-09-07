#include <fstream>
#include <iostream>
#include <string> 
#include <sstream>
#include <cstdlib>
using namespace std;
 
/* CODE MEMORY*/

struct Code_Line { 

       int file_line_number; 
       int address;              /* integer (multiple of 4)*/
       string instruction_string;
                 };

struct Code_Memory { 
        Code_Line code[100];        /* filled in by reading from code file.*/
                   };

struct Data_Memory {
        int  base_address;       /*integer (where this starts)*/
        int  data_mem[100];        /*integers, indexed by offset of word address from base address */
                   };


struct Register{
        int  value;
        bool status= true;           /* VALID or INVALID */
               };

struct Register_File {
        Register reg[16];       /* array of struct Registers*/
                     };

struct Instruction_info {
        int  PC;
        string instruction_string;    /*(YOU WILL FIND IT USEFUL TO ADD A NOP)*/ 
        string source_register1;
        string source_register2;
        string dest_register;
        string literal;
        string opcode;
        int load_target; 
        int store_target;
        int x,y,z,l;
        int buffer;
/*	int source_registers_address;
        int source_registers_value;
 
        int dest_register_address;         
        int dest_register_value;*/
 
        int  target_memory_addr; /*(address, null if absent)*/  
        int  target_memory_data; /*(data, null if absent) */

                        };

struct stage {
        Instruction_info* Input_Instruction; 
        Instruction_info* Output_Instruction; 
        bool Stalled = true;         /*True or False*/
	bool init = false;
}F,D,E,M,W;

struct Flags{  
       bool zero= true;  
       bool carry= true;
       bool Negative= false;
            }; 

struct Stats { 
       int cycle=0;
       bool temp;
             };

void fetch(stage* Fetch, stage* Decode){
	Fetch -> Output_Instruction -> instruction_string = Fetch -> Input_Instruction -> instruction_string;
	//cout<< endl <<"fetch output is" << endl <<Fetch-> Output_Instruction->instruction_string<< endl;
	Fetch -> Output_Instruction -> PC = Fetch -> Input_Instruction -> PC;
	//cout<< "PC value in fetch output stage is:"<< Fetch -> Input_Instruction -> PC;
	Decode -> init = true;
}

void decode(stage* Decode,stage* Fetch, stage* Execute){
	if (Decode -> init)
	{
		Decode -> Input_Instruction->instruction_string = Fetch -> Output_Instruction -> instruction_string;
		Decode -> Input_Instruction -> PC = Fetch -> Output_Instruction -> PC;
		//cout<< endl <<"decode input is" <<endl << Decode -> Input_Instruction->instruction_string << endl;
		string s = Decode -> Input_Instruction->instruction_string;
		//cout<< "string is" << s<< endl;
		string opcode;
		string operands;
		string dest_reg;
		string src_reg1;
		string src_reg2;
		string literal;
		string temp;
		opcode = s.substr(0,s.find(' '));
		//cout<<"opcode is:"<<opcode<< endl;

		if((opcode == "ADD")|| (opcode == "MUL") || (opcode== "SUB") || (opcode == "XOR") || (opcode== "AND") || (opcode == "OR")){
			  operands= s.substr(s.find(' ')+1);
			  //cout<<"operands are:"<<operands<<endl;

			  dest_reg = operands.substr(0,operands.find(','));
			  //cout<<"destination register:"<<dest_reg<<endl;  

			  temp = operands.substr(operands.find(' ')+1);
			  
			  src_reg1 = temp.substr(0,temp.find(','));
			  //cout<< "source reg1:"<< src_reg1<<endl;  
			 
			  temp = temp.substr(temp.find(' ')+1);
			  //cout<< "temp is" << temp<< endl;

			  src_reg2 = temp.substr(0,temp.find(','));
			  //cout<< "source reg2:"<< src_reg2<<endl;
	  	}
		else if ((opcode == "MOVC")){
			 operands= s.substr(s.find(' ')+1);
			 //cout<<"operands are:"<<operands<<endl;

			 dest_reg= operands.substr(0,operands.find(','));
			 //cout<<"destination register:"<< dest_reg <<endl;

			 literal= operands.substr(operands.find(' ')+2);
			 //cout<<"literal:"<< literal << endl; 
	 	 } 
		else if((opcode == "LOAD")){
	  
			  operands= s.substr(s.find(' ') + 1);
			  //cout<<"operands are:"<<operands<<endl;

			  dest_reg = operands.substr(0,operands.find(','));
			  //cout<<"destination register:"<<dest_reg<<endl;  
			  
			  temp = operands.substr(operands.find(' ')+1);
			  //cout<< "temp"<< temp <<endl;

			  src_reg1 = temp.substr(0,temp.find(','));
			  //cout<< "source reg1:"<< src_reg1<<endl;  
			 
			  
			  //cout<< "temp:" << temp << endl;
			  
			  literal = temp.substr(temp.find(' ')+2);
			  //cout<< "literal value:"<< literal <<endl;
	 
	  	}
	else if((opcode == "STORE")){
		   operands= s.substr(s.find(' ') + 1);
		   //cout<<"operands are:"<<operands<<endl;
		  
		   src_reg1= operands.substr(0,operands.find(','));
		   //cout<<"source register1:"<< src_reg1<< endl;

		   temp = operands.substr(operands.find(' ')+1);
		   //cout<< "temp"<< temp <<endl;  
		   
		   src_reg2 = temp.substr(0,temp.find(','));
		   //cout<< "source reg2:"<< src_reg2<<endl; 

		   literal = temp.substr(temp.find(' ')+2);
		   //cout<< "literal value:"<< literal <<endl;

	   	}

		Decode->Output_Instruction->source_register1 = src_reg1;
		Decode->Output_Instruction->source_register2 = src_reg2;
		Decode->Output_Instruction->dest_register = dest_reg;
		Decode->Output_Instruction->literal = literal;
		Decode->Output_Instruction->opcode= opcode;
		Decode->Output_Instruction-> PC = Decode -> Input_Instruction -> PC;
		Decode -> Output_Instruction->instruction_string = Decode -> Input_Instruction -> instruction_string;
		//cout<<"PC value in decode output stage is:"<< Decode->Output_Instruction-> PC<<endl;
		Execute -> init = true;
	}
}


void execute(stage* Decode,stage* Execute,Register_File* re, stage* Memory){
	if (Execute -> init)
	{
		int x, y, z, l;
		int buffer;
		Execute->Input_Instruction->instruction_string = Decode -> Output_Instruction->instruction_string;
		Execute->Input_Instruction->source_register1 = Decode->Output_Instruction->source_register1;
		Execute->Input_Instruction->source_register2 = Decode->Output_Instruction->source_register2;
		Execute->Input_Instruction->dest_register = Decode->Output_Instruction->dest_register;
		Execute->Input_Instruction->literal = Decode->Output_Instruction->literal;
		Execute->Input_Instruction->opcode= Decode->Output_Instruction->opcode;
		Execute->Input_Instruction-> PC = Decode->Output_Instruction-> PC;

		//cout<<"literal is"<< Execute->Input_Instruction->literal<<endl;

		if((Execute->Input_Instruction->opcode == "ADD") || (Execute->Input_Instruction->opcode == "SUB") || 
			(Execute->Input_Instruction->opcode == "MUL") || (Execute->Input_Instruction->opcode == "AND") || 
		     		(Execute->Input_Instruction->opcode == "XOR") || (Execute->Input_Instruction->opcode == "OR")){
			  x = atoi(Execute->Input_Instruction->source_register1.substr(1).c_str());
			 //cout<< "source reg1 num is" << x << endl;

			  y = atoi(Execute -> Input_Instruction -> source_register2.substr(1).c_str());
			 //cout<< "source reg2 num is" << y << endl; 

			  z = atoi(Execute->Input_Instruction-> dest_register.substr(1).c_str());       
			 //cout<< "dest reg num is" << z << endl;    

			 if(Execute->Input_Instruction->opcode == "ADD"){
				  buffer =  re->reg[x].value +  re->reg[y].value;
				  //cout<< "added value is" << buffer <<endl;
	 		 }
	 		else if(Execute->Input_Instruction->opcode == "SUB"){
				 buffer =  re->reg[x].value -  re->reg[y].value;
				 //cout<< "subtracted value is" << buffer << endl; 
		 	}
			else if(Execute->Input_Instruction->opcode == "MUL"){
				  buffer =  re->reg[x].value * re->reg[y].value;
				  //cout<< "multiplied value is" << buffer; 
			}
			else if(Execute->Input_Instruction->opcode == "OR"){
			  buffer = re->reg[x].value || re->reg[y].value;
			  //cout<<" OR value is" <<buffer;
			 }
			else if(Execute->Input_Instruction->opcode == "XOR"){
				buffer = re->reg[x].value ^ re->reg[y].value;
				//cout<<" XOR value is" <<buffer;
			 }
			else if(Execute->Input_Instruction->opcode == "AND"){
				 buffer = re->reg[x].value & re->reg[y].value;
				 //cout<<" AND value is" <<buffer;
			}
	 
		}
		else if(Execute->Input_Instruction->opcode == "MOVC"){
			 z = atoi(Execute->Input_Instruction->dest_register.substr(1).c_str());       
			 //cout<< "dest reg num is" << z << endl;     

			 int l = atoi(Execute->Input_Instruction->literal.c_str());
			// cout<< "literal num  is" << l << endl; 
			  
			 buffer = l;
			 //cout<<" MOVC value is" << buffer;
		}

		else if(Execute->Input_Instruction->opcode == "LOAD"){
			 x = atoi(Execute->Input_Instruction->source_register1.substr(1).c_str());
			 //cout<< "source reg1 num is" << x << endl;
				  
			  l = atoi(Execute->Input_Instruction->literal.c_str());
			 //cout<< "literal num  is" << l << endl; 
				 
			  z = atoi(Execute->Input_Instruction->dest_register.substr(1).c_str());       
			 //cout<< "dest reg num is" << z << endl;
				 
			 Execute->Input_Instruction-> load_target =  re->reg[x].value + l;
			 //cout<<" LOAD target address is" << Execute->Input_Instruction-> load_target << endl;
	 
	 	} 

		else if(Execute->Input_Instruction->opcode == "STORE"){
			   x = atoi(Execute->Input_Instruction->source_register1.substr(1).c_str());
			  //cout<< "source reg1 num is" << x << endl;
			  
			   y = atoi(Execute -> Input_Instruction -> source_register2.substr(1).c_str());
			 // cout<< "source reg2 num is" << y << endl; 

			   l = atoi(Execute->Input_Instruction->literal.c_str());
			  //cout<< "literal num  is" << l << endl; 
			  
			  Execute->Input_Instruction-> store_target  = re->reg[y].value + l;
			   //cout<< "STORE target address is" << Execute->Input_Instruction-> store_target<<endl ;
	  
	   	}   

		Execute->Output_Instruction->source_register1= Execute->Input_Instruction->source_register1;
		Execute->Output_Instruction->source_register2= Execute->Input_Instruction->source_register2;
		Execute->Output_Instruction->dest_register = Execute->Input_Instruction->dest_register;
		Execute->Output_Instruction->literal = Decode->Output_Instruction->literal;
		Execute->Output_Instruction-> opcode = Execute->Input_Instruction-> opcode;
		Execute->Output_Instruction-> PC = Execute->Input_Instruction-> PC;
		Execute->Output_Instruction-> store_target = Execute->Input_Instruction-> store_target;
		Execute->Output_Instruction-> load_target =  Execute->Input_Instruction-> load_target;
		Execute->Output_Instruction-> z = z;
		Execute->Output_Instruction-> x = x;
		Execute->Output_Instruction-> y = y;
		Execute->Output_Instruction-> buffer = buffer;
		Execute->Output_Instruction->instruction_string = Execute->Input_Instruction->instruction_string;

		//cout<<"opcode is"<< Execute->Output_Instruction->opcode <<endl;
		Memory -> init = true;
	}

}


void memory(stage* Execute,stage* Memory,Register_File* re, Data_Memory* dm, stage* WriteBack){

	if (Memory -> init)
	{
		int p;
		int q;
		Memory->Input_Instruction->instruction_string = Execute->Output_Instruction->instruction_string;
		Memory->Input_Instruction->source_register1 = Execute->Output_Instruction->source_register1;
		Memory->Input_Instruction->source_register2 = Execute->Output_Instruction->source_register2;
		Memory->Input_Instruction->dest_register = Execute->Output_Instruction->dest_register;
		Memory->Input_Instruction->literal = Execute->Output_Instruction->literal;
		Memory->Input_Instruction->opcode = Execute->Output_Instruction->opcode;
		Memory->Input_Instruction-> PC = Execute -> Output_Instruction-> PC;
		Memory->Input_Instruction-> store_target = Execute->Output_Instruction-> store_target;
		Memory->Input_Instruction-> load_target = Execute->Output_Instruction-> load_target;
		Memory->Input_Instruction-> z= Execute->Output_Instruction-> z;
		Memory->Input_Instruction-> x= Execute->Output_Instruction-> x;
		Memory->Input_Instruction-> y= Execute->Output_Instruction-> y;
		Memory->Input_Instruction-> buffer = Execute->Output_Instruction-> buffer ;
		//cout<< "store target from execute stage is"<< Execute->Output_Instruction-> store_target  << endl;

	  	if(Memory->Input_Instruction->opcode == "LOAD"){
			   int x= Memory->Input_Instruction-> load_target;
			   p = dm->data_mem[x];
			   //cout<< "computed memory address for load is:"<< p<< endl;
	    	}

	  	if(Memory->Input_Instruction->opcode == "STORE"){
			    int x = atoi(Execute->Input_Instruction->source_register1.substr(1).c_str());
			   //cout<< "source reg1 num is" << x << endl;
			   dm->data_mem[Memory->Input_Instruction-> store_target] = re->reg[x].value;
			   //cout<< "computed memory address for store is:"<< Memory->Input_Instruction-> store_target << endl;
	   	} 

		Memory->Output_Instruction->instruction_string = Memory->Input_Instruction->instruction_string;
		Memory->Output_Instruction->source_register1 = Memory->Input_Instruction->source_register1;
		Memory->Output_Instruction->source_register2 = Memory->Input_Instruction->source_register2;
		Memory->Output_Instruction->dest_register = Memory->Input_Instruction->dest_register;
		Memory->Output_Instruction->literal = Memory->Input_Instruction->literal;
		Memory->Output_Instruction-> opcode = Memory->Input_Instruction->opcode;
		Memory->Output_Instruction-> PC = Memory->Input_Instruction-> PC;
		Memory->Output_Instruction-> store_target= Memory->Input_Instruction-> store_target;
		Memory->Output_Instruction-> load_target = Memory->Input_Instruction-> load_target;
		Memory->Output_Instruction-> z= Memory->Input_Instruction-> z;
		Memory->Output_Instruction-> buffer= Memory->Input_Instruction-> buffer;
		WriteBack -> init = true;
	}
}

void writeback(stage* WriteBack,stage* Memory,Register_File* re, Data_Memory* dm)
{
	if (WriteBack -> init)
	{
		WriteBack->Input_Instruction->instruction_string = Memory->Output_Instruction->instruction_string;
		WriteBack->Input_Instruction->source_register1 = Memory->Output_Instruction->source_register1;
		WriteBack->Input_Instruction->source_register2 = Memory->Output_Instruction->source_register2;
		WriteBack->Input_Instruction->dest_register = Memory->Output_Instruction->dest_register;
		WriteBack->Input_Instruction->literal = Memory->Output_Instruction->literal;
		WriteBack->Input_Instruction->opcode = Memory->Output_Instruction-> opcode;
		WriteBack->Input_Instruction-> PC = Memory->Output_Instruction-> PC;
		WriteBack->Input_Instruction-> z = Memory->Output_Instruction-> z;
		WriteBack->Input_Instruction-> buffer = Memory->Output_Instruction-> buffer;
		//cout<<" buffer value is"<< WriteBack->Input_Instruction-> buffer << endl;

	  	if((WriteBack->Input_Instruction->opcode) != "STORE"){

		 re->reg[WriteBack->Input_Instruction-> z].value = WriteBack->Input_Instruction-> buffer;
		 //cout<< "dest reg written value is"<<re->reg[WriteBack->Input_Instruction-> z].value<<endl;    
	  	}
	}
}


int main(int argc, char* argv[])
{
	int cycle,stag,PC=4000,i=1;
	Register_File reg;
	fstream fIn;
	Data_Memory dmem;
	Code_Memory cmem;
	int k=0;
	string stage;
	int clock_cycle = 0;
	int total_cycles = 0;
	int index = 0;

	/*stage F;
	stage D;
	stage E;
	stage M;
	stage W;*/
	//Instruction_info ii; 
	fIn.open( "sampleinput.txt", ios::in );

	F.Input_Instruction = new Instruction_info;
	F.Output_Instruction = new Instruction_info;

	D.Input_Instruction = new Instruction_info;
	D.Output_Instruction = new Instruction_info;

	E.Input_Instruction = new Instruction_info;
	E.Output_Instruction = new Instruction_info;

	M.Input_Instruction = new Instruction_info;
	M.Output_Instruction = new Instruction_info;

	W.Input_Instruction = new Instruction_info;
	W.Output_Instruction = new Instruction_info;

	if( fIn.is_open() )
	{
		string s;
		while( getline( fIn, s )){
		    //cout << s << endl;
		    cmem.code[k].instruction_string = s;
		    cmem.code[k].address = 4000 + ((k + 1) * 4);
		    //cout <<"code memory initialised \n"<< cmem.code[k].instruction_string << endl;
			k++;
		    // Tokenize s here into columns (probably on spaces)
	  	}
	  	fIn.close();
	}
       	else{
       		cout << "Error opening file " << errno << endl;
        }

	while(true)
	{
		cout << endl << endl;
		cout<<"enter the stag to be activated 1.Initialisation 2.simulation 3.display";
		cin>>stag;

	   	if(stag==1){
			cout<<"Initialisation completed\n";
			int source_registers_value=0;         
			int dest_register_value=0;
			int address= address*4;
			int base_address=0;
	     		for(int i=0;i<16;i++){
		      		reg.reg[i].value= 0;
		      		//cout<<"initial register values are"<<reg.reg[i].value << endl;
	      		}
	     		for(int j=0; j<100; j++){
		      		dmem.data_mem[j]=0;
		      		//cout<<"data memory initialised"<<dmem.data_mem[j]<<endl;
	     		}
			F.Input_Instruction -> instruction_string = cmem.code[0].instruction_string;
			F.Input_Instruction-> PC = PC;
			index++;
	       	}

	       	else if(stag == 2){
			cout<<"simulation started"<<endl;
		  	cout<<"enter no. of cycles"<<endl;
		  	cin>> cycle;
		  	int i=1;
		  	while(i <= cycle)
		  	{
		    		writeback(&W,&M,&reg,&dmem);
		   		memory(&E,&M,&reg,&dmem, &W); 
		   		execute(&D,&E,&reg, &M);
		   		decode(&D,&F, &E);
		   		fetch(&F, &D);

				if (M.Output_Instruction -> instruction_string == "nop")
				{
					cout << "Simulation Completed for all instructions" << endl; 
					break;
				}

				if (PC < (((k - 1) * 4) + 4000))
				{
		    			PC= PC + 4;
					F.Input_Instruction-> PC = PC;
					F.Input_Instruction -> instruction_string = cmem.code[index].instruction_string;
			       		cout<<"Input instruction is"<< F.Input_Instruction -> instruction_string;
					index++;
				}
		     		else
				{
					F.Input_Instruction -> instruction_string = "nop";
				}

				total_cycles++;

		   		i++;
				cout << endl << endl;
				cout << "Fetch Output - " << F.Input_Instruction -> instruction_string << endl;
				cout << "Decode Output - " << D.Input_Instruction -> instruction_string << endl;
				cout << "Execute Output - " << E.Input_Instruction -> instruction_string << endl;
				cout << "Memory Output - " << M.Input_Instruction -> instruction_string << endl;
				cout << "Write Back Output - " << W.Input_Instruction -> instruction_string << endl;

			}
	       	}
	 
	      	else if(stag == 3){
			cout<<"display mode"<<endl;
		     	for(int k=0; k<=16 ; k++)
		      	{
		      		cout<<"Register values is" << k << " - " <<reg.reg[k].value<<endl; 
		      	}
		     	for(int m=0; m<100; m++)
		      	{
		      		cout<<"data memory value is" << m << " - " <<dmem.data_mem[m]<<endl;
		      	}
	      	}
		else
		{
			break;
		}
	}
	return 0;
}















//Fetch.Input_Instruction-> PC= PC;
//fetch(&Fetch);
//PC= PC + 4;
//cout<< "program counter value is" <<PC;
//decode(&Decode,&Fetch);
//execute(&Decode,&Execute,&reg);
//memory(&Execute,&Memory,&reg,&dmem); 
//writeback(&WriteBack,&Memory,&reg,&dmem);
//stats.cycle++;



/*
int main () {
   char data[100];

   // open a file in read mode.
   ifstream infile; 
   infile.open("input.txt"); 
 
   cout << "Reading from the file" << endl; 
   infile >> data; 

   // write the data at the screen.
   cout << data << endl;
   
   // again read the data from the file and display it.

   // close the opened file.
   infile.close();*/


/*int main()
{
	char  ch;
        const char *fileName="input.txt";
	
	//declare object
	ifstream file;
	
	//open file
	file.open(fileName,ios::in);
	if(!file)
	{
		cout<<"Error in opening file!!!"<<endl;
		return -1; //return from main
	}
	
	//read and print file content
	while (!file.eof()) 
	{
		file >> noskipws >> ch;	//reading from file
		cout << ch;	//printing
	}
	//close the file
	file.close();
	
	return 0;
}*/



