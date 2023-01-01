#include "PEFile.h"
#include <cstring>

#define COFF_SIZE 20
#define OPT_HDR_SIZE 240

PEFile::PEFile(char* _NAME, FILE* _Ppefile, char* chto_delat) {
	
	NAME = _NAME;
	Ppefile = _Ppefile;
	SDV=defineSDV();
	if(chto_delat[1]=='s') {
		if(isPE()) {
		  printf("PE\n");
		  exit(0);
		} else{
		  printf("Not PE\n");
		  exit(1);
		}
	} else if(chto_delat[1]=='m'){
		printFuncs();
	}



}

int PEFile::defineSDV(){
	fseek(Ppefile, 0, SEEK_SET);
	int buf;
	fseek(Ppefile,0x3c,SEEK_SET);
	fread(&buf,4,1,Ppefile);
	return buf;
}


bool PEFile::isPE(){
	
	
	char bf;
	fseek(Ppefile,SDV,SEEK_SET);
	
	fread(&bf,1,1,Ppefile);
	if(bf!='P') return false;
	
	fread(&bf,1,1,Ppefile);
	if(bf!='E') return false;
	
	fread(&bf,1,1,Ppefile);
	if(bf!='\0') return false;
	
	fread(&bf,1,1,Ppefile);
	if(bf!='\0') return false;
	
	return true;
}

void PEFile::printFuncs(){
	long optionalHeaderAddress=SDV+4+COFF_SIZE;
	int import_table_rva;
	
	fseek(Ppefile,optionalHeaderAddress+120,SEEK_SET);
	fread(&import_table_rva,4,1,Ppefile);
	int import_table_SZ;
	fread(&import_table_SZ,4,1,Ppefile);
	
	//find section
	fseek(Ppefile,0,SEEK_SET);
	fseek(Ppefile,optionalHeaderAddress+OPT_HDR_SIZE,SEEK_SET);
	
	long section_raw=0, section_rva=0, section_virtual_size=0;
	int c=-1;
	while(true){
		c++;
		fseek(Ppefile,0,SEEK_SET);
		fseek(Ppefile,optionalHeaderAddress+OPT_HDR_SIZE+40*c+8,SEEK_SET);
		fread(&section_virtual_size,4,1,Ppefile);
		
		fseek(Ppefile,0,SEEK_SET);
		fseek(Ppefile,optionalHeaderAddress+OPT_HDR_SIZE+40*c+12,SEEK_SET);
		fread(&section_rva,4,1,Ppefile);
		
		fseek(Ppefile,0,SEEK_SET);
		fseek(Ppefile,optionalHeaderAddress+OPT_HDR_SIZE+40*c+20,SEEK_SET);
		fread(&section_raw,4,1,Ppefile);
		if(import_table_rva>=section_rva &&import_table_rva<=section_rva +section_virtual_size) {
			break;
		}
		
	}

	
       int import_raw = section_raw + import_table_rva - section_rva; 
       for(int i=0;i<section_virtual_size;i+=20){ //i<import_table_SZ
       	int lib_rva;
       	fseek(Ppefile,import_raw+i+12,SEEK_SET);
       	fread(&lib_rva,4,1,Ppefile);
       	if(lib_rva==0) break;
       	int lib_raw=section_raw + lib_rva - section_rva;
       	fseek(Ppefile,lib_raw,SEEK_SET);
       	char buf[100];
       	fread(&buf,sizeof(buf),1,Ppefile);
       	printf("%s\n",buf);
       	
   	        //funcs
       	fseek(Ppefile,import_raw+i,SEEK_SET);
       	int fTable_rva;
       	fread(&fTable_rva,4,1,Ppefile);
       	int fTable_raw=section_raw + fTable_rva - section_rva;
       	int k=0;
       	while(true){
       		fseek(Ppefile,fTable_raw+k*8,SEEK_SET);
       		int func_rva=0;
       		fread(&func_rva,4,1,Ppefile);
       		if(func_rva==0) break;
       		
       		if(func_rva-((func_rva<<1)>>1)==0){
       			
       			int func_raw=section_raw + func_rva - section_rva;
       			fseek(Ppefile,func_raw+1,SEEK_SET);
       			fread(&buf,sizeof(buf)-1,1,Ppefile);
       			printf("    ");
       			for(int i=1;i<sizeof(buf);i++){
       			if(buf[i]=='\0') break;
       			  printf("%c",buf[i]);
       			}
       			printf("%s","\n");
       		}
       		k++;
       	}
       }
}



