#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <ncurses.h>
#include <unistd.h>

WINDOW *create_newwin(int height, int width, int starty, int startx);
void destroy_win(WINDOW *local_win);
WINDOW *square[9][9];
char win_label[256];

unsigned int startxs[9];
unsigned int startys[9]; // = { 0, 0, 0, 7, 7, 7, 14, 14, 14 };

unsigned int pmark[9][9];

unsigned char histogram[9];

const unsigned char oneBits[] = {0,1,1,2,1,2,2,3};

unsigned char CountOnes(unsigned int x)
{
    unsigned char results;
    results = oneBits[(x)&0x07];
    results += oneBits[(x>>3)&0x07];
    results += oneBits[(x>>6)&0x07];
    return results;
}

signed char OneHot(unsigned int mask) {
  if(!mask)
    return -1;
  signed char c = 0;
  while(!(mask & 1) && mask) {
    ++c;
    mask >>= 1;
  }
  return c;
}

int main(int argc, char *argv[])
{
  WINDOW *my_win;
	int startx, starty, width, height;
	int ch;

  for(int i = 0; i < 9; ++i) {
    for(int j = 0; j < 9; ++j) {
      pmark[i][j] = 511;
    }
  }

  if(argc > 1) {
    FILE *fp;

    fp = fopen(argv[1], "r");
    if(fp) {
      signed char c, d;
      unsigned char r, s;
      r = 0; s = 0;
      while((c = getc(fp)) != EOF) {
        switch(c) {
          case '\n':
            r = 0; ++s;
          break;
          case '1':
          case '2':
          case '3':
          case '4':
          case '5':
          case '6':
          case '7':
          case '8':
          case '9':
            if((r > 8) || (s > 8)) {
              printf("malformated input file at line %d character %d.\n", r, s);
              exit(0);
            }
            d = atoi(&c);
            printf("pmark[%d][%d] = 1 << (%d - 1)\n", s, r, d);
            pmark[s][r] = 1 << (d - 1);
            ++r;
          break;
          default:
            ++r;
          break;
        }
        if(c != '\n') {
        } else {
        }
      }
      fclose(fp);
    }
  } else {
    pmark[0][2] = 1 << (4 - 1);
    pmark[0][7] = 1 << (5 - 1);
    pmark[1][1] = 1 << (5 - 1);
    pmark[1][2] = 1 << (1 - 1);
    pmark[1][6] = 1 << (8 - 1);
    pmark[2][4] = 1 << (9 - 1);
    pmark[2][5] = 1 << (3 - 1);
    pmark[3][0] = 1 << (2 - 1);
    pmark[3][4] = 1 << (5 - 1);
    pmark[3][6] = 1 << (1 - 1);
    pmark[4][3] = 1 << (7 - 1);
    pmark[4][8] = 1 << (8 - 1);
    pmark[5][0] = 1 << (8 - 1);
    pmark[5][3] = 1 << (6 - 1);
    pmark[5][8] = 1 << (2 - 1);
    pmark[6][5] = 1 << (7 - 1);
    pmark[6][7] = 1 << (3 - 1);
    pmark[7][4] = 1 << (3 - 1);
    pmark[7][6] = 1 << (2 - 1);
    pmark[7][7] = 1 << (9 - 1);
    pmark[8][0] = 1 << (6 - 1);
    pmark[8][1] = 1 << (4 - 1);

    /*  pmark[0][2] = 1 << (2 - 1);
    pmark[1][0] = 1 << (3 - 1);
    pmark[1][3] = 1 << (1 - 1);
    pmark[1][4] = 1 << (4 - 1);
    pmark[1][5] = 1 << (5 - 1);
    pmark[2][0] = 1 << (6 - 1);
    pmark[2][1] = 1 << (4 - 1);
    pmark[2][2] = 1 << (5 - 1);
    pmark[2][7] = 1 << (7 - 1);
    pmark[3][0] = 1 << (7 - 1);
    pmark[3][3] = 1 << (6 - 1);
    pmark[3][4] = 1 << (1 - 1);
    pmark[3][8] = 1 << (8 - 1);
    pmark[4][0] = 1 << (4 - 1);
    pmark[4][2] = 1 << (8 - 1);
    pmark[4][6] = 1 << (3 - 1);
    pmark[4][8] = 1 << (2 - 1);
    pmark[5][0] = 1 << (9 - 1);
    pmark[5][4] = 1 << (3 - 1);
    pmark[5][5] = 1 << (8 - 1);
    pmark[5][8] = 1 << (5 - 1);
    pmark[6][1] = 1 << (7 - 1);
    pmark[6][6] = 1 << (8 - 1);
    pmark[6][7] = 1 << (4 - 1);
    pmark[6][8] = 1 << (3 - 1);
    pmark[7][3] = 1 << (8 - 1);
    pmark[7][4] = 1 << (2 - 1);
    pmark[7][5] = 1 << (9 - 1);
    pmark[7][8] = 1 << (7 - 1);
    pmark[8][6] = 1 << (1 - 1);
    pmark[8][7] = 1 << (2 - 1);*/
  }

  initscr();			/* Start curses mode 		*/
  cbreak();			/* Line buffering disabled, Pass on
         * everty thing to me 		*/
  keypad(stdscr, TRUE);		/* I need that nifty F1 	*/

  height = 5;
  width = 5;
  for(int i = 0; i < 9; ++i) {
    startxs[i] = startys[i] = 2 + (i * 5) + 2 * (i / 3);
  }

  unsigned char changed = 1;
  signed char w;
  unsigned int bpmark;

  while(changed) {
    changed = 0;
    for(int i = 0; i < 9; ++i) {
      memset(histogram, 0, 9);
      for(int j = 0; j < 9; ++j) {
        if(CountOnes(pmark[i][j]) == 1) {
          w = OneHot(pmark[i][j]);
          for(int k = 0; k < 9; ++k) {
            if((pmark[i][k] & pmark[i][j]) && (CountOnes(pmark[i][k]) > 1) && (k != j)) {
              bpmark = pmark[i][k];
              pmark[i][k] &= ~(pmark[i][j]);
              if(bpmark == pmark[i][k]) {
                printf("marki[%d][%d]/%d 0x%02X -> 0x%02X\n", i, j, k, bpmark, pmark[i][k]);
                printf("not changed! 0x%02X -> 0x%02X\n", bpmark, pmark[i][k]);
                exit(-1);
              }
              changed = 1;
              printf("marki[%d][%d]/%d 0x%02X -> 0x%02X\n", i, j, k, bpmark, pmark[i][k]);
            }
            if((pmark[k][j] & pmark[i][j]) && (CountOnes(pmark[k][j]) > 1) && (k != i)) {
              bpmark = pmark[k][j];
              pmark[k][j] &= ~(pmark[i][j]);
              if(bpmark == pmark[k][j]) {
                printf("markj[%d][%d](%d) 0x%02X / [%d][%d](%d) 0x%02X - not changed! 0x%02X -> 0x%02X\n", k, j, CountOnes(pmark[k][j]), pmark[k][j], i, j, CountOnes(pmark[i][j]), pmark[i][j], bpmark, pmark[k][j]);
                exit(-1);
              } else
              changed = 1;
              //printf("markj[%d][%d]/%d\n", i, j, k);
            }
          }
          for(int y = 3*(i/3); y < (3*(i/3)+3); ++y) {
            for(int x = 3*(j/3); x < (3*(j/3)+3); ++x) {
              if((y!=i) && (x!=j) && (pmark[y][x] & pmark[i][j]) && (CountOnes(pmark[y][x]) > 1)) {
                bpmark = pmark[y][x];
                pmark[y][x] &= ~(pmark[i][j]);
                if(bpmark == pmark[y][x]) {
                  printf("markxy[%d][%d] -> [%d][%d]=%d\n", i, j, x, y, w);
                  printf("not changed! 0x%02X -> 0x%02X\n", bpmark, pmark[x][y]);
                  exit(-1);
                }
                changed = 1;
                //printf("markxy[%d][%d] -> [%d][%d]=%d\n", i, j, x, y, w);
              }
            }
          }
        }
        for(int k = 0; k < 9; ++k) {
          if(pmark[i][j] & (1 << k))
            ++histogram[k];
        }
      }
      for(int k = 0; k < 9; ++k)
        if(histogram[k] == 1) {
          for(int j = 0; j < 9; ++j)
            if((CountOnes(pmark[i][j]) > 1) && (pmark[i][j] & (1 << k))) {
              bpmark = pmark[i][j];
              pmark[i][j] = (1 << k);
              if(bpmark == pmark[i][j]) {
                printf("mark ij [%d][%d]/%d - not changed! 0x%02X -> 0x%02X\n", i, j, k+1, bpmark, pmark[i][j]);
                exit(-1);
              } else
              changed = 1;
              //printf("mark ij [%d][%d]/%d\n", i, j, k+1);
            }
        }
    }

    for(int j = 0; j < 9; ++j) {
      memset(histogram, 0, 9);
      for(int i = 0; i < 9; ++i) {
        for(int k = 0; k < 9; ++k) {
          if(pmark[i][j] & (1 << k))
            ++histogram[k];
        }
      }
      for(int k = 0; k < 9; ++k)
        if(histogram[k] == 1) {
          for(int i = 0; i < 9; ++i)
            if((CountOnes(pmark[i][j]) > 1) && (pmark[i][j] & (1 << k))) {
              bpmark = pmark[i][j];
              pmark[i][j] = (1 << k);
              if(bpmark == pmark[i][j]) {
                printf("mark5 [%d][%d] - not changed 0x%02X -> 0x%02X\n", i, j, bpmark, pmark[i][j]);
                exit(-1);
              } else
              changed = 1;
              //printf("mark5 [%d][%d] 0x%02X -> 0x%02X\n", i, j, bpmark, pmark[i][j]);
            }
        }
    }
    for(int x = 0; x < 9; x += 3) {
      for(int y = 0; y < 9; y += 3) {
        memset(histogram, 0, 9);
        //printf("checking square [%d][%d]\n", x, y);
        for(int i = x; i < x+3; ++i) {
          for(int j = y; j < y+3; ++j) {
            for(int k = 0; k < 9; ++k) {
              if(pmark[i][j] & (1 << k))
                ++histogram[k];
            }
          }
        }
        for(int k = 0; k < 9; ++k) {
          if(histogram[k] == 1) {
            for(int i = x; i < x+3; ++i) {
              for(int j = y; j < y+3; ++j) {
                if((CountOnes(pmark[i][j]) > 1) && (pmark[i][j] & (1 << k))) {
                  pmark[i][j] = (1 << k);
                  changed = 1;
                }
              }
            }
          }
        }
      }
    }
  }

	printw("Press F1 to exit");
	refresh();
  for(int j = 0; j < 9; ++j) {
    for(int i = 0; i < 9; ++i) {
      square[i][j] = create_newwin(height, width, startys[i], startxs[j]);
      sprintf(win_label, "%d%d\0", i, j);
      wprintf(square[i][j], win_label);
      if(CountOnes(pmark[i][j]) > 1) {
        unsigned k = 0;
        sprintf(win_label, "%03X", pmark[i][j]);
        mvwprintw(square[i][j], 0, 1, win_label);
        sprintf(win_label, "c:%d", CountOnes(pmark[i][j]));
        mvwprintw(square[i][j], 4, 1, win_label);
        for(int y = 0; y < 3; ++y) {
          for(int x = 0; x < 3; ++x) {
            if((1 << k) & pmark[i][j]) {
              sprintf(win_label, "%d", k+1);
              mvwprintw(square[i][j], y+1, x+1, win_label);
            }
            ++k;
          }
        }
      } else {
        if(OneHot(pmark[i][j]) >= 0)
          sprintf(win_label, "%d", 1+OneHot(pmark[i][j]));
        else
          sprintf(win_label, "X");
        mvwprintw(square[i][j], 2, 2, win_label);
      }
      wrefresh(square[i][j]);
    }
  }

	while((ch = getch()) != KEY_F(1))
	{	switch(ch)
		{	case KEY_LEFT:
				destroy_win(square[4][4]);
				square[4][4] = create_newwin(height, width, startys[4],--startxs[4]);
				break;
			case KEY_RIGHT:
				destroy_win(square[4][4]);
				square[4][4] = create_newwin(height, width, startys[4],++startxs[4]);
				break;
			case KEY_UP:
				destroy_win(square[4][4]);
				square[4][4] = create_newwin(height, width, --startys[4],startxs[4]);
				break;
			case KEY_DOWN:
				destroy_win(square[4][4]);
				square[4][4] = create_newwin(height, width, ++startys[4],startxs[4]);
				break;
		}
	}

  endwin();			/* End curses mode		  */
	return 0;
}

WINDOW *create_newwin(int height, int width, int starty, int startx)
{	WINDOW *local_win;

	local_win = newwin(height, width, starty, startx);
	box(local_win, 0 , 0);		/* 0, 0 gives default characters
					 * for the vertical and horizontal
					 * lines			*/
	wrefresh(local_win);		/* Show that box 		*/

	return local_win;
}

void destroy_win(WINDOW *local_win)
{
	/* box(local_win, ' ', ' '); : This won't produce the desired
	 * result of erasing the window. It will leave it's four corners
	 * and so an ugly remnant of window.
	 */
	wborder(local_win, ' ', ' ', ' ',' ',' ',' ',' ',' ');
	/* The parameters taken are
	 * 1. win: the window on which to operate
	 * 2. ls: character to be used for the left side of the window
	 * 3. rs: character to be used for the right side of the window
	 * 4. ts: character to be used for the top side of the window
	 * 5. bs: character to be used for the bottom side of the window
	 * 6. tl: character to be used for the top left corner of the window
	 * 7. tr: character to be used for the top right corner of the window
	 * 8. bl: character to be used for the bottom left corner of the window
	 * 9. br: character to be used for the bottom right corner of the window
	 */
	wrefresh(local_win);
	delwin(local_win);
}
