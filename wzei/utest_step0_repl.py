import unittest
from step0_repl import REPL


class REPL_read_Test(unittest.TestCase):
    def test_read_00(self):
        repl = REPL()
        self.assertEqual(repl.repl_read(""), "")

    def test_read_01(self):
        repl = REPL()
        self.assertEqual(repl.repl_read("abcABC123"), "abcABC123")

    def test_read_02(self):
        repl = REPL()
        self.assertEqual(repl.repl_read("hello mal world"), "hello mal world")

    def test_read_03(self):
        repl = REPL()
        self.assertEqual(repl.repl_read('''[]{}"'* ;:()'''), '''[]{}"'* ;:()''')
        
if __name__ == '__main__':
    unittest.main()