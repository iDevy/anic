#include <stdio.h>
#include <string>
using std::string;
#include <map>
using std::map;

#include "../src/constantDefs.h"

map<string,int> tokenMap;

int mapToken(string s) {
	map<string,int>::iterator queryBuf = tokenMap.find(s);
	if (queryBuf != tokenMap.end()) { // query hit
		return queryBuf->second;
	} else { // query miss
		int newId = tokenMap.size();
		tokenMap.insert( make_pair(s, newId) );
		return newId;
	}
}

// parses the lexer table into an includable .h file with the appropriate struct representation
int main() {
	// input file
	FILE *in;
	in = fopen("./src/lexerTable.txt","r");
	if (in == NULL) { // if file open failed, return an error
		return -1;
	}
	// output files
	FILE *out;
	out = fopen("./var/lexerStruct.h","w");
	if (out == NULL) { // if file open failed, return an error
		return -1;
	}
	FILE *out2;
		out2 = fopen("./var/lexerStruct.cpp","w");
		if (out2 == NULL) { // if file open failed, return an error
			return -1;
	}
	// print the necessary prologue into the output file
	fprintf(out, "#ifndef _LEXER_STRUCT_H_\n");
	fprintf(out, "#define _LEXER_STRUCT_H_\n");
	fprintf(out, "#include \"../src/lexer.h\"\n\n");
	fprintf(out, "#define LEXER_STRUCT \\\n");
	fprintf(out, "static LexerNode lexerNode[256][256]; \\\n");
	fprintf(out, "static int lexerNodesInitialized = 0; \\\n");
	fprintf(out, "if (!lexerNodesInitialized) { \\\n");
	fprintf(out, "\tfor (int i = 0; i<256; i++) { \\\n");
	fprintf(out, "\t\tfor (int j = 0; j<256; j++) { \\\n");
	fprintf(out, "\t\t\tlexerNode[i][j].valid = 0; \\\n");
	fprintf(out, "\t\t} \\\n");
	fprintf(out, "\t} \\\n");
	// now, process the input file
	// data buffers
	char lineBuf[MAX_STRING_LENGTH];
	int fromState;
	char c;
	char tokenTypeCString[MAX_STRING_LENGTH];
	int toState;
	// read the lexer data
	for(;;) {
		char *retVal = fgets(lineBuf, MAX_STRING_LENGTH, in);
		if (retVal == NULL) { // if we've reached the end of the line, break out of the loop
			break;
		}
		// the line was valid, so now try parsing the data out of it
		int retVal2 = sscanf(lineBuf, "%d %c %s %d", &fromState, &c, tokenTypeCString, &toState);
		string tokenTypeString(tokenTypeCString); // create a c++ string out of the c string
		if (retVal2 >= 0 && retVal2 <= 3) { // if it was a blank/incomplete line, skip it
			continue;
		} else if (retVal2 == 4) { // else if it was a valid data line, process it normally
			int tokenType = mapToken(tokenTypeString); // get the token mapping
			fprintf(out, "\tlexerNode[%d][%d] = (LexerNode){ 1, %d, %d }; \\\n", fromState, c, tokenType, toState);
		}
	}
	// terminate the structure definition
	fprintf(out, "\tlexerNodesInitialized = 1; \\\n");
	fprintf(out, "} \n\n");
	// print out token definitions
	fprintf(out, "#define NUM_TOKENS %d\n\n", tokenMap.size());
	for (map<string,int>::iterator queryBuf = tokenMap.begin(); queryBuf != tokenMap.end(); queryBuf++) {
		fprintf(out, "#define TOKEN_%s %d\n", queryBuf->first.c_str(), queryBuf->second);
	}
	fprintf(out, "\n");
	// print out the tokenType2String forward declaration to the .h
	fprintf(out, "char *tokenType2String(int tokenType);\n\n");
	// print out tokenType2String to the .cpp
	fprintf(out2, "#include \"lexerStruct.h\"\n\n");
	fprintf(out2, "char *tokenType2String(int tokenType) {\n");
	fprintf(out2, "\tswitch(tokenType) {\n");
	for (map<string,int>::iterator queryBuf = tokenMap.begin(); queryBuf != tokenMap.end(); queryBuf++) {
		fprintf(out2, "\t\tcase TOKEN_%s:\n", queryBuf->first.c_str());
		fprintf(out2, "\t\t\treturn \"%s\";\n", queryBuf->first.c_str());
		fprintf(out2, "\t\t\tbreak;\n");
	}
	fprintf(out2, "\t}\n");
	fprintf(out2, "\treturn NULL; // can't happen if the above covers all cases, which it should\n");
	fprintf(out2, "}\n");
	fprintf(out, "#endif\n");
	// finally, return normally
	return 0;
}