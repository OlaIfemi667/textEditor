/* includes */
#include <unistd.h>
#include <stdlib.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <ctype.h>
#include <errno.h>

/* defines */ 

#define CTRL_KEY(k) ((k) & 0x1f)


/* data */

struct editorConfig{
	int screenrows;
	int screencols;
	struct termios orig_termios;
};
struct editorConfig E;

/* fonctions */

int getWindowsSize(int *rows, int *cols)
{
	struct winsize ws;

	if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0)
	{
		return -1;
	}
	else
	{
		*cols = ws.ws_col;
		*rows = ws.ws_row;
		return 0;
	}
}

void die(const char *s)
{
	write(STDOUT_FILENO, "\x1b[2J", 4); //
	write(STDOUT_FILENO, "\x1b[H", 3); //
	perror(s);
	exit(1);
}

void disableRawMode()
{
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1)
		die("tcsetattr");
}

void enableRawMode() // le rw mode permet d'interpreter chaque keypress comme elle vienne
{
	if (tcgetattr(STDIN_FILENO, &E.orig_termios) == -1)
		die("tcgetattr"); // dans la structure ttermios on récupere les attribut du terminals
	atexit(disableRawMode); //utiliser atexit pour executer disableRawMode et donc désactiver le raw mode


	struct termios raw = E.orig_termios;//on déclare la structure termios
	
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

/* append  buffer */

struct abuf {
	char* b; // la structure pour apppend les char contient 
		 // b est le char actuel
		 // len est sa taille
	int len;
};


#define ABUF_INIT {NULL, 0}


void abAppend(struct abuf *ab, const char *s, int len)
{
	// ici on ajoute au abuf(append buffer) 
	
	char *new = realloc(ab->b, ab->len + len); // pour eviter les probleme de mémoire on réalloue  En gros réalloue la mémoire du abuf et on y ajoute la nouvelle len
						   // new retourne le meme pointer
	if (new == NULL)
		return;
	//ici on modifier new pour pouvoir l'utiliser et update le abuf
	//
	//
	memcpy(&new[ab->len], s, len); // on ajoute la la position [ab->len] le string en entiere ( len pour spécifier la taille à ajouter)
	ab->b = new; //update abuf string
	ab->len += len; //update abuf len

}


void abFree(struct abuf *ab)
{
	free(ab->b);
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
void editorDrawRows(struct abuf *ab)
{
	int y;
	for (y = 0; y < E.screenrows; y++)
	{
		abAppend(ab, "~", 1);

		if (y < E.screenrows - 1)
		{
			abAppend(ab, "\r\n", 2);
		}
	}
}

void editorRefreshScreen()
{
	struct abuf ab = ABUF_INIT;
	

	abAppend(&ab, "\x1b[?25l", 6);
	abAppend(&ab, "\x1b[2J", 4); //
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
	abAppend(&ab, "\x1b[H", 3);  //
					    //2- repositionner le curseur
					    //
					    //La commande H prend deux argument, rows, column. le column et row commence par 1
					    //
					    //eg: "\x1b[1;1H" , 
					  
	editorDrawRows(&ab);
	abAppend(&ab, "\x1b[H", 3);

	abAppend(&ab, "\x1b[?25h", 6);


	write(STDOUT_FILENO, ab.b, ab.len);
	abFree(&ab);
}



/* init */

 void initEditor()
{
	if (getWindowsSize(&E.screenrows, &E.screencols) == -1)
		die("getWindowsSize");
}


int main()
{
	enableRawMode();
	initEditor();
	while(1)
	{
		editorRefreshScreen();
		editorProcessKeypress();
	}

	return 0;
}
