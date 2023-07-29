# binary convert tools

def decimal(binary):
    binary_lst = [*binary]
    decimal = 0
    n_power = len(binary_lst) - 1
    for num in binary_lst:
        if int(num) > 0:
            decimal += 2 ** n_power
        n_power -= 1
    return decimal

print(decimal("1101"))