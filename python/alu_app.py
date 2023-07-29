# alu_app.py
import tkinter as tk
import tkinter.font as tkfont

##############################
# ALU
##############################
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

##############################
# GUI
##############################
def update_outputs(*args):
    x = list(map(int, list(input_x.get())))
    y = list(map(int, list(input_y.get())))
    # only update when inputs are same length
    if (len(x) == len(y)):
	    zx = zx_var.get()
	    nx = nx_var.get()
	    zy = zy_var.get()
	    ny = ny_var.get()
	    f = f_var.get()
	    no = no_var.get()
	    # call ALU
	    out, zr, ng = ALU(x, y, zx, nx, zy, ny, f, no)
	    # set the outputs
	    out_text.set(''.join(map(str, out)))
	    zr_text.set(str(zr))
	    ng_text.set(str(ng))

# create the main window
root = tk.Tk()
root.title('ALU Simulator')

# styling
font_mono = tkfont.Font(family='Courier', size=16, weight='bold')

# create the input label and text entry
input_label_x = tk.Label(root, text='x')
input_label_x.grid(row=0, column=0, padx=5, pady=5, sticky='w')

input_x = tk.Entry(root, font=font_mono, textvariable=tk.StringVar(value='0000000000000000'), width=30)
input_x.bind("<KeyRelease>", update_outputs)
input_x.grid(row=0, column=1, padx=5, pady=5)

input_label_y = tk.Label(root, text='y')
input_label_y.grid(row=1, column=0, padx=5, pady=5, sticky='w')

input_y = tk.Entry(root, font=font_mono, textvariable=tk.StringVar(value='1111111111111111'), width=30)
input_y.bind("<KeyRelease>", update_outputs)
input_y.grid(row=1, column=1, padx=5, pady=5)

# create the checkboxes
zx_var = tk.IntVar()
zx_check = tk.Checkbutton(root, text='zx', variable=zx_var, command=update_outputs)
# zx_check.grid(row=2, column=0, padx=5, pady=5)

nx_var = tk.IntVar()
nx_check = tk.Checkbutton(root, text='nx', variable=nx_var, command=update_outputs)
# nx_check.grid(row=2, column=1, padx=5, pady=5)

zy_var = tk.IntVar()
zy_check = tk.Checkbutton(root, text='zy', variable=zy_var, command=update_outputs)
# zy_check.grid(row=2, column=2, padx=5, pady=5)

ny_var = tk.IntVar()
ny_check = tk.Checkbutton(root, text='ny', variable=ny_var, command=update_outputs)
# ny_check.grid(row=2, column=3, padx=5, pady=5)

f_var = tk.IntVar()
f_check = tk.Checkbutton(root, text='f', variable=f_var, command=update_outputs)
# f_check.grid(row=2, column=4, padx=5, pady=5)

no_var = tk.IntVar()
no_check = tk.Checkbutton(root, text='no', variable=no_var, command=update_outputs)
# no_check.grid(row=2, column=5, padx=5, pady=5)

# max_width = max(
# 	zx_check.winfo_reqwidth(),
# 	nx_check.winfo_reqwidth(),
# 	zy_check.winfo_reqwidth(),
# 	f_check.winfo_reqwidth(),
# 	no_check.winfo_reqwidth()
# )

# zx_check.config(width=max_width)
# nx_check.config(width=max_width)
# zy_check.config(width=max_width)
# f_check.config(width=max_width)
# no_check.config(width=max_width)

zx_check.grid(row=2, column=0, padx=5, pady=5)
nx_check.grid(row=2, column=1, padx=5, pady=5)
zy_check.grid(row=2, column=2, padx=5, pady=5)
ny_check.grid(row=2, column=3, padx=5, pady=5)
f_check.grid(row=2, column=4, padx=5, pady=5)
no_check.grid(row=2, column=5, padx=5, pady=5)

# create the output labels and text entries
out_label = tk.Label(root, text='Output:')
out_text = tk.StringVar()
out_entry = tk.Entry(root, width=30, textvariable=out_text, state='readonly')

out_label.grid(row=5, column=0, padx=5, pady=5, sticky='w')
out_entry.grid(row=5, column=1, padx=5, pady=5)

zr_label = tk.Label(root, text='zr:')
zr_text = tk.StringVar()
zr_entry = tk.Entry(root, width=5, textvariable=zr_text, state='readonly')

zr_label.grid(row=6, column=0, padx=5, pady=5, sticky='w')
zr_entry.grid(row=6, column=1, padx=5, pady=5)

ng_label = tk.Label(root, text='ng:')
ng_text = tk.StringVar()
ng_entry = tk.Entry(root, width=5, textvariable=ng_text, state='readonly')

ng_label.grid(row=7, column=0, padx=5, pady=5, sticky='w')
ng_entry.grid(row=7, column=1, padx=5, pady=5)

# set initial checks
zx_check.select()
nx_check.select()
zy_check.select()
# ny_check.select()
f_check.select()
# no_check.select()

# set the initial values of the outputs to empty strings
out_text.set('')
zr_text.set('')
ng_text.set('')

update_outputs()

# start the main event loop
root.mainloop()
