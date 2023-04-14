assignment_table = {}

# fact: person(michael)

# color/1
color = ['red', 'green', 'blue']
# person/1
_person = ['michael', 'adam']

person = {
	1: [],
	2: []
}

person[1].append('michael')
person[1].append('adam')

# print('michael' in person[1])

person[2].append(person[1])

# compound: person(michael, nationality(sweden, swedish)).

# query: ?- color(green)

# print('green' in color)
# print('black' in color)

def query(fact, variable):
	if not variable.isupper():
		return str(variable) in color
	else:
		return_str = ''
		for item in fact:
			return_str = return_str + str(variable) + ' = ' + str(item) + ' ; '
			assignment_table[str(variable)] = item

		return return_str

print(query(color, 'green'))
print(query(color, 'black'))
print(query(color, 'X'))
print(assignment_table)