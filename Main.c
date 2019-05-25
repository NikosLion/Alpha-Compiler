#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "A_vm.h"
#include "parser.h"


int main(int argc, char **argv){

  programVarOffset=0;
  functionLocalOffset=0;
  formalArgOffset=0;
  scopeSpaceCounter=1;
  int code_instr=0;

  FILE *ifp, *ofp;
  ifp = fopen(argv[1], "r");

  if (ifp) {
	  yyin = ifp;
  }else{
	  fprintf(stderr, "Can't open input file!\n");
	  return(1);
  }

  if (argc > 2){
	  ofp = fopen(argv[2], "wb+");
	  if (ofp){
		    GOUT = ofp;
	  }
	  else{
		    fprintf(stderr, "Can't open output file %s!\n",argv[2]);
		    return(1);
	  }
  }else{
	  GOUT = stdout;
  }

  init_symTable();
  init_queue();

  yyparse();

  print_symTable(GOUT);
  print_quads(GOUT);

  call_generators();
  patch_incomplete_jumps();
  print_instructions_table(GOUT);

  convert_to_binary();
  Read_froms_Binary();

  avm_initStack();
  setGlobmem();

  /*while(code_instr<currCode){
    execute_cycle();
    code_instr++;
  }*/


  fclose(yyin);

  if(GOUT!=stdout){
	   fclose(GOUT);
  }

  return 0;
}
