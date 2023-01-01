.PHONY: all clean
all: pe-parser

clean:
	rm -rf *.o pe-parser
	
PEFile.o:  src/PEFile.cpp
	g++ -c -o src/PEFile.o src/PEFile.cpp
	
pe-parser.o: src/pe-parser.cpp
	g++ -c -o src/pe-parser.o src/pe-parser.cpp
	
pe-parser: src/PEFile.o src/pe-parser.o
	g++ -o pe-parser src/PEFile.o src/pe-parser.o
	
validation-pe-tests: all
	python3 -m tests ValidatingPeTestCases -f
	
import-dll-tests: all
	python3 -m tests ImportDllTestCases -f
	
import-function-tests: all
	python3 -m tests ImportFunctionTestCases -f
