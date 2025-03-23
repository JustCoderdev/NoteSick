#ifndef C_TYPES
#define C_TYPES


typedef enum {
	true  = (1 == 1),
	false = (1 != 1)
} bool;

typedef enum {
	failure = true,
	success = false
} error;

typedef int error_no;

typedef unsigned char n8;
typedef signed char i8;

typedef unsigned short n16;
typedef signed short i16;

typedef unsigned int n32;
typedef signed int i32;

typedef unsigned long n64;
typedef signed long i64;

typedef float f32;
typedef double f64;

typedef struct {
	n32 length;
	char* const chars;
} FString;

typedef struct {
	n32 length;
	const char* const chars;
} CFString;

typedef const char* const CCString;
typedef char* const CString;


#endif /* C_TYPES */
