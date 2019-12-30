#ifndef IDENT_HASH_H_
#define IDENT_HASH_H_

#include "cheaders.h"

struct ident *get_hashed_ident(char *name, int ns);
struct ident *ident_new(char *name, int ns);
void init_identifiers_bt();

struct ident {
    char *name;
    int ns;
};

#define NS_C89  (1 << 0)
#define NS_C99  (1 << 1)
#define NS_C11  (1 << 2)
#define NS_PRP  (1 << 3)
#define NS_GNU  (1 << 4)
#define NS_MSV  (1 << 5)
#define NS_RID  (1 << 6)
#define NS_ALL  (NS_C89 | NS_C99 | NS_C11)

struct ident * alignof_ident;
struct ident * auto_ident;
struct ident * break_ident;
struct ident * case_ident;
struct ident * char_ident;
struct ident * const_ident;
struct ident * continue_ident;
struct ident * default_ident;
struct ident * do_ident;
struct ident * double_ident;
struct ident * else_ident;
struct ident * enum_ident;
struct ident * extern_ident;
struct ident * float_ident;
struct ident * for_ident;
struct ident * goto_ident;
struct ident * if_ident;
struct ident * inline_ident;
struct ident * int_ident;
struct ident * long_ident;
struct ident * register_ident;
struct ident * restrict_ident;
struct ident * return_ident;
struct ident * short_ident;
struct ident * signed_ident;
struct ident * sizeof_ident;
struct ident * static_ident;
struct ident * struct_ident;
struct ident * switch_ident;
struct ident * typedef_ident;
struct ident * union_ident;
struct ident * unsigned_ident;
struct ident * void_ident;
struct ident * volatile_ident;
struct ident * while_ident;
struct ident * _Alignas_ident;
struct ident * _Atomic_ident;
struct ident * _Bool_ident;
struct ident * _Complex_ident;
struct ident * _Generic_ident;
struct ident * _Imaginary_ident;
struct ident * _Noreturn_ident;
struct ident * _Static_assert_ident;
struct ident * _Thread_local_ident;
struct ident * __VA_ARGS___ident;
struct ident * __func___ident;
struct ident * define_ident;
struct ident * defined_ident;
struct ident * elif_ident;
struct ident * endif_ident;
struct ident * error_ident;
struct ident * ifdef_ident;
struct ident * ifndef_ident;
struct ident * include_ident;
struct ident * line_ident;
struct ident * pragma_ident;
struct ident * undef_ident;
struct ident * warning_ident;
struct ident * __LINE___ident;
struct ident * __FILE___ident;
struct ident * __DATE___ident;
struct ident * __TIME___ident;
struct ident * include_next_ident;
struct ident * __PRETTY_FUNCTION___ident;
struct ident * __COUNTER___ident;
struct ident * __FUNCTION___ident;
struct ident * __FUNCSIG___ident;
struct ident * __FUNCDNAME___ident;
struct ident * asm_ident;
struct ident * __asm_ident;
struct ident * __asm___ident;
struct ident * __alignof_ident;
struct ident * __alignof___ident;
struct ident * __attribute_ident;
struct ident * __attribute___ident;
struct ident * __complex_ident;
struct ident * __complex___ident;
struct ident * __const_ident;
struct ident * __const___ident;
struct ident * __inline_ident;
struct ident * __inline___ident;
struct ident * __restrict_ident;
struct ident * __restrict___ident;
struct ident * __signed_ident;
struct ident * __signed___ident;
struct ident * __thread_ident;
struct ident * typeof_ident;
struct ident * __typeof_ident;
struct ident * __typeof___ident;
struct ident * __volatile_ident;
struct ident * __volatile___ident;
struct ident * __label___ident;
struct ident * __extension___ident;
struct ident * __declspec_ident;
struct ident * __cdecl_ident;
struct ident * __stdcall_ident;
struct ident * __fastcall_ident;
struct ident * __thiscall_ident;
struct ident * __regcall_ident;
struct ident * __vectorcall_ident;
struct ident * __forceinline_ident;
struct ident * __unaligned_ident;
struct ident * __super_ident;
struct ident * _asm_ident;
struct ident * _int8_ident;
struct ident * __int8_ident;
struct ident * _int16_ident;
struct ident * __int16_ident;
struct ident * _int32_ident;
struct ident * __int32_ident;
struct ident * _cdecl_ident;
struct ident * _fastcall_ident;
struct ident * _stdcall_ident;
struct ident * _thiscall_ident;
struct ident * _vectorcall_ident;
struct ident * _inline_ident;
struct ident * _declspec_ident;
struct ident * once_ident;
struct ident * pack_ident;
struct ident * push_ident;
struct ident * pop_ident;

#endif /* IDENT_HASH_H_ */
