shell: main.c parser.tab.c lex.yy.c
	gcc -g -Wall -Wextra -lreadline -o shell main.c parser.tab.c lex.yy.c

lex.yy.c: lexer.l parser.tab.h
	flex --header-file=lexer.h lexer.l

parser.tab.c parser.tab.h: parser.y
	bison -d parser.y

.PHONY: clean
clean:
	rm lex.yy.c lexer.h parser.tab.c parser.tab.h
