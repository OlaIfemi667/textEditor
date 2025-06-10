/* includes */
#include <unistd.h>
#include <stdlib.h>
#include <termios.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>

/* defines */ 

#define CTRL_KEY(k) ((k) & 0x1f)


/* data */
struct termios orig_termios;

/* fonctions */

void die(const char *s)
{
	write(STDOUT_FILENO, "\x1b[2J", 4); //
	write(STDOUT_FILENO, "\x1b[H", 3); //
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



/* terminal */
char editorReadKey()
{
	int nread;
	char c;
	while ((nread =  read(STDIN_FILENO, &c, 1)) != 1)
	{
		if (nread == 1 && errno != EAGAIN)
			die("read");

	}
	return c;
}

/* input */

void editorProcessKeypress()
{
	char c = editorReadKey();

	switch (c) {
		case CTRL_KEY('q'):
			write(STDOUT_FILENO, "\x1b[2J", 4); //
			write(STDOUT_FILENO, "\x1b[H", 3); //
			exit(0);
			break;
	}
}

/* output */

void editorRefreshScreen()
{
	write(STDOUT_FILENO, "\x1b[2J", 4); //
					    //1 - cette fonction permet de nettoyer le screen
					    //let break it down
					    //on utilise write et 4 ce qui signifie qu'on veut écrire sur 4 octets (bytes)
					    //ici on veut ecrire une escape sequence
					    //Elle commence donc pas l'escape character: \x1b ou 27 en décimal (1st byte)
					    //le character [ (2nd byte)
					    //2 (3rd byte)
					    //J : erase in display (4th byte)
					    //
					    //en gros 2J mit ensembe signifie supprimer toute les lignes (2 est le paramètre dans ce cas, il existe aussi les paramètre 0, 1)
					    //
					    //
					    //pour ce text editor nous utiliserons les escape sequence de VT100
	write(STDOUT_FILENO, "\x1b[H", 3);  //
					    //2- repositionner le curseur
					    //
					    //La commande H prend deux argument, rows, column. le column et row commence par 1
					    //
					    //eg: "\x1b[1;1H" , 
					  

}



/* init */
int main()
{
	enableRawMode();
	while(1)
	{
		editorRefreshScreen();
		editorProcessKeypress();
	}

	return 0;
}
