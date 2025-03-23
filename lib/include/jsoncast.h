#ifndef JSON_CAST
#define JSON_CAST

#include<ctypes.h>

typedef enum {
	JSON_Type_NONE = 0,

	JSON_Type_string,
	JSON_Type_number,
	JSON_Type_object,
	JSON_Type_array,
	JSON_Type_false,
	JSON_Type_true,
	JSON_Type_null
} JSON_Type;

typedef struct {
	JSON_Type type;
	void* value;
} JSON_Value;

typedef struct {
	FString name;
	JSON_Type type;
	JSON_Type subtype;
	void* ptr;
} JSON_Field;

#define JSON_Field_add_number(STRUCT, NAME) JSON_Field_add_(#NAME, &((STRUCT).(NAME)), JSON_Type_number)

JSON_Field JSON_Field_add_(CCString name, void* ptr, JSON_Type type)
{
	JSON_Field field = {0};
	field.name = fstr_from_cstr(name);
	field.type = type;
	field.ptr = ptr;
	return field;
}


#endif /* JSON_CAST */
