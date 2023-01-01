#pragma once
#include <string>

class PEFile
{
public:
	PEFile(char* _NAME, FILE* Ppefile, char* chto_delat);
    

private:
    //PROPERTIES
    char* NAME;
    FILE* Ppefile;
    int SDV;

    
    // FUNCTIONS
    int defineSDV();
    bool isPE();
    void printFuncs();
    

};
