flex cmenos.l
bison -d cmenos.y
gcc -c *.c -fno-builtin-exp -Wno-implicit-function-declaration
gcc *.o -lfl -o cmenos -fno-builtin-exp
