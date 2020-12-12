/* file: lisp.h
 * author: Jim Mayfield
 */

#ifndef LISP_HEADER
#define LISP_HEADER

typedef int BOOL;
#define string_equal(s1,s2)	(strcmp((s1),(s2)) == 0)

/*_________________Errors_________________*/

typedef enum {
  FIRST_OF_NONLIST,
  REST_OF_NONLIST,
  ILLFORMED_DOTTED_PAIR,
  TOO_MANY_RIGHT_PARENS,
  ILLEGAL_FUNCTION_SPEC,
  EOF_IN_LIST,
  BAD_FILE_SPEC,
  FILE_OPEN_FAILURE,
  BAD_DEFUN,
  TOO_FEW_ARGS,
  TOO_MANY_ARGS
} lisp_error_type;

/*_________________SEXPRS and Tokens_________________*/

/* If a symbol has a function associated with it, it can either be
   a special form, a built-in (i.e. compiled) function, or a user-
   defined function (i.e. a lambda form). */
typedef enum {
  NO_FN,
  SPECIAL_FORM,
  BUILTIN,
  USER_DEFINED
} lisp_function_type;

typedef enum {

  /* s-expressions */
  SYMBOL,
  CONS_CELL,

  /* tokens */
  DOT,
  LEFT_PAREN,
  RIGHT_PAREN,
  END_OF_INPUT

} lisp_object_type;

typedef struct lisp_object_tag *lisp_object;

typedef struct lisp_object_tag {
  lisp_object_type obj_type;
  union {
    char *token_name;
    struct {
      lisp_object car;
      lisp_object cdr;
    } cons_field;
    struct {
      char *symbol_name;
      lisp_function_type function_type;
      void *function_def;
      int numargs;
    } symbol_field;
  } fields;
} lisp_object_struct;

/*_________________Symbol Table_________________*/

#define HASH_TABLE_SIZE	269
#define HASH_MULTIPLIER	131
#define MAX_NAME_LEN	128

typedef lisp_object hash_table_type[HASH_TABLE_SIZE];

/*_________________Environments_________________*/

typedef lisp_object environment_type;

#define THE_EMPTY_ENVIRONMENT	nil_object

/*_________________Function Types_________________*/

#define MAX_ARGS_TO_BUILTIN	2

typedef lisp_object (*fn_0args)(void);
typedef lisp_object (*fn_1arg)(lisp_object);
typedef lisp_object (*fn_2args)(lisp_object, lisp_object);
typedef lisp_object (*special_form_eval_fn)(lisp_object, environment_type);

/*_________________Basic Selectors, Predicates and Constructors_________________*/

/* Selectors */
#define second(x)		(first(rest(x)))
#define third(x)		(first(rest(rest(x))))
#define lambda_vars(x)		(second(x))
#define lambda_body(x)		(third(x))
#define if_part(clause)		first(clause)
#define then_part(clause)	second(clause)
#define binding_symbol(binding)	first(binding)
#define binding_value(binding)	rest(binding)
#define object_type(object)	((object)->obj_type)
#define token_name(token)	((token)->fields.token_name)
#define symbol_name(symbol)	((symbol)->fields.symbol_field.symbol_name)
#define function_type(s)	((s)->fields.symbol_field.function_type)
#define function_def(s)		((s)->fields.symbol_field.function_def)
#define function_numargs(s)	((s)->fields.symbol_field.numargs)
#define car_field(s)		((s)->fields.cons_field.car)
#define cdr_field(s)		((s)->fields.cons_field.cdr)

/* Predicates */
#define is_token(s)		((s)->obj_type == DOT || \
				 (s)->obj_type == LEFT_PAREN || \
				 (s)->obj_type == RIGHT_PAREN || \
				 (s)->obj_type == END_OF_INPUT)
#define is_symbol(s)		((s)->obj_type == SYMBOL)
#define is_cons(s)		((s)->obj_type == CONS_CELL)
#define is_list(s)		((s) == nil_object || is_cons(s))
#define is_lambda(s)		(is_cons(s) && first(s) == intern("lambda"))
#define is_self_evaluating(s)	((s) == nil_object || (s) == intern("t"))
#define is_special_form(s)	(is_cons(s) && is_symbol(first(s)) && function_type(first(s)) == SPECIAL_FORM)

/* Constructors */
#define make_binding(s,v)	cons((s),(v))
#define make_lambda(arglist,body)	(cons(intern("lambda"), \
					      cons((arglist), cons((body), nil_object))))

/*_________________Function Prototypes_________________*/

lisp_object
first(lisp_object obj);

lisp_object
rest(lisp_object obj);

lisp_object
cons(lisp_object obj1, lisp_object obj2);

lisp_object
intern (char *name);

lisp_object
lisp_read(FILE *infile);

lisp_object
read_head(FILE *);

lisp_object
eval (lisp_object obj, environment_type environment);

lisp_object
eval_cond(lisp_object obj, environment_type environment);

lisp_object
eval_list(lisp_object obj, environment_type environment);

environment_type
make_environment(lisp_object formals, lisp_object actuals, environment_type environment);

void
print_body(lisp_object obj, FILE *outfile);

void
lisp_print(lisp_object obj, FILE *outfile);

lisp_object
lisp_error(lisp_error_type error, lisp_object object);

#endif /* ifndef LISP_HEADER */
