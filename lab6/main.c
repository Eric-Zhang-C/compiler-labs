/*
 * main.c
 */

#include <stdio.h>
#include "util.h"
#include "symbol.h"
#include "string.h"
#include "graph.h"
#include "types.h"
#include "absyn.h"
#include "errormsg.h"
#include "temp.h" /* needed by translate.h */
#include "tree.h" /* needed by frame.h */
#include "assem.h"
#include "frame.h" /* needed by translate.h and printfrags prototype */
#include "translate.h"
#include "env.h"
#include "semant.h" /* function prototype for transProg */
#include "canon.h"
#include "prabsyn.h"
#include "printtree.h"
#include "escape.h" 
#include "parse.h"
#include "codegen.h"
#include "regalloc.h"

extern bool anyErrors;

/*Lab6: complete the function doProc
 * 1. initialize the F_tempMap
 * 2. initialize the register lists (for register allocation)
 * 3. do register allocation
 * 4. output (print) the assembly code of each function
 
 * Uncommenting the following printf can help you debugging.*/

/* print the assembly language instructions to filename.s */
static void doProc(FILE *out, F_frame frame, T_stm body)
{
 AS_proc proc;
 struct RA_result allocation;
 T_stmList stmList;
 AS_instrList iList;
 struct C_block blo;

 F_tempMap = Temp_empty();
 //1. initialize the F_tempMap
 Init_F_TempMap();

 printf("doProc for function %s:\n", S_name(F_name(frame)));
 printStmList(stdout, T_StmList(body, NULL));
 printf("-------====IR tree=====-----\n");

 stmList = C_linearize(body);
 printStmList(stdout, stmList);
 printf("-------====Linearlized=====-----\n");

 blo = C_basicBlocks(stmList);
 C_stmListList stmLists = blo.stmLists;
//  for (; stmLists; stmLists = stmLists->tail) {
//  	printStmList(stdout, stmLists->head);
// 	printf("------====Basic block=====-------\n");
//  }

 stmList = C_traceSchedule(blo);
 printStmList(stdout, stmList);
 printf("-------====trace=====-----\n");
 iList  = F_codegen(frame, stmList); /* 9 */
 //AS_printInstrList(stdout, iList, Temp_layerMap(F_tempMap, Temp_name()));

 printf("----======before RA=======-----\n");
 struct RA_result ra = RA_regAlloc(frame, iList);  /* 11 */
 printf("----======after RA=======-----\n");
 
 proc =	F_procEntryExit3(frame, ra.il);

 string procName = S_name(F_name(frame));
 fprintf(out, ".globl %s\n", procName);
 fprintf(out, ".type %s, @function\n", procName);
 //prologue
 fprintf(out, "%s", proc->prolog);
 AS_printInstrList (out, proc->body,
                       Temp_layerMap(F_tempMap, ra.coloring));
 //epilog
 fprintf(out, "%s", proc->epilog);
 fprintf(out, ".size %s, .-%s\n", procName, procName);
}

void doStr(FILE *out, Temp_label label, string str) {
	//fprintf(out, ".section .rodata\n");
	fprintf(out, "%s:\n", S_name(label));
  int length = strlen(str);
	//it may contains zeros in the middle of string. To keep this work, we need to print all the charactors instead of using fprintf(str)
  fprintf(out, ".long %lu\n", length);
  fprintf(out, ".string \"");
	int i = 0;
	for (; i < length; i++) {
    if(str[i] == '\n'){
      fprintf(out, "\\n");
    }else if(str[i] == '\t'){
      fprintf(out, "\\t");
    }else if(str[i] == '\"'){
      fprintf(out, "\\\"");
    }else{
		  fprintf(out, "%c", str[i]);
    }
	}
	fprintf(out, "\"\n");
}

int main(int argc, string *argv)
{
 A_exp absyn_root;
 S_table base_env, base_tenv;
 F_fragList frags;
 char outfile[100];
 FILE *out = stdout;

 if (argc==2) {
   absyn_root = parse(argv[1]);
   if (!absyn_root)
     return 1;
     
#if 0
   pr_exp(out, absyn_root, 0); /* print absyn data structure */
   fprintf(out, "\n");
#endif

   //Lab 6: escape analysis
   //If you have implemented escape analysis, uncomment this
   Esc_findEscape(absyn_root); /* set varDec's escape field */

   frags = SEM_transProg(absyn_root);
   if (anyErrors) return 1; /* don't continue */

   /* convert the filename */
   sprintf(outfile, "%s.s", argv[1]);
   out = fopen(outfile, "w");
   /* Chapter 8, 9, 10, 11 & 12 */
  //  for (;frags;frags=frags->tail){
  //    if(frags->head->kind == F_procFrag) {
  //      doProc(out, frags->head->u.proc.frame, frags->head->u.proc.body);
	//    }
  //  }
  //  for (;frags;frags=frags->tail){
  //    if (frags->head->kind == F_stringFrag){
	//     doStr(out, frags->head->u.stringg.label, frags->head->u.stringg.str);
  //    }
  //  }
    F_fragList fragments;
    fprintf(out, ".text\n");
    for (fragments = frags; fragments; fragments=fragments->tail)
        if (fragments->head->kind == F_procFrag)
            doProc(out, fragments->head->u.proc.frame, fragments->head->u.proc.body);

    fprintf(out, ".section .rodata\n");
    for (fragments = frags; fragments; fragments=fragments->tail)
        if (fragments->head->kind == F_stringFrag)
            doStr(out, fragments->head->u.stringg.label, fragments->head->u.stringg.str);

   fclose(out);
   return 0;
 }
 EM_error(0,"usage: tiger file.tig");
 return 1;
}
