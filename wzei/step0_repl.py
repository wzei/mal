import sys

class REPL:
    def __init__(self):
        pass

    def repl_read(self, line: str) -> str:
        return line
    
    def repl_eval(self, ast: str, env: dict) -> str:
        return ast
    
    def repl_print(self, exp: str) -> str:
        return exp
    
    def repl_rep(self, line: str) -> str:
        return self.repl_print(self.repl_eval(self.repl_read(line), {}))

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
