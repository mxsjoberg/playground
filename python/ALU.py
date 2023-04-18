# ALU.py

# if (zx == 1)  set x = 0           : 16-bit
# if (nx == 1)  set x = ~x          : 16-bit bitwise not
# if (zy == 1)  set y = 0           : 16-bit constant
# if (ny == 1)  set y = ~y          : 16-bit bitwise not
# if (f == 1)   set out = x + y     : 16-bit twos complement addition
# if (f == 0)   set out = x & y     : 16-bit bitwise and
# if (no == 1)  set out = ~out      : 16-bit bitwise not
# if (out == 0) set zr = 1          : 1-bit
# if (out < 0)  set ng = 1          : 1-bit

def binary_to_signed(lst):
    binary_str = ''.join(map(str, lst))
    if (binary_str[0] == '1'):
        # invert bits
        inverted_str = ''.join(['1' if c == '0' else '0' for c in binary_str[1:]])
        decimal = -int(inverted_str, 2) - 1
    else:
        decimal = int(binary_str, 2)
    return decimal

assert binary_to_signed([1,1,1,1,1,1,1,1]) == -1

def twos_complement_addition(a, b):
    # convert to binary strings
    a_bin = ''.join(map(str, a))
    b_bin = ''.join(map(str, b))
    # bits
    bits = max(len(a_bin), len(b_bin))
    # convert to twos complement
    a_twos = int(a_bin, 2)
    b_twos = int(b_bin, 2)
    # addition
    result_twos = a_twos + b_twos
    # convert result to binary string
    result = bin(result_twos & (2 ** bits - 1))[2:]
    # convert result to list of ints
    result = list(map(int, result))
    # padding (if needed)
    if (len(result) < bits):
        for bit in range(bits - len(result)):
            # append to front
            result.insert(0, 0)
    return list(map(int, result))

assert twos_complement_addition([0,1,0,1], [0,1,0,1]) == [1,0,1,0]

def ALU(x, y, zx=0, nx=0, zy=0, ny=0, f=0, no=0):
    out = [0] * len(x)
    zr=0
    ng=0
    # if (zx == 1)  set x = 0
    if zx == 1:
        for i in range(len(x)): x[i] = 0
    # if (nx == 1)  set x = ~x
    if nx == 1:
        for i in range(len(x)):
            if x[i] == 1: x[i] = 0
            else: x[i] = 1
    # if (zy == 1)  set y = 0
    if zy == 1:
        for i in range(len(y)): y[i] = 0
    # if (ny == 1)  set y = ~y
    if ny == 1:
        for i in range(len(y)):
            if y[i] == 1: y[i] = 0
            else: y[i] = 1
    # if (f == 1)   set out = x + y
    if f == 1:
        out = twos_complement_addition(x, y)
    # if (f == 0)   set out = x & y
    if f == 0:
        for i in range(len(x)):
            if x[i] and y[i]: out[i] = 1
            else: out[i] = 0
    # if (no == 1)  set out = ~out
    if no == 1:
        for i in range(len(out)):
            if out[i] == 1: out[i] = 0
            else: out[i] = 1
    # if (out == 0) set zr = 1
    if all(bit == 0 for bit in out):
        zr = 1
    # if (out < 0)  set ng = 1
    out_decimal = binary_to_signed(out)
    if out_decimal < 0:
        ng = 1
    return out, zr, ng

# |        x         |        y         |zx |nx |zy |ny | f |no |       out        |zr |ng |
tests = """
  | 0000000000000000 | 1111111111111111 | 1 | 0 | 1 | 0 | 1 | 0 | 0000000000000000 | 1 | 0 |
  | 0000000000000000 | 1111111111111111 | 1 | 1 | 1 | 1 | 1 | 1 | 0000000000000001 | 0 | 0 |
  | 0000000000000000 | 1111111111111111 | 1 | 1 | 1 | 0 | 1 | 0 | 1111111111111111 | 0 | 1 |
  | 0000000000000000 | 1111111111111111 | 0 | 0 | 1 | 1 | 0 | 0 | 0000000000000000 | 1 | 0 |
  | 0000000000000000 | 1111111111111111 | 1 | 1 | 0 | 0 | 0 | 0 | 1111111111111111 | 0 | 1 |
  | 0000000000000000 | 1111111111111111 | 0 | 0 | 1 | 1 | 0 | 1 | 1111111111111111 | 0 | 1 |
  | 0000000000000000 | 1111111111111111 | 1 | 1 | 0 | 0 | 0 | 1 | 0000000000000000 | 1 | 0 |
  | 0000000000000000 | 1111111111111111 | 0 | 0 | 1 | 1 | 1 | 1 | 0000000000000000 | 1 | 0 |
  | 0000000000000000 | 1111111111111111 | 1 | 1 | 0 | 0 | 1 | 1 | 0000000000000001 | 0 | 0 |
  | 0000000000000000 | 1111111111111111 | 0 | 1 | 1 | 1 | 1 | 1 | 0000000000000001 | 0 | 0 |
  | 0000000000000000 | 1111111111111111 | 1 | 1 | 0 | 1 | 1 | 1 | 0000000000000000 | 1 | 0 |
  | 0000000000000000 | 1111111111111111 | 0 | 0 | 1 | 1 | 1 | 0 | 1111111111111111 | 0 | 1 |
  | 0000000000000000 | 1111111111111111 | 1 | 1 | 0 | 0 | 1 | 0 | 1111111111111110 | 0 | 1 |
  | 0000000000000000 | 1111111111111111 | 0 | 0 | 0 | 0 | 1 | 0 | 1111111111111111 | 0 | 1 |
  | 0000000000000000 | 1111111111111111 | 0 | 1 | 0 | 0 | 1 | 1 | 0000000000000001 | 0 | 0 |
  | 0000000000000000 | 1111111111111111 | 0 | 0 | 0 | 1 | 1 | 1 | 1111111111111111 | 0 | 1 |
  | 0000000000000000 | 1111111111111111 | 0 | 0 | 0 | 0 | 0 | 0 | 0000000000000000 | 1 | 0 |
  | 0000000000000000 | 1111111111111111 | 0 | 1 | 0 | 1 | 0 | 1 | 1111111111111111 | 0 | 1 |
  | 0000000000010001 | 0000000000000011 | 1 | 0 | 1 | 0 | 1 | 0 | 0000000000000000 | 1 | 0 |
  | 0000000000010001 | 0000000000000011 | 1 | 1 | 1 | 1 | 1 | 1 | 0000000000000001 | 0 | 0 |
  | 0000000000010001 | 0000000000000011 | 1 | 1 | 1 | 0 | 1 | 0 | 1111111111111111 | 0 | 1 |
  | 0000000000010001 | 0000000000000011 | 0 | 0 | 1 | 1 | 0 | 0 | 0000000000010001 | 0 | 0 |
  | 0000000000010001 | 0000000000000011 | 1 | 1 | 0 | 0 | 0 | 0 | 0000000000000011 | 0 | 0 |
  | 0000000000010001 | 0000000000000011 | 0 | 0 | 1 | 1 | 0 | 1 | 1111111111101110 | 0 | 1 |
  | 0000000000010001 | 0000000000000011 | 1 | 1 | 0 | 0 | 0 | 1 | 1111111111111100 | 0 | 1 |
  | 0000000000010001 | 0000000000000011 | 0 | 0 | 1 | 1 | 1 | 1 | 1111111111101111 | 0 | 1 |
  | 0000000000010001 | 0000000000000011 | 1 | 1 | 0 | 0 | 1 | 1 | 1111111111111101 | 0 | 1 |
  | 0000000000010001 | 0000000000000011 | 0 | 1 | 1 | 1 | 1 | 1 | 0000000000010010 | 0 | 0 |
  | 0000000000010001 | 0000000000000011 | 1 | 1 | 0 | 1 | 1 | 1 | 0000000000000100 | 0 | 0 |
  | 0000000000010001 | 0000000000000011 | 0 | 0 | 1 | 1 | 1 | 0 | 0000000000010000 | 0 | 0 |
  | 0000000000010001 | 0000000000000011 | 1 | 1 | 0 | 0 | 1 | 0 | 0000000000000010 | 0 | 0 |
  | 0000000000010001 | 0000000000000011 | 0 | 0 | 0 | 0 | 1 | 0 | 0000000000010100 | 0 | 0 |
  | 0000000000010001 | 0000000000000011 | 0 | 1 | 0 | 0 | 1 | 1 | 0000000000001110 | 0 | 0 |
  | 0000000000010001 | 0000000000000011 | 0 | 0 | 0 | 1 | 1 | 1 | 1111111111110010 | 0 | 1 |
  | 0000000000010001 | 0000000000000011 | 0 | 0 | 0 | 0 | 0 | 0 | 0000000000000001 | 0 | 0 |
  | 0000000000010001 | 0000000000000011 | 0 | 1 | 0 | 1 | 0 | 1 | 0000000000010011 | 0 | 0 |
"""

for test in tests.split('\n'):
    test = test.split('|')[1:-1]
    if (len(test) < 11):
        pass
    else:
        p = [col.strip() for col in test]
        # print(p[0:-3])
        # print(ALU(list(map(int, list(p[0]))), list(map(int, list(p[1]))), zx=int(p[2]), nx=int(p[3]), zy=int(p[4]), ny=int(p[5]), f=int(p[6]), no=int(p[7])))
        out, zr, ng = ALU(list(map(int, list(p[0]))), list(map(int, list(p[1]))), zx=int(p[2]), nx=int(p[3]), zy=int(p[4]), ny=int(p[5]), f=int(p[6]), no=int(p[7]))
        assert out == list(map(int, list(p[8])))
        assert zr == int(p[9])
        assert ng == int(p[10])
