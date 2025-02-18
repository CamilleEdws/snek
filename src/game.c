#include "game.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "snake_utils.h"

/* Helper function definitions */
static void set_board_at(game_t *game, unsigned int row, unsigned int col, char ch);
static bool is_tail(char c);
static bool is_head(char c);
static bool is_snake(char c);
static char body_to_tail(char c);
static char head_to_body(char c);
static unsigned int get_next_row(unsigned int cur_row, char c);
static unsigned int get_next_col(unsigned int cur_col, char c);
static void find_head(game_t *game, unsigned int snum);
static char next_square(game_t *game, unsigned int snum);
static void update_tail(game_t *game, unsigned int snum);
static void update_head(game_t *game, unsigned int snum);

/* Task 1 */
game_t *create_default_game() {
  game_t *game = (game_t *)malloc(sizeof(game_t));

  //making space for the board
  game -> num_rows = 18;
  game->board = (char **)malloc(game->num_rows * sizeof(char *));

//setting up the snakes
  game -> num_snakes = 1;
  game -> snakes = (snake_t *)malloc(sizeof(snake_t));

  game->snakes[0].tail_row = 2;
  game->snakes[0].tail_col = 2;
  game->snakes[0].head_row = 2;
  game->snakes[0].head_col = 4;
  game->snakes[0].live = true;

//creating the board?
  const char *board[] = {
  "####################\n",
  "#                  #\n",
  "# d>D    *         #\n",
  "#                  #\n",
  "#                  #\n",
  "#                  #\n",
  "#                  #\n",
  "#                  #\n",
  "#                  #\n",
  "#                  #\n",
  "#                  #\n",
  "#                  #\n",
  "#                  #\n",
  "#                  #\n",
  "#                  #\n",
  "#                  #\n",
  "#                  #\n",
  "####################\n",
  };

  for (int row = 0; row < game->num_rows; row++) {
    game -> board[row] = (char *)malloc(22 * sizeof(char));
    strcpy(game->board[row], board[row]);
  }


  return game;
}

/* Task 2 */
void free_game(game_t *game) {
    for (unsigned int i = 0; i < game->num_rows; i++) {
        free(game->board[i]);
    }

    free(game->board);
    free(game->snakes);

    free(game);
  return;
}

/* Task 3 */
void print_board(game_t *game, FILE *fp) {
  for(int row = 0; row < game -> num_rows; row++ ){
    fprintf(fp, "%s", game -> board[row]);
  }
  
  return;
}

/*
  Saves the current game into filename. Does not modify the game object.
  (already implemented for you).
*/
void save_board(game_t *game, char *filename) {
  FILE *f = fopen(filename, "w");
  print_board(game, f);
  fclose(f);
}

/* Task 4.1 */

/*
  Helper function to get a character from the board
  (already implemented for you).
*/
char get_board_at(game_t *game, unsigned int row, unsigned int col) { return game->board[row][col]; }

/*
  Helper function to set a character on the board
  (already implemented for you).
*/
static void set_board_at(game_t *game, unsigned int row, unsigned int col, char ch) {
  game->board[row][col] = ch;
}

/*
  Returns true if c is part of the snake's tail.
  The snake consists of these characters: "wasd"
  Returns false otherwise.
*/
static bool is_tail(char c) {
  if(c == 'w' || c == 'a' || c == 's' || c == 'd') {
    return true;
  }
  return false;
}

/*
  Returns true if c is part of the snake's head.
  The snake consists of these characters: "WASDx"
  Returns false otherwise.
*/
static bool is_head(char c) {
  if(c == 'W' || c == 'A' || c == 'S' || c == 'D' || c == 'x') {
    return true;
  }
  return false;
}

static bool is_body(char c) {
  if (c == '^' || c == '<' || c == '>' || c == 'v') {
    return true;
  }
  return false;
}

/*
  Returns true if c is part of the snake.
  The snake consists of these characters: "wasd^<v>WASDx"
*/
static bool is_snake(char c) {
  if (is_head(c) || is_tail(c) || is_body(c)){
    return true;
  }
  return false;
}

/*
  Converts a character in the snake's body ("^<v>")
  to the matching character representing the snake's
  tail ("wasd").
*/
static char body_to_tail(char c) {
  switch(c){
    case '^':
      c = 'w'; 
      break;
    case 'v':
      c = 's'; 
      break;
    case '<':
      c = 'a'; 
      break;
    case '>':
      c = 'd'; 
      break;
  }
  return c;
}

/*
  Converts a character in the snake's head ("WASD")
  to the matching character representing the snake's
  body ("^<v>").
*/
static char head_to_body(char c) {
  switch(c){
    case 'W':
      c = '^'; 
      break;
    case 'S':
      c = 'v'; 
      break;
    case 'A':
      c = '<'; 
      break;
    case 'D':
      c = '>'; 
      break;
  }
  return c;
}

/*
  Returns cur_row + 1 if c is 'v' or 's' or 'S'.
  Returns cur_row - 1 if c is '^' or 'w' or 'W'.
  Returns cur_row otherwise.
*/
static unsigned int get_next_row(unsigned int cur_row, char c) {
  if(c == '^' || c == 'W' || c == 'w'){
    return cur_row - 1;
  } else if(c == 'v' || c == 's' || c == 'S'){
    return cur_row + 1;
  }
  return cur_row;
}

/*
  Returns cur_col + 1 if c is '>' or 'd' or 'D'.
  Returns cur_col - 1 if c is '<' or 'a' or 'A'.
  Returns cur_col otherwise.
*/
static unsigned int get_next_col(unsigned int cur_col, char c) {
  if(c == '<' || c == 'A' || c == 'a'){
    return cur_col - 1;
  } else if(c == '>' || c == 'd' || c == 'D'){
    return cur_col + 1;
  }
  return cur_col;
}

/*
  Task 4.2

  Helper function for update_game. Return the character in the cell the snake is moving into.

  This function should not modify anything.
*/
static char next_square(game_t *game, unsigned int snum) {
  char next_tile = ' ';
  snake_t snake = game -> snakes[snum];  

  unsigned int snake_head_col = snake.head_col; 
  unsigned int snake_head_row = snake.head_row;

  char snake_head_char = get_board_at(game,snake_head_row, snake_head_col);

  unsigned int new_col = get_next_col(snake_head_col, snake_head_char); 
  unsigned int new_row = get_next_row(snake_head_row, snake_head_char);

  if (new_row >= game->num_rows || new_col >= strlen(game->board[new_row])){
    printf("somehow the snake is going past the wall!!");
    return '#';
  }
  
  next_tile = get_board_at(game, new_row, new_col);

  return next_tile;
}

/*
  Task 4.3

  Helper function for update_game. Update the head...

  ...on the board: add a character where the snake is moving

  ...in the snake struct: update the row and col of the head

  Note that this function ignores food, walls, and snake bodies when moving the head.
*/
static void update_head(game_t *game, unsigned int snum) {
  snake_t *snake = &(game -> snakes[snum]);
  unsigned int snake_row = snake -> head_row; 
  unsigned int snake_col = snake -> head_col;

  char snake_head = get_board_at(game, snake_row, snake_col); //finding the current char of the snake head
  unsigned int new_col = get_next_col(snake_col, snake_head); 
  unsigned int new_row = get_next_row(snake_row, snake_head);
  
  if(get_board_at(game, new_row, new_col) == '#' || is_snake(get_board_at(game, new_row, new_col))) {
    return; 
  }

  char new_body_char = head_to_body(snake_head); //finding the new body char
  set_board_at(game, snake_row, snake_col, new_body_char); //changing what was the head to a body charachter

  
  //updating snake values 
  snake -> head_col = new_col; 
  snake -> head_row = new_row;
  set_board_at(game, new_row, new_col, snake_head); //box where head is supposed to go is now updated

  return;
}

/*
  Task 4.4

  Helper function for update_game. Update the tail...

  ...on the board: blank out the current tail, and change the new
  tail from a body character (^<v>) into a tail character (wasd)

  ...in the snake struct: update the row and col of the tail
*/
static void update_tail(game_t *game, unsigned int snum) {
  snake_t *snake = &(game->snakes[snum]);
  unsigned int snake_row = snake -> tail_row; 
  unsigned int snake_col = snake -> tail_col;
  
  char snake_tail = get_board_at(game, snake_row, snake_col); //finding the current char of the snake tail
  set_board_at(game, snake_row, snake_col, ' '); //changing what was the tail to a ' '

  unsigned int new_col = get_next_col(snake_col, snake_tail); 
  unsigned int new_row = get_next_row(snake_row, snake_tail);

  //updating snake values 
  snake -> tail_col = new_col; 
  snake -> tail_row = new_row;
  
  char new_snake_tail = get_board_at(game, new_row, new_col);
  char new_body_char = body_to_tail(new_snake_tail); //finding the new body char

  set_board_at(game, new_row, new_col, new_body_char); //box where body is supposed to go is now updated to be the tail

  return;
}

/* Task 4.5 */
void update_game(game_t *game, int (*add_food)(game_t *game)) {
  //updating each snakes position
  for (unsigned int i = 0; i < game ->num_snakes; i++) {
    snake_t *snake = &(game->snakes[i]);

    if(snake -> live == false) { //if snake alr dead, skip checking everything 
      break;
    }

    char new_tile = next_square(game, i); 

    if(new_tile == '#' || is_snake(new_tile)) {
      snake -> live = false; 
      set_board_at(game, snake -> head_row ,snake -> head_col, 'x');
      continue;
    }

    //we have determined the snake can move to a new spot! 
    update_head(game, i);

    if(new_tile == '*') { //if the snake ate a fruit, no need to update the tail~
      add_food(game);
      continue;;
    } 

    update_tail(game, i);

  }
  
  return;
}

/* Task 5.1 */
char *read_line(FILE *fp) {
    int buffer = 10;
    char *line_read = malloc(buffer);
    if (!line_read) return NULL;

    if (!fgets(line_read, buffer, fp)) {
        free(line_read);
        return NULL;
    }

    char *end_of_line = strchr(line_read, '\n');

    while (!end_of_line) {
        buffer *= 2;
        char *new_line_read = realloc(line_read, buffer);
        if (!new_line_read) {
            free(line_read);
            return NULL;
        }
        line_read = new_line_read;

        if (!fgets(line_read + strlen(line_read), buffer - strlen(line_read), fp)) {
            break;
        }
        end_of_line = strchr(line_read, '\n');
    }

    return line_read;
}


/* Task 5.2 */
game_t *load_board(FILE *fp) {
    game_t *game = malloc(sizeof(game_t));

    game->num_snakes = 0; 
    game->snakes = NULL;

    game->board = NULL; 
    game->num_rows = 0;


    char *line = read_line(fp);
    while (line != NULL) {
      game -> num_rows++; 
      char **new_board = realloc(game->board, (game->num_rows) * sizeof(char *));
      game->board = new_board; 
      game->board[game->num_rows - 1] = line;
      
      line = read_line(fp);
    }

  return game;
}

/*
  Task 6.1

  Helper function for initialize_snakes.
  Given a snake struct with the tail row and col filled in,
  trace through the board to find the head row and col, and
  fill in the head row and col in the struct.
*/
static void find_head(game_t *game, unsigned int snum) {
  snake_t *snake = &(game->snakes[snum]);
  
  unsigned int row = snake->tail_row;
  unsigned int col = snake->tail_col;
  char body_part = get_board_at(game, row, col);

  while(!is_head(body_part)) {
    row = get_next_row(row, body_part);
    col = get_next_col(col, body_part);
    body_part = get_board_at(game, row, col);
  }

  snake->head_col = col; 
  snake->head_row = row;

  return;
}

/* Task 6.2 */
game_t *initialize_snakes(game_t *game) {
  game -> num_snakes = 0; 
  game -> snakes = NULL; 

  for(unsigned int row = 0; row < game -> num_rows; row++) {
    unsigned int col_len = strlen(game->board[row]);
    for (unsigned int col = 0; col < col_len; col ++) {
      char tile_val = get_board_at(game, row, col); 
      
      if (is_tail(tile_val)) {
        game -> snakes = (snake_t *)realloc(game -> snakes, sizeof(snake_t)* (game -> num_snakes + 1));
        game->snakes[game -> num_snakes].live = true;
        game->snakes[game -> num_snakes].tail_row = row;
        game->snakes[game -> num_snakes].tail_col = col;

        find_head(game, game -> num_snakes);
        game -> num_snakes++; 
      }
    }
  }
  return game;
}
