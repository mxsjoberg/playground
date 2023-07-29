# min max problem
def min_max():
	# python
	y = lambda x : x * (10 - x)
	def MAX(fn, res, range_):
	    for n in range_: res.append((n, fn(n)))
	    return print(max(res, key = lambda tuple: tuple[1]))

	MAX(y, [], range(100))
	# (5, 25)

	y = lambda x : -x * (10 - x)
	def MIN(fn, res, range_):
	    for n in range_: res.append((n, fn(n)))
	    return print(min(res, key = lambda tuple: tuple[1]))

	MIN(y, [], range(100))
	# (5, -25)

# min_max()

import numpy as np

A = np.random.randint(1, 100, size = (3, 3))
b = np.random.randint(1, 100, size = (3, 1))

x = (np.linalg.inv(A.T @ A)) @ A.T @ b
print(np.allclose(A @ x, b))

# or numpy built in
print(np.allclose(A @ np.linalg.lstsq(A, b, rcond = -1)[0], b))
