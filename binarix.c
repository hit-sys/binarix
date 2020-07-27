/*
 * binarix - a matrix styled linux console locker
 * Copyright (C) Raffael Himmelreich <raffi@exception.at>
 *
 *  This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General 
 * Public License as published by the Free Software Foundation;
 * either version 2 of the License, or (at your option) any later
 * version.
 *  This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
 * the GNU General Public License for more details.
 *  You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA.
 *
 */

#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <curses.h>
#include <pwd.h>
#include <stdlib.h>
#include <shadow.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define DIALOG_PASS_WIDTH  19
#define DIALOG_PASS_HEIGHT 3
#define DIALOG_PASS_TEXT "\n  Enter password. "

#define DIALOG_HELP_WIDTH 35
#define DIALOG_HELP_HEIGHT 7
#define DIALOG_HELP_TEXT "\n"                                  \
                         "  Type any key, followed by the\n"   \
                         "  user's password and press enter\n" \
                         "  to unlock the console!\n"          \
                         "\n"                                  \
                         "  Proceed with any key."


int  x, y;
int  bbox;
int  **field;
int  vt;
char ch;
char userpass[127];

void printfield();
void fillfield();
void changefield();
void printhelp(int);
void getpassword();
void comparepasswords();
void push_snake(int, int);
char *crypt();
WINDOW *win;

WINDOW *dialog(int height, int width, int starty, int startx, char *text);
extern int setsecurity(void);


int main()
{
	int tmp, i;

	if(geteuid() != 0)
	{
		fprintf(stderr, "Is the binary suid root?\n");
		exit(1);
	}

	getpassword();
	setuid(getuid());
	setgid(getgid());
	setsecurity();

	initscr();
	srand(time(0));

	getmaxyx(stdscr, y, x);
	curs_set(0);

	field = (int **) malloc (x * sizeof(int *));

	for(i = 0; i < x; i++)
	{
		field[i] = (int *) malloc (y * sizeof (int));
		for(tmp = 0; tmp < y; tmp++)
			field[i][tmp] = 2;
	}

	if(has_colors())
	{
		start_color();
		init_pair(1, COLOR_GREEN,  COLOR_BLACK);
		attron(COLOR_PAIR(1));
	}

	nodelay(stdscr, 1);
	noecho();

	changefield();

	endwin();

	return 0;
}

void printfield()
{
	int tmp, i;

	erase();

	for(i=0; i < y; i++)
	{
		for(tmp=0; tmp < x; tmp++)
		{
			if(field[tmp][i] == 2)
				printw(" ");
			else
				printw("%d", field[tmp][i]);
		}
	}

	if(bbox == 1)
	{
		refresh();
		win = dialog(DIALOG_PASS_HEIGHT, DIALOG_PASS_WIDTH,\
			    (y-DIALOG_PASS_HEIGHT)/2, (x-DIALOG_PASS_WIDTH)/2,\
			    DIALOG_PASS_TEXT);
		comparepasswords();
	} else {
		refresh();
	}

	usleep(100000);
	changefield();
}

void changefield()
{
	int i;

        for(i=0; i < x; i++)
        {
			if(rand()%25 < 20)
				continue;

			if(rand()%40 < 10)
			{
				push_snake(2, i);
			} else {
				push_snake(rand()%2, i);
			}
        }
	
	if((ch=getch()) != ERR)
	{
		switch(ch)
		{
			case '?': printhelp(0);
			          break;
			default:  bbox = 1;
			          break;
		}
	}
	
	printfield();
}

void printhelp(int signr)
{
	refresh();
	win = dialog(DIALOG_HELP_HEIGHT, DIALOG_HELP_WIDTH,\
		    (y-DIALOG_HELP_HEIGHT)/2, (x-DIALOG_HELP_WIDTH)/2,\
		    DIALOG_HELP_TEXT);
	nodelay(stdscr, 0);
	getch();
	nodelay(stdscr, 1);
}

void getpassword()
{
        struct spwd *pw;
        struct passwd *passwd;

	passwd = getpwuid(getuid());
	pw = getspnam(passwd->pw_name);

	strcpy(userpass, pw->sp_pwdp);
}

void comparepasswords()
{
	char ch;
	char pass[127];
	int count;

	count=0;
	nodelay(stdscr, 0);

	while((ch = getch()) != '\n')
		pass[count++] = ch;

	nodelay(stdscr, 1);

	pass[count] = '\0';

	if (strcmp(crypt(pass, userpass), userpass) == 0)
	{
        	endspent();
		endwin();
		exit(0);

	} else {
		sleep(1);
		bbox = 0;
		nodelay(stdscr, 1);
	}
}

WINDOW *dialog(int height, int width, int starty, int startx, char *text)
{
	WINDOW *dialog_win;

	dialog_win = newwin(height, width, starty, startx);
	wprintw(dialog_win, text);
	box(dialog_win, 0, 0);

	wrefresh(dialog_win);

	return dialog_win;
}

void push_snake(int element, int col)
{
	int i;

	for(i=y; i > 1; i--)
		field[col][i-1] = field[col][i-2];

	field[col][0] = element;
}
