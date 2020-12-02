import sys

class REPL:
    def __init__(self):
        pass

    def repl_read(self, line: str) -> str:
        return line
    
    def repl_eval(self, ast: list, env: dict) -> str:
        return ast
    
    def repl_print(self, exp: list) -> str:
        return self.pr_str(exp)
    
    def repl_rep(self, line: str) -> str:
        ret = self.repl_read(line)
        ast = self.parse(self.read_str(ret))
        return self.repl_print(self.repl_eval(ast, {}))

    def read_str(self, s: str) -> list:
        "Convert a string into a list of tokens."
        return s.replace('(',' ( ').replace(')',' ) ').split()

    def parse(self, tokens: list) -> list:
        token = tokens.pop(0)
        if '(' == token:
            L = []
            while tokens[0] != ')':
                L.append(self.parse(tokens))
            tokens.pop(0) # pop off ')'
            return L
        else:
            return str(token)


    def pr_str(self, ast: list) -> str:
        "Convert a Python object back into a Lisp-readable string."
        if isinstance(ast, list):
            return '(' + ' '.join(map(self.pr_str, ast)) + ')'
        else:
            return str(ast)
        


#---------------------------------------

class terminal(object):
    def __init__(self, func):
        self.func = func
    
    def __call__(self) -> str:
        print("user> ", end="")
        return self.func()


@terminal
def my_readline() -> str:
	return input()
#---------------------------------------

# repl loop
while True:
    try:
        repl = REPL()
        line = my_readline()
        if line == None: break
        if line == "": continue
        print(repl.repl_rep(line))
    except EOFError:
        exit()
    except Exception as e:
        print("".join(traceback.format_exception(*sys.exc_info())))
