/********************************************************************

 This file contains fake conio.h based on some weird code from StackOverflow.
 It's used in non-ncurses version of the game, because AFAIK, there is no conio.h 
 in C. And Cygwin - which I'm using myself to annoy Windows users. Or something.

*********************************************************************/
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
 
int getch(void) {
    struct termios oldattr, newattr;
    int ch;
    tcgetattr( STDIN_FILENO, &oldattr );
    newattr = oldattr;
    newattr.c_lflag &= ~( ICANON | ECHO );
    tcsetattr( STDIN_FILENO, TCSANOW, &newattr );
    ch = getchar();
    tcsetattr( STDIN_FILENO, TCSANOW, &oldattr );
    return ch;
}
