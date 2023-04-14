# https://en.wikipedia.org/wiki/Delta_rule
import numpy as np
from prettytable import PrettyTable

# load iris dataset
# -----------------------------------------
from sklearn import datasets

iris = datasets.load_iris()

# configuration variables
# -----------------------------------------------------------
# initial values
w = [1, 2, 3, 4]

# learning rate
n = 0.1

# iterations
iterations = 100 * len(iris.data)

# dataset
# -----------------------------------------------------------
X = iris.data

A = []
for i in range(len(iris.target)):
    # augment iris data
    A.append([iris.target[i], X[i][0], X[i][1], X[i][2], X[i][3]])

# shuffle augmented samples
np.random.shuffle(A)

# training set
X = []
for i in range(len(A[:125])): X.append(A[i][1:])
X_t = []
for i in range(len(A[:125])): X_t.append(A[i][0])

# test set
Y = []
for i in range(len(A[125:])): Y.append(A[i][1:])
Y_t = []
for i in range(len(A[125:])): Y_t.append(A[i][0])

# transfer_function
# -----------------------------------------------------------
def transfer_function(w, x):
    wx = np.dot(w, x)
    if (wx > 10):
        return 2
    elif (wx > 0.5):
        return 1
    else:
        return 0

# sequential delta learning algorithm
# -----------------------------------------------------------
result = []
for o in range(int(iterations / len(X))):
    for i in range(len(X)):
        if ((i + 1 + (len(X) * o)) > iterations):
            break

        w_prev = w
        x = X[i]
        y = transfer_function(w, x)

        # calculate update part
        update = np.zeros(len(x))
        for j in range(len(x)):
            update[j] = n * (X_t[i] - y) * x[j]
        
        # add update part to a
        w = np.add(w, update)
        # append result
        result.append((str(i + 1 + (len(X) * o)), np.round(w_prev, 4), np.round(x, 4), np.round(y, 4), np.round(X_t[i], 4), np.round(w, 4)))

# prettytable
# -----------------------------------------------------------
pt = PrettyTable(('iteration', 'w', 'x', 'y = H(wx)', 't', 'w_new'))
for row in result: pt.add_row(row)

pt.align['iteration'] = 'c'
pt.align['w'] = 'l'
pt.align['x'] = 'l'
pt.align['y = H(wx)'] = 'l'
pt.align['t'] = 'c'
pt.align['w_new'] = 'l'

#print(pt)
print(pt[-1])

# TEST: accuracy
# -----------------------------------------------------------
w_final = result[-1][1]
result = []
pt_test = PrettyTable(('sample, w=' + str(w_final), 'predicted', 'target', 'correct?'))
for i in range(len(Y)):
    x = Y[i]
    c = transfer_function(w_final, x)
    pt_test.add_row([Y[i], c, Y_t[i], c == Y_t[i]])
    # append to result
    result.append(c == Y_t[i])

print(pt_test)
print('Accuracy: ' + str(result.count(True) / len(result)))
