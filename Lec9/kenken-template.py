from z3 import *

# Specify a board by declaring its dimensions
# followed by a list of cages and target answers

# The cages in this page
# https://en.wikipedia.org/wiki/KenKen
# can be represented as follows
dim = 6
cages = [
    ['+', 11, [(1,1), (2,1)]],  # val, op, argcoords
    ['/',  2, [(1,2), (1,3)]],
    ['*', 20, [(1,4), (2,4)]],
    ['*',  6, [(1,5), (1,6), (2,6), (3,6)]],
    ['-',  3, [(2,2), (2,3)]], 
    ['/',  3, [(2,5), (3,5)]],    
    ['*',240, [(3,1), (3,2), (4,1), (4,2)]],
    ['*',  6, [(3,3), (3,4)]],
    ['*',  6, [(4,3), (5,3)]],
    ['+',  7, [(4,4), (5,4), (5,5)]],
    ['*', 30, [(4,5), (4,6)]],
    ['*',  6, [(5,1), (5,2)]],
    ['+',  9, [(5,6), (6,6)]],
    ['+',  8, [(6,1), (6,2), (6,3)]],
    ['/',  2, [(6,4), (6,5)]]
    ]

# dim x dim matrix of integer variables
# x_1_1 ... x_6_6 are the constrained int vars
# X[0,0] thru x[5,5] are the user-level cells
# ... each of which bears the int vars

X = [ [ Int("x_%s_%s" % (i+1, j+1)) for j in range(dim) ]
      for i in range(dim) ]

# Now, each cell contains a value in {1, ..., dim}
cells_c  = [ And(1 <= X[i][j], X[i][j] <= dim)
             for i in range(dim) for j in range(dim) ]

# each row contains a digit at most once
rows_c   = [ Distinct(X[i]) for i in range(dim) ]

# each column contains a digit at most once
cols_c   = [ Distinct([ X[i][j] for i in range(dim) ])
             for j in range(dim) ]

from functools import reduce

def add_one_cage_constr(cage, s):

def add_one_cage_constr(cage, s):
    '''Given a cage and a solver s, s.add the correct constraint'''
    def val(cage):
        return cage[1]
    def op(cage):
        return cage[0]
    def argcoords(cage):
        return cage[2]
    vals = list(map(lambda p: X[p[0]-1] [p[1]-1], argcoords(cage)))
    def plus_expr(cage):
        return reduce(lambda x,y: x+y, vals)
    def times_expr(cage):
        return reduce(lambda x,y: x*y, vals)
    arg0 = argcoords(cage)[0] # arg0 and arg1 are coord pairs
    arg1 = argcoords(cage)[1] # applicable for the '-' and '/' cases only
    if op(cage)=='+':
        # print("adding ", val(cage), " equals ", plus_expr(cage))
        s.add( val(cage) == plus_expr(cage) )
    elif op(cage)=='*':
        ...
    elif (cage[0]=='-'): #-- finish
    elif (cage[0]=='/'): #--not quite correct below - fix it !!
        s.add( val(cage) == X[arg0[0]-1][arg0[1]-1] / X[arg1[0]-1][arg1[1]-1] )  
    else:
        print("Erroneous cage operator")
#--------------- 
            
s = Solver()

s.add( cells_c + rows_c + cols_c )

for cage in cages:
    add_one_cage_constr(cage, s)

if s.check() == sat:
    m = s.model()
    r = [ [ m.evaluate(X[i][j]) for j in range(dim) ]
          for i in range(dim) ]
    print_matrix(r)
else:
    print("failed to solve")
