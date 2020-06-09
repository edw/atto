#define _GNU_SOURCE
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef int (*get_char_fn)(void *);
typedef void (*unget_char_fn)(void *, int);

enum { FALSE, TRUE };

typedef enum token_type {
  UNKNOWN=-2, END=-1, TAG=0,
  SYMBOL=1, INTEGER=2, STRING=3,
  OPEN_PAREN=4, CLOSE_PAREN=5, APOSTROPHE=6
} token_type;

typedef struct lexer {
  void *cookie;
  long current_line_no;
  long current_line_pos;
  long current_offset;
  char current_char;
  char next_char;
  get_char_fn  get_char;
  unget_char_fn unget_char;
  int token_line_no;
  int token_line_pos;
  char *token_chs;
  char n_token_chs;
  int token_chs_cap;
} lexer;

typedef struct token {
  int type;
  int line_no;
  int line_pos;
  char *chs;
} token;

const int FIRST_LINE_NO = 1;
const int PRE_FIRST_LINE_POS = -1;

void lexer_next_char(lexer *s);

void init_lexer(lexer *s, get_char_fn get_char, unget_char_fn unget_char) {
  s->get_char = get_char;
  s->unget_char = unget_char;
  s->current_char = '\0';
  s->current_line_no = FIRST_LINE_NO;
  s->current_line_pos = PRE_FIRST_LINE_POS;
  s->token_chs = NULL;
  s->n_token_chs = 0;
  s->token_chs_cap = -1;
  lexer_next_char(s);
}

void destory_lexer(lexer *s) {
  if (s->token_chs != NULL)
    free(s->token_chs);
}

void lexer_append(lexer *s, int new_ch) {
  char ch = (char)new_ch;

  if (ch == '\0')
    return;

  if (s->token_chs_cap == -1) {
    s->token_chs = calloc(1, sizeof(char));
    if (s->token_chs == NULL)
      exit(EXIT_FAILURE);
    s->token_chs_cap = 0;
  } else if (s->token_chs_cap == s->n_token_chs) {
    if (s->token_chs_cap == 0)
      s->token_chs_cap = 1;
    else
      s->token_chs_cap <<= 1;

    s->token_chs = realloc(s->token_chs, (s->token_chs_cap+1) * sizeof(char));

    if (s->token_chs == NULL)
      exit(EXIT_FAILURE);
  }
  s->token_chs[s->n_token_chs++] = ch;
  s->token_chs[s->n_token_chs] = '\0';
}

void lexer_next_char(lexer *s) {
  int ch = s->get_char(s->cookie);
  lexer_append(s, s->current_char);
  s->current_offset++;
  s->current_char = ch;
  if (ch == '\n') {
    s->current_line_no++;
    s->current_line_pos = PRE_FIRST_LINE_POS;
  }
  s->current_line_pos++;
}

char* token_description_alloc(token *t) {
  int line = t->line_no;
  int col = t->line_pos;
  char *message;

  switch (t->type) {
  case OPEN_PAREN:
    asprintf(&message, "OPEN_PAREN at %d:%d", line, col);
    break;
  case CLOSE_PAREN:
    asprintf(&message, "CLOSE_PAREN at %d:%d", line, col);
    break;
  case INTEGER:
    asprintf(&message, "INTEGER (%s) at %d:%d", t->chs, line, col);
    break;
  case SYMBOL:
    asprintf(&message, "SYMBOL (%s) at %d:%d", t->chs, line, col);
    break;
  case TAG:
    asprintf(&message, "TAG (%s) at %d:%d", t->chs, line, col);
    break;
  case STRING:
    asprintf(&message, "STRING (%s) at %d:%d", t->chs, line, col);
    break;
  case APOSTROPHE:
    asprintf(&message, "APOSTROPHE at %d:%d", line, col);
    break;
  default:
    asprintf(&message, "Token type %d at %d:%d", t->type, line, col);
    break;
  }
  return message;
};

int lexer_peek(lexer *s) {
  int ch = s->get_char(s->cookie);
  s->unget_char(s->cookie, ch);
  return ch;
}

void lexer_abort(lexer *s, char *message) {

}

void token_set(token* t, lexer *s, int token_type) {
  t->chs = s->token_chs;
  t->line_no = s->token_line_no;
  t->line_pos = s->token_line_pos;
  t->type = token_type;
}

int isnumsym(int ch) {
  if((ch == '(') || (ch == ')') || (ch == '"') || (ch == '\'')
     || isspace(ch))
    return FALSE;

  return TRUE;
}

void lexer_skip_whitespace_and_comments(lexer *s) {
  do {
    while(isspace(s->current_char))
      lexer_next_char(s);
    if(s->current_char == ';') {
      while(s->current_char != '\n')
        lexer_next_char(s);
      lexer_next_char(s);
    } else {
      return;
    }
  } while(TRUE);
}

void lexer_reset_token(lexer *s) {
  if (s->token_chs_cap == -1) {
    s->token_chs = calloc(1, sizeof(char));
    if (s->token_chs == NULL)
      exit(EXIT_FAILURE);
    s->token_chs_cap = 0;
  }
  s->token_line_no = s->current_line_no;
  s->token_line_pos = s->current_line_pos;
  s->n_token_chs = 0;
  s->token_chs[s->n_token_chs] = '\0';
}

int lexer_get_token(lexer *s, token *t) {
  token_type inferred_type;

  lexer_skip_whitespace_and_comments(s);
  lexer_reset_token(s);

  if (s->current_char == '\0') {
    token_set(t, s, END);
    lexer_next_char(s);

  } else if (s->current_char == '\'') {
    token_set(t, s, APOSTROPHE);
    lexer_next_char(s);

  } else if (s->current_char == '(') {
    token_set(t, s, OPEN_PAREN);
    lexer_next_char(s);

  } else if (s->current_char == ')') {
    token_set(t, s, CLOSE_PAREN);
    lexer_next_char(s);

  } else if (s->current_char == '"') {
    lexer_next_char(s);
    while (s->current_char != '"')
      lexer_next_char(s);
    token_set(t, s, STRING);
    lexer_next_char(s);

  } else if (s->current_char == '#') {
    lexer_next_char(s);
    while (isnumsym(s->current_char))
      lexer_next_char(s);
    token_set(t, s, TAG);

  } else {
    inferred_type = INTEGER;

    if ((s->current_char == '-') || (s->current_char == '+')
        || isdigit(s->current_char)) {
      /* Do nothing. */
    } else {
      inferred_type = SYMBOL;
    }

    if (!isnumsym(lexer_peek(s))) {
      if (isdigit(s->current_char))
        token_set(t, s, INTEGER);
      else
        token_set(t, s, SYMBOL);

      lexer_next_char(s);
    } else {
      lexer_next_char(s);
      while(isnumsym(s->current_char)) {
        if (!isdigit(s->current_char)) {
          inferred_type = SYMBOL;
        }
        lexer_next_char(s);
      }
      token_set(t, s, inferred_type);
    }
  }

  return (t->type != END);
}

/* Parser */

typedef struct parser {
  lexer lex;
  token current_token, next_token;
} parser;

void parser_next_token(parser *p);

void init_parser(parser *p, get_char_fn get, unget_char_fn unget) {
  init_lexer(&p->lex, get, unget);
  p->current_token.chs = p->next_token.chs = NULL;
  parser_next_token(p);
  parser_next_token(p);
}

void destroy_parser(parser *p) {
  destory_lexer(&p->lex);
  if (p->current_token.chs != NULL)
    free(p->current_token.chs);
}

void dump_parser(parser *p) {
  fprintf(stderr, "<Parser %p current_token.chs: %p next_token.chs: %p>\n",
          p, p->current_token.chs, p->next_token.chs);
}

void parser_next_token(parser *p) {
  char *chs;

  if (p->current_token.chs != NULL)
    free(p->current_token.chs);

  if (p->next_token.chs != NULL) {
    chs = strdup(p->next_token.chs);
    if (chs == NULL)
      exit(EXIT_FAILURE);
  } else {
    chs = NULL;
  }

  p->current_token = p->next_token;
  p->current_token.chs = chs;

  lexer_get_token(&p->lex, &p->next_token);
}

/* Main code */

int ungotten = '\0';
void stdin_unget_char(void *cookie, int ch) {
  ungotten = ch;
}

int stdin_get_char(void *cookie) {
  int ch;

  if (ungotten != '\0') {
    ch = ungotten;
    ungotten = '\0';
    return ch;
  } else {
    ch = getchar();
    if (ch == EOF)
      return '\0';
    else
      return ch;
  }
}

int main(int argc, char **argv) {
  char *description;
  parser parser;

  init_parser(&parser, stdin_get_char, stdin_unget_char);

  while (parser.current_token.type != END) {
    description = token_description_alloc(&parser.current_token);
    if (description == NULL)
      exit(EXIT_FAILURE);
    printf("%s\n", description);
    free(description);
    parser_next_token(&parser);
  }

  destroy_parser(&parser);
  return 0;
}
