#ifndef EvaValue_h
#define EvaValue_h

enum class EvaValueType {
	NUMBER,
};

// tagged union
struct EvaValue {
	EvaValueType type;
	// union stores values of different size at same memory space
	union {
		double number;
		// etc
	};
};

// constructors
#define NUMBER(value) ((EvaValue){ EvaValueType::NUMBER, .number = value })

#endif