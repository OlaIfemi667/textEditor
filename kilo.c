#include <unistd.h>
#include <stdlib.h>
#include <termios.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>

struct termios orig_termios;

void die(const char *s)
{
	perror(s);
	exit(1);
}

void disableRawMode()
{
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1)
		die("tcsetattr");
}

void enableRawMode() // le rw mode permet d'interpreter chaque keypress comme elle vienne
{
	if (tcgetattr(STDIN_FILENO, &orig_termios) == -1)
		die("tcgetattr"); // dans la structure ttermios on récupere les attribut du terminals
	atexit(disableRawMode); //utiliser atexit pour executer disableRawMode et donc désactiver le raw mode


	struct termios raw = orig_termios;//on déclare la structure termios
	
	raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON); // IXON: pour disable ctrl+S  ctrl+Q
	
	raw.c_cflag |= (CS8);

	raw.c_oflag &= ~(OPOST); 

	raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN); // cet opération "&= ~()" est utiliser pour inverser le bit de les locales flags spécifiés (par exemple si 1, 1 devient 0)
							 // ISIG pour ctrl+C crtl+Z
							 // etc... XD
	
	raw.c_cc[VMIN] = 0; //
	raw.c_cc[VTIME] = 1;
	
		
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
		die("tcsetattr");
}	

int main()
{
	enableRawMode();
	while(1)
	{
		char c = '\0';

		if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN) die("read");

		if (iscntrl(c))
			printf("%d\r\n", c);
		else
			printf("%d ('%c')\r\n", c, c);
		if (c == CTRL_KEY('q'))
			break;
	}

	return 0;
}
