#ifndef EvaValue_h
#define EvaValue_h

#include <string>

enum class EvaValueType {
	NUMBER,
	OBJECT,
};

enum class ObjectType {
	STRING,
};

struct Object {
	Object(ObjectType type) : type(type) {}
	ObjectType type;
};

struct StringObject : public Object {
	StringObject(const std::string& value) : Object(ObjectType::STRING), value(value) {}
	std::string value;
};



// tagged union
struct EvaValue {
	EvaValueType type;
	// union stores values of different size at same memory space
	union {
		double number;
		Object* object;
	};
};

// constructors
#define NUMBER(value) ((EvaValue){ EvaValueType::NUMBER, .number = value })

// heap allocations
#define ALLOC_STRING(value) ((EvaValue){ EvaValueType::OBJECT, .object = (Object*) new StringObject(value) })

// accessors
#define AS_NUMBER(EvaValue) ((double)(EvaValue).number)
#define AS_STRING(EvaValue) (((StringObject*)(EvaValue).object)->value)

// testers
#define IS_NUMBER(EvaValue) ((EvaValue).type == EvaValueType::NUMBER)
#define IS_STRING(EvaValue) ((EvaValue).type == EvaValueType::OBJECT && ((Object*)(EvaValue).object)->type == ObjectType::STRING)

#endif