# Alpha-Compiler
An implementation for the compiler of Alpha programming language in collaboration
with a fellow computer science student, Kostas Skantzas.

Alpha is a prototype programming language developed by Anthony Savvidis,
Affiliated Researcher to ICS-FORTH and Professor of Computer Science, University of Crete.

The implementation consisted of 5 phases. In the first two we had to provide a lexical analyzer
using LEX (lexical analyzer generator) and a syntax analyzer using YACC (syntax analyzer
generator). A symbol table which we implemented in C (programming language) was used to store Identifier
tokens of the Alpha language during these two initial phases.
The third phase consists of the production of intermediate code (3-address code or QUADS) which are
stored in an array.
In the fourth phase, byte code is produced through the QUADS and the binary file for the final execution of 
the user's code is generated.
The fifth and final phase is the implementation of the AVM (Alpha Virtual Machine) which loads and executes
the binary file.

