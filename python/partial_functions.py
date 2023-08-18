from functools import partial

def power(base, exp): return base ** exp

# partial functions
square = partial(power, exp=2)
cube = partial(power, exp=3)

print(square(2))
# 4
print(cube(2))
# 8

# partial functions with map
numbers = [1, 2, 3, 4, 5]
squares = map(square, numbers)
cubes = map(cube, numbers)

print(list(squares))
# [1, 4, 9, 16, 25]
print(list(cubes))
# [1, 8, 27, 64, 125]
