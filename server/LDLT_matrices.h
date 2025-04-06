#pragma once
#include <iostream>
#include <vector>
#include "CSRMatrix.h"


class LDLT_matrices {
    private:
    uint32_t n;
    CSRMatrix L_matrix = CSRMatrix();
    CSRMatrix D_matrix = CSRMatrix();
    CSRMatrix LT_matrix = CSRMatrix();
    std::vector<cpp_dec_float_100> b;

    public:

    LDLT_matrices(const CSRMatrix&);

    std::vector<cpp_dec_float_100> solution();
    const std::vector<cpp_dec_float_100>& get_b();
    static void show_error(const std::vector<cpp_dec_float_100>&, const std::vector<cpp_dec_float_100>&);
    
    CSRMatrix get_CSRMatrix();
};