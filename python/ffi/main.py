import ctypes

# load the shared library
square_lib = ctypes.CDLL('./square.so')

# specify function return and argument types
square_lib.square.restype = ctypes.c_int
square_lib.square.argtypes = [ctypes.c_int]

# call the foreign function
num = 5
result = square_lib.square(num)

print(f"the square of {num} is {result}")
# the square of 5 is 25

# libc -> https://gist.github.com/PewZ/8b473c2a6888c5c528635550d07c6186
libc = ctypes.CDLL('libc.dylib')
libc.printf(b"hello world\n")
# hello world
