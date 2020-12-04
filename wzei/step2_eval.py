import sys
import types

def RepresentsInt(s):
    try: 
        int(s)
        return True
    except ValueError:
        return False

def repl_read(line: str) -> str:
    return line

def repl_eval(ast: list, env: dict) -> int:
    arg = []
    for i in ast:
        if type(i) is list:
            arg.append(repl_eval(i, env))
        elif i in env:
            arg.append(env[i])
        elif RepresentsInt(i):
            arg.append(int(i))
    return arg[0](arg[1], arg[2])

def repl_print(exp: list) -> str:
    return pr_str(exp)

def repl_rep(line: str) -> str:
    ret = repl_read(line)
    ast = parse(read_str(ret))
    return repl_print(repl_eval(ast, repl_env))

def read_str(s: str) -> list:
    "Convert a string into a list of tokens."
    return s.replace('(',' ( ').replace(')',' ) ').split()

def parse(tokens: list) -> list:
    token = tokens.pop(0)
    if '(' == token:
        L = []
        while tokens[0] != ')':
            L.append(parse(tokens))
        tokens.pop(0) # pop off ')'
        return L
    else:
        return str(token)


def pr_str(ast: list) -> str:
    "Convert a Python object back into a Lisp-readable string."
    if isinstance(ast, list):
        return '(' + ' '.join(map(pr_str, ast)) + ')'
    else:
        return str(ast)

def my_readline() -> str:
    print("user> ", end="")
    return input()
#---------------------------------------

repl_env = {}
repl_env['+'] = lambda a,b: a+b
repl_env['-'] = lambda a,b: a-b
repl_env['*'] = lambda a,b: a*b
repl_env['/'] = lambda a,b: int(a/b)

# repl loop
def repl():
    while True:
        try:
            line = my_readline()
            if line == None: break
            if line == "": continue
            print(repl_rep(line))
        except EOFError:
            exit()
        except Exception as e:
            print("".join(traceback.format_exception(*sys.exc_info())))


if __name__ == '__main__':
    repl()