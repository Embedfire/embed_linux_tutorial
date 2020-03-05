# i.mx6ul_board
This is an linux demo code for the i.mx6ul development board.

Makefile uses static linking, if you do not use static linking:
  change `FLAG = -g -Werror -I. -Iinclude -static` to `FLAG = -g -Werror -I. -Iinclude`