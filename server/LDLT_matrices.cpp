#include "LDLT_matrices.h"


const std::vector<cpp_dec_float_100>& LDLT_matrices::get_b() {
    return b;
}


CSRMatrix create_M_matrices_vec(CSRMatrix object, std::vector<CSRMatrix>& M_matrices) {
    std::vector<uint32_t> M_count_in_rows_vec;
    std::vector<uint32_t> M_columns_indexes_vec;
    std::vector<cpp_dec_float_100> M_values_vec;
    M_count_in_rows_vec.reserve(object.get_first_count());

    std::vector<std::pair<uint32_t, uint32_t>> start_and_end_pos;
    start_and_end_pos.reserve(object.get_first_count());
    start_and_end_pos.push_back(std::make_pair(0, object.get_count_in_rows()[0]));
    for (uint32_t i : std::ranges::views::iota(static_cast<uint32_t>(1), object.get_first_count())) {
        start_and_end_pos.push_back(std::make_pair(object.get_count_in_rows()[i - 1], object.get_count_in_rows()[i]));
    }

    uint32_t start_pos, end_pos, cur_count = 0;
    cpp_dec_float_100 cur_divider;
    for (uint32_t i : std::ranges::views::iota(static_cast<uint32_t>(0), object.get_first_count() - 1)) {
        cur_divider = object.get_values()[start_and_end_pos[i].first];

        for (uint32_t j : std::ranges::views::iota(static_cast<uint32_t>(0), object.get_first_count())) {
            if (start_and_end_pos[j].first < start_and_end_pos[j].second && j > i
                && object.get_columns_indexes()[start_and_end_pos[j].first] == i) {
                M_columns_indexes_vec.push_back(i);
                M_values_vec.push_back(-object.get_values()[start_and_end_pos[j].first] / cur_divider);
                ++cur_count;
                ++start_and_end_pos[j].first;
            }
            M_columns_indexes_vec.push_back(j);
            M_values_vec.push_back(1);
            if (j == 0) {
                M_count_in_rows_vec.push_back(cur_count + 1);
            } else {
                M_count_in_rows_vec.push_back(cur_count + 1 + M_count_in_rows_vec.back());
            }
            cur_count = 0;
        }
        
        CSRMatrix M(object.get_first_count(), M_values_vec.size(), std::move(M_count_in_rows_vec), std::move(M_columns_indexes_vec), std::move(M_values_vec));
        M_matrices.push_back(M);
        //* M.save_matrix();
        object = M * object;
        //* object.save_matrix();
        start_and_end_pos.clear();
        M_count_in_rows_vec.clear();
        M_columns_indexes_vec.clear();
        M_values_vec.clear();
        start_and_end_pos.push_back(std::make_pair(0, object.get_count_in_rows()[0]));
        for (uint32_t i : std::ranges::views::iota(static_cast<uint32_t>(1), object.get_first_count())) {
            start_and_end_pos.push_back(std::make_pair(object.get_count_in_rows()[i - 1], object.get_count_in_rows()[i]));
        }
    }
    return object;
}


CSRMatrix get_M_inverse_matrix(const CSRMatrix& M) {
    std::vector<uint32_t> count_in_rows_vec;
    std::vector<uint32_t> columns_indexes_vec;
    std::vector<cpp_dec_float_100> values_vec;
    count_in_rows_vec.reserve(M.get_first_count());
    columns_indexes_vec.reserve(M.get_second_count());
    values_vec.reserve(M.get_second_count());

    for (uint32_t i : std::ranges::views::iota(static_cast<uint32_t>(0), M.get_first_count())) {
        count_in_rows_vec.push_back(M.get_count_in_rows()[i]);
    } 
    uint32_t start_pos, end_pos;
    for (uint32_t i : std::ranges::views::iota(static_cast<uint32_t>(0), M.get_first_count())) {
        end_pos = count_in_rows_vec[i];
        if (i == 0) {
            start_pos = 0;
        } else {
            start_pos = count_in_rows_vec[i - 1];
        }
        for (uint32_t j : std::ranges::views::iota(start_pos, end_pos)) {
            columns_indexes_vec.push_back(M.get_columns_indexes()[j]);
            values_vec.push_back(M.get_values()[j]);
        }
        if (end_pos - start_pos > 1) {
            values_vec[start_pos] *= -1;
        }
    }
    CSRMatrix inverse_matrix(M.get_first_count(), M.get_second_count(), std::move(count_in_rows_vec), std::move(columns_indexes_vec), std::move(values_vec));
    //* inverse_matrix.save_matrix();
    return inverse_matrix;
}


CSRMatrix get_D_matrix(const CSRMatrix& U_matrix) {
    std::vector<uint32_t> count_in_rows_vec;
    std::vector<uint32_t> columns_indexes_vec;
    std::vector<cpp_dec_float_100> values_vec;
    count_in_rows_vec.reserve(U_matrix.get_first_count());
    columns_indexes_vec.reserve(U_matrix.get_first_count());
    values_vec.reserve(U_matrix.get_first_count());

    count_in_rows_vec.push_back(1);
    columns_indexes_vec.push_back(0);
    values_vec.push_back(U_matrix.get_values()[0]);
    for (uint32_t i : std::ranges::views::iota(static_cast<uint32_t>(1), U_matrix.get_first_count())) {
        count_in_rows_vec.push_back(count_in_rows_vec.back() + 1);
        columns_indexes_vec.push_back(i);
        values_vec.push_back(U_matrix.get_values()[U_matrix.get_count_in_rows()[i - 1]]);
    }

    CSRMatrix result(U_matrix.get_first_count(), U_matrix.get_first_count(), std::move(count_in_rows_vec), std::move(columns_indexes_vec), std::move(values_vec));
    return result;
}


LDLT_matrices::LDLT_matrices(const CSRMatrix& object) : n(object.get_first_count()) {
    std::vector<CSRMatrix> M_matrices;
    M_matrices.reserve(n - 1);
    CSRMatrix U_matrix = create_M_matrices_vec(object, M_matrices);
    // U_matrix.save_matrix();
    D_matrix = get_D_matrix(U_matrix);
    // D_matrix.save_matrix();
    L_matrix = get_M_inverse_matrix(M_matrices.front());
    for (uint32_t i : std::ranges::views::iota(static_cast<uint32_t>(1), M_matrices.size())) {
        L_matrix *= get_M_inverse_matrix(M_matrices[i]);
    }
    // L_matrix.save_matrix();
    LT_matrix = CSRMatrix::get_transposed_matrix(L_matrix);
    // LT_matrix.save_matrix();
    CSRMatrix result = CSRMatrix();
    result = L_matrix * D_matrix * LT_matrix;
    // result.save_matrix();
    // b = {16, 14, 15, -2, 16, -1 ,10};
    b = CSRMatrix::generate_b(object.get_values(), object.get_first_count());
    // CSRMatrix::save_vector(b);
}


CSRMatrix LDLT_matrices::get_CSRMatrix() {
    CSRMatrix answer = L_matrix * D_matrix * LT_matrix;
    return answer;
}


std::vector<cpp_dec_float_100> Ly_b_solution(const CSRMatrix& L_matrix, const std::vector<cpp_dec_float_100>& b) {
    std::vector<cpp_dec_float_100> y;

    std::pair<uint32_t, uint32_t> start_and_end_pos_index;
    for (uint32_t i : std::ranges::views::iota(static_cast<uint32_t>(0), L_matrix.get_first_count())) {
        if (i == 0) {
            start_and_end_pos_index = std::make_pair(0, L_matrix.get_count_in_rows()[0] - 1);
        } else {
            start_and_end_pos_index = std::make_pair(L_matrix.get_count_in_rows()[i - 1], L_matrix.get_count_in_rows()[i] - 1);
        }
        cpp_dec_float_100 cur_sum = 0;
        for (uint32_t j : std::ranges::views::iota(start_and_end_pos_index.first, start_and_end_pos_index.second)) {
            cur_sum += L_matrix.get_values()[j] * y[L_matrix.get_columns_indexes()[j]];
        }
        y.push_back(b[i] - cur_sum);
    }
    
    return y;
}


std::vector<cpp_dec_float_100> LTx_y_reverse_D_solution(const CSRMatrix& LT_matrix, const std::vector<cpp_dec_float_100>& y) {
    std::vector<cpp_dec_float_100> x;

    std::pair<uint32_t, uint32_t> start_and_end_pos_index;
    for (uint32_t i = LT_matrix.get_first_count(); i > 0; --i) {
        if (i == 1) {
            start_and_end_pos_index = std::make_pair(1, LT_matrix.get_count_in_rows()[0]);
        } else {
            start_and_end_pos_index = std::make_pair(LT_matrix.get_count_in_rows()[i - 2] + 1, LT_matrix.get_count_in_rows()[i - 1]);
        }
        cpp_dec_float_100 cur_sum = 0;
        for (uint32_t j = start_and_end_pos_index.first; j < start_and_end_pos_index.second; ++j) {
            cur_sum += LT_matrix.get_values()[j] * x[abs(static_cast<int>(LT_matrix.get_columns_indexes()[j] - y.size() + 1))];
        }
        x.push_back(y[i - 1] - cur_sum);
    }
    
    std::reverse(x.begin(), x.end());
    return x;
}


std::vector<cpp_dec_float_100> LDLT_matrices::solution() {
    std::vector<cpp_dec_float_100> y{Ly_b_solution(L_matrix, b)};
    
    for (uint32_t i : std::ranges::views::iota(static_cast<uint32_t>(0), y.size())) {
        y[i] /= D_matrix.get_values()[i];
    }

    std::vector<cpp_dec_float_100> x{LTx_y_reverse_D_solution(LT_matrix, y)};
    return x;
}


void LDLT_matrices::show_error(const std::vector<cpp_dec_float_100>& Ax, const std::vector<cpp_dec_float_100>& b) {
    cpp_dec_float_100 max = abs(Ax.front() - b.front());
    for (uint32_t i : std::ranges::views::iota(static_cast<uint32_t>(1), b.size())) {
        if (max < abs(Ax[i] - b[i])) {
            max = abs(Ax[i] - b[i]);
        }
    }

    if (b.size() < 50) {
        for (uint32_t i : std::ranges::views::iota(static_cast<uint32_t>(0), b.size())) {
            cpp_dec_float_100 cur_val = abs(Ax[i] - b[i]);
            std::string str_number = cur_val.str(5, std::ios_base::scientific), str_max = max.str(5, std::ios_base::scientific);
            if (i == 0) {
                std::cout << "           / " << str_number << " \\" << '\n';
            } else if (i == b.size() / 2) {
                std::cout << "|Ax - b| = " << "| " << str_number << " |" << ",    ||Ax - b|| = " << str_max << "\n";
            } else if (i == b.size() - 1) {
                std::cout << "           \\ " << str_number << " /" << '\n';
            } else {
                std::cout << "           | " << str_number << " |" << '\n';
            }
        }
    } else {
        std::string str_max = max.str(5, std::ios_base::scientific);
        std::cout << "||Ax - b|| = " << str_max << "\n";
    }
}
