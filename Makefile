shell: main.c parser.tab.c lex.yy.c command.c command.h change_dir.h
	gcc -g -Wall -Wextra -o shell main.c command.c parser.tab.c lex.yy.c -lreadline 

lex.yy.c: lexer.l parser.tab.h
	flex --header-file=lexer.h lexer.l

parser.tab.c parser.tab.h: parser.y
	bison -d parser.y

.PHONY: clean
clean:
	rm lex.yy.c lexer.h parser.tab.c parser.tab.h
