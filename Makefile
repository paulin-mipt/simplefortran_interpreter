all: flex bison build

flex:
	flex -i -o lex.yy.cpp fortran.l
bison:
	bison -o fortran.tab.cpp -d fortran.y
build:
	g++ lex.yy.cpp fortran.tab.cpp syntax_engine.cpp -std=c++0x
