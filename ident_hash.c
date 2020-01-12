#include "ident_hash.h"
#include "hashmap.h"

static HashMap *IDENTIFIERS;
static int initialized = 0;

struct ident *ident_new(char *name, int ns)
{
    struct ident *id = malloc(sizeof(struct ident));
    id->name = strdup(name);
    id->ns = ns;
    return id;
}

struct ident *get_hashed_ident(char *name, int ns)
{
    struct ident *id = HashMap_get(IDENTIFIERS, name);
    if (id) {
        return id;
    }
    struct ident *newid = ident_new(name, ns);
    HashMap_add(IDENTIFIERS, name, newid);
    return newid;
}

void init_identifiers_bt()
{
    if (initialized) {
        //error
    }

    IDENTIFIERS = HashMap_new_str();
    initialized = 1;

    alignof_ident = get_hashed_ident("alignof", NS_C89 | NS_C99 | NS_C11);
    auto_ident = get_hashed_ident("auto", NS_C89 | NS_C99 | NS_C11);
    break_ident = get_hashed_ident("break", NS_C89 | NS_C99 | NS_C11);
    case_ident = get_hashed_ident("case", NS_C89 | NS_C99 | NS_C11);
    char_ident = get_hashed_ident("char", NS_C89 | NS_C99 | NS_C11);
    const_ident = get_hashed_ident("const", NS_C89 | NS_C99 | NS_C11);
    continue_ident = get_hashed_ident("continue", NS_C89 | NS_C99 | NS_C11);
    default_ident = get_hashed_ident("default", NS_C89 | NS_C99 | NS_C11);
    do_ident = get_hashed_ident("do", NS_C89 | NS_C99 | NS_C11);
    double_ident = get_hashed_ident("double", NS_C89 | NS_C99 | NS_C11);
    else_ident = get_hashed_ident("else", NS_C89 | NS_C99 | NS_C11);
    enum_ident = get_hashed_ident("enum", NS_C89 | NS_C99 | NS_C11);
    extern_ident = get_hashed_ident("extern", NS_C89 | NS_C99 | NS_C11);
    float_ident = get_hashed_ident("float", NS_C89 | NS_C99 | NS_C11);
    for_ident = get_hashed_ident("for", NS_C89 | NS_C99 | NS_C11);
    goto_ident = get_hashed_ident("goto", NS_C89 | NS_C99 | NS_C11);
    if_ident = get_hashed_ident("if", NS_C89 | NS_C99 | NS_C11);
    inline_ident = get_hashed_ident("inline", NS_C89 | NS_C99 | NS_C11);
    int_ident = get_hashed_ident("int", NS_C89 | NS_C99 | NS_C11);
    long_ident = get_hashed_ident("long", NS_C89 | NS_C99 | NS_C11);
    register_ident = get_hashed_ident("register", NS_C89 | NS_C99 | NS_C11);
    restrict_ident = get_hashed_ident("restrict", NS_C89 | NS_C99 | NS_C11);
    return_ident = get_hashed_ident("return", NS_C89 | NS_C99 | NS_C11);
    short_ident = get_hashed_ident("short", NS_C89 | NS_C99 | NS_C11);
    signed_ident = get_hashed_ident("signed", NS_C89 | NS_C99 | NS_C11);
    sizeof_ident = get_hashed_ident("sizeof", NS_C89 | NS_C99 | NS_C11);
    static_ident = get_hashed_ident("static", NS_C89 | NS_C99 | NS_C11);
    struct_ident = get_hashed_ident("struct", NS_C89 | NS_C99 | NS_C11);
    switch_ident = get_hashed_ident("switch", NS_C89 | NS_C99 | NS_C11);
    typedef_ident = get_hashed_ident("typedef", NS_C89 | NS_C99 | NS_C11);
    union_ident = get_hashed_ident("union", NS_C89 | NS_C99 | NS_C11);
    unsigned_ident = get_hashed_ident("unsigned", NS_C89 | NS_C99 | NS_C11);
    void_ident = get_hashed_ident("void", NS_C89 | NS_C99 | NS_C11);
    volatile_ident = get_hashed_ident("volatile", NS_C89 | NS_C99 | NS_C11);
    while_ident = get_hashed_ident("while", NS_C89 | NS_C99 | NS_C11);
    _Alignas_ident = get_hashed_ident("_Alignas", NS_C89 | NS_C99 | NS_C11);
    _Atomic_ident = get_hashed_ident("_Atomic", NS_C89 | NS_C99 | NS_C11);
    _Bool_ident = get_hashed_ident("_Bool", NS_C89 | NS_C99 | NS_C11);
    _Complex_ident = get_hashed_ident("_Complex", NS_C89 | NS_C99 | NS_C11);
    _Generic_ident = get_hashed_ident("_Generic", NS_C89 | NS_C99 | NS_C11);
    _Imaginary_ident = get_hashed_ident("_Imaginary", NS_C89 | NS_C99 | NS_C11);
    _Noreturn_ident = get_hashed_ident("_Noreturn", NS_C89 | NS_C99 | NS_C11);
    _Static_assert_ident = get_hashed_ident("_Static_assert", NS_C89 | NS_C99 | NS_C11);
    _Thread_local_ident = get_hashed_ident("_Thread_local", NS_C89 | NS_C99 | NS_C11);
    __VA_ARGS___ident = get_hashed_ident("__VA_ARGS__", NS_C89 | NS_C99 | NS_C11);
    __func___ident = get_hashed_ident("__func__", NS_C89 | NS_C99 | NS_C11);
    define_ident = get_hashed_ident("define", NS_ALL | NS_PRP);
    defined_ident = get_hashed_ident("defined", NS_ALL | NS_PRP);
    elif_ident = get_hashed_ident("elif", NS_ALL | NS_PRP);
    endif_ident = get_hashed_ident("endif", NS_ALL | NS_PRP);
    error_ident = get_hashed_ident("error", NS_ALL | NS_PRP);
    ifdef_ident = get_hashed_ident("ifdef", NS_ALL | NS_PRP);
    ifndef_ident = get_hashed_ident("ifndef", NS_ALL | NS_PRP);
    include_ident = get_hashed_ident("include", NS_ALL | NS_PRP);
    line_ident = get_hashed_ident("line", NS_ALL | NS_PRP);
    pragma_ident = get_hashed_ident("pragma", NS_ALL | NS_PRP);
    undef_ident = get_hashed_ident("undef", NS_ALL | NS_PRP);
    warning_ident = get_hashed_ident("warning", NS_ALL | NS_PRP);
    __LINE___ident = get_hashed_ident("__LINE__", NS_ALL | NS_PRP);
    __FILE___ident = get_hashed_ident("__FILE__", NS_ALL | NS_PRP);
    __DATE___ident = get_hashed_ident("__DATE__", NS_ALL | NS_PRP);
    __TIME___ident = get_hashed_ident("__TIME__", NS_ALL | NS_PRP);
    include_next_ident = get_hashed_ident("include_next", NS_ALL | NS_PRP);
    __PRETTY_FUNCTION___ident = get_hashed_ident("__PRETTY_FUNCTION__", NS_ALL | NS_PRP);
    __COUNTER___ident = get_hashed_ident("__COUNTER__", NS_ALL | NS_PRP);
    __FUNCTION___ident = get_hashed_ident("__FUNCTION__", NS_ALL | NS_PRP);
    __FUNCSIG___ident = get_hashed_ident("__FUNCSIG__", NS_ALL | NS_PRP);
    __FUNCDNAME___ident = get_hashed_ident("__FUNCDNAME__", NS_ALL | NS_PRP);
    asm_ident = get_hashed_ident("asm", NS_RID | NS_GNU);
    __asm_ident = get_hashed_ident("__asm", NS_RID | NS_GNU);
    __asm___ident = get_hashed_ident("__asm__", NS_RID | NS_GNU);
    __alignof_ident = get_hashed_ident("__alignof", NS_RID | NS_GNU);
    __alignof___ident = get_hashed_ident("__alignof__", NS_RID | NS_GNU);
    __attribute_ident = get_hashed_ident("__attribute", NS_RID | NS_GNU);
    __attribute___ident = get_hashed_ident("__attribute__", NS_RID | NS_GNU);
    __complex_ident = get_hashed_ident("__complex", NS_RID | NS_GNU);
    __complex___ident = get_hashed_ident("__complex__", NS_RID | NS_GNU);
    __const_ident = get_hashed_ident("__const", NS_RID | NS_GNU);
    __const___ident = get_hashed_ident("__const__", NS_RID | NS_GNU);
    __inline_ident = get_hashed_ident("__inline", NS_RID | NS_GNU);
    __inline___ident = get_hashed_ident("__inline__", NS_RID | NS_GNU);
    __restrict_ident = get_hashed_ident("__restrict", NS_RID | NS_GNU);
    __restrict___ident = get_hashed_ident("__restrict__", NS_RID | NS_GNU);
    __signed_ident = get_hashed_ident("__signed", NS_RID | NS_GNU);
    __signed___ident = get_hashed_ident("__signed__", NS_RID | NS_GNU);
    __thread_ident = get_hashed_ident("__thread", NS_RID | NS_GNU);
    typeof_ident = get_hashed_ident("typeof", NS_RID | NS_GNU);
    __typeof_ident = get_hashed_ident("__typeof", NS_RID | NS_GNU);
    __typeof___ident = get_hashed_ident("__typeof__", NS_RID | NS_GNU);
    __volatile_ident = get_hashed_ident("__volatile", NS_RID | NS_GNU);
    __volatile___ident = get_hashed_ident("__volatile__", NS_RID | NS_GNU);
    __label___ident = get_hashed_ident("__label__", NS_RID | NS_GNU);
    __extension___ident = get_hashed_ident("__extension__", NS_RID | NS_GNU);
    __declspec_ident = get_hashed_ident("__declspec", NS_RID | NS_MSV);
    __cdecl_ident = get_hashed_ident("__cdecl", NS_RID | NS_MSV);
    __stdcall_ident = get_hashed_ident("__stdcall", NS_RID | NS_MSV);
    __fastcall_ident = get_hashed_ident("__fastcall", NS_RID | NS_MSV);
    __thiscall_ident = get_hashed_ident("__thiscall", NS_RID | NS_MSV);
    __regcall_ident = get_hashed_ident("__regcall", NS_RID | NS_MSV);
    __vectorcall_ident = get_hashed_ident("__vectorcall", NS_RID | NS_MSV);
    __forceinline_ident = get_hashed_ident("__forceinline", NS_RID | NS_MSV);
    __unaligned_ident = get_hashed_ident("__unaligned", NS_RID | NS_MSV);
    __super_ident = get_hashed_ident("__super", NS_RID | NS_MSV);
    _asm_ident = get_hashed_ident("_asm", NS_RID | NS_MSV);
    _int8_ident = get_hashed_ident("_int8", NS_RID | NS_MSV);
    __int8_ident = get_hashed_ident("__int8", NS_RID | NS_MSV);
    _int16_ident = get_hashed_ident("_int16", NS_RID | NS_MSV);
    __int16_ident = get_hashed_ident("__int16", NS_RID | NS_MSV);
    _int32_ident = get_hashed_ident("_int32", NS_RID | NS_MSV);
    __int32_ident = get_hashed_ident("__int32", NS_RID | NS_MSV);
    _cdecl_ident = get_hashed_ident("_cdecl", NS_RID | NS_MSV);
    _fastcall_ident = get_hashed_ident("_fastcall", NS_RID | NS_MSV);
    _stdcall_ident = get_hashed_ident("_stdcall", NS_RID | NS_MSV);
    _thiscall_ident = get_hashed_ident("_thiscall", NS_RID | NS_MSV);
    _vectorcall_ident = get_hashed_ident("_vectorcall", NS_RID | NS_MSV);
    _inline_ident = get_hashed_ident("_inline", NS_RID | NS_MSV);
    _declspec_ident = get_hashed_ident("_declspec", NS_RID | NS_MSV);
    once_ident = get_hashed_ident("once", NS_RID | NS_MSV);
    pack_ident = get_hashed_ident("pack", NS_RID | NS_MSV);
    push_ident = get_hashed_ident("push", NS_RID | NS_MSV);
    pop_ident = get_hashed_ident("pop", NS_RID | NS_MSV);
}
