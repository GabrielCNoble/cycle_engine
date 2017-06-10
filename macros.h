
#define unlikely(expr) __builtin_expect(!!(expr), 0)
#define likely(expr) __builtin_expect(!!(expr), 1)

#define RadToDeg(x)  (180.0/3.14159265)*x
#define DegToRad(x) ((3.14159265*x)/180.0)
