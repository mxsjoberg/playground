import sys
# import modules at runtime
# import importlib

# class Module:
#   def add(_type, x1, x2):
#       if _type == "float":
#           return float(x1) + float(x2)
#       elif _type == "int":
#           return int(x1) + int(x2)

# program = """
# Module add float 1 2
# """

# tokens = program.split()
# print(tokens)

# result = getattr(getattr(sys.modules[__name__], tokens[0]), tokens[1])(tokens[2], tokens[3], tokens[4])
# print(result)

# args and kwargs
class Module:
    def add(*args, **kwargs):
        type_ = globals()['__builtins__'].getattr(globals()['__builtins__'], kwargs['type'])
        return sum(map(type_, args))

program = """
Module add 1 2 3 type=float
"""
tokens = program.split()
# print(tokens)

# def get_help(fname):


def get_args(tokens):
    args = []
    kwargs = {}
    for token in tokens:
        if '=' in token:
            k, v = token.split('=', 1)
            kwargs[k] = v
        else:
            args.append(token)

    return args, kwargs

args, kwargs = get_args(tokens[2:])
print(args, kwargs)
# print(getattr(sys.modules[__name__], tokens[0]))
# print(getattr(getattr(sys.modules[__name__], tokens[0]), tokens[1]))

result = getattr(getattr(sys.modules[__name__], tokens[0]), tokens[1])(*args, **kwargs)
print(result)


