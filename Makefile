all: recurse corecurse corecusym

SRC := 
HDR :=
LIBDIR :=
INCDIR := .

recurse: recurse.cpp
	g++ -O2 -o $@ -std=gnu++23  -g -Wall -Wextra   $<

corecurse: corecurse.cpp
	g++ -O2 -o $@ -std=gnu++23  -g -Wall -Wextra   $<

corecusym: corecurse_symtrans.cpp
	g++ -O2 -o $@ -std=gnu++23  -g -Wall -Wextra   $<

clean:
	rm recurse corecurse corecusym
