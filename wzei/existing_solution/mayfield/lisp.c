/* file: lisp.c
 * author: Jim Mayfield
 * This program is a simple implementation of LISP.  It comprises:
 *    > Error routines
 *    > Routines to create symbols
 *    > Primitive LISP functions
 *    > Read routines
 *    > Write routines
 *    > Eval and Apply
 *    > A read-eval-print loop
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "lisp.h"

/*_________________Commonly used symbols and tokens_________________*/

lisp_object nil_object;
lisp_object t_object;
lisp_object dot_token;
lisp_object left_paren_token;
lisp_object right_paren_token;
lisp_object end_of_input_token;

/*_________________Error Handling_________________*/

struct error_msg_tag {
  lisp_error_type error;
  char *message;
} error_message_table[] = {
  FIRST_OF_NONLIST,		"Attempt to take first of a non-list",
  REST_OF_NONLIST,		"Attempt to take rest of a non-list",
  ILLFORMED_DOTTED_PAIR,	"Illformed dotted pair",
  TOO_MANY_RIGHT_PARENS,	"Too many right parentheses",
  ILLEGAL_FUNCTION_SPEC,	"Illegal function specification",
  EOF_IN_LIST,			"Premature end of file",
  BAD_FILE_SPEC,		"Illegal filename specification",
  FILE_OPEN_FAILURE,		"Unable to open file",
  BAD_DEFUN,			"Illegal defun syntax",
  TOO_FEW_ARGS,			"Too few arguments to function",
  TOO_MANY_ARGS,		"Too many arguments to function",
};
#define NUM_NONFATAL_MSGS (sizeof(error_message_table)/sizeof(struct error_msg_tag))

/* Report an error, returning nil or dying as appropriate */
lisp_object
lisp_error(lisp_error_type error, lisp_object object)
{
  int i;

  for (i = 0; i < NUM_NONFATAL_MSGS; i++)
    if (error_message_table[i].error == error) {
      fprintf(stderr, "ERROR: %s.\nOffending object: ", error_message_table[i].message);
      lisp_print(object, stderr);
      fprintf(stderr, "\n");
      return(nil_object);
    }
  fprintf(stderr, "MYSTERIOUS INTERNAL ERROR.\n");
  exit(EXIT_FAILURE);
}

/*_________________Symbols_________________*/

hash_table_type hash_table;

/* Return a hash value for this name. */
int
hash(char *name)
{
  int value = 0;

  while (*name != '\0')
    value = (value * HASH_MULTIPLIER + *name++) % HASH_TABLE_SIZE;
  return(value);
}

/* Return the symbol with this name, or NULL if it is not found. */
lisp_object
lookup(char *name, lisp_object symbols)
{
  if (symbols == nil_object)
    return(NULL);
  else if (string_equal(symbol_name(first(symbols)), name))
    return(first(symbols));
  else return(lookup(name, rest(symbols)));
}

lisp_object
create_symbol(char *name)
{
  lisp_object symbol;
  char *symbol_name;

  symbol = malloc(sizeof(lisp_object_struct));
  assert (symbol != NULL);
  symbol_name = malloc(strlen(name) + 1);
  assert(symbol_name != NULL);
  strcpy(symbol_name, name);
  object_type(symbol) = SYMBOL;
  symbol_name(symbol) = symbol_name;
  function_type(symbol) = NO_FN;
  function_def(symbol) = NULL;
  function_numargs(symbol) = -1;
  return(symbol);
}

/* Add this symbol to the hash table. */
void
enter(lisp_object symbol)
{
  int hash_val;

  hash_val = hash(symbol_name(symbol));
  hash_table[hash_val] = cons(symbol, hash_table[hash_val]);
}

/* Return the symbol with this name, creating it if necessary. */
lisp_object
intern (char *name)
{
  lisp_object symbol;

  symbol = lookup(name, hash_table[hash(name)]);
  if (symbol != NULL)
    return(symbol);
  else {
    symbol = create_symbol(name);
    enter(symbol);
    return(symbol);
  }
}

/*_________________Lisp Primitives_________________*/

lisp_object
first(lisp_object obj)
{
  if (is_cons(obj))
    return(car_field(obj));
  else return(lisp_error(FIRST_OF_NONLIST, obj));
}

lisp_object
rest(lisp_object obj)
{
  if (is_cons(obj))
    return(cdr_field(obj));
  else return(lisp_error(REST_OF_NONLIST, obj));
}

lisp_object
cons(lisp_object obj1, lisp_object obj2)
{
  lisp_object new = malloc(sizeof(lisp_object_struct));

  assert(new != NULL);
  object_type(new) = CONS_CELL;
  car_field(new) = obj1;
  cdr_field(new) = obj2;
  return(new);
}

lisp_object
atom(lisp_object obj)
{
  if (is_symbol(obj))
    return(t_object);
  else return(nil_object);
}

lisp_object
eql(lisp_object obj1, lisp_object obj2)
{
  if (obj1 == obj2)
    return(t_object);
  else return(nil_object);
}

/*_________________Input Routines_________________*/

/* This is the lisp tokenizer; it returns a symbol, or one of `(', `)', `.', or EOF */
lisp_object
ratom(FILE *infile)
{
  int c;
  static char buf[MAX_NAME_LEN];
  char *ptr = buf;

  do {
    c = getc(infile);
    if (c == ';')
      do c = getc(infile); while (c != '\n' && c != EOF);
  } while (isspace(c));
  switch (c) {
  case EOF:
    return(end_of_input_token);
  case '(':
    return(left_paren_token);
  case ')':
    return(right_paren_token);
  case '.':
    return(dot_token);
  default:
    *ptr++ = c;
    while ((c = getc(infile)) != EOF && !isspace(c) && c != '(' && c != ')')
      *ptr++ = c;
    /* Return the unused character to the input. */
    if (c != EOF)
      ungetc(c, infile);
    *ptr = '\0';
    return(intern(buf));
  }
}

/* Read just one more cdr for this s-expression. */
lisp_object
read_cdr(FILE *infile)
{
  lisp_object cdr;
  lisp_object token;

  cdr = lisp_read(infile);
  token = ratom(infile);

  if (object_type(token) == RIGHT_PAREN)
    return(cdr);
  else return(lisp_error(ILLFORMED_DOTTED_PAIR, cdr));
}

/* Read the remainder of this list. */
lisp_object
read_tail(FILE *infile)
{
  lisp_object token;
  lisp_object temp;

  token = ratom(infile);
  switch(object_type(token)) {
  case SYMBOL:
    return(cons(token, read_tail(infile)));
  case LEFT_PAREN:
    /* Make sure the read_head is done first. */
    temp = read_head(infile);
    return(cons(temp, read_tail(infile)));
  case DOT:
    return(read_cdr(infile));
  case RIGHT_PAREN:
    return(nil_object);
  case END_OF_INPUT:
    return(lisp_error(EOF_IN_LIST, token));
  }
}

/* Read a list. */
lisp_object
read_head(FILE *infile)
{
  lisp_object token;
  lisp_object temp;

  token = ratom(infile);
  switch(object_type(token)) {
  case SYMBOL:
    return(cons(token, read_tail(infile)));
  case LEFT_PAREN:
    /* Make sure the read_head is done first. */
    temp = read_head(infile);
    return(cons(temp, read_tail(infile)));
  case RIGHT_PAREN:
    return(nil_object);
  case DOT:
    return(lisp_error(ILLFORMED_DOTTED_PAIR, token));
  case END_OF_INPUT:
    return(lisp_error(EOF_IN_LIST, token));
  }
}

/* Read in and return one s-expression. Return the token (not the symbol)
   end_of_input_token on EOF. */
lisp_object
lisp_read(FILE *infile)
{
  lisp_object token;

  token = ratom(infile);
  switch(object_type(token)) {
  case SYMBOL:
    return(token);
  case LEFT_PAREN:
    return(read_head(infile));
  case RIGHT_PAREN:
    return(lisp_error(TOO_MANY_RIGHT_PARENS, token));
  case DOT:
    return(lisp_error(ILLFORMED_DOTTED_PAIR, token));
  case END_OF_INPUT:
    return(end_of_input_token);
  }
}

lisp_object
load(lisp_object filename)
{
  if (!is_symbol(filename))
    return(lisp_error(BAD_FILE_SPEC, filename));
  else {
    FILE *infile;

    infile = fopen(filename->fields.symbol_field.symbol_name, "r");
    if (infile == NULL)
      return(lisp_error(FILE_OPEN_FAILURE, filename));
    else {
      lisp_object obj;

      while ((obj = lisp_read(infile)) != end_of_input_token) {
	lisp_print(eval(obj, THE_EMPTY_ENVIRONMENT), stdout);
	putchar('\n');
      }
      return(t_object);
    }
  }
}

lisp_object
read_from_stdin(void)
{
  return(lisp_read(stdin));
}

/*_________________Output Routines_________________*/

/* Print an s-expression on the specified file handle. */
void
lisp_print(lisp_object obj, FILE *outfile)
{
  if (is_token(obj))
    fprintf(outfile, "__TOKEN_%s__", obj->fields.token_name);
  else if (is_symbol(obj))
    fprintf(outfile, "%s", obj->fields.symbol_field.symbol_name);
  else {
    putc('(', outfile);
    print_body(obj, outfile);
  }
}

/* Print the remainder of this s-expression. */
void
print_body(lisp_object obj, FILE *outfile)
{
  lisp_print(first(obj), outfile);
  if (!is_list(rest(obj))) {
    fprintf(outfile, " . ");
    lisp_print(rest(obj), outfile);
    putc(')', outfile);
  }
  else if (rest(obj) == nil_object)
    putc(')', outfile);
  else {
    putc(' ', outfile);
    print_body(rest(obj), outfile);
  }
}

lisp_object
print_to_stdout(lisp_object object)
{
  lisp_print(object, stdout);
  putchar('\n');
  return(t_object);
}

/*_________________Eval and Apply_________________*/

int
length(lisp_object list)
{
  if (list == nil_object)
    return(0);
  else if (!is_cons(list))
    return(-1); /* ????? */
  else return(1 + length(rest(list)));
}

lisp_object
symbol_value(lisp_object obj, environment_type environment)
{
  if (environment == nil_object)
    return(nil_object);
  else if (obj == binding_symbol(first(environment)))
    return(binding_value(first(environment)));
  else return(symbol_value(obj, rest(environment)));
}

environment_type
make_environment(lisp_object vars, lisp_object values, environment_type environment)
{
  if (vars == nil_object && values == nil_object)
    return(environment);
  else if (vars == nil_object)
    return(lisp_error(TOO_MANY_ARGS, values));
  else if (values == nil_object)
    return(lisp_error(TOO_FEW_ARGS, vars));
  else return(cons(make_binding(first(vars), first(values)),
		   make_environment(rest(vars), rest(values), environment)));
}

lisp_object
apply(lisp_object fn, lisp_object args, environment_type environment)
{
  if (is_symbol(fn) && function_type(fn) == BUILTIN) {
    if (length(args) > function_numargs(fn))
      return(lisp_error(TOO_MANY_ARGS, args));
    else if (length(args) < function_numargs(fn))
      return(lisp_error(TOO_FEW_ARGS, args));
    else switch (function_numargs(fn)) {
    case 0:
      return(((fn_0args) function_def(fn))());
    case 1:
      return(((fn_1arg) function_def(fn))(first(args)));
    case 2:
      return(((fn_2args) function_def(fn))(first(args), second(args)));
    }
  }
  else if (is_symbol(fn) && function_type(fn) == USER_DEFINED)
    return(apply((lisp_object) function_def(fn), args, environment));
  else if (is_lambda(fn))
    return(eval(lambda_body(fn),
		make_environment(lambda_vars(fn), args, environment)));
  else return(lisp_error(ILLEGAL_FUNCTION_SPEC, fn));
}

lisp_object
eval_defun(lisp_object fn, environment_type environment)
{
  if (!is_cons(fn) ||
      !is_symbol(first(fn)) ||
      !is_cons(rest(fn)) ||
      !is_list(second(fn)) ||
      !is_cons(rest(rest(fn))))
    return(lisp_error(BAD_DEFUN, fn));
  else {
    function_type(first(fn)) = USER_DEFINED;
    function_def(first(fn)) = make_lambda(second(fn), third(fn));
    return(first(fn));
  }
}

lisp_object
eval_cond(lisp_object clauses, environment_type environment)
{
  if (clauses == nil_object)
    return(nil_object);
  else if (eval(if_part(first(clauses)), environment) != nil_object)
    return(eval(then_part(first(clauses)), environment));
  else return(eval_cond(rest(clauses), environment));
}

lisp_object
eval_quote(lisp_object object, environment_type environment)
{
  return(first(object));
}

lisp_object
list_eval(lisp_object list, environment_type environment)
{
  if (list == nil_object)
    return(nil_object);
  else return(cons(eval(first(list), environment), list_eval(rest(list), environment)));
}

void
def_special_form(char *name, special_form_eval_fn eval_fn)
{
  lisp_object symbol;

  symbol = intern(name);
  function_type(symbol) = SPECIAL_FORM;
  function_def(symbol) = eval_fn;
  function_numargs(symbol) = 1;
}

lisp_object
eval (lisp_object obj, environment_type environment)
{
  if (is_self_evaluating(obj))
    return(obj);
  else if (is_symbol(obj))
    return(symbol_value(obj, environment));
  else if (is_special_form(obj))
    return(((special_form_eval_fn)(function_def(first(obj))))(rest(obj), environment));
  else return(apply(first(obj), list_eval(rest(obj), environment), environment));
}

/*_________________Initialization_________________*/

/* Associate a particular built-in function with the named symbol. */
void
def_builtin(char *name, void *fn, int numargs)
{
  lisp_object symbol;

  assert(numargs >= 0 && numargs <= MAX_ARGS_TO_BUILTIN);
  symbol = intern(name);
  function_type(symbol) = BUILTIN;
  function_numargs(symbol) = numargs;
  function_def(symbol) = fn;
}

lisp_object
create_token(lisp_object_type type, char *name)
{
  lisp_object token;

  token = malloc(sizeof(lisp_object_struct));
  assert(token != NULL);
  object_type(token) = type;
  token_name(token) = name;
  return(token);
}

/* Perform startup initialization */
void
init(void)
{
  int i;

  /* Tokens */
  left_paren_token = create_token(LEFT_PAREN, "(");
  right_paren_token = create_token(RIGHT_PAREN, ")");
  dot_token = create_token(DOT, ".");
  end_of_input_token = create_token(END_OF_INPUT, "EOF");

  /* Symbol Table */
  nil_object = create_symbol("nil");
  for (i = 0; i < HASH_TABLE_SIZE; i++)
    hash_table[i] = nil_object;
  enter(nil_object);
  t_object = intern("t");

  /* Function definitions */
  def_special_form("quote", eval_quote);
  def_special_form("cond", eval_cond);
  def_special_form("defun", eval_defun);

  def_builtin("first", first, 1);
  def_builtin("rest", rest, 1);
  def_builtin("cons", cons, 2);
  def_builtin("atom", atom, 1);
  def_builtin("eql", eql, 2);
  def_builtin("load", load, 1);
  def_builtin("read", read_from_stdin, 0);
  def_builtin("print", print_to_stdout, 1);
}

/*_________________Read-Eval-Print_________________*/

void
main(void)
{
  lisp_object sexpr;
  lisp_object value;

  init();
  for (;;) {
    printf("-> ");
    sexpr = lisp_read(stdin);
    if (sexpr == end_of_input_token)
      break;
    value = eval(sexpr, THE_EMPTY_ENVIRONMENT);
    lisp_print(value, stdout);
    putchar('\n');
  }
  printf("\nbye.\n");
}
