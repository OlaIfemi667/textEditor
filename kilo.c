#include <unistd.h>
#include <stdlib.h>
#include <termios.h>
#include <stdio.h>
#include <ctype.h>

struct termios orig_termios;

void disableRawMode()
{
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void enableRawMode() // le rw mode permet d'interpreter chaque keypress comme elle vienne
{
	tcgetattr(STDIN_FILENO, &orig_termios); // dans la structure ttermios on récupere les attribut du terminals
	atexit(disableRawMode); //utiliser atexit pour executer disableRawMode et donc désactiver le raw mode


	struct termios raw = orig_termios;//on déclare la structure termios
	
	raw.c_iflag &= ~(IXON);

	raw.c_lflag &= ~(ECHO | ICANON | ISIG); // cet opération "&= ~()" est utiliser pour inverser le bit de les locales flags spécifiés (par exemple si 1, 1 devient 0)
	
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}	

int main()
{
	enableRawMode();
	char c;
	while(read(STDIN_FILENO, &c, 1) == 1 && c != 'q')
	{
		if (iscntrl(c))
			printf("%d\n", c);
else
			printf("%d ('%c')\n", c, c);
	}

	return 0;
}
