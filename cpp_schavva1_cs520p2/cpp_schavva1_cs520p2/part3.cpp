#include <fstream>
#include <iostream>
#include <string> 
#include <sstream>
#include <cstdlib>
using namespace std;
 
/* CODE MEMORY*/

bool Mem_Stall = false;
bool Execution_Stall = false;
bool decode_dependency = false;
bool fetch_dependency = false;
bool Decode_stall =false;
bool branch_set = false;
bool branch_negative = false;
bool jump_negative = false;
int branch_pc;
bool exe_branch;
bool exe_jump;
bool exe_halt;
int jaltarget;
bool m_stall_1 = false;
bool m_stall_2 = false;
bool md_stall = false;
bool mul_dep = false;


struct Code_Line { 

       int file_line_number; 
       int address;              /* integer (multiple of 4)*/
       string instruction_string;
                 };

struct Code_Memory { 
        		Code_Line code[100];        /* filled in by reading from code file.*/
                   };

struct Data_Memory 
	{
		int  base_address;       /*integer (where this starts)*/
		int  data_mem[4000];        /*integers, indexed by offset of word address from base address */
        };


struct Register
	{
		int  value;
		bool status= false;  
		bool valid = false; 
		bool allocate= false;   /* VALID or INVALID */
					
        };

struct Register_File 
	{
		Register reg[16]; 
				      /* array of struct Registers*/
        };

struct PhysicalRegister
	{
		int  value;
		bool status= false;  
		bool valid = false; 
		bool allocate= false;   /* VALID or INVALID */
					
        };

struct PhysicalRegister_File
	{
		PhysicalRegister preg[32];
	};		

struct RenameTable
	{
		int address;
		int physicalreg;	
	};	

struct Instruction_info {
        int  PC;
        string instruction_string = "nop";    /*(YOU WILL FIND IT USEFUL TO ADD A NOP)*/ 
        int source_register1;
        int source_register2;
        string dest_register;
        int literal;
        string opcode = "nop";
        int load_target; 
        int store_target;
        int x,y,z,l;
        int buffer;
	bool stalled = false;
	string src_name1;
	string src_name2;
	
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
        bool Stalled = false;         /*True or False*/
	bool init = false;
}F,D,E,EM1,EM2,IQ,M,W,M_IFU,ED1,ED2,ED3,ED4,M_MUL;

struct Flags{  
       bool zero= false;  
       bool carry= false;
       bool Negative= false;
            }; 

struct Stats { 
       int cycle=0;
       bool temp;
             };

void fetch(stage* Fetch, stage* Decode)
{

	
		
		if(Fetch -> Output_Instruction -> stalled && Decode -> Input_Instruction-> stalled)
		{
			Fetch -> Input_Instruction-> stalled = true;
		}
		else
		{
			Fetch -> Input_Instruction-> stalled = false;
		}
		
	   	if(Decode-> Output_Instruction-> stalled)
		{
			if(!Decode -> Input_Instruction-> stalled)
			{
				Fetch -> Output_Instruction -> instruction_string = Fetch -> Input_Instruction -> instruction_string;
				//cout<< endl <<"fetch output is" << endl <<Fetch-> Output_Instruction->instruction_string<< endl;
				Fetch -> Output_Instruction -> PC = Fetch -> Input_Instruction -> PC;
				//cout<< "PC value in fetch output stage is:"<< Fetch -> Input_Instruction -> PC;
		
				Decode -> Input_Instruction->instruction_string = Fetch -> Output_Instruction -> instruction_string;
				Decode -> Input_Instruction -> PC = Fetch -> Output_Instruction -> PC;
				
				Fetch -> Output_Instruction -> stalled = true;
				Decode -> Input_Instruction-> stalled = true;
			}
			else 
			{
				return;
			}
		}
		else if(!Decode-> Output_Instruction-> stalled)
		{
			
				Fetch -> Output_Instruction -> instruction_string = Fetch -> Input_Instruction -> instruction_string;
				//cout<< endl <<"fetch output is" << endl <<Fetch-> Output_Instruction->instruction_string<< endl;
				Fetch -> Output_Instruction -> PC = Fetch -> Input_Instruction -> PC;
				//cout<< "PC value in fetch output stage is:"<< Fetch -> Input_Instruction -> PC;
		
				Decode -> Input_Instruction->instruction_string = Fetch -> Output_Instruction -> instruction_string;
				Decode -> Input_Instruction -> PC = Fetch -> Output_Instruction -> PC;
				
				Fetch -> Input_Instruction -> stalled = false;
				Fetch -> Output_Instruction -> stalled = false;
				Decode -> Input_Instruction-> stalled = false;
		}		
				
	        Decode -> init = true;
	
	
}

void decode(stage* Decode,stage* Execute, stage* Execute_M1, Register_File* re,stage* Execute_D1){
	if (Decode -> init)
	{
		//string opcode;
		//Decode -> Input_Instruction->instruction_string = Fetch -> Output_Instruction -> instruction_string;
		//Decode -> Input_Instruction -> PC = Fetch -> Output_Instruction -> PC;
		//cout<< endl <<"decode input is" <<endl << Decode -> Input_Instruction->instruction_string << endl;
		
			if( Execution_Stall && !Decode -> Output_Instruction -> stalled && !decode_dependency)
			{
				//cout<<"Num 1"<< endl;
				Decode -> Output_Instruction -> stalled = false;
			}
			else if( Execution_Stall || decode_dependency || md_stall)
			{
				//cout<<"Num 2"<< endl;
				//cout<< "exec stall: " << Execution_Stall << "  decode_dependency: "<< decode_dependency << "   md_stall: " << md_stall << endl; 
				Decode -> Output_Instruction -> stalled = true;
			}
			else if( !Execution_Stall && !decode_dependency && !md_stall)
			{
				//cout<<"Num 3"<< endl;
				Decode -> Output_Instruction -> stalled = false;
			}
			if( Decode_stall)
			{
				//cout<<"Num 4"<< endl;
				Decode -> Output_Instruction -> stalled = true;
			}
			/*if (m_stall_1)
			{
				cout<<"Num 5"<< endl;
				md_stall = true;
			}*/
			if(!Decode-> Output_Instruction-> stalled){
			
				Decode-> Output_Instruction->instruction_string = Decode -> Input_Instruction -> instruction_string;
				Decode-> Output_Instruction-> PC = Decode -> Input_Instruction -> PC;
		


				string s = Decode -> Input_Instruction->instruction_string;
				//cout<< "string is" << s<< endl;
				string opcode;
				string operands;
				string dest_reg;
				string src_reg1;
				string src_reg2;
				int src_r1;
				int src_r2;
				int dest_r;
				string literal;
				int lit;
				string temp,temp2;
				int x,y,z;
				opcode = s.substr(0,s.find(','));
				Decode-> Output_Instruction-> opcode = opcode;
				//cout<<"opcode is:"<<opcode<< endl;

				if((opcode == "ADD")|| (opcode == "MUL") || (opcode== "SUB") || (opcode == "XOR") || (opcode== "AND") || (opcode == "OR") || (opcode == "DIV")){

					  operands= s.substr(s.find(',')+1);
					  //cout<<"operands are:"<<operands<<endl;
					  dest_reg = operands.substr(0,operands.find(','));
					  z = atoi(dest_reg.substr(1).c_str());       
			 		  cout<< "dest reg num is" << z << endl;    
					  cout<<"destination reg is "<<dest_reg<<endl;  

					  temp = operands.substr(operands.find(',')+1);
					  //cout<< "temp is" << temp<< endl;

					  src_reg1 = temp.substr(0,temp.find(','));
					  cout<< "source reg1 :"<< src_reg1<<endl;

					  src_r1 = re -> reg[atoi(src_reg1.substr(1).c_str())].value;
					  x =  atoi(src_reg1.substr(1).c_str());
			    		  cout<< "src_1 is" <<  src_r1 << endl;
					
  					  src_reg2 = temp.substr(temp.find(',')+1);
					  src_r2 = re -> reg[atoi(src_reg2.substr(1).c_str())].value;
		  		          y = atoi(src_reg2.substr(1).c_str());
			 		  cout<< "source reg2 num is" << y << endl; 
						
					  dest_reg = 
					  Decode-> Output_Instruction-> dest_register = dest_reg;
					  Decode-> Output_Instruction-> src_name1 = src_r1;
					  Decode-> Output_Instruction-> src_name2 = src_r2;
						
					 // cout<< "decode output src_name1 is"<<  Decode-> Output_Instruction-> src_name1 << endl;
					  Decode-> Output_Instruction-> x = x;
			    		  Decode-> Output_Instruction-> y = y;
					  Decode-> Output_Instruction-> z = z;
					  //cout<< "source reg2 :"<< src_reg2<<endl; 

					     
					 
					  if( re -> reg[x].status  &&  re -> reg[y].status && re -> reg[z].status)
					  {
				     		//cout<< "x value"<< x <<endl <<"y value" << y <<endl << "z vaue" <<z<<endl;
						//cout<< "x status"<< re -> reg[x].status <<endl <<"y status" << re -> reg[x].status <<endl << "z status" << re -> reg[x].status <<endl;
						src_r1 = re -> reg[atoi(src_reg1.substr(1).c_str())].value;
						src_r2 = re -> reg[atoi(src_reg2.substr(1).c_str())].value;
						//cout << "source 1 value - " << src_r1 << endl;
						//cout << "source 2 value - " << src_r2 << endl;
						if (md_stall && opcode == "MUL")
						{
							Decode-> Output_Instruction-> stalled = true;
							Execute_M1-> Input_Instruction-> stalled = true;
						}
						decode_dependency= false;
					  }	
					  else	
					  {
						Decode-> Output_Instruction-> stalled = true;
						
						decode_dependency = true;
						//cout<< "decode dependencyyyyy :"<< decode_dependency << endl;
					  	if(opcode =="MUL"){
							Execute_M1-> Input_Instruction-> stalled = true;
							//cout<< "decode stalled " <<Decode-> Output_Instruction-> stalled<< endl;
						}
						else if (opcode == "DIV")
						{
							Execute_D1-> Input_Instruction-> stalled = true;
						}
						else{
							Execute-> Input_Instruction-> stalled = true;
						}
					  }

					if ((opcode == "ADD")|| (opcode == "MUL") || (opcode== "SUB") || (opcode == "DIV"))
					{
						branch_pc = Decode -> Output_Instruction -> PC;
					}
					if (opcode == "MUL")
					{
						if (m_stall_1)
						{
							//cout<<"Num 5"<< endl;
							md_stall = true;
						}
					}

			  	}
				/*else if(opcode == "DIV")
				{
					  operands= s.substr(s.find(',')+1);
					  dest_reg = operands.substr(0,operands.find(','));
					  z = atoi(dest_reg.substr(1).c_str());       
			 		  cout<< "dest reg num is" << z << endl;    
					  //cout<<"destination reg is "<<dest_reg<<endl;  

					  temp = operands.substr(operands.find(',')+1);
					  //cout<< "temp is" << temp<< endl;

					  src_reg1 = temp.substr(0,temp.find(','));
					  cout<< "source reg1 :"<< src_reg1<<endl;

					  src_r1 = re -> reg[atoi(src_reg1.substr(1).c_str())].value;
					  x =  atoi(src_reg1.substr(1).c_str());
			    		  cout<< "src_1 is" <<  src_r1 << endl;
					
  					  src_reg2 = temp.substr(temp.find(',')+1);
					  src_r2 = re -> reg[atoi(src_reg2.substr(1).c_str())].value;
		  		          y = atoi(src_reg2.substr(1).c_str());
			 		  cout<< "source reg2 num is" << y << endl; 
					
					  Decode-> Output_Instruction-> dest_register = dest_reg;
					  Decode-> Output_Instruction-> src_name1 = src_r1;
					  Decode-> Output_Instruction-> src_name2 = src_r2;
						
					  //cout<< "decode output src_name1 is"<<  Decode-> Output_Instruction-> src_name1 << endl;
					  Decode-> Output_Instruction-> x = x;
			    		  Decode-> Output_Instruction-> y = y;
					  Decode-> Output_Instruction-> z = z;
					  //cout<< "source reg2 :"<< src_reg2<<endl; 
					  
					  
				}*/
				else if ((opcode == "MOVC")){
					 operands= s.substr(s.find(',')+1);
					 //cout<<"operands are:"<<operands<<endl;

					 dest_reg= operands.substr(0,operands.find(','));
					 //cout<<"destination register:"<< dest_reg <<endl;
					 dest_r = re-> reg[atoi(dest_reg.substr(1).c_str())].value;
					 z = atoi(dest_reg.substr(1).c_str());

					 

					 literal= operands.substr(operands.find(',')+2);
					 lit = atoi(literal.c_str());
					 //cout<<"literal:"<< literal << endl; 

					 Decode-> Output_Instruction-> z = z;
					 Decode-> Output_Instruction-> dest_register = dest_reg;
					 Decode-> Output_Instruction-> literal = lit; 
					
					if( re -> reg[z].status){
				
						decode_dependency= false;
					}
					else{
					
						 Decode-> Output_Instruction-> stalled = true;
						 decode_dependency= true;
						 Execute-> Input_Instruction-> stalled = true;
					}


			 	 } 
				else if((opcode == "LOAD")){
			  
					  operands= s.substr(s.find(',') + 1);
					  //cout<<"operands are:"<<operands<<endl;

					  dest_reg = operands.substr(0,operands.find(','));
					  //cout<< "suvvi is"<< dest_reg<< endl;

					  dest_r = re-> reg[atoi(dest_reg.substr(1).c_str())].value;
					  //cout<<"destination register:"<<dest_reg<<endl; 

					  z = atoi(dest_reg.substr(1).c_str()); 
					  
					  temp = operands.substr(operands.find(',')+1);
					  //cout<< "temp"<< temp <<endl;

					  src_reg1 = temp.substr(0,temp.find(','));
					  src_r1 = re -> reg[atoi(src_reg1.substr(1).c_str())].value;
					  //cout<< "source reg1:"<< src_reg1<<endl;  
					  x = atoi(src_reg1.substr(1).c_str());
					 
					  literal = temp.substr(temp.find(',')+2);
					  lit = atoi(literal.c_str());
					  //cout<< "literal value:"<< lit <<endl;

 					  Decode-> Output_Instruction-> src_name1 = src_r1;
					  Decode-> Output_Instruction-> x = x;
					  Decode-> Output_Instruction-> z = z;
					  Decode-> Output_Instruction-> dest_register = dest_reg;
					  Decode-> Output_Instruction-> literal =  lit;
			 
					  if(re -> reg[x].status && re-> reg[z].status){

						src_r1 = re -> reg[atoi(src_reg1.substr(1).c_str())].value;
						decode_dependency= false;
					  }
					  else{
						Decode-> Output_Instruction-> stalled = true;
						decode_dependency= true;
						Execute-> Input_Instruction-> stalled = true;
					  }
			  	}

				else if((opcode == "STORE")){
					   operands= s.substr(s.find(',') + 1);
					   //cout<<"operands are:"<<operands<<endl;
					  
					   src_reg1= operands.substr(0,operands.find(','));
					  
					   //cout<<"source register1:"<< src_reg1<< endl;

					
					   src_r1 = re -> reg[atoi(src_reg1.substr(1).c_str())].value;
					   x= atoi(src_reg1.substr(1).c_str());

					   temp = operands.substr(operands.find(',')+1);
					  // cout<< "temp"<< temp <<endl;  
					   
					   src_reg2 = temp.substr(0,temp.find(','));
						
					   //cout<< "source reg2:"<< src_reg2<<endl; 
					
					   src_r2 = re -> reg[atoi(src_reg2.substr(1).c_str())].value;
					   y= atoi(src_reg2.substr(1).c_str());

					   Decode-> Output_Instruction-> src_name1 = src_r1;
					   Decode-> Output_Instruction-> src_name2 = src_r2;
					   Decode-> Output_Instruction-> x = x;
					   Decode-> Output_Instruction-> y = y;
	


					   literal = temp.substr(temp.find(',')+2);
					   lit = atoi(literal.c_str());
					   Decode-> Output_Instruction-> literal =  lit;
					   //cout<< "literal value:"<< literal <<endl;

						   if(re -> reg[x].status && re -> reg[y].status){
				
								 src_r1 = re -> reg[atoi(src_reg1.substr(1).c_str())].value;
								 src_r2 = re -> reg[atoi(src_reg2.substr(1).c_str())].value;
								 decode_dependency= false;
						   }		
						   else {
								Decode-> Output_Instruction-> stalled = true;		
								decode_dependency= true;
								Execute-> Input_Instruction-> stalled = true;
						   }
			   
				}
				else if((opcode == "BZ") || (opcode == "BNZ"))
				{
					if(s.find("-") == -1)
					{
						literal = s.substr(s.find(',')+2);
						lit = atoi(literal.c_str());
						//cout << "literal value is" << lit << endl;
						//Decode -> Output_Instruction -> literal = lit;
					}
					else
					{
						literal = s.substr(s.find('-')+1);
						lit = atoi(literal.c_str());
						//cout << "literal value is" << lit << endl;
						//Decode -> Output_Instruction -> literal = lit;
					}
					
					Decode-> Output_Instruction-> literal =  lit;
					Decode-> Output_Instruction-> src_name1 = "R16";
					Decode-> Output_Instruction-> src_name2 = "";
					Decode-> Output_Instruction-> x = 16;
					Decode-> Output_Instruction-> dest_register = "";
					

					if( re -> reg[16].status){
				
						decode_dependency= false;
					}
					else
					{
						Decode-> Output_Instruction-> stalled = true;		
						decode_dependency= true;
					}
				}	
							
				else if(opcode == "JUMP")
				{

					 if(s.find("-") == -1)
					 { 
						 operands= s.substr(s.find(',')+1);
						 //cout<<"operands are:"<<operands<<endl;

						 src_reg1= operands.substr(0,operands.find(','));
						 //cout<<"destination register:"<< dest_reg <<endl;
						 src_r1 = re-> reg[atoi(src_reg1.substr(1).c_str())].value;
						 x = atoi(src_reg1.substr(1).c_str());

						 literal= operands.substr(operands.find(',')+2);
						 lit = atoi(literal.c_str());
						 //cout<<"literal:"<< literal << endl; 
						 jump_negative = false;
						
								
					}
					else
					{
						 operands= s.substr(s.find(',')+1);
						 //cout<<"operands are:"<<operands<<endl;

						 src_reg1= operands.substr(0,operands.find(','));
						 x = atoi(src_reg1.substr(1).c_str());
						// cout<< "source reg is :" << src_reg1 << endl; 
						 //cout<< "x is"<< x << endl ;
						 
						 src_r1 = re-> reg[atoi(dest_reg.substr(1).c_str())].value;
						
						 literal= operands.substr(operands.find('-')+1);
						 lit = atoi(literal.c_str());
						// cout<<"lit is:"<< lit << endl; 
						
						 jump_negative = true;
					}

						 

       					 Decode-> Output_Instruction-> x = x;
					 Decode-> Output_Instruction-> dest_register = dest_reg;
					 Decode-> Output_Instruction-> literal = lit; 
	
					if( re -> reg[x].status)
					{
				
						decode_dependency= false;
					}
					else{
					
						 Decode-> Output_Instruction-> stalled = true;
						 decode_dependency= true;
						 Execute-> Input_Instruction-> stalled = true;
					}	
		
				}
				else if(opcode == "JAL")
				{
					 if(s.find("-") == -1)
						{
							operands= s.substr(s.find(',') + 1);
							dest_reg = operands.substr(0,operands.find(','));
							z = atoi(dest_reg.substr(1).c_str());
							//cout<< "dest_reg:" << dest_reg<< endl; 
							//cout<< "z is"<< z << endl ;
							
							temp = operands.substr(operands.find(',')+1);
							src_reg1 = temp.substr(0,temp.find(','));
							//cout<< "src_reg1:" << src_reg1 << endl; 

							src_r1 = re -> reg[atoi(src_reg1.substr(1).c_str())].value;
							//cout<< "source reg1:"<< src_r1<<endl;  
							x = atoi(src_reg1.substr(1).c_str());
							//cout<< "x is"<< x<< endl ;
							
							literal = temp.substr(temp.find(',')+2);
							lit = atoi(literal.c_str());
							//cout<< "literal value:"<< lit <<endl;
						
							
							jump_negative = false;
						}
					 else
						{
							operands= s.substr(s.find(',') + 1);
							dest_reg = operands.substr(0,operands.find(','));
							z = atoi(dest_reg.substr(1).c_str());
							//cout<< "dest_reg:" << dest_reg<< endl; 
							//cout<< "z is"<< z << endl ;
							
							temp = operands.substr(operands.find(',')+1);
							src_reg1 = temp.substr(0,temp.find(','));
							//cout<< "src_reg1:" << src_reg1 << endl; 

							src_r1 = re -> reg[atoi(src_reg1.substr(1).c_str())].value;
							//cout<< "source reg1:"<< src_r1<<endl;  
							x = atoi(src_reg1.substr(1).c_str());
							//cout<< "x is"<< x<< endl ;
							
							literal = temp.substr(temp.find('-')+1);
							lit = atoi(literal.c_str());
							//cout<< "literal value:"<< lit <<endl;

							jump_negative = true;
						}	
					
					
				         Decode-> Output_Instruction-> src_name1 = src_r1; // value inside source reg 1
					 Decode-> Output_Instruction-> x = x;    
				         Decode-> Output_Instruction-> z = z; 
				         Decode-> Output_Instruction-> dest_register = dest_reg;
					 Decode-> Output_Instruction-> literal =  lit;
							


					if(re -> reg[x].status)
					{
						
						src_r1 = re -> reg[atoi(src_reg1.substr(1).c_str())].value;  
						//cout << "source reg value for jal:" << src_r1 << endl;
					}
					else
					{
						Decode-> Output_Instruction-> stalled = true;
						decode_dependency= true;
						
					}
					if(re -> reg[z].status && !Decode-> Output_Instruction-> stalled )
					{
						decode_dependency= false;
					}
					else
					{
						decode_dependency= true;
						Decode-> Output_Instruction-> stalled = true;
					}


				
				}
				
				else if(opcode == "HALT")
				{
					Decode-> Output_Instruction-> src_name1 = "";
					Decode-> Output_Instruction-> src_name2 = "";				
					
				}


				//cout<<"PC value in decode output stage is:"<< Decode->Output_Instruction-> PC<<endl;
			
				if(opcode != "MUL" && opcode!= "DIV" && opcode != "HALT")
				{

					if (!decode_dependency)
					{
						if(opcode != "STORE" && opcode != "nop")
						{
						//cout<< "ERRORRRRR"<<endl;
							re -> reg[Decode-> Output_Instruction->z].status = false;
							if((opcode =="ADD") || (opcode == "SUB"))
							{
									re -> reg[16].status = false;
									//cout<< "cao " <<re -> reg[Decode-> Output_Instruction->z].status <<endl;
									//cout<< "cao2 is" << Decode-> Output_Instruction->z << endl;
							}	
							Execute-> Input_Instruction-> stalled = false;
						}
				
					}
					else
					{
							
						Execute-> Input_Instruction-> stalled = true;
					}
									
					if(Execution_Stall)
					{
						Decode_stall = true;
						Decode-> Output_Instruction-> stalled = true;
						Execute-> Input_Instruction-> stalled = false;
					}
						
					Execute-> Input_Instruction->instruction_string = Decode -> Output_Instruction->instruction_string;
					Execute-> Input_Instruction->source_register1 = src_r1;
					Execute-> Input_Instruction->source_register2 = src_r2;
					Execute-> Input_Instruction->dest_register = Decode-> Output_Instruction-> dest_register;
					Execute-> Input_Instruction->literal = Decode-> Output_Instruction-> literal;
					Execute-> Input_Instruction->opcode= Decode-> Output_Instruction-> opcode;
					Execute-> Input_Instruction-> PC = Decode->Output_Instruction-> PC;			
					Execute-> Input_Instruction-> x = Decode-> Output_Instruction-> x;
					Execute-> Input_Instruction-> y = Decode-> Output_Instruction-> y;
					Execute-> Input_Instruction-> z = Decode-> Output_Instruction-> z;
					//Execute_M1-> Input_Instruction-> stalled = true;
					
					Execute_M1-> Input_Instruction->instruction_string = "nop";
					Execute_M1-> Input_Instruction->opcode= "nop";

					Execute_D1-> Input_Instruction->instruction_string = "nop";
					Execute_D1-> Input_Instruction->opcode= "nop";
					
					
				}
				else if(opcode == "MUL")
				{
					Execute_M1-> Input_Instruction->instruction_string = Decode -> Output_Instruction->instruction_string;
					Execute_M1-> Input_Instruction->source_register1 = src_r1;
					Execute_M1-> Input_Instruction->source_register2 = src_r2;
					Execute_M1-> Input_Instruction->dest_register = Decode-> Output_Instruction-> dest_register;
					Execute_M1-> Input_Instruction->literal = Decode-> Output_Instruction-> literal;
					Execute_M1-> Input_Instruction->opcode= Decode-> Output_Instruction-> opcode;
					Execute_M1-> Input_Instruction-> PC = Decode->Output_Instruction-> PC;
					Execute_M1-> Input_Instruction-> x = Decode-> Output_Instruction-> x;
					Execute_M1-> Input_Instruction-> y = Decode-> Output_Instruction-> y;
					Execute_M1-> Input_Instruction-> z = Decode-> Output_Instruction-> z;

					//Execute_M1-> Input_Instruction-> stalled = false;
					Execute-> Input_Instruction->instruction_string = "nop";
					Execute-> Input_Instruction->opcode= "nop";
					
					Execute_D1-> Input_Instruction->instruction_string = "nop";
					Execute_D1-> Input_Instruction->opcode= "nop";
					
					if(!decode_dependency && opcode != "nop" && !md_stall)
					{
						//cout<< "ERRORRRRR"<<endl;
						re -> reg[Decode-> Output_Instruction->z].status = false;
						Execute_M1-> Input_Instruction-> stalled = false;
					}
					else
					{
						//cout<< "OMG"<< endl;
						Execute_M1-> Input_Instruction-> stalled = true;
					    //Execute-> Input_Instruction-> stalled = true;
					}
					

					if (Execute_M1-> Input_Instruction-> stalled && Decode-> Output_Instruction-> stalled)
					{
						mul_dep = true;
					}
				}
				else if(opcode == "DIV" || opcode == "HALT")
				{
					Execute_D1-> Input_Instruction->instruction_string = Decode -> Output_Instruction->instruction_string;
					Execute_D1-> Input_Instruction->source_register1 = src_r1;
					Execute_D1-> Input_Instruction->source_register2 = src_r2;
					Execute_D1-> Input_Instruction->dest_register = Decode-> Output_Instruction-> dest_register;
					Execute_D1-> Input_Instruction->literal = Decode-> Output_Instruction-> literal;
					Execute_D1-> Input_Instruction->opcode= Decode-> Output_Instruction-> opcode;
					Execute_D1-> Input_Instruction-> PC = Decode->Output_Instruction-> PC;
					Execute_D1-> Input_Instruction-> x = Decode-> Output_Instruction-> x;
					Execute_D1-> Input_Instruction-> y = Decode-> Output_Instruction-> y;
					Execute_D1-> Input_Instruction-> z = Decode-> Output_Instruction-> z;
				
					Execute-> Input_Instruction->instruction_string = "nop";
					Execute-> Input_Instruction->opcode= "nop";
					Execute_M1-> Input_Instruction->instruction_string = "nop";
					Execute_M1-> Input_Instruction->opcode= "nop";
					if(!decode_dependency && opcode != "nop" && opcode != "HALT")
					{
						//cout<< "ERRORRRRR"<<endl;
						re -> reg[Decode-> Output_Instruction->z].status = false;
						Execute_D1-> Input_Instruction-> stalled = false;
					}
					else if (opcode != "HALT")
					{    
						//cout<< "div testing"<< endl;
						Execute_D1-> Input_Instruction-> stalled = true;
					    //Execute-> Input_Instruction-> stalled = true;
					}
				}	
			}
			Execute -> init = true; 
			if (!m_stall_1 && md_stall)
			{
				md_stall = false;
				Execute_M1 -> Output_Instruction -> stalled = false;
				Execute_M1 -> Input_Instruction -> stalled = false;
			}
		}

	
	//cout << "EM1 stalled - " << Execute_M1-> Input_Instruction-> stalled << endl;
}

void IssueQueue(stage* Issueq,stage* Execute,stage* Execute_M1,stage* Execute_M2,stage* Execute_D1,stage* Execute_D2,stage* Execute_D3,stage* Execute_D4)
{
			Issueq-> Output_Instruction-> instruction_string = Issueq-> Input_Instruction->instruction_string;
			Issueq-> Output_Instruction-> PC = Issueq-> Input_Instruction-> PC;	
			Issueq-> Output_Instruction-> source_register1 = Issueq-> Input_Instruction->source_register1;
			Issueq-> Output_Instruction-> source_register2 = Issueq-> Input_Instruction->source_register2;
			Issueq-> Output_Instruction-> dest_register = Issueq-> Input_Instruction->dest_register;
			Issueq-> Output_Instruction-> literal = Issueq-> Input_Instruction->literal;
			Issueq-> Output_Instruction-> opcode = Issueq-> Input_Instruction->opcode;
			Issueq-> Output_Instruction-> z = Issueq-> Input_Instruction-> z;
			Issueq-> Output_Instruction-> x = Issueq-> Input_Instruction-> x;
			Issueq-> Output_Instruction-> y = Issueq-> Input_Instruction-> y;
			Issueq-> Output_Instruction-> buffer = Issueq-> Input_Instruction-> buffer;





	

}

void execute(stage* Execute, stage* Memory,stage* Mem_IFU,stage* Mem_MUL,stage* Execute_M1,stage* Execute_M2,stage* Execute_D1,stage* Execute_D2,stage* Execute_D3,stage* Execute_D4,Flags* flag1 )
{
	bool div_stall = false;	
	bool mul_stall = false;
	//bool Execution_stall;
	if (Execute -> init)
	{
		if(Execute_D4-> Input_Instruction->opcode == "DIV" || Execute_D4-> Input_Instruction->opcode == "HALT" ) //div4
		{
			//cout<< "DIV4 normal Testing"<< endl;
			Execute_D4-> Output_Instruction-> instruction_string = Execute_D4-> Input_Instruction->instruction_string;
			Execute_D4-> Output_Instruction-> PC = Execute_D4-> Input_Instruction-> PC;	
			Execute_D4-> Output_Instruction-> source_register1 = Execute_D4-> Input_Instruction->source_register1;
			Execute_D4-> Output_Instruction-> source_register2 = Execute_D4-> Input_Instruction->source_register2;
			Execute_D4-> Output_Instruction-> dest_register = Execute_D4-> Input_Instruction->dest_register;
			Execute_D4-> Output_Instruction-> literal = Execute_D4-> Input_Instruction->literal;
			Execute_D4-> Output_Instruction-> opcode = Execute_D4-> Input_Instruction->opcode;
			Execute_D4-> Output_Instruction-> z = Execute_D4-> Input_Instruction-> z;
			Execute_D4-> Output_Instruction-> x = Execute_D4-> Input_Instruction-> x;
			Execute_D4-> Output_Instruction-> y = Execute_D4-> Input_Instruction-> y;
			Execute_D4-> Output_Instruction-> buffer = Execute_D4-> Input_Instruction-> buffer;

			Memory->Input_Instruction-> instruction_string = Execute_D4->Output_Instruction->instruction_string;
			Memory->Input_Instruction-> PC = Execute_D4-> Output_Instruction-> PC;
			Memory->Input_Instruction-> source_register1 = Execute_D4-> Output_Instruction->source_register1;
			Memory->Input_Instruction-> source_register2 = Execute_D4-> Output_Instruction->source_register2;
			Memory->Input_Instruction-> dest_register = Execute_D4-> Output_Instruction->dest_register;
			Memory->Input_Instruction-> literal = Execute_D4-> Output_Instruction-> literal;
			Memory->Input_Instruction-> opcode = Execute_D4-> Output_Instruction-> opcode;
			Memory-> Input_Instruction-> z = Execute_D4-> Output_Instruction-> z;
			Memory-> Input_Instruction-> x = Execute_D4-> Output_Instruction-> x;
			Memory-> Input_Instruction-> y = Execute_D4-> Output_Instruction-> y;
			Memory-> Input_Instruction-> buffer = Execute_D4-> Output_Instruction-> buffer;
			Memory -> init = true;
			//cout << "MUL 2 - " << Memory->Input_Instruction-> opcode << endl;
		}
		
		if(Execute_D4-> Input_Instruction->opcode != "DIV") //div 4 nop
		{
			//cout<< "DIV4 nop Testing"<< endl;
			Execute_D4-> Output_Instruction-> instruction_string = Execute_D4-> Input_Instruction->instruction_string;
			Execute_D4-> Output_Instruction-> PC = Execute_D4-> Input_Instruction-> PC;	
			Execute_D4-> Output_Instruction-> source_register1 = Execute_D4-> Input_Instruction->source_register1;
			Execute_D4-> Output_Instruction-> source_register2 = Execute_D4-> Input_Instruction->source_register2;
			Execute_D4-> Output_Instruction-> dest_register = Execute_D4-> Input_Instruction->dest_register;
			Execute_D4-> Output_Instruction-> literal = Execute_D4-> Input_Instruction->literal;
			Execute_D4-> Output_Instruction-> opcode = Execute_D4-> Input_Instruction->opcode;
			Execute_D4-> Output_Instruction-> z = Execute_D4-> Input_Instruction-> z;
			Execute_D4-> Output_Instruction-> x = Execute_D4-> Input_Instruction-> x;
			Execute_D4-> Output_Instruction-> y = Execute_D4-> Input_Instruction-> y;
			Execute_D4-> Output_Instruction-> buffer = Execute_D4-> Input_Instruction-> buffer;

			Memory->Input_Instruction-> instruction_string = Execute_D4->Output_Instruction->instruction_string;
			Memory->Input_Instruction-> PC = Execute_D4-> Output_Instruction-> PC;
			Memory->Input_Instruction-> source_register1 = Execute_D4-> Output_Instruction->source_register1;
			Memory->Input_Instruction-> source_register2 = Execute_D4-> Output_Instruction->source_register2;
			Memory->Input_Instruction-> dest_register = Execute_D4-> Output_Instruction->dest_register;
			Memory->Input_Instruction-> literal = Execute_D4-> Output_Instruction-> literal;
			Memory->Input_Instruction-> opcode = Execute_D4-> Output_Instruction-> opcode;
			Memory-> Input_Instruction-> z = Execute_D4-> Output_Instruction-> z;
			Memory-> Input_Instruction-> x = Execute_D4-> Output_Instruction-> x;
			Memory-> Input_Instruction-> y = Execute_D4-> Output_Instruction-> y;
			Memory-> Input_Instruction-> buffer = Execute_D4-> Output_Instruction-> buffer;
			div_stall = true;
			//Memory -> init = true;
		}
		
		if(Execute_D3-> Input_Instruction->opcode == "DIV" || Execute_D3-> Input_Instruction->opcode == "HALT") //div3
		{
			//cout<< "DIV3 normal Testing"<< endl;
			Execute_D3-> Output_Instruction-> instruction_string = Execute_D3-> Input_Instruction->instruction_string;
			Execute_D3-> Output_Instruction-> PC = Execute_D3-> Input_Instruction-> PC;	
			Execute_D3-> Output_Instruction-> source_register1 = Execute_D3-> Input_Instruction->source_register1;
			Execute_D3-> Output_Instruction-> source_register2 = Execute_D3-> Input_Instruction->source_register2;
			Execute_D3-> Output_Instruction-> dest_register = Execute_D3-> Input_Instruction->dest_register;
			Execute_D3-> Output_Instruction-> literal = Execute_D3-> Input_Instruction->literal;
			Execute_D3-> Output_Instruction-> opcode = Execute_D3-> Input_Instruction->opcode;
			Execute_D3-> Output_Instruction-> z = Execute_D3-> Input_Instruction-> z;
			Execute_D3-> Output_Instruction-> x = Execute_D3-> Input_Instruction-> x;
			Execute_D3-> Output_Instruction-> y = Execute_D3-> Input_Instruction-> y;
			Execute_D3-> Output_Instruction-> buffer = Execute_D3-> Input_Instruction-> buffer;

			Execute_D4->Input_Instruction-> instruction_string = Execute_D3->Output_Instruction->instruction_string;
			Execute_D4->Input_Instruction-> PC = Execute_D3-> Output_Instruction-> PC;
			Execute_D4->Input_Instruction-> source_register1 = Execute_D3-> Output_Instruction->source_register1;
			Execute_D4->Input_Instruction-> source_register2 = Execute_D3-> Output_Instruction->source_register2;
			Execute_D4->Input_Instruction-> dest_register = Execute_D3-> Output_Instruction->dest_register;
			Execute_D4->Input_Instruction-> literal = Execute_D3-> Output_Instruction-> literal;
			Execute_D4->Input_Instruction-> opcode = Execute_D3-> Output_Instruction-> opcode;
			Execute_D4-> Input_Instruction-> z = Execute_D3-> Output_Instruction-> z;
			Execute_D4-> Input_Instruction-> x = Execute_D3-> Output_Instruction-> x;
			Execute_D4-> Input_Instruction-> y = Execute_D3-> Output_Instruction-> y;
			Execute_D4-> Input_Instruction-> buffer = Execute_D3-> Output_Instruction-> buffer;
			//Memory -> init = true;
			//cout << "MUL 2 - " << Memory->Input_Instruction-> opcode << endl;
		}
		if(Execute_D3-> Input_Instruction->opcode != "DIV") //div 3 nop
		{
			Execute_D3-> Output_Instruction-> instruction_string = Execute_D3-> Input_Instruction->instruction_string;
			Execute_D3-> Output_Instruction-> PC = Execute_D3-> Input_Instruction-> PC;	
			Execute_D3-> Output_Instruction-> source_register1 = Execute_D3-> Input_Instruction->source_register1;
			Execute_D3-> Output_Instruction-> source_register2 = Execute_D3-> Input_Instruction->source_register2;
			Execute_D3-> Output_Instruction-> dest_register = Execute_D3-> Input_Instruction->dest_register;
			Execute_D3-> Output_Instruction-> literal = Execute_D3-> Input_Instruction->literal;
			Execute_D3-> Output_Instruction-> opcode = Execute_D3-> Input_Instruction->opcode;
			Execute_D3-> Output_Instruction-> z = Execute_D3-> Input_Instruction-> z;
			Execute_D3-> Output_Instruction-> x = Execute_D3-> Input_Instruction-> x;
			Execute_D3-> Output_Instruction-> y = Execute_D3-> Input_Instruction-> y;
			Execute_D3-> Output_Instruction-> buffer = Execute_D3-> Input_Instruction-> buffer;

			Execute_D4->Input_Instruction-> instruction_string = Execute_D3->Output_Instruction->instruction_string;
			Execute_D4->Input_Instruction-> PC = Execute_D3-> Output_Instruction-> PC;
			Execute_D4->Input_Instruction-> source_register1 = Execute_D3-> Output_Instruction->source_register1;
			Execute_D4->Input_Instruction-> source_register2 = Execute_D3-> Output_Instruction->source_register2;
			Execute_D4->Input_Instruction-> dest_register = Execute_D3-> Output_Instruction->dest_register;
			Execute_D4->Input_Instruction-> literal = Execute_D3-> Output_Instruction-> literal;
			Execute_D4->Input_Instruction-> opcode = Execute_D3-> Output_Instruction-> opcode;
			Execute_D4-> Input_Instruction-> z = Execute_D3-> Output_Instruction-> z;
			Execute_D4-> Input_Instruction-> x = Execute_D3-> Output_Instruction-> x;
			Execute_D4-> Input_Instruction-> y = Execute_D3-> Output_Instruction-> y;
			Execute_D4-> Input_Instruction-> buffer = Execute_D3-> Output_Instruction-> buffer;
			//Memory -> init = true;
		}
		if(Execute_D2-> Input_Instruction->opcode == "DIV" || Execute_D2-> Input_Instruction->opcode == "HALT") //div2
		{
			//cout<< "DIV2 normal Testing"<< endl;
			Execute_D2-> Output_Instruction-> instruction_string = Execute_D2-> Input_Instruction->instruction_string;
			Execute_D2-> Output_Instruction-> PC = Execute_D2-> Input_Instruction-> PC;	
			Execute_D2-> Output_Instruction-> source_register1 = Execute_D2-> Input_Instruction->source_register1;
			Execute_D2-> Output_Instruction-> source_register2 = Execute_D2-> Input_Instruction->source_register2;
			Execute_D2-> Output_Instruction-> dest_register = Execute_D2-> Input_Instruction->dest_register;
			Execute_D2-> Output_Instruction-> literal = Execute_D2-> Input_Instruction->literal;
			Execute_D2-> Output_Instruction-> opcode = Execute_D2-> Input_Instruction->opcode;
			Execute_D2-> Output_Instruction-> z = Execute_D2-> Input_Instruction-> z;
			Execute_D2-> Output_Instruction-> x = Execute_D2-> Input_Instruction-> x;
			Execute_D2-> Output_Instruction-> y = Execute_D2-> Input_Instruction-> y;
			Execute_D2-> Output_Instruction-> buffer = Execute_D2-> Input_Instruction-> buffer;

			Execute_D3->Input_Instruction-> instruction_string = Execute_D2->Output_Instruction->instruction_string;
			Execute_D3->Input_Instruction-> PC = Execute_D2-> Output_Instruction-> PC;
			Execute_D3->Input_Instruction-> source_register1 = Execute_D2-> Output_Instruction->source_register1;
			Execute_D3->Input_Instruction-> source_register2 = Execute_D2-> Output_Instruction->source_register2;
			Execute_D3->Input_Instruction-> dest_register = Execute_D2-> Output_Instruction->dest_register;
			Execute_D3->Input_Instruction-> literal = Execute_D2-> Output_Instruction-> literal;
			Execute_D3->Input_Instruction-> opcode = Execute_D2-> Output_Instruction-> opcode;
			Execute_D3-> Input_Instruction-> z = Execute_D2-> Output_Instruction-> z;
			Execute_D3-> Input_Instruction-> x = Execute_D2-> Output_Instruction-> x;
			Execute_D3-> Input_Instruction-> y = Execute_D2-> Output_Instruction-> y;
			Execute_D3-> Input_Instruction-> buffer = Execute_D2-> Output_Instruction-> buffer;
			//Memory -> init = true;
			//cout << "MUL 2 - " << Memory->Input_Instruction-> opcode << endl;
		}
		
		if(Execute_D2-> Input_Instruction->opcode != "DIV") //div 2 nop
		{
			//cout<< "DIV2 nop Testing"<< endl;
			Execute_D2-> Output_Instruction-> instruction_string = Execute_D2-> Input_Instruction->instruction_string;
			Execute_D2-> Output_Instruction-> PC = Execute_D2-> Input_Instruction-> PC;	
			Execute_D2-> Output_Instruction-> source_register1 = Execute_D2-> Input_Instruction->source_register1;
			Execute_D2-> Output_Instruction-> source_register2 = Execute_D2-> Input_Instruction->source_register2;
			Execute_D2-> Output_Instruction-> dest_register = Execute_D2-> Input_Instruction->dest_register;
			Execute_D2-> Output_Instruction-> literal = Execute_D2-> Input_Instruction->literal;
			Execute_D2-> Output_Instruction-> opcode = Execute_D2-> Input_Instruction->opcode;
			Execute_D2-> Output_Instruction-> z = Execute_D2-> Input_Instruction-> z;
			Execute_D2-> Output_Instruction-> x = Execute_D2-> Input_Instruction-> x;
			Execute_D2-> Output_Instruction-> y = Execute_D2-> Input_Instruction-> y;
			Execute_D2-> Output_Instruction-> buffer = Execute_D2-> Input_Instruction-> buffer;

			Execute_D3->Input_Instruction-> instruction_string = Execute_D2->Output_Instruction->instruction_string;
			Execute_D3->Input_Instruction-> PC = Execute_D2-> Output_Instruction-> PC;
			Execute_D3->Input_Instruction-> source_register1 = Execute_D2-> Output_Instruction->source_register1;
			Execute_D3->Input_Instruction-> source_register2 = Execute_D2-> Output_Instruction->source_register2;
			Execute_D3->Input_Instruction-> dest_register = Execute_D2-> Output_Instruction->dest_register;
			Execute_D3->Input_Instruction-> literal = Execute_D2-> Output_Instruction-> literal;
			Execute_D3->Input_Instruction-> opcode = Execute_D2-> Output_Instruction-> opcode;
			Execute_D3-> Input_Instruction-> z = Execute_D2-> Output_Instruction-> z;
			Execute_D3-> Input_Instruction-> x = Execute_D2-> Output_Instruction-> x;
			Execute_D3-> Input_Instruction-> y = Execute_D2-> Output_Instruction-> y;
			Execute_D3-> Input_Instruction-> buffer = Execute_D2-> Output_Instruction-> buffer;
			//Memory -> init = true;
		}
		
		
		if(Execute_M2-> Input_Instruction->opcode == "MUL" && !Execute_M2-> Output_Instruction->stalled) //mul2
		{
			//cout<< "MUL2 normal testing"<< endl;
			Execute_M2-> Output_Instruction-> instruction_string = Execute_M2-> Input_Instruction->instruction_string;
			Execute_M2-> Output_Instruction-> PC = Execute_M2-> Input_Instruction-> PC;	
			Execute_M2-> Output_Instruction-> source_register1 = Execute_M2-> Input_Instruction->source_register1;
			Execute_M2-> Output_Instruction-> source_register2 = Execute_M2-> Input_Instruction->source_register2;
			Execute_M2-> Output_Instruction-> dest_register = Execute_M2-> Input_Instruction->dest_register;
			Execute_M2-> Output_Instruction-> literal = Execute_M2-> Input_Instruction->literal;
			Execute_M2-> Output_Instruction-> opcode = Execute_M2-> Input_Instruction->opcode;
			Execute_M2-> Output_Instruction-> z = Execute_M2-> Input_Instruction-> z;
			Execute_M2-> Output_Instruction-> x = Execute_M2-> Input_Instruction-> x;
			Execute_M2-> Output_Instruction-> y = Execute_M2-> Input_Instruction-> y;
			Execute_M2-> Output_Instruction-> buffer = Execute_M2-> Input_Instruction-> buffer;

			/*Memory->Input_Instruction-> instruction_string = Execute_M2->Output_Instruction->instruction_string;
			Memory->Input_Instruction-> PC = Execute_M2-> Output_Instruction-> PC;
			Memory->Input_Instruction-> source_register1 = Execute_M2-> Output_Instruction->source_register1;
			Memory->Input_Instruction-> source_register2 = Execute_M2-> Output_Instruction->source_register2;
			Memory->Input_Instruction-> dest_register = Execute_M2-> Output_Instruction->dest_register;
			Memory->Input_Instruction-> literal = Execute_M2-> Output_Instruction-> literal;
			Memory->Input_Instruction-> opcode = Execute_M2-> Output_Instruction-> opcode;
			Memory-> Input_Instruction-> z = Execute_M2-> Output_Instruction-> z;
			Memory-> Input_Instruction-> x = Execute_M2-> Output_Instruction-> x;
			Memory-> Input_Instruction-> y = Execute_M2-> Output_Instruction-> y;
			Memory-> Input_Instruction-> buffer = Execute_M2-> Output_Instruction-> buffer;
			Memory -> init = true;*/
			//cout<< "d4 output opcode: "<<Execute_D4-> Output_Instruction-> opcode<<endl;

			if (Execute_D4-> Output_Instruction-> opcode == "nop")
			{
				
				Memory->Input_Instruction->source_register1= Execute_M2->Output_Instruction->source_register1;
				Memory->Input_Instruction->source_register2= Execute_M2->Output_Instruction->source_register2;
				Memory->Input_Instruction->dest_register = Execute_M2->Output_Instruction->dest_register;
				Memory->Input_Instruction->literal = Execute_M2->Output_Instruction->literal;
				Memory->Input_Instruction-> opcode = Execute_M2->Output_Instruction-> opcode;
				Memory->Input_Instruction-> PC = Execute_M2->Output_Instruction-> PC;
				Memory->Input_Instruction-> store_target = Execute_M2->Output_Instruction-> store_target;
				Memory->Input_Instruction-> load_target =  Execute_M2->Output_Instruction-> load_target;
				Memory->Input_Instruction-> z = Execute_M2-> Output_Instruction-> z;
				Memory->Input_Instruction-> x = Execute_M2-> Output_Instruction-> x;
				Memory->Input_Instruction-> y = Execute_M2-> Output_Instruction-> y;
				Memory->Input_Instruction-> buffer = Execute_M2 -> Output_Instruction -> buffer;
				Memory->Input_Instruction->instruction_string = Execute_M2->Output_Instruction->instruction_string;
				cout << "Check!!" << endl;
			}
			else
			{
				//cout<< "first test div"<< endl;
				//Mem_MUL->Input_Instruction->source_register1= Execute_M2->Output_Instruction->source_register1;
				//cout<< "Testing DIVVVV"<< endl;
				Mem_MUL->Input_Instruction->source_register2= Execute_M2->Output_Instruction->source_register2;
				
				Mem_MUL->Input_Instruction->dest_register = Execute_M2->Output_Instruction->dest_register;
				Mem_MUL->Input_Instruction->literal = Execute_M2->Output_Instruction->literal;
					
				Mem_MUL->Input_Instruction-> opcode = Execute_M2->Output_Instruction-> opcode;
				Mem_MUL->Input_Instruction-> PC = Execute_M2->Output_Instruction-> PC;
				Mem_MUL->Input_Instruction-> store_target = Execute_M2->Output_Instruction-> store_target;
				Mem_MUL->Input_Instruction-> load_target =  Execute_M2->Output_Instruction-> load_target;
				Mem_MUL->Input_Instruction-> z = Execute_M2-> Output_Instruction-> z;
				Mem_MUL->Input_Instruction-> x = Execute_M2-> Output_Instruction-> x;
				Mem_MUL->Input_Instruction-> y = Execute_M2-> Output_Instruction-> y;
				Mem_MUL->Input_Instruction-> buffer = Execute_M2 -> Output_Instruction -> buffer ;
				Mem_MUL->Input_Instruction->instruction_string = Execute_M2->Output_Instruction->instruction_string;
				Mem_MUL->Input_Instruction->stalled = false;
				Execute_M2 -> Output_Instruction -> stalled = true;
				m_stall_2 = true;
				//cout << "Testing " << endl;
			}
			Memory -> init = true;
			//cout << "MUL 2 - " << Memory->Input_Instruction-> opcode << endl;
		}
		if(Execute_M2-> Input_Instruction->opcode == "nop" && Execute_M2-> Output_Instruction-> instruction_string != "nop" && !Execute_M2-> Output_Instruction->stalled) //mul 2 nop
		{
			//cout<< "MUL2 nop testing"<< endl;
			Execute_M2-> Output_Instruction-> instruction_string = Execute_M2-> Input_Instruction->instruction_string;
			Execute_M2-> Output_Instruction-> PC = Execute_M2-> Input_Instruction-> PC;	
			Execute_M2-> Output_Instruction-> source_register1 = Execute_M2-> Input_Instruction->source_register1;
			Execute_M2-> Output_Instruction-> source_register2 = Execute_M2-> Input_Instruction->source_register2;
			Execute_M2-> Output_Instruction-> dest_register = Execute_M2-> Input_Instruction->dest_register;
			Execute_M2-> Output_Instruction-> literal = Execute_M2-> Input_Instruction->literal;
			Execute_M2-> Output_Instruction-> opcode = Execute_M2-> Input_Instruction->opcode;
			Execute_M2-> Output_Instruction-> z = Execute_M2-> Input_Instruction-> z;
			Execute_M2-> Output_Instruction-> x = Execute_M2-> Input_Instruction-> x;
			Execute_M2-> Output_Instruction-> y = Execute_M2-> Input_Instruction-> y;
			Execute_M2-> Output_Instruction-> buffer = Execute_M2-> Input_Instruction-> buffer;

			if(Execute_D4-> Output_Instruction-> opcode == "nop")
			{
				Memory->Input_Instruction-> instruction_string = Execute_M2->Output_Instruction->instruction_string;
				Memory->Input_Instruction-> PC = Execute_M2-> Output_Instruction-> PC;
				Memory->Input_Instruction-> source_register1 = Execute_M2-> Output_Instruction->source_register1;
				Memory->Input_Instruction-> source_register2 = Execute_M2-> Output_Instruction->source_register2;
				Memory->Input_Instruction-> dest_register = Execute_M2-> Output_Instruction->dest_register;
				Memory->Input_Instruction-> literal = Execute_M2-> Output_Instruction-> literal;
				Memory->Input_Instruction-> opcode = Execute_M2-> Output_Instruction-> opcode;
				Memory-> Input_Instruction-> z = Execute_M2-> Output_Instruction-> z;
				Memory-> Input_Instruction-> x = Execute_M2-> Output_Instruction-> x;
				Memory-> Input_Instruction-> y = Execute_M2-> Output_Instruction-> y;
				Memory-> Input_Instruction-> buffer = Execute_M2-> Output_Instruction-> buffer;
				//Memory -> init = true;
			}
			mul_stall = true;
		}
			//cout<< "exec D1 stalled is "<< Execute_D1-> Input_Instruction-> stalled << endl;
		if(!Execute_D1-> Input_Instruction-> stalled ) //div 1
		{      	
			//cout<< "DIV TESTING" << endl;
			int x,y,z,buffer;
			Execute_D1-> Output_Instruction->instruction_string = Execute_D1-> Input_Instruction->instruction_string;
			Execute_D1-> Output_Instruction-> PC = Execute_D1-> Input_Instruction-> PC;
			Execute_D1-> Output_Instruction->source_register1 = Execute_D1-> Input_Instruction->source_register1;
			//cout<<"suvvi is"<< Execute_D1-> Output_Instruction->source_register1 << endl;
			Execute_D1-> Output_Instruction->source_register2 = Execute_D1-> Input_Instruction->source_register2; 
			Execute_D1-> Output_Instruction->dest_register = Execute_D1-> Input_Instruction->dest_register;
			Execute_D1-> Output_Instruction->literal = Execute_D1-> Input_Instruction->literal;
			Execute_D1-> Output_Instruction->opcode = Execute_D1-> Input_Instruction->opcode;
			Execute_D1-> Output_Instruction-> x = Execute_D1-> Input_Instruction-> x;
			Execute_D1-> Output_Instruction-> y = Execute_D1-> Input_Instruction-> y;
			Execute_D1-> Output_Instruction-> z = Execute_D1-> Input_Instruction-> z;
		
			//cout << "Opcode - "  << Execute_D1-> Output_Instruction->opcode << endl;
			if (Execute_D1-> Output_Instruction->opcode == "DIV")
			{
				x = Execute_D1->Output_Instruction->source_register1;
				//cout<< "source reg1 value is" << x << endl;

				y = Execute_D1->Output_Instruction ->source_register2;
				//cout<< "source reg2 value is" << y << endl; 

				z = atoi(Execute_D1->Output_Instruction-> dest_register.substr(1).c_str());
				buffer =  x / y;
				//cout << "buffer value is " << buffer << endl;
			}
			else if(Execute_D1->Output_Instruction->opcode == "HALT")
			{
				//cout << "Halt Testing" << endl;		
				exe_halt = true;
			}


			Execute_D2-> Input_Instruction->instruction_string = Execute_D1-> Output_Instruction->instruction_string;
			Execute_D2-> Input_Instruction->PC = Execute_D1-> Output_Instruction->PC;
			Execute_D2-> Input_Instruction->source_register1 = Execute_D1-> Output_Instruction->source_register1;
			Execute_D2-> Input_Instruction->source_register2 = Execute_D1-> Output_Instruction->source_register2;
			Execute_D2-> Input_Instruction->dest_register = Execute_D1-> Output_Instruction->dest_register;
			Execute_D2-> Input_Instruction->literal = Execute_D1-> Output_Instruction->literal;
			Execute_D2-> Input_Instruction->opcode = Execute_D1-> Output_Instruction->opcode;
			Execute_D2-> Input_Instruction-> z = Execute_D1-> Output_Instruction-> z;
			Execute_D2-> Input_Instruction-> x = Execute_D1-> Output_Instruction-> x;
			Execute_D2-> Input_Instruction-> y = Execute_D1-> Output_Instruction-> y;
			Execute_D2-> Input_Instruction-> buffer = buffer;
			
			//cout<< "buffer is" << Execute_D2-> Input_Instruction-> buffer << endl;
		}
		else //div1 nop
  		{
			Execute_D1-> Output_Instruction->instruction_string = "nop";
			Execute_D1-> Output_Instruction->opcode = "nop";
			//cout  << "Testing DIV" << endl;
			
			Execute_D2-> Input_Instruction->instruction_string = "nop";
			Execute_D2-> Input_Instruction->opcode = "nop";
		}	

		if(Execute_M1-> Input_Instruction -> opcode == "MUL" && !Execute_M1-> Input_Instruction-> stalled ) //mul 1
		{
			int x,y,z,buffer;
			Execute_M1-> Output_Instruction->instruction_string = Execute_M1-> Input_Instruction->instruction_string;
			Execute_M1-> Output_Instruction-> PC = Execute_M1-> Input_Instruction-> PC;
			Execute_M1-> Output_Instruction->source_register1 = Execute_M1-> Input_Instruction->source_register1;
			//cout<<"suvvi is"<< Execute_M1-> Output_Instruction->source_register1 << endl;
			Execute_M1-> Output_Instruction->source_register2 = Execute_M1-> Input_Instruction->source_register2; 
			Execute_M1-> Output_Instruction->dest_register = Execute_M1-> Input_Instruction->dest_register;
			Execute_M1-> Output_Instruction->literal = Execute_M1-> Input_Instruction->literal;
			Execute_M1-> Output_Instruction->opcode = Execute_M1-> Input_Instruction->opcode;
			Execute_M1-> Output_Instruction-> x = Execute_M1-> Input_Instruction-> x;
			Execute_M1-> Output_Instruction-> y = Execute_M1-> Input_Instruction-> y;
			Execute_M1-> Output_Instruction-> z = Execute_M1-> Input_Instruction-> z;
		
			
			if (Execute_M1-> Input_Instruction->opcode == "MUL")
			{
				x = Execute_M1->Output_Instruction->source_register1;
				//cout<< "source reg1 value is" << x << endl;

				y = Execute_M1->Output_Instruction ->source_register2;
				//cout<< "source reg2 value is" << y << endl; 

				z = atoi(Execute_M1->Output_Instruction-> dest_register.substr(1).c_str());
				buffer =  x * y;
				//cout << "buffer value is " << buffer << endl;
			}




			Execute_M2-> Input_Instruction->instruction_string = Execute_M1-> Output_Instruction->instruction_string;
			Execute_M2-> Input_Instruction->PC = Execute_M1-> Output_Instruction->PC;
			Execute_M2-> Input_Instruction->source_register1 = Execute_M1-> Output_Instruction->source_register1;
			Execute_M2-> Input_Instruction->source_register2 = Execute_M1-> Output_Instruction->source_register2;
			Execute_M2-> Input_Instruction->dest_register = Execute_M1-> Output_Instruction->dest_register;
			Execute_M2-> Input_Instruction->literal = Execute_M1-> Output_Instruction->literal;
			Execute_M2-> Input_Instruction->opcode = Execute_M1-> Output_Instruction->opcode;
			Execute_M2-> Input_Instruction-> z = Execute_M1-> Output_Instruction-> z;
			Execute_M2-> Input_Instruction-> x = Execute_M1-> Output_Instruction-> x;
			Execute_M2-> Input_Instruction-> y = Execute_M1-> Output_Instruction-> y;
			Execute_M2-> Input_Instruction-> buffer = buffer;

			if (m_stall_2)
			{
				m_stall_1 = true;
				Execute_M2-> Input_Instruction -> stalled = true;
				Execute_M1-> Output_Instruction -> stalled = true;
			}
			
			//cout<< "buffer is" << Execute_M2-> Input_Instruction-> buffer << endl;
		}
		
		if (mul_dep && !Execute_M1-> Output_Instruction-> stalled)
		{
			Execute_M1->Output_Instruction -> instruction_string = "nop";
			Execute_M2->Input_Instruction -> instruction_string = "nop";
			Execute_M1->Output_Instruction -> opcode = "nop";
			Execute_M2->Input_Instruction -> opcode = "nop";
		}
		else if(Execute_M1-> Input_Instruction -> opcode != "MUL" && !Execute_M1-> Input_Instruction-> stalled && !Execute_M1-> Output_Instruction->stalled) //mul1 nop
		{
			int x,y,z,buffer;
			Execute_M1-> Output_Instruction->instruction_string = Execute_M1-> Input_Instruction->instruction_string;
			Execute_M1-> Output_Instruction-> PC = Execute_M1-> Input_Instruction-> PC;
			Execute_M1-> Output_Instruction->source_register1 = Execute_M1-> Input_Instruction->source_register1;
			Execute_M1-> Output_Instruction->source_register2 = Execute_M1-> Input_Instruction->source_register2; 
			Execute_M1-> Output_Instruction->dest_register = Execute_M1-> Input_Instruction->dest_register;
			Execute_M1-> Output_Instruction->literal = Execute_M1-> Input_Instruction->literal;
			Execute_M1-> Output_Instruction->opcode = Execute_M1-> Input_Instruction->opcode;
			//cout  << "Testing MUL" << endl;
			
			Execute_M1-> Output_Instruction-> x = Execute_M1-> Input_Instruction-> x;
			Execute_M1-> Output_Instruction-> y = Execute_M1-> Input_Instruction-> y;
			Execute_M1-> Output_Instruction-> z = Execute_M1-> Input_Instruction-> z;
			
		

			Execute_M2-> Input_Instruction->instruction_string = Execute_M1-> Output_Instruction->instruction_string;
			Execute_M2-> Input_Instruction->PC = Execute_M1-> Output_Instruction->PC;
			Execute_M2-> Input_Instruction->source_register1 = Execute_M1-> Output_Instruction->source_register1;
			Execute_M2-> Input_Instruction->source_register2 = Execute_M1-> Output_Instruction->source_register2;
			Execute_M2-> Input_Instruction->dest_register = Execute_M1-> Output_Instruction->dest_register;
			Execute_M2-> Input_Instruction->literal = Execute_M1-> Output_Instruction->literal;
			Execute_M2-> Input_Instruction->opcode = Execute_M1-> Output_Instruction->opcode;
			Execute_M2-> Input_Instruction-> z = Execute_M1-> Output_Instruction-> z;
			Execute_M2-> Input_Instruction-> x = Execute_M1-> Output_Instruction-> x;
			Execute_M2-> Input_Instruction-> y = Execute_M1-> Output_Instruction-> y;
			Execute_M2-> Input_Instruction-> buffer = buffer;
		}
								
									



                if (!Execute->Input_Instruction->stalled && !Execute->Output_Instruction->stalled) //normal IFU
		{                                                     
			int x, y, z, l;
			int buffer;
			string opcode;
			
			Execute->Output_Instruction->instruction_string = Execute->Input_Instruction->instruction_string;
			Execute->Output_Instruction->source_register1 = Execute->Input_Instruction->source_register1;
			Execute->Output_Instruction->source_register2 = Execute->Input_Instruction->source_register2;
			Execute->Output_Instruction->dest_register = Execute->Input_Instruction->dest_register;
			Execute->Output_Instruction->literal = Execute->Input_Instruction->literal;
			Execute->Output_Instruction->opcode= Execute->Input_Instruction->opcode;
			Execute->Output_Instruction-> PC = Execute->Input_Instruction-> PC;

			Execute->Output_Instruction->x = Execute-> Input_Instruction-> x;
			Execute->Output_Instruction->y = Execute-> Input_Instruction-> y;
			Execute->Output_Instruction->z = Execute-> Input_Instruction-> z;
			
			opcode = Execute->Output_Instruction->opcode;
			//cout<<"literal is----"<< Execute->Input_Instruction->literal<<endl;

			if((opcode == "ADD") || (opcode == "SUB") || (opcode == "AND") || (opcode == "XOR") || (opcode == "OR"))
			{
				  x = Execute->Output_Instruction->source_register1;
				 //cout<< "source reg1 num is" << x << endl;

				  y = Execute -> Output_Instruction -> source_register2;
				 //cout<< "source reg2 num is" << y << endl; 

				 z = atoi(Execute->Output_Instruction-> dest_register.substr(1).c_str());       
				 //cout<< "dest reg num is" << z << endl;    

				 if(Execute->Output_Instruction->opcode == "ADD"){
					  buffer =  x + y;
					 // cout<< "added value is" << buffer <<endl;
		 		 }
		 		else if(Execute->Output_Instruction->opcode == "SUB"){
					 buffer =  x - y;
					 //cout<< "subtracted value is" << buffer << endl; 
			 	}

				else if(Execute->Output_Instruction->opcode == "MUL"){
					  buffer =  x * y;
					//  cout<< "multiplied value is" << buffer; 

				}
				else if(Execute->Output_Instruction->opcode == "OR"){
				        buffer = (x || y);
				  //cout<<" OR value is" <<buffer;
				 }
				else if(Execute->Output_Instruction->opcode == "XOR"){
					buffer = (x ^ y);
					//cout<<" XOR value is" <<buffer;
				 }
				else if(Execute->Output_Instruction->opcode == "AND"){
					 buffer = (x & y);
					 //cout<<" AND value is" <<buffer;
				}
		 
			}
			else if(Execute->Output_Instruction->opcode == "MOVC"){
				 z = atoi(Execute->Output_Instruction->dest_register.substr(1).c_str());       
				 // cout<< "dest reg num is" << z << endl;     

				 int l = Execute->Output_Instruction->literal;
				 //cout<< "literal num  is" << l << endl; 
				  
				 buffer = l;
				 //cout<<" MOVC value is" << buffer<<endl;
			}

			else if(Execute->Output_Instruction->opcode == "LOAD"){
				 x = Execute->Output_Instruction->source_register1;
				 //cout<< "source reg1 num is" << x << endl;
					  
				  l = Execute->Output_Instruction->literal;
				 //cout<< "literal num  is" << l << endl; 
					 
				  z = atoi(Execute->Output_Instruction->dest_register.substr(1).c_str());       
				 //cout<< "dest reg num is" << z << endl;
					 
				 Execute->Output_Instruction-> load_target =  x + l;
				 //cout<<" LOAD target address is" << Execute->Input_Instruction-> load_target << endl;
		 
		 	} 

			else if(Execute->Output_Instruction->opcode == "STORE"){
				  x = Execute->Output_Instruction->source_register1;
				  //cout<< "source reg1 num is" << x << endl;
				  
				  y = Execute -> Output_Instruction -> source_register2;
				  //cout<< "source reg2 num is" << y << endl; 

				  l = Execute->Output_Instruction->literal;
				  //cout<< "literal num  is" << l << endl; 
				  
				  Execute->Output_Instruction-> store_target  = y + l;
				  //cout<< "STORE target address is" << Execute->Output_Instruction-> store_target<< endl ;
		  
		   	}   


			else if((Execute->Output_Instruction->opcode == "BZ") || (Execute->Output_Instruction->opcode == "BNZ"))
			{
				l = Execute->Output_Instruction->literal;
				x = Execute->Output_Instruction->source_register1;
				if(Execute->Output_Instruction->opcode == "BZ")
				{
					if( flag1 -> zero)
					{
					 	if(branch_negative) 
						{
							
							buffer = Execute->Output_Instruction-> PC - l;
						}
						else
						{
							buffer = Execute->Output_Instruction-> PC + l;
							//cout << "Buffer value - " << buffer << endl;
						}
						exe_branch = true;
						Execute -> Output_Instruction -> buffer = buffer;
					
					}		
					else 
					{
					 	exe_branch = false;
					}
				}
				else
				{
					if(!flag1 -> zero)
					{
						if(branch_negative)
						{
							buffer = Execute->Output_Instruction-> PC - l;
						}
						else
						{
							buffer = Execute->Output_Instruction-> PC + l;
						}
						exe_branch = true;
						Execute -> Output_Instruction -> buffer = buffer;
					}
					else
					{
						exe_branch = false;
					}
				}
			}
			else if(Execute->Output_Instruction->opcode == "JUMP")
			{
				int l = Execute->Output_Instruction->literal;
				x = Execute->Output_Instruction->source_register1;
				//cout<<"x is:" << x << endl ;
				
				if(jump_negative)
				{
					buffer = x - l;
					//cout<< "buffer for jump is:" << buffer<< endl;
					exe_jump = true;
				}
				else
				{
					buffer = x + l;
					//cout<< "buffer for jump is:" << buffer<< endl;
					exe_jump = true;
				}
			}
			else if(Execute->Output_Instruction->opcode == "JAL")
			{
				int l = Execute->Output_Instruction->literal;
				x = Execute->Output_Instruction->source_register1;
				//cout<< "literal for jal is:" << l<< endl;
				if(jump_negative)
				{
					buffer = x - l ;
					exe_jump = true;	
					jaltarget = Execute->Output_Instruction-> PC + 4;
					//cout<< "x is:" << x << endl;
					//cout<< "jal target is:"<< jaltarget << endl;
					//cout<< "buffer for jal is:" << buffer<< endl;
				}														        
				else 
				{
					buffer = x + l ;
					exe_jump = true;
					jaltarget = Execute->Output_Instruction-> PC + 4;
					//cout<< "x is:" << x << endl;
					//cout<< "buffer for jal is:" << buffer<< endl;
					//cout<< "jal target is:"<< jaltarget << endl;
					
				}
				Execute -> Output_Instruction -> buffer = buffer;
			}

			//cout << "Execute_M2-> Output_Instruction-> buffer - " << Execute_M2-> Output_Instruction-> buffer << endl;


			Execute -> Output_Instruction -> buffer = buffer;
			if (Execute_M2-> Output_Instruction-> opcode == "nop" && Execute_D4-> Output_Instruction-> opcode == "nop" )
			{
				Memory->Input_Instruction->source_register1= Execute->Output_Instruction->source_register1;
				Memory->Input_Instruction->source_register2= Execute->Output_Instruction->source_register2;
				Memory->Input_Instruction->dest_register = Execute->Output_Instruction->dest_register;
				Memory->Input_Instruction->literal = Execute->Output_Instruction->literal;
				Memory->Input_Instruction-> opcode = Execute->Output_Instruction-> opcode;
				Memory->Input_Instruction-> PC = Execute->Output_Instruction-> PC;
				Memory->Input_Instruction-> store_target = Execute->Output_Instruction-> store_target;
				Memory->Input_Instruction-> load_target =  Execute->Output_Instruction-> load_target;
				Memory->Input_Instruction-> z = Execute-> Output_Instruction-> z;
				Memory->Input_Instruction-> x = Execute-> Output_Instruction-> x;
				Memory->Input_Instruction-> y = Execute-> Output_Instruction-> y;
				Memory->Input_Instruction-> buffer = Execute -> Output_Instruction -> buffer;
				Memory->Input_Instruction->instruction_string = Execute->Output_Instruction->instruction_string;
				//cout << "Check!!" << endl;
			}
			else if (Execute->Output_Instruction->instruction_string != "nop")
			{
				Mem_IFU->Input_Instruction->source_register1= Execute->Output_Instruction->source_register1;
				Mem_IFU->Input_Instruction->source_register2= Execute->Output_Instruction->source_register2;
				Mem_IFU->Input_Instruction->dest_register = Execute->Output_Instruction->dest_register;
				Mem_IFU->Input_Instruction->literal = Execute->Output_Instruction->literal;
				Mem_IFU->Input_Instruction-> opcode = Execute->Output_Instruction-> opcode;
				Mem_IFU->Input_Instruction-> PC = Execute->Output_Instruction-> PC;
				Mem_IFU->Input_Instruction-> store_target = Execute->Output_Instruction-> store_target;
				Mem_IFU->Input_Instruction-> load_target =  Execute->Output_Instruction-> load_target;
				Mem_IFU->Input_Instruction-> z = Execute-> Output_Instruction-> z;
				Mem_IFU->Input_Instruction-> x = Execute-> Output_Instruction-> x;
				Mem_IFU->Input_Instruction-> y = Execute-> Output_Instruction-> y;
				Mem_IFU->Input_Instruction-> buffer = Execute -> Output_Instruction -> buffer ;
				Mem_IFU->Input_Instruction->instruction_string = Execute->Output_Instruction->instruction_string;
				Mem_IFU->Input_Instruction->stalled = false;
				Execute -> Output_Instruction -> stalled = true;
				Execution_Stall = true;
				//cout << "Testing" << endl;
			}
			Memory -> init = true;

			//cout<<"opcode is"<< Execute->Output_Instruction->opcode <<endl;
		}
		
		else if (Execute->Input_Instruction->stalled && !Execute->Output_Instruction->stalled)
		{
			//cout << "Check!!" << endl;
			Execute->Output_Instruction->instruction_string = "nop";
			Execute->Output_Instruction->opcode= "nop";

			if (Execute_M2-> Output_Instruction-> opcode == "nop")
			{
				Memory->Input_Instruction-> opcode = Execute->Output_Instruction-> opcode;
				Memory->Input_Instruction->instruction_string = Execute->Output_Instruction->instruction_string;
				
			}
			else
			{
				Mem_IFU->Input_Instruction-> opcode = Execute->Output_Instruction-> opcode;
				Mem_IFU->Input_Instruction->instruction_string = Execute->Output_Instruction->instruction_string;
			}
		}

		if (div_stall && m_stall_2)
		{
			m_stall_1 = false;
			m_stall_2 = false;
			Execute_M2-> Output_Instruction -> stalled = false;
			Execute_M2 -> Input_Instruction -> stalled = false;
			Execute_M1-> Output_Instruction -> stalled = false;
		}
		if ((mul_stall || div_stall) && Execute_M2-> Output_Instruction-> opcode == "nop" && Execute_D4-> Output_Instruction-> opcode == "nop")
		{
			Execution_Stall = false;
			Execute -> Output_Instruction -> stalled = false;
		}
	}

}



void memory(stage* Memory,Register_File* re, Data_Memory* dm, stage* WriteBack, stage* Mem_IFU,stage* Mem_MUL){

	if (Memory -> init)
	{
		int p;
		int q;
		if (Memory->Input_Instruction->opcode != "nop")
		{
			Memory->Output_Instruction->instruction_string = Memory->Input_Instruction->instruction_string;
			Memory->Output_Instruction->source_register1 = Memory->Input_Instruction->source_register1;
			Memory->Output_Instruction->source_register2 = Memory->Input_Instruction->source_register2;
			Memory->Output_Instruction->dest_register = Memory->Input_Instruction->dest_register;
			Memory->Output_Instruction->literal = Memory->Input_Instruction->literal;
			Memory->Output_Instruction->opcode = Memory->Input_Instruction->opcode;
			Memory->Output_Instruction-> PC = Memory -> Input_Instruction-> PC;
			Memory->Output_Instruction-> store_target = Memory->Input_Instruction-> store_target;
			Memory->Output_Instruction-> load_target = Memory->Input_Instruction-> load_target;
			Memory->Output_Instruction-> z= Memory->Input_Instruction-> z;
			Memory->Output_Instruction-> x= Memory->Input_Instruction-> x;
			Memory->Output_Instruction-> y= Memory->Input_Instruction-> y;
			Memory->Output_Instruction-> buffer = Memory->Input_Instruction-> buffer ;
			//cout << "Mul Check" << endl;
		}
		else if(Mem_MUL->Input_Instruction->opcode != "nop")
		{
			Memory->Output_Instruction->instruction_string = Mem_MUL->Input_Instruction->instruction_string;
			Memory->Output_Instruction->source_register1 = Mem_MUL->Input_Instruction->source_register1;
			Memory->Output_Instruction->source_register2 = Mem_MUL->Input_Instruction->source_register2;
			Memory->Output_Instruction->dest_register = Mem_MUL->Input_Instruction->dest_register;
			Memory->Output_Instruction->literal = Mem_MUL->Input_Instruction->literal;
			Memory->Output_Instruction->opcode = Mem_MUL->Input_Instruction->opcode;
			Memory->Output_Instruction-> PC = Mem_MUL -> Input_Instruction-> PC;
			Memory->Output_Instruction-> store_target = Mem_MUL->Input_Instruction-> store_target;
			Memory->Output_Instruction-> load_target = Mem_MUL->Input_Instruction-> load_target;
			Memory->Output_Instruction-> z= Mem_MUL->Input_Instruction-> z;
			Memory->Output_Instruction-> x= Mem_MUL->Input_Instruction-> x;
			Memory->Output_Instruction-> y= Mem_MUL->Input_Instruction-> y;
			Memory->Output_Instruction-> buffer = Mem_MUL->Input_Instruction-> buffer ;
			Mem_MUL->Input_Instruction-> stalled = true;
			Mem_MUL->Input_Instruction->instruction_string = "nop";
			Mem_MUL->Input_Instruction->opcode = "nop";	


		}
		else
		{
			Memory->Output_Instruction->instruction_string = Mem_IFU->Input_Instruction->instruction_string;
			Memory->Output_Instruction->source_register1 = Mem_IFU->Input_Instruction->source_register1;
			Memory->Output_Instruction->source_register2 = Mem_IFU->Input_Instruction->source_register2;
			Memory->Output_Instruction->dest_register = Mem_IFU->Input_Instruction->dest_register;
			Memory->Output_Instruction->literal = Mem_IFU->Input_Instruction->literal;
			Memory->Output_Instruction->opcode = Mem_IFU->Input_Instruction->opcode;
			Memory->Output_Instruction-> PC = Mem_IFU -> Input_Instruction-> PC;
			Memory->Output_Instruction-> store_target = Mem_IFU->Input_Instruction-> store_target;
			Memory->Output_Instruction-> load_target = Mem_IFU->Input_Instruction-> load_target;
			Memory->Output_Instruction-> z= Mem_IFU->Input_Instruction-> z;
			Memory->Output_Instruction-> x= Mem_IFU->Input_Instruction-> x;
			Memory->Output_Instruction-> y= Mem_IFU->Input_Instruction-> y;
			Memory->Output_Instruction-> buffer = Mem_IFU->Input_Instruction-> buffer ;
			Mem_IFU->Input_Instruction-> stalled = true;
			Mem_IFU->Input_Instruction->instruction_string = "nop";
			Mem_IFU->Input_Instruction->opcode = "nop";
			Mem_Stall = true;
			
		}
		//cout<< "store target from execute stage is"<< Execute->Output_Instruction-> store_target  << endl;

	  	if(Memory->Output_Instruction->opcode == "LOAD"){
			   int x= Memory->Output_Instruction-> load_target;
			   p = dm->data_mem[x];
			   Memory->Output_Instruction-> buffer = p;

			  // cout<< "computed memory addr:"<< p<< endl;
	    	}

	  	if(Memory->Output_Instruction->opcode == "STORE"){
			    int x = Memory->Output_Instruction->x;
			   //cout<< "source reg1 num is" << x << endl;
			   dm->data_mem[Memory->Output_Instruction-> store_target] = x;
			   //cout << "lalalal "<< Memory->Output_Instruction->store_target<< endl;
			   //cout<< "computed memory address for store is:"<< Memory->Input_Instruction-> store_target << endl;
	   	} 

		WriteBack->Input_Instruction->instruction_string = Memory->Output_Instruction->instruction_string;
		WriteBack->Input_Instruction->source_register1 = Memory->Output_Instruction->source_register1;
		WriteBack->Input_Instruction->source_register2 = Memory->Output_Instruction->source_register2;
		WriteBack->Input_Instruction->dest_register = Memory->Output_Instruction->dest_register;
		WriteBack->Input_Instruction->literal = Memory->Output_Instruction->literal;
		WriteBack->Input_Instruction-> opcode = Memory->Output_Instruction->opcode;
		WriteBack->Input_Instruction-> PC = Memory->Output_Instruction-> PC;
		WriteBack->Input_Instruction-> store_target= Memory->Output_Instruction-> store_target;
		WriteBack->Input_Instruction-> load_target = Memory->Output_Instruction-> load_target;
		WriteBack->Input_Instruction-> z= Memory->Output_Instruction-> z;
		WriteBack->Input_Instruction-> buffer= Memory->Output_Instruction-> buffer;
		WriteBack -> init = true;
		
	}
}



void writeback(stage* WriteBack,Register_File* re, Flags* flag1)
{
	if (WriteBack -> init)
	{
		WriteBack->Output_Instruction->instruction_string = WriteBack->Input_Instruction->instruction_string;
		WriteBack->Output_Instruction->source_register1 = WriteBack->Input_Instruction->source_register1;
		WriteBack->Output_Instruction->source_register2 = WriteBack->Input_Instruction->source_register2;
		WriteBack->Output_Instruction->dest_register = WriteBack->Input_Instruction->dest_register;
		WriteBack->Output_Instruction->literal = WriteBack->Input_Instruction->literal;
		WriteBack->Output_Instruction->opcode = WriteBack->Input_Instruction->opcode;
		WriteBack->Output_Instruction-> PC = WriteBack->Input_Instruction-> PC;
		WriteBack->Output_Instruction-> z = WriteBack->Input_Instruction-> z;
		WriteBack->Output_Instruction-> buffer = WriteBack->Input_Instruction-> buffer;
		//cout<<" buffer value - "<< WriteBack->Input_Instruction-> z << endl;
		if(WriteBack->Output_Instruction->opcode == "JAL")
		{
			//cout << "JAL Testing" << endl;
			re->reg[WriteBack->Output_Instruction-> z].value = jaltarget;
		}

	  	else if(WriteBack->Output_Instruction->opcode != "STORE" && WriteBack->Output_Instruction->opcode != "" && WriteBack->Output_Instruction->opcode != "nop" && WriteBack->Output_Instruction->opcode != "BZ" && WriteBack->Output_Instruction->opcode != "BNZ" && WriteBack->Output_Instruction->opcode != "JUMP" && WriteBack->Output_Instruction->opcode != "HALT")
		{
			//cout<< "dest reg written value - " << branch_pc <<endl;  
			re->reg[WriteBack->Output_Instruction-> z].value = WriteBack->Output_Instruction-> buffer; 
			if (branch_pc == WriteBack -> Output_Instruction -> PC)
			{
				if (WriteBack->Output_Instruction-> buffer == 0)
				{
					 flag1 -> zero = 1;
				}
				else if (WriteBack->Output_Instruction-> buffer != 0)
				{
					 flag1 -> zero = 0;
				}
			}
		     
	  	}
	}
}


int main(int argc, char* argv[])
{
	int cycle,stag,PC=4000,i=1;
	Register_File reg;
	RenameTable reg1;
	PhysicalRegister_File preg;
	
	fstream fIn;
	Data_Memory dmem;
	Code_Memory cmem;
	int k=0;
	string stage;
	int clock_cycle = 0;
	int total_cycles = 0;
	int index = 0;
        string filename;
	Flags flag;
	flag.zero = false;

	/*stage F;
	stage D;
	stage E;
	stage M;
	stage W;*/
	//Instruction_info ii; 
        cout<<"enter the filename to be read"<<endl;
        //getline(cin,filename);
        cin >> filename;
        fIn.open(filename, ios::in );
	//fIn.open( "sample.txt", ios::in );

	F.Input_Instruction = new Instruction_info;
	F.Output_Instruction = new Instruction_info;

	D.Input_Instruction = new Instruction_info;
	D.Output_Instruction = new Instruction_info;
	
	E.Input_Instruction = new Instruction_info;
	E.Output_Instruction = new Instruction_info;

	EM1.Input_Instruction = new Instruction_info;
	EM1.Input_Instruction -> instruction_string = "nop";
	EM1.Output_Instruction = new Instruction_info;
	EM1.Output_Instruction -> instruction_string = "nop";

	EM2.Output_Instruction = new Instruction_info;
	EM2.Input_Instruction = new Instruction_info;
	EM2.Input_Instruction -> instruction_string = "nop";
	EM2.Output_Instruction -> instruction_string = "nop";

	M.Input_Instruction = new Instruction_info;
	M.Output_Instruction = new Instruction_info;

	W.Input_Instruction = new Instruction_info;
	W.Output_Instruction = new Instruction_info;

	M_IFU.Input_Instruction = new Instruction_info;
	M_IFU.Output_Instruction = new Instruction_info;
	M_IFU.Input_Instruction -> stalled = true;
	M_IFU.Input_Instruction -> instruction_string = "nop";

	M_MUL.Input_Instruction = new Instruction_info;
	M_MUL.Output_Instruction = new Instruction_info;
	M_MUL.Input_Instruction -> stalled = true;
	M_MUL.Input_Instruction -> instruction_string = "nop";

	ED1.Input_Instruction = new Instruction_info;
	ED1.Output_Instruction = new Instruction_info;
	
	ED2.Input_Instruction = new Instruction_info;	
	ED2.Output_Instruction = new Instruction_info;		
		
	ED3.Input_Instruction = new Instruction_info;	
	ED3.Output_Instruction = new Instruction_info;		
	
	ED4.Input_Instruction = new Instruction_info;	
	ED4.Output_Instruction = new Instruction_info;

	IQ.Input_Instruction = new Instruction_info;	
	IQ.Output_Instruction = new Instruction_info;		
	
	


	if( fIn.is_open() )
	{
		string s;
		while( getline( fIn, s )){
		    //cout << s << endl;
		    cmem.code[k].instruction_string = s;
		    cmem.code[k].address = 4000 + ((k + 1) * 4);
			k++;
			 //cout <<"code memory initialised"<< k << endl;
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
		cout<<"enter the stage to be activated 1.Initialisation 2.simulation 3.display 4.quit";
		cin>>stag;

	   	if(stag==1){
			cout<<"Initialisation completed\n";
			int source_registers_value=0;         
			int dest_register_value=0;
			int address= address*4;
			int base_address=0;
	     		for(int i=0;i<=16;i++){
		      		reg.reg[i].value= 7;
		      		//cout<<"initial register values are"<<reg.reg[i].value << endl;
	      		}
			for( int p=0;p<32;p++)
			{
				preg.preg[i].value= 0;
				//cout<<"initial physical register values are"<<preg.preg[i].value << endl;
			}
	     		for(int j=0; j<4000; j+=4){
		      		dmem.data_mem[j]=20;
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
		    		writeback(&W,&reg,&flag);
                         	//cout<< "wb comp"<<endl;

		   		
				
				memory(&M,&reg,&dmem, &W,&M_IFU,&M_MUL); 
				//cout<< "mem comp"<<endl;
				if(Mem_Stall){
					E.Output_Instruction-> stalled = false;
					Mem_Stall = false;
					Execution_Stall = false;
					Decode_stall = false;
					
					if(!decode_dependency && !Execution_Stall)
					{
						E.Input_Instruction-> stalled = false;
						D.Output_Instruction-> stalled = false;
						D.Input_Instruction-> stalled = false;
	   				}
				}
		   		execute(&E,&M,&M_IFU,&M_MUL,&EM1,&EM2,&ED1,&ED2,&ED3,&ED4,&flag);		
				//cout<< "exe comp - " << exe_halt <<endl;
				if(exe_branch)
				{
					EM1.Input_Instruction->	instruction_string = "nop";
					EM1.Input_Instruction-> opcode = "nop";
					EM1.Input_Instruction-> stalled = false;

					E.Input_Instruction->instruction_string = "nop";
					E.Input_Instruction -> opcode = "nop";	
					E.Input_Instruction-> stalled = false;		
					
					D.Input_Instruction->instruction_string = "nop";
					D.Input_Instruction -> opcode = "nop";	
					D.Input_Instruction-> stalled = false;	
					
					D.Output_Instruction-> instruction_string = "nop";
					D.Output_Instruction-> opcode = "nop";
					D.Output_Instruction-> stalled = false;
							
					F.Input_Instruction->instruction_string = "nop";
					F.Input_Instruction -> opcode = "nop";	
					F.Input_Instruction-> stalled = false;	
					
					F.Output_Instruction-> instruction_string = "nop";
					F.Output_Instruction-> opcode = "nop";
					F.Output_Instruction-> stalled = false;
					
					exe_branch = false;
					PC = E.Output_Instruction-> buffer - 4;
				}			
				else if(exe_jump)
				{
					EM1.Input_Instruction->	instruction_string = "nop";
					EM1.Input_Instruction-> opcode = "nop";
					EM1.Input_Instruction-> stalled = false;

					E.Input_Instruction->instruction_string = "nop";
					E.Input_Instruction -> opcode = "nop";	
					E.Input_Instruction-> stalled = false;		
					
					D.Input_Instruction->instruction_string = "nop";
					D.Input_Instruction -> opcode = "nop";	
					D.Input_Instruction-> stalled = false;	
					
					D.Output_Instruction-> instruction_string = "nop";
					D.Output_Instruction-> opcode = "nop";
					D.Output_Instruction-> stalled = false;
							
					F.Input_Instruction->instruction_string = "nop";
					F.Input_Instruction -> opcode = "nop";	
					F.Input_Instruction-> stalled = false;	
					
					F.Output_Instruction-> instruction_string = "nop";
					F.Output_Instruction-> opcode = "nop";
					F.Output_Instruction-> stalled = false;
					exe_jump= false;
					PC = E.Output_Instruction-> buffer - 4;
				}
				else if(exe_halt)
				{
					
					EM1.Input_Instruction->	instruction_string = "nop";
					EM1.Input_Instruction-> opcode = "nop";
					EM1.Input_Instruction-> stalled = false;

					E.Input_Instruction->instruction_string = "nop";
					E.Input_Instruction -> opcode = "nop";	
					E.Input_Instruction-> stalled = false;		
					
					D.Input_Instruction->instruction_string = "nop";
					D.Input_Instruction -> opcode = "nop";	
					D.Input_Instruction-> stalled = false;	
					
					D.Output_Instruction-> instruction_string = "nop";
					D.Output_Instruction-> opcode = "nop";
					D.Output_Instruction-> stalled = false;
							
					F.Input_Instruction->instruction_string = "nop";
					F.Input_Instruction -> opcode = "nop";	
					F.Input_Instruction-> stalled = false;	
					
					F.Output_Instruction-> instruction_string = "nop";
					F.Output_Instruction-> opcode = "nop";
					F.Output_Instruction-> stalled = false;
				}	
				
				IssueQueue(&IQ,&E,&EM1,&EM2,&ED1,&ED2,&ED3,&ED4);
		   		decode(&D,&E,&EM1,&reg,&ED1);
				//cout<< "dec comp - "<< D.Output_Instruction -> stalled << endl;
				
		   		fetch(&F, &D);
				//cout<< "fet comp"<<endl;
				
				/*cout<< "suvvi2 is " <<D.Output_Instruction -> x<< endl;
				cout<< "suvvi3 is " <<D.Output_Instruction -> y<< endl;
				cout<< "suvvi4 is " <<D.Output_Instruction -> z<<endl;*/
				
				if(W.Output_Instruction -> opcode != "STORE" && W.Output_Instruction -> opcode != "nop" && W.Output_Instruction -> opcode != "BZ" && W.Output_Instruction -> opcode != "BNZ" && W.Output_Instruction -> opcode != "JUMP" && W.Output_Instruction -> opcode != "HALT"){
					
						 reg.reg[W.Output_Instruction -> z].status = true;

						 if (W.Output_Instruction -> opcode == "ADD" || W.Output_Instruction -> opcode == "SUB" || W.Output_Instruction -> opcode == "MUL" || W.Output_Instruction -> opcode == "DIV")
						{
							 reg.reg[16].status = true;
						}
						 //cout<< "writeback" << W.Output_Instruction -> z<< endl;
				}
				
				if(D.Output_Instruction -> stalled)
				{
					
					int destination;
					int x,y,z;
					
				        if( D.Output_Instruction -> dest_register == "")
					{
						if(D.Output_Instruction-> src_name1 != "" && D.Output_Instruction-> src_name2 != "")
						{
							if(reg.reg[D.Output_Instruction->x].status && reg.reg[D.Output_Instruction->y].status)
							{
								D.Output_Instruction -> stalled = false;
								decode_dependency = false;
								if(D.Output_Instruction -> opcode == "MUL")
								{
								 	EM1.Input_Instruction-> stalled = false;
								}									
								else if( D.Output_Instruction -> opcode == "DIV")
								{
									
								}
								else
								{
									E.Input_Instruction-> stalled = false;
								}
								if( D.Output_Instruction -> opcode != "STORE" || D.Output_Instruction -> opcode != "nop")
								{
									destination = D.Output_Instruction-> z;
									reg.reg[destination].status= false;
								}
							}
							if(reg.reg[D.Output_Instruction->x].status)
							{
								if(D.Output_Instruction -> opcode == "MUL")
								{
									EM1.Input_Instruction-> x   = reg.reg[D.Output_Instruction->x].value;
								}
								else if(D.Output_Instruction -> opcode == "DIV")
								{
								
								}
								else
								{
									E.Input_Instruction-> x = reg.reg[D.Output_Instruction->x].value;
								}
							}
							if(reg.reg[D.Output_Instruction->y].status)
							{
								if(D.Output_Instruction -> opcode == "MUL")
								{
									EM1.Input_Instruction-> y   = reg.reg[D.Output_Instruction->y].value;	
								}
								else if(D.Output_Instruction -> opcode == "DIV")
								{
								
								}
								else
								{
									E.Input_Instruction-> y = reg.reg[D.Output_Instruction->y].value;

								}
							}
						}
						else if(D.Output_Instruction-> src_name1 != "" && D.Output_Instruction-> src_name2 == "")
						{
							if(reg.reg[D.Output_Instruction->x].status)
							{
								// cout << "Condition Test" << endl;
								D.Output_Instruction -> stalled = false;
								decode_dependency = false;
								E.Input_Instruction-> stalled = false;
								/*if (D.Output_Instruction -> opcode == "BZ" || D.Output_Instruction -> opcode == "BNZ")
								{
									if (D.Output_Instruction -> opcode == "BZ")
									{
										if (reg.reg[16].value == 1)
										{
											exe_branch = true;
											cout << "Condition Test" << endl;
										}
										else
										{
											exe_branch = false;
										}
									}
									else
									{
										if (reg.reg[16].value == 0)
										{
											exe_branch = true;
										}
										else
										{
											exe_branch = false;
										}
									}
								}*/
								/*if(D.Output_Instruction -> opcode != "STORE" || D.Output_Instruction -> opcode != "nop")
								{
									destination = D.Output_Instruction-> z;
									reg.reg[destination].status= false;
								}*/	
								
							}
																			
						}
					}	
		
					else if(D.Output_Instruction -> dest_register != "")
					{
						if(D.Output_Instruction-> src_name1 != "" && D.Output_Instruction-> src_name2 != "")
						{
							//cout << reg.reg[D.Output_Instruction->x].status << " - " << D.Output_Instruction->x << endl;
							//cout << reg.reg[D.Output_Instruction->y].status << " - " << D.Output_Instruction->y << endl;
							//cout << reg.reg[D.Output_Instruction->z].status << " - " << D.Output_Instruction->z << endl;
							if(reg.reg[D.Output_Instruction->x].status && reg.reg[D.Output_Instruction->y].status && reg.reg[D.Output_Instruction->z].status)
							{
								D.Output_Instruction -> stalled = false;
								decode_dependency = false;
								if(D.Output_Instruction -> opcode == "MUL")
								{
									EM1.Input_Instruction-> stalled = false;
									mul_dep = false;
									//cout << "Testing MUl ---- " << endl;
								}
								else if(D.Output_Instruction -> opcode == "DIV")
								{
									ED1.Input_Instruction-> stalled = false;
								}
								else
								{
									E.Input_Instruction-> stalled = false;
								}
								if(D.Output_Instruction -> opcode != "STORE" || D.Output_Instruction -> opcode != "nop") 
								{
									destination = D.Output_Instruction-> z;
									reg.reg[destination].status= false;
									if (D.Output_Instruction -> opcode == "ADD" || D.Output_Instruction -> opcode == "SUB" || D.Output_Instruction -> opcode == "MUL" || D.Output_Instruction -> opcode == "DIV")
									{
										reg.reg[16].status= false;
									}
								}
							}
							if(reg.reg[D.Output_Instruction->x].status)
							{
								if(D.Output_Instruction -> opcode == "MUL")
								{
									EM1.Input_Instruction-> source_register1 = reg.reg[D.Output_Instruction->x].value;
									//cout<< " x value is ----" << E.Input_Instruction-> source_register1 << endl ;
									//cout<< "given x value is " << reg.reg[D.Output_Instruction->x].value << endl; 
								}
								else if(D.Output_Instruction -> opcode == "DIV")
								{
									ED1.Input_Instruction-> source_register1 = reg.reg[D.Output_Instruction->x].value;
								}
								else
								{
									E.Input_Instruction-> source_register1 = reg.reg[D.Output_Instruction->x].value;		
									//cout<< " x value is" << E.Input_Instruction-> source_register1 << endl;
									//cout<< "given x value is " << reg.reg[D.Output_Instruction->x].value << endl; 
								}
							}
							if(reg.reg[D.Output_Instruction->y].status)
							{
								if(D.Output_Instruction -> opcode == "MUL")
								{
									EM1.Input_Instruction-> source_register2 = reg.reg[D.Output_Instruction->y].value;
									//cout<< " y value is ----" << E.Input_Instruction-> source_register2 << endl;
									//cout<< "given y value is " << reg.reg[D.Output_Instruction->y].value << endl; 	
								}
								else if(D.Output_Instruction -> opcode == "DIV")
								{
									EM1.Input_Instruction-> source_register1 = reg.reg[D.Output_Instruction->y].value;
								}
								else
								{
									E.Input_Instruction-> source_register2 = reg.reg[D.Output_Instruction->y].value;	
									//cout<< " y value is" << E.Input_Instruction-> source_register2 << endl;
									//cout<< "given y value is " << reg.reg[D.Output_Instruction->y].value<< endl; 	
								}
							}
						}
						else if(D.Output_Instruction-> src_name1 != "" && D.Output_Instruction-> src_name2 == "")
						{
							if(reg.reg[D.Output_Instruction->z].status && reg.reg[D.Output_Instruction->x].status)
							{
								D.Output_Instruction -> stalled = false;
								decode_dependency = false;
								E.Input_Instruction-> stalled = false;
								if(D.Output_Instruction -> opcode != "STORE" || D.Output_Instruction -> opcode != "nop")
								{
									destination = D.Output_Instruction-> z;
									reg.reg[destination].status= false;
								}
							}
						}
					}
					
				}

				//cout<< "k value is" << k << endl << " PC value is"<< PC;
				if (PC < (((k - 1) * 4) + 4000))
				{
					if (!F.Input_Instruction -> stalled && !exe_halt)
					{
			    			PC= PC + 4;
						//cout<< "kakakak" << endl;
						F.Input_Instruction-> PC = PC;
						F.Input_Instruction -> instruction_string = cmem.code[(PC - 4000)/4].instruction_string;
				       		//cout<<"Input instruction is "<< F.Input_Instruction -> instruction_string<< endl;
					}
				}
		     		else if (F.Input_Instruction -> instruction_string == F.Output_Instruction -> instruction_string)
				{
					F.Input_Instruction -> instruction_string = "nop";
				}

				total_cycles++;

		   		i++;
                              
                                cout << endl;
				
				/*if(F.Input_Instruction -> instruction_string == "" || F.Input_Instruction -> instruction_string == "nop")
				{
					cout << "Fetch in 	:	" << F.Input_Instruction -> instruction_string <<"   FO stalled: " << F.Input_Instruction -> stalled << endl;
					
				}
				else 
				{
					cout << "Fetch in 	:	I(" << (F.Input_Instruction->PC-4000)/4 << ")  " << F.Input_Instruction -> instruction_string <<"   FO stalled: " << F.Input_Instruction -> stalled   						<<endl; 
					
				}*/
				
				if(F.Output_Instruction -> instruction_string == "" || F.Output_Instruction -> instruction_string == "nop")
				{
					cout << "Fetch out	:	" << F.Output_Instruction -> instruction_string <<"   FO stalled: " << F.Output_Instruction -> stalled << endl;
					
				}
				else 
				{
					cout << "Fetch out	:	I(" << (F.Output_Instruction->PC-4000)/4 << ")  " << F.Output_Instruction -> instruction_string <<"   FO stalled: " << F.Output_Instruction -> stalled 						<<endl; 
					
				}
				
				/*if(D.Input_Instruction -> instruction_string == "" || D.Input_Instruction -> instruction_string == "nop")
				{ 
					cout << "DRF in   	:	" << D.Input_Instruction -> instruction_string <<"   DO stalled: " << D.Input_Instruction -> stalled<< endl;
				}
				else
				{
				        cout << "DRF in   	:	I(" << (D.Input_Instruction->PC-4000)/4 << ")  " << D.Input_Instruction -> instruction_string <<"   DO stalled: " << D.Input_Instruction -> stalled 						<<endl;
				}*/	

				if(D.Output_Instruction -> instruction_string == "" || D.Output_Instruction -> instruction_string == "nop")
				{ 
					cout << "DRF out  	:	" << D.Output_Instruction -> instruction_string<<"   DO stalled: " << D.Output_Instruction -> stalled << endl;
				}
				else
				{
				        cout << "DRF out  	:	I(" << (D.Output_Instruction->PC-4000)/4 << ")  " << D.Output_Instruction -> instruction_string <<"   DO stalled: " << D.Output_Instruction -> stalled 						<<endl;
				}
				/*if(E.Input_Instruction -> instruction_string == "" || E.Input_Instruction -> instruction_string == "nop")
				{
					cout << "INTFU in 	:	" << E.Input_Instruction -> instruction_string <<"   EO stalled: " << E.Input_Instruction -> stalled << endl;
				}
				else
				{
					cout << "INTFU in 	:	I(" << (E.Input_Instruction->PC-4000)/4 << ")  " << E.Input_Instruction -> instruction_string <<"   EO stalled: " << E.Input_Instruction -> stalled 						<<endl;
				}*/
			

				if(E.Output_Instruction -> instruction_string == "" || E.Output_Instruction -> instruction_string == "nop")
				{
					cout << "INTFU out 	:	" << E.Output_Instruction -> instruction_string <<"   EO stalled: " << E.Output_Instruction -> stalled << endl;
				}
				else
				{
					cout << "INTFU out 	:	I(" << (E.Output_Instruction->PC-4000)/4 << ")  " << E.Output_Instruction -> instruction_string <<"   EO stalled: " << E.Output_Instruction -> stalled 						<<endl;
				}

				/*if(EM1.Input_Instruction -> instruction_string == "" || EM1.Input_Instruction -> instruction_string == "nop")
				{
					cout << "MUL1  in  	:	" << EM1.Input_Instruction -> instruction_string <<"   EM1 stalled: "<< EM1.Input_Instruction -> stalled<< endl;
				}
				else
				{
					cout << "MUL1  in  	:	I(" << (EM1.Input_Instruction->PC-4000)/4 << ")  " << EM1.Input_Instruction -> instruction_string <<"   EM1 stalled: "<< EM1.Input_Instruction -> 						stalled<<endl;
				}*/
				if(EM1.Output_Instruction -> instruction_string == "" || EM1.Output_Instruction -> instruction_string == "nop")
				{
					cout << "MUL1  out 	:	" << EM1.Output_Instruction -> instruction_string <<"   EM1 stalled: "<< EM1.Output_Instruction ->stalled<< endl;
				}
				else
				{
					cout << "MUL1  out 	:	I(" << (EM1.Output_Instruction->PC-4000)/4 << ")  " << EM1.Output_Instruction -> instruction_string <<"   EM1 stalled: "<< EM1.Output_Instruction -> 						stalled<<endl;
				}

				/*if(EM2.Input_Instruction -> instruction_string == "" || EM2.Input_Instruction -> instruction_string == "nop")
				{
					cout << "MUL2  in  	:	" << EM2.Input_Instruction -> instruction_string <<"   EM2 stalled: "<< EM2.Input_Instruction ->stalled<< endl;
				}
				else
				{
					cout << "MUL2  in  	:	I(" << (EM2.Input_Instruction->PC-4000)/4 << ")  " << EM2.Input_Instruction -> instruction_string <<"   EM2 stalled: "<< EM2.Input_Instruction -> 					stalled<<endl;
				}*/				
		
				if(EM2.Output_Instruction -> instruction_string == "" || EM2.Output_Instruction -> instruction_string == "nop")
				{
					cout << "MUL2  out  	:	" << EM2.Output_Instruction -> instruction_string <<"   EM2 stalled: "<< EM2.Output_Instruction -> stalled<< endl;
				}
				else
				{
					cout << "MUL2  out  	:	I(" << (EM2.Output_Instruction->PC-4000)/4 << ")  " << EM2.Output_Instruction -> instruction_string <<"   EM2 stalled: "<< EM2.Output_Instruction -> 					stalled<<endl;
				}
				
				/*if(ED1.Input_Instruction -> instruction_string == "" || ED1.Input_Instruction -> instruction_string == "nop")
				{
					cout << "DIV1  in  	:	" << ED1.Input_Instruction -> instruction_string <<"   DIV1 stalled: "<< ED1.Input_Instruction -> stalled<< endl;
				}
				else
				{
					cout << "DIV1  in  	:	I(" << (ED1.Input_Instruction->PC-4000)/4 << ")  " << ED1.Input_Instruction -> instruction_string <<"   DIV1 stalled: "<< ED1.Input_Instruction -> 					stalled<<endl;
				}*/
				
				if(ED1.Output_Instruction -> instruction_string == "" || ED1.Output_Instruction -> instruction_string == "nop")
				{
					cout << "DIV1  out  	:	" << ED1.Output_Instruction -> instruction_string<<"   DIV1 stalled: "<< ED1.Output_Instruction -> stalled << endl;
				}
				else
				{
					cout << "DIV1  out  	:	I(" << (ED1.Output_Instruction->PC-4000)/4 << ")  " << ED1.Output_Instruction -> instruction_string <<"   DIV1 stalled: "<< ED1.Output_Instruction -> 					stalled<<endl;
				}

				/*if(ED2.Input_Instruction -> instruction_string == "" || ED2.Input_Instruction -> instruction_string == "nop")
				{
					cout << "DIV2  in  	:	" << ED2.Input_Instruction -> instruction_string << endl;
				}
				else
				{
					cout << "DIV2  in  	:	I(" << (ED2.Input_Instruction->PC-4000)/4 << ")  " << ED2.Input_Instruction -> instruction_string <<"   DIV2 stalled: "<< ED2.Input_Instruction -> 					stalled<<endl;
				}*/
				
				if(ED2.Output_Instruction -> instruction_string == "" || ED2.Output_Instruction -> instruction_string == "nop")
				{
					cout << "DIV2  out  	:	" << ED2.Output_Instruction -> instruction_string << endl;
				}
				else
				{
					cout << "DIV2  out  	:	I(" << (ED2.Output_Instruction->PC-4000)/4 << ")  " << ED2.Output_Instruction -> instruction_string <<"   DIV2 stalled: "<< ED2.Output_Instruction -> 					stalled<<endl;
				}
				
				/*if(ED3.Input_Instruction -> instruction_string == "" || ED3.Input_Instruction -> instruction_string == "nop")
				{
					cout << "DIV3  in  	:	" << ED3.Input_Instruction -> instruction_string <<"   DIV3 stalled: "<< ED3.Input_Instruction ->stalled<< endl;
				}
				else
				{
					cout << "DIV3  in 	:	I(" << (ED3.Input_Instruction->PC-4000)/4 << ")  " << ED3.Input_Instruction -> instruction_string <<"   DIV3 stalled: "<< ED3.Input_Instruction -> 					stalled<<endl;
				}*/

				if(ED3.Output_Instruction -> instruction_string == "" || ED3.Output_Instruction -> instruction_string == "nop")
				{
					cout << "DIV3  out  	:	" << ED3.Output_Instruction -> instruction_string <<"   DIV3 stalled: "<< ED3.Output_Instruction -> stalled<< endl;
				}
				else
				{
					cout << "DIV3  out  	:	I(" << (ED3.Output_Instruction->PC-4000)/4 << ")  " << ED3.Output_Instruction -> instruction_string <<"   DIV3 stalled: "<< ED3.Output_Instruction -> 					stalled<<endl;
				}
				
				/*if(ED4.Input_Instruction -> instruction_string == "" || ED4.Input_Instruction -> instruction_string == "nop")
				{
					cout << "DIV4  in  	:	" << ED4.Input_Instruction -> instruction_string <<"   DIV4 stalled: "<< ED4.Input_Instruction -> stalled<< endl;
				}
				else
				{
					cout << "DIV4  in  	:	I(" << (ED4.Input_Instruction->PC-4000)/4 << ")  " << ED4.Input_Instruction -> instruction_string <<"   DIV4 stalled: "<< ED4.Input_Instruction -> 					stalled<<endl;
				}*/
				
				
				if(ED4.Output_Instruction -> instruction_string == "" || ED4.Output_Instruction -> instruction_string == "nop")
				{
					cout << "DIV4  out  	:	" << ED4.Output_Instruction -> instruction_string <<"   DIV4 stalled: "<< ED4.Output_Instruction -> stalled<< endl;
				}
				else
				{
					cout << "DIV4  out  	:	I(" << (ED4.Output_Instruction->PC-4000)/4 << ")  " << ED4.Output_Instruction -> instruction_string <<"   DIV4 stalled: "<< ED4.Output_Instruction -> 					stalled<<endl;
				}
				
				
				/*if(M.Input_Instruction -> instruction_string == "" || M.Input_Instruction -> instruction_string == "nop")
				{
					cout << "Mem in    	:	" << M.Input_Instruction -> instruction_string <<"   Mem stalled: "<< M.Input_Instruction -> stalled << endl;
				}
				else
				{
					cout << "Mem in    	:	I(" << (M.Input_Instruction->PC-4000)/4 << ")  " << M.Input_Instruction -> instruction_string <<"   Mem stalled: "<< M.Input_Instruction -> stalled<< 						endl;
				}*/
				

				if(M.Output_Instruction -> instruction_string == "" || M.Output_Instruction -> instruction_string == "nop")
				{
					cout << "Mem out   	:	" << M.Output_Instruction -> instruction_string <<"   Mem stalled: "<< M.Output_Instruction ->stalled<< endl;
				}
				else
				{
					cout << "Mem out   	:	I(" << (M.Output_Instruction->PC-4000)/4 << ")  " << M.Output_Instruction -> instruction_string <<"   Mem stalled: "<< M.Output_Instruction ->stalled<< 						endl;
				}
				
				/*if(W.Input_Instruction -> instruction_string == "" || W.Input_Instruction -> instruction_string == "nop")
				{
					cout << "WB  in   	:	" << W.Input_Instruction -> instruction_string <<"   WB stalled: "<< W.Input_Instruction -> stalled<< endl;
				}
				else
				{
					cout << "WB  in		:	I(" << (W.Input_Instruction->PC-4000)/4 << ")  " << W.Input_Instruction -> instruction_string <<"   WB stalled: "<< W.Input_Instruction -> stalled<<  						endl;
				}*/

				if(W.Output_Instruction -> instruction_string == "" || W.Output_Instruction -> instruction_string == "nop")
				{
					cout << "WB  out	:	" << W.Output_Instruction -> instruction_string <<"   WB stalled: "<< W.Output_Instruction -> stalled<< endl;
				}
				else
				{
					cout << "WB  out	:	I(" << (W.Output_Instruction->PC-4000)/4 << ")  " << W.Output_Instruction -> instruction_string <<"   WB stalled: "<< W.Output_Instruction -> stalled<< endl;
				}
				cout << "Clock Cycles - " << total_cycles << endl;



				

				if (W.Output_Instruction -> instruction_string != "nop" && F.Output_Instruction -> instruction_string == "nop" && D.Output_Instruction -> instruction_string == "nop" && 					E.Output_Instruction -> instruction_string == "nop" && EM1.Output_Instruction -> instruction_string == "nop" && EM2.Output_Instruction -> instruction_string == "nop" && 					M.Output_Instruction ->instruction_string == "nop" && ED1.Output_Instruction -> instruction_string == "nop" && ED2.Output_Instruction -> instruction_string == "nop" && ED3.Output_Instruction -> instruction_string == "nop" && ED4.Output_Instruction -> instruction_string == "nop")
				{
					cout << "Simulation Completed for all instructions. Total Cycles - " << total_cycles  << endl; 
					break;
				}

			}
	       	}
	 
	      	else if(stag == 3){
			cout<<"display mode"<<endl;
		     	for(int k=0; k<=16 ; k++)
		      	{
		      		cout<< k << "register value is" <<"-" <<reg.reg[k].value << " - " << reg.reg[k].status <<endl; 
		      	}
			cout<< endl;
		     	for(int m=0; m<400; m+=4)
		      	{
		      		cout<<"data memory value is " << m << " - " <<dmem.data_mem[m]<<endl;
		      	}
	      	}
		else if(stag == 4)
		{
			break;
		}
	}
	return 0;
}
