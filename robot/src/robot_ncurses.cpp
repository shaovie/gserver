#include "robot_ncurses.h"
#include <ncurses.h>
#include <locale.h>

WINDOW *create_newwin(int height, int width, int starty, int startx)
{
  WINDOW *local_win;
  local_win = newwin(height, width, starty, startx);
  wborder(local_win, '|', '|', '-', '-', '-', '-', '-', '-');
  wrefresh(local_win); /*刷新窗口缓冲，显示box */
  return local_win;
}
void destroy_win(WINDOW *local_win)
{
  wborder(local_win, ' ',' ',' ',' ',' ',' ',' ',' ');
  wrefresh(local_win);
  delwin(local_win);
}
void login_ncurses::input_account(char *account)
{
  WINDOW *login_win = NULL;
  int row = 0, col = 0;

  setlocale(LC_ALL,"");  // must before the first init
  initscr();
  cbreak();

  int height = 6;
  int width = 30;
  int starty = (LINES - height) / 2;
  int startx = (COLS - width) / 2;

  refresh();
  login_win = create_newwin(height, width, starty, startx);

  getmaxyx(login_win, row, col);
  mvwprintw(login_win, row/2, (col - 9 - 16)/2, "account: ");
  wrefresh(login_win);

  wattron(login_win, A_BOLD);
  wgetnstr(login_win, account, 31);
  wattroff(login_win, A_BOLD);

  destroy_win(login_win);
  refresh();
  endwin();
}
void login_ncurses::input_name(char *name)
{
  WINDOW *login_win = NULL;
  int row = 0, col = 0;

  initscr();			/* Start curses mode 		  */
  cbreak();

  int height = 6;
  int width = 30;
  int starty = (LINES - height) / 2;
  int startx = (COLS - width) / 2;

  refresh();
  login_win = create_newwin(height, width, starty, startx);

  getmaxyx(login_win, row, col);
  mvwprintw(login_win, row/2, (col - 9 - 16)/2, "new player name: ");
  wrefresh(login_win);

  wattron(login_win, A_BOLD);
  wgetnstr(login_win, name, sizeof(name) - 1);
  wattroff(login_win, A_BOLD);

  destroy_win(login_win);
  refresh();
  endwin();
}
