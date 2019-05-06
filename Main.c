#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Table_queue.h"
#include "parser.h"

int main(int argc, char **argv){

  //init_symTable();
  programVarOffset=0;
  functionLocalOffset=0;
  formalArgOffset=0;
  scopeSpaceCounter=1;

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
		    fprintf(stderr, "Can't open output file %s!\n", argv[2]);
		    return(1);
	  }
  }else{
	  GOUT = stdout;
  }

  init_queue();
  yyparse();

  print_symTable(GOUT);
  print_quads(GOUT);

  fclose(yyin);

  if(GOUT!=stdout){
	   fclose(GOUT);
  }

  return 0;
}
