#ifndef H7OS_TLSFBIT_H
#define H7OS_TLSFBIT_H

#define tlsf_decl static

tlsf_decl int tlsf_ffs(unsigned int word){
    return __builtin_ffs(word) - 1;
}

tlsf_decl int tlsf_fls(unsigned int word){
    const int bit = word ? 32 - __builtin_clz(word) : 0;
    return bit - 1;
}

#define tlsf_fls_sizet tlsf_fls

#undef tlsf_decl

#endif //H7OS_TLSFBIT_H
