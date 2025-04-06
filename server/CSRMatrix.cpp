#include "CSRMatrix.h"


uint32_t get_number(const std::string& str, uint16_t& cur_pos) {
    uint32_t value = 0;
    while(cur_pos < str.size() && str[cur_pos] != ' ') {
        value = value * 10 + str[cur_pos] - '0';
        ++cur_pos;
    }
    ++cur_pos;
    return value;
}


cpp_dec_float_100 get_value(const std::string& str, uint16_t& cur_pos) {
    std::string::const_iterator iter = std::find(str.cbegin() + cur_pos, str.cend(), 'e');
    bool is_str_contain_exponent{iter != str.cend()};

    cpp_dec_float_100 value;
    int64_t integer_part = 0;
    uint64_t fractional_part = 0;

    bool is_number_negative{false};
    if (str[cur_pos] == '-') {
        is_number_negative = true;
        ++cur_pos;
    }
    while (cur_pos < str.size() && str[cur_pos] != '.' && str[cur_pos] != 'e') {
        integer_part = integer_part * 10 + str[cur_pos] - '0';
        ++cur_pos;
    }
    
    uint16_t save_degree = cur_pos;
    if (str[cur_pos] != 'e') {
        ++cur_pos;
        while (cur_pos < str.size() && str[cur_pos] != 'e') {
            fractional_part = fractional_part * 10 + str[cur_pos] - '0';
            ++cur_pos;
        }
    }

    value = integer_part + static_cast<cpp_dec_float_100>(fractional_part) / std::pow(10, cur_pos - save_degree - 1);

    if (is_number_negative) {
        value *= -1;
    }

    
    if (is_str_contain_exponent) {
        ++cur_pos;
        int16_t degree;
        if (str[cur_pos] == '-') {
            ++cur_pos;
            degree = -1 * get_number(str, cur_pos);
        } else {
            degree = get_number(str, cur_pos);
        }
        value = value * std::pow(10, degree);
    }
    // std::string str_val{std::to_string(value)};

    return value;
}


CSRMatrix::CSRMatrix(const std::vector<std::vector<cpp_dec_float_100>>& matrix) {
    first_count = matrix.size();
    second_count = 0;
    count_in_rows.reserve(first_count);
    for (uint32_t i : std::ranges::views::iota(static_cast<uint32_t>(0), first_count)) {
        uint32_t count_in_cur_str = 0;
        for (uint32_t j : std::ranges::views::iota(static_cast<uint32_t>(0), matrix.front().size())) {
            if (matrix[i][j] != 0) {
                ++count_in_cur_str;
                columns_indexes.push_back(j);
                values.push_back(matrix[i][j]);
            }
        }
        if (i == 0) {
            count_in_rows.push_back(count_in_cur_str);
        } else {
            count_in_rows.push_back(count_in_cur_str + count_in_rows.back());
        }
    }
    second_count = columns_indexes.size();
}


CSRMatrix::CSRMatrix(const CSRMatrix& other) : 
    first_count(other.first_count), second_count(other.second_count),
    count_in_rows(other.count_in_rows),
    columns_indexes(other.columns_indexes),
    values(other.values) {}


CSRMatrix::CSRMatrix(uint32_t first_count_t, uint32_t second_count_t, std::vector<uint32_t>&& count_in_rows_t, 
                     std::vector<uint32_t>&& columns_indexes_t, std::vector<cpp_dec_float_100>&& values_t) {
    first_count = first_count_t;
    second_count = second_count_t;
    count_in_rows = std::move(count_in_rows_t);
    columns_indexes = std::move(columns_indexes_t);
    values = std::move(values_t);
}


CSRMatrix::CSRMatrix(CSRMatrix&& other) : 
    first_count(other.first_count), second_count(other.second_count),
    count_in_rows(std::move(other.count_in_rows)),
    columns_indexes(std::move(other.columns_indexes)),
    values(std::move(other.values)) {}


struct String_from_file {
    uint32_t i_index;
    uint32_t j_index;
    cpp_dec_float_100 value;
};


struct Compare {
    bool operator()(const String_from_file& first, const String_from_file& second) {
        return first.j_index < second.j_index;
    }
};


std::string edit_file(const std::string &file_path) {
    std::string str_from_file;
    std::ifstream file(file_path);
    getline(file, str_from_file);

    std::string new_file_name{"../tests/test_file.txt"};
    std::ofstream new_file(new_file_name);

    uint16_t cur_pos = 0;
    uint32_t j_index, i_index;
    new_file << str_from_file << '\n';

    cur_pos = 0;
    bool is_cool = true;
    while (getline(file, str_from_file)) {
        cur_pos = 0;
        j_index = get_number(str_from_file, cur_pos);
        i_index = get_number(str_from_file, cur_pos);
        if (j_index >= i_index) {
            new_file << str_from_file << '\n';
        } else {
            is_cool = false;
        }
    }
    file.close();

    if (!is_cool) {
        remove("../tests/test_file.txt");
        return "../tests/test_file.txt";
    }
    new_file.close();

    return file_path;
}


CSRMatrix::CSRMatrix(std::string& file_name_t) {
    std::string file_name = edit_file(file_name_t);

    std::string str_from_file;
    uint16_t cur_pos = 0;
    std::ifstream file(file_name);
    getline(file, str_from_file);

    first_count = get_number(str_from_file, cur_pos);
    get_number(str_from_file, cur_pos);
    second_count = get_number(str_from_file, cur_pos) * 2;

    count_in_rows.reserve(first_count);
    columns_indexes.reserve(second_count);
    values.reserve(second_count);
    
    std::map<uint32_t, std::vector<std::pair<uint32_t, cpp_dec_float_100>>> display_contaiter;

    uint32_t i_index, j_index;
    cpp_dec_float_100 value;
    std::string reserve_string;
    bool is_new_sequence = false;
    std::vector<String_from_file> data_to_emplase;

    while ((std::getline(file, str_from_file) || !reserve_string.empty()) && count_in_rows.size() < first_count) {
        if (!reserve_string.empty()) {
            cur_pos = 0;
            j_index = get_number(reserve_string, cur_pos);
            i_index = get_number(reserve_string, cur_pos);
            value = get_value(reserve_string, cur_pos);
            data_to_emplase.emplace_back(String_from_file(i_index, j_index, value));
            reserve_string.clear();
        }
        if (count_in_rows.size() < first_count - 1) {
            do {
                cur_pos = 0;
                j_index = get_number(str_from_file, cur_pos);
                i_index = get_number(str_from_file, cur_pos);
                value = get_value(str_from_file, cur_pos);
                if (!data_to_emplase.empty()) {
                    if (data_to_emplase.back().i_index == i_index) {
                        data_to_emplase.emplace_back(String_from_file(i_index, j_index, value));
                    } else {
                        reserve_string = str_from_file;
                        is_new_sequence = true;
                    }
                    if (i_index != j_index) {
                        display_contaiter[j_index].push_back(std::make_pair(i_index, value));
                    }
                } else {
                    data_to_emplase.emplace_back(String_from_file(i_index, j_index, value));
                }
            } while (!is_new_sequence && std::getline(file, str_from_file));
        }

        if (!data_to_emplase.empty()) {
            i_index = data_to_emplase.back().i_index;
        } else {
            i_index = count_in_rows.size() + 1;
        }

        uint32_t cur_row_index = 0;
        if (display_contaiter.contains(i_index)) {
            cur_row_index = display_contaiter[i_index].size();
        }

        for (uint32_t i : std::ranges::views::iota(static_cast<uint32_t>(0), cur_row_index)) {
            j_index = display_contaiter[i_index][i].first;
            value = display_contaiter[i_index][i].second;
            data_to_emplase.emplace_back(String_from_file(i_index, j_index, value));
        }
        display_contaiter.erase(i_index);

        std::sort(data_to_emplase.begin(), data_to_emplase.end(), Compare());

        if (count_in_rows.size() == 0) {
            count_in_rows.push_back(data_to_emplase.size());
        } else {
            count_in_rows.push_back(count_in_rows.back() + data_to_emplase.size());
        }

        for (uint32_t i : std::ranges::views::iota(static_cast<uint32_t>(0), data_to_emplase.size())) {
            columns_indexes.push_back(data_to_emplase[i].j_index - 1);
            values.push_back(data_to_emplase[i].value);
        }

        // std::cout << cur_first_count << '\n';
        // for (uint32_t i : std::ranges::views::iota(cur_second_count, data_to_emplase.size() + cur_second_count)) {
        //     std::cout << columns_indexes[i] << ' ';
        // }
        // std::cout << '\n';
        // for (uint32_t i : std::ranges::views::iota(cur_second_count, data_to_emplase.size() + cur_second_count)) {
        //     std::cout << values[i] << ' ';
        // }
        // std::cout << "\n\n";

        is_new_sequence = false;
        data_to_emplase.clear();
    }

    for (std::map<uint32_t, std::vector<std::pair<uint32_t, cpp_dec_float_100>>>::iterator iter = display_contaiter.begin(); iter != display_contaiter.end(); ++iter) {
        count_in_rows.push_back(count_in_rows.back() + iter->second.size());

        for (uint32_t i : std::ranges::views::iota(static_cast<uint32_t>(0), iter->second.size())) {
            columns_indexes.push_back(iter->second[i].first - 1);
            values.push_back(iter->second[i].second);
        }
    }
    second_count = columns_indexes.size();
    file.close();
}


uint32_t CSRMatrix::get_first_count() const {
    return first_count;
}


uint32_t CSRMatrix::get_second_count() const {
    return second_count;
}


const std::vector<uint32_t>& CSRMatrix::get_count_in_rows() const {
    return count_in_rows;
}


const std::vector<uint32_t>& CSRMatrix::get_columns_indexes() const {
    return columns_indexes;
}


const std::vector<cpp_dec_float_100>& CSRMatrix::get_values() const {
    return values;
}


uint32_t CSRMatrix::get_count_of_columns(const std::vector<uint32_t> columns_array, uint32_t second_count) const{
    uint32_t max = columns_array[0];
    for (uint32_t i : std::ranges::views::iota(static_cast<uint32_t>(1), second_count)) {
        if (max < columns_array[i]) {
            max = columns_array[i];
        }
    }
    return max + 1;
}


std::istream& operator>>(std::istream& is_buffer, CSRMatrix& other) {
    *std::ostream_iterator<std::string>(std::cout, "\n") = "Input the number of rows:";
    other.first_count = *std::istream_iterator<uint32_t>(is_buffer);
    *std::ostream_iterator<std::string>(std::cout, "\n") = "Input the number of non-zero elements:";
    other.second_count = *std::istream_iterator<uint32_t>(is_buffer);

    *std::ostream_iterator<std::string>(std::cout, "\n") = 
    "Input the number of non-zero elements (on the ith line is the number of non-zero elements from the 0th to the ith line)";
    other.count_in_rows.reserve(other.first_count);
    for (uint32_t i : std::ranges::views::iota(static_cast<uint32_t>(0), other.first_count)) {
        other.count_in_rows.push_back(*std::istream_iterator<uint32_t>(std::cin));
    }

    *std::ostream_iterator<std::string>(std::cout, "\n") = 
    "Sequentially input the indexes of the columns containing non-zero elements from the first to the last row";
    other.columns_indexes.reserve(other.second_count);
    for (uint32_t i : std::ranges::views::iota(static_cast<uint32_t>(0), other.second_count)) {
        other.columns_indexes.push_back(*std::istream_iterator<uint32_t>(std::cin));
    }

    *std::ostream_iterator<std::string>(std::cout, "\n") = 
    "Input the values of non-zero elements sequentially from the first to the last line";
    other.values.reserve(other.second_count);
    for (uint32_t i : std::ranges::views::iota(static_cast<uint32_t>(0), other.second_count)) {
        other.values.push_back(*std::istream_iterator<cpp_dec_float_100>(std::cin));
    }

    return is_buffer;
}


template <typename To, typename From>
To converted_to(From const& value) {
    return value.template convert_to<To>();
}


std::ostream& operator<<(std::ostream& os_buffer, const CSRMatrix& other) {
    *std::ostream_iterator<std::string>(os_buffer, "\n") = "Matrix:";
    uint32_t count_of_columns = other.get_count_of_columns(other.columns_indexes, other.second_count);
    uint32_t count_elems_in_cur_row;
    uint32_t cur_j = 0;
    for (uint32_t i : std::ranges::views::iota(static_cast<uint32_t>(0), other.first_count)) {
        if (i == 0) {
            count_elems_in_cur_row = other.count_in_rows.front();
        } else {
            count_elems_in_cur_row = other.count_in_rows[i] - other.count_in_rows[i - 1];
        }
        uint32_t j = 0;
        uint32_t cur_count = 0;
        while (j < count_of_columns) {
            if (j == other.columns_indexes[cur_j] && cur_count < count_elems_in_cur_row) {
                double intpart;
                double double_cur_value = converted_to<double, cpp_dec_float_100>(other.values[cur_j]);
                if (std::modf(double_cur_value, &intpart) == 0.0) {
                    uint16_t left_indentation = (20 - std::to_string(static_cast<int64_t>(other.values[cur_j])).size()) / 2;
                    *std::ostream_iterator<std::string>(std::cout) = std::string(left_indentation, ' ');
                    *std::ostream_iterator<int64_t>(std::cout) = (static_cast<int64_t>(other.values[cur_j]));
                    *std::ostream_iterator<std::string>(std::cout) = std::string(20 - std::to_string(static_cast<int64_t>(other.values[cur_j])).size() - left_indentation, ' ');
                } else if (fabs(double_cur_value) > 0.000001){
                    std::string double_str_to_output{std::to_string(double_cur_value)};
                    while (double_str_to_output.back() == '0') {
                        double_str_to_output.pop_back();
                    }
                    if (double_str_to_output.back() == '.') {
                        double_str_to_output.pop_back();
                    }
                    uint16_t left_indentation = (20 - double_str_to_output.size()) / 2;
                    *std::ostream_iterator<std::string>(std::cout) = std::string(left_indentation, ' ');
                    *std::ostream_iterator<std::string>(std::cout) = double_str_to_output;
                    *std::ostream_iterator<std::string>(std::cout) = std::string(20 - double_str_to_output.size() - left_indentation, ' ');
                }
                ++cur_j;
                ++cur_count;
                
            } else {
                *std::ostream_iterator<std::string>(std::cout) = std::string(9, ' ');
                *std::ostream_iterator<int64_t>(std::cout) = 0;
                *std::ostream_iterator<std::string>(std::cout) = std::string(10, ' ');
            }
            ++j;
        }
        *std::ostream_iterator<std::uint8_t>(os_buffer) = '\n';
    }
    return os_buffer;
}


void CSRMatrix::save_vector(const std::vector<cpp_dec_float_100>& vec) {
    std::string file_name{"test1"};
    while (std::filesystem::exists(file_name + ".txt")) {
        uint16_t count = 0;

        while (file_name.back() == '9') {
            ++count;
            file_name.pop_back();
        }

        if (isdigit(file_name.back())) {
            ++file_name.back();
        } else {
            file_name += '1';
        }

        for (uint16_t i : std::ranges::views::iota(static_cast<uint16_t>(0), count)) {
            file_name += '0';
        }
    }
    std::ofstream save_matrix_file(file_name + ".txt");
    for (uint32_t i : std::ranges::views::iota(static_cast<uint32_t>(0), vec.size())) {
        double intpart;
        double double_cur_value = converted_to<double, cpp_dec_float_100>(vec[i]);

        if (std::modf(double_cur_value, &intpart) == 0) {
            *std::ostream_iterator<int64_t>(save_matrix_file) = (static_cast<int64_t>(vec[i]));
        } else if (fabs(double_cur_value) > 0.000001){
            std::string double_str_to_output{std::to_string(double_cur_value)};
            while (double_str_to_output.back() == '0') {
                double_str_to_output.pop_back();
            }
            if (double_str_to_output.back() == '.') {
                double_str_to_output.pop_back();
            }
            *std::ostream_iterator<std::string>(save_matrix_file) = double_str_to_output;
        }
        *std::ostream_iterator<std::uint8_t>(save_matrix_file) = '\n';
    }
    save_matrix_file.close();
    return;
}


void CSRMatrix::save_matrix() {
    std::string file_name{"test1"};
    while (std::filesystem::exists(file_name + ".txt")) {
        uint16_t count = 0;

        while (file_name.back() == '9') {
            ++count;
            file_name.pop_back();
        }

        if (isdigit(file_name.back())) {
            ++file_name.back();
        } else {
            file_name += '1';
        }

        for (uint16_t i : std::ranges::views::iota(static_cast<uint16_t>(0), count)) {
            file_name += '0';
        }
    }
    std::ofstream save_matrix_file(file_name + ".txt");

    *std::ostream_iterator<std::string>(save_matrix_file, "\n") = "Matrix:";
    uint32_t count_of_columns = get_count_of_columns(columns_indexes, second_count);
    uint32_t count_elems_in_cur_row;
    uint32_t cur_j = 0;
    
    for (uint32_t i : std::ranges::views::iota(static_cast<uint32_t>(0), first_count)) {
        if (i == 0) {
            count_elems_in_cur_row = count_in_rows.front();
        } else {
            count_elems_in_cur_row = count_in_rows[i] - count_in_rows[i - 1];
        }
        uint32_t j = 0;
        uint32_t cur_count = 0;
        while (j < count_of_columns) {

            if (j == columns_indexes[cur_j] && cur_count < count_elems_in_cur_row) {
                double intpart;
                double double_cur_value = converted_to<double, cpp_dec_float_100>(values[cur_j]);

                if (std::modf(double_cur_value, &intpart) == 0) {
                    uint16_t left_indentation = (20 - std::to_string(static_cast<int64_t>(values[cur_j])).size()) / 2;
                    *std::ostream_iterator<std::string>(save_matrix_file) = std::string(left_indentation, ' ');
                    *std::ostream_iterator<int64_t>(save_matrix_file) = (static_cast<int64_t>(values[cur_j]));
                    *std::ostream_iterator<std::string>(save_matrix_file) = std::string(20 - std::to_string(static_cast<int64_t>(values[cur_j])).size() - left_indentation, ' ');
                } else if (fabs(double_cur_value) > 0.000001){
                    std::string double_str_to_output{std::to_string(double_cur_value)};
                    while (double_str_to_output.back() == '0') {
                        double_str_to_output.pop_back();
                    }
                    if (double_str_to_output.back() == '.') {
                        double_str_to_output.pop_back();
                    }
                    uint16_t left_indentation = (20 - double_str_to_output.size()) / 2;
                    *std::ostream_iterator<std::string>(save_matrix_file) = std::string(left_indentation, ' ');
                    *std::ostream_iterator<std::string>(save_matrix_file) = double_str_to_output;
                    *std::ostream_iterator<std::string>(save_matrix_file) = std::string(20 - double_str_to_output.size() - left_indentation, ' ');
                }
                ++cur_count;
                ++cur_j;
            } else {
                *std::ostream_iterator<std::string>(save_matrix_file) = std::string(9, ' ');
                *std::ostream_iterator<int64_t>(save_matrix_file) = 0;
                *std::ostream_iterator<std::string>(save_matrix_file) = std::string(10, ' ');
            }
            ++j;
        }
        *std::ostream_iterator<std::uint8_t>(save_matrix_file) = '\n';
    }
    save_matrix_file.close();
    return;
}


// std::string CSRMatrix::generate_matrix(uint32_t n, uint32_t count_not_zero) {
//     srand(time(0));
//     std::string new_file_name{"file.txt"};
//     std::ofstream new_file(new_file_name);
//     new_file << n << ' ' << n << ' ' << count_not_zero << '\n';
//     count_not_zero -= n;
//     uint32_t cur_count_non_nul;
//     for (uint32_t i : std::ranges::views::iota(static_cast<uint32_t>(0), n)) {
//         if (count_not_zero > 1 && n - i - 1 > 0) {
//             cur_count_non_nul = (rand() % std::min(n - i - 1, count_not_zero) + 1) / 5;
//         } else {
//             cur_count_non_nul = 0;
//         }
//         count_not_zero -= cur_count_non_nul;
//         new_file << i + 1 << ' ' << i + 1 << ' ' << 1 << '\n';
//         std::set<uint32_t> unique_numbers;
//         std::random_device rd; // Используем для генерации случайного начального значения
//         std::mt19937 gen(rd()); // Генератор случайных чисел
//         std::normal_distribution<> dis(i + 1, (n - i) / 5 + 1); // Равномерное распределение
//         while (unique_numbers.size() < cur_count_non_nul) {
//             uint32_t number = static_cast<int>(dis(gen)); // Генерируем случайное число
//             if (number > i + 1 && number < n) {
//                 unique_numbers.insert(number); // Генерируем случайное число и добавляем в множество
//             }
//         }
//         for (std::set<uint32_t>::iterator iter = unique_numbers.begin(); iter != unique_numbers.end(); ++iter) {
//             new_file << *iter << ' ' << i + 1 << ' ' << 1 << '\n';
//         }  
//     }
//     return "file.txt";
// }


cpp_dec_float_100 generate_value(uint64_t abs_min, uint64_t abs_max) {
    cpp_dec_float_100 intrger_part = abs_min + std::rand() % (abs_max - abs_min + 1);

    uint8_t random_number = std::rand() % (1 + 1);
    if (random_number == 0) {
        intrger_part *= -1;
    }
    random_number = std::rand() % (1 + 1);
    if (random_number == 0) {
        cpp_dec_float_100 fractional_part = abs_min + std::rand() % (abs_max - abs_min + 1);
        fractional_part = fractional_part / pow(10, fractional_part.str().length());
        intrger_part += fractional_part;
    }
    return intrger_part;
}


// std::string CSRMatrix::generate_matrix(uint32_t n, uint32_t count_not_zero, uint64_t abs_min, uint64_t abs_max) {
//     std::srand(static_cast<unsigned int>(std::time(0)));
//     std::string new_file_name{"file.txt"};
//     std::ofstream new_file(new_file_name);
//     count_not_zero = (count_not_zero - n) / 2;
//     std::vector<std::vector<uint32_t>> columns_arrays;
//     new_file << n << ' ' << n << ' ' << count_not_zero << '\n';
//     uint32_t cur_count_non_nul;
//     uint32_t cur_j = 0, k, ratio;
//     for (uint32_t i : std::ranges::views::iota(static_cast<uint32_t>(0), n - 1)) {
//         columns_arrays.push_back({i});
//         std::set<uint32_t> unique_indexes;
//         if (i < n / 2) {
//             ratio = (rand() % (n/2 - i) + 1);
//         } else {
//             ratio = (rand() % (n - i + 1) + 1);
//         }
//         if (cur_j != 0 && count_not_zero > 0) {
//             cur_count_non_nul = rand() % std::min(cur_j + 1, count_not_zero);
//         } else {
//             cur_count_non_nul = 0;
//         }
//         cur_count_non_nul /= (rand() % (i + 1) + 1);
//         count_not_zero -= cur_count_non_nul;
//         while (unique_indexes.size() < cur_count_non_nul) {
//             uint32_t index = rand() % (cur_j + 1); 
//             if (index > 0) {
//                 unique_indexes.insert(index);
//             }
//         }
//         for (std::set<uint32_t>::const_iterator iter = unique_indexes.cbegin(); iter != unique_indexes.cend(); ++iter) {
//             columns_arrays[i - *iter].push_back(i + *iter);
//         }
//         if (n % 2 != 0) {
//             if (i < n / 2) {
//                 ++cur_j;
//             } else {
//                 --cur_j;
//             }
//             k = cur_j;
//             if (i >= n % 2) {
//                 ++k;
//             }
//         } else {
//             if (i < n / 2 - 1) {
//                 ++cur_j;
//             } else if (i >= n / 2){
//                 --cur_j;
//             }
//             k = cur_j;
//             if (i >= n % 2 - 1) {
//                 ++k;
//             }
//         }
//         unique_indexes.clear();
//         if (k != 0 && count_not_zero > 0) {
//             cur_count_non_nul = rand() % std::min(k + 1, count_not_zero);
//         } else {
//             cur_count_non_nul = 0;
//         }
//         cur_count_non_nul /= (rand() % (i + 1) + 1);
//         count_not_zero -= cur_count_non_nul;
//         while (unique_indexes.size() < cur_count_non_nul) {
//             uint32_t index = rand() % k; 
//             unique_indexes.insert(index);
//         }
//         for (std::set<uint32_t>::const_iterator iter = unique_indexes.cbegin(); iter != unique_indexes.cend(); ++iter) {
//             columns_arrays[i - *iter].push_back(i + *iter + 1);
//         }
//     }
//     columns_arrays.push_back({n-1});
//     for (uint32_t i : std::ranges::views::iota(static_cast<uint32_t>(0), n)) {
//         for (uint32_t j : columns_arrays[i]) {
//             new_file << j + 1 << ' ' << i + 1 << ' ' << generate_value(abs_min, abs_max) << '\n';
//         }
//     }
//     return "file.txt";
// }


std::string CSRMatrix::generate_matrix(uint32_t n, uint32_t count_not_zero, uint64_t abs_min, uint64_t abs_max) {
    std::srand(static_cast<unsigned int>(std::time(0)));
    std::string new_file_name{"file.txt"};
    std::ofstream new_file(new_file_name);
    count_not_zero = (count_not_zero - n) / 2;

    std::vector<std::vector<uint32_t>> columns_arrays;
    uint32_t cur_count_non_nul;
    uint32_t cur_j = 0, k;
    std::pair<uint32_t, uint32_t> first_prev_pos{0, 0}, second_prev_pos{0, 0}, copy{0,0};

    bool flag_1 = false;
    for (uint32_t i : std::ranges::views::iota(static_cast<uint32_t>(0), n - 1)) {
        columns_arrays.push_back({i});
        std::set<uint32_t> unique_indexes;

        if (i != 0 && count_not_zero > 0) {
            if (i >= 2 && flag_1 && i < n - 2) {
                cur_count_non_nul = rand() % std::min(i - first_prev_pos.first, cur_j - (i - first_prev_pos.first) + 1);
            } else {
                cur_count_non_nul = rand() % std::min(cur_j + 1, count_not_zero + 1);
            }
        } else {
            cur_count_non_nul = 0;
        }

        count_not_zero -= cur_count_non_nul;
        uint32_t index = 0;
        bool special_flag = (cur_count_non_nul == 0 && flag_1);
        while (unique_indexes.size() < cur_count_non_nul || special_flag) {
            if (!flag_1) {
                index = rand() % (cur_j + 1);
                while (index == 0) {
                    index = rand() % (cur_j + 1);
                }
            } else {
                index = rand() % (std::max(cur_count_non_nul, static_cast<uint32_t>(1)) + 1);
                special_flag = false;
            }
            unique_indexes.insert(index);
        }

        uint8_t sign = rand() % 2;
        if (i >= 2  && flag_1 && i < n - 2) {
            for (std::set<uint32_t>::const_iterator iter = unique_indexes.cbegin(); iter != unique_indexes.cend(); ++iter) {
                if ((sign == 0 || i - 1 == first_prev_pos.first) && n - 1 > first_prev_pos.second) {
                    columns_arrays[first_prev_pos.first - *iter].push_back(first_prev_pos.second + *iter);
                    if (copy.first >= first_prev_pos.first - *iter) {
                        copy = std::make_pair(first_prev_pos.first + 1 - *iter, first_prev_pos.second + 1 + *iter);
                    }
                } else {
                    columns_arrays[first_prev_pos.first + *iter].push_back(first_prev_pos.second  - *iter);
                    if (copy.first <= first_prev_pos.first + *iter) {
                        copy = std::make_pair(first_prev_pos.first + 1 + *iter, first_prev_pos.second + 1 - *iter);
                    }
                }
            }
        } else {
            for (std::set<uint32_t>::const_iterator iter = unique_indexes.cbegin(); iter != unique_indexes.cend(); ++iter) {
                columns_arrays[i - *iter].push_back(i + *iter);
                first_prev_pos = std::make_pair(i - *iter + 1, i + *iter + 1);
                copy = first_prev_pos;
                flag_1 = true;
            }
        }
        first_prev_pos = copy;
        if (copy.first > i || copy.second >= n || i == n - 3) {
            flag_1 = false;
        }

        if (n % 2 != 0) {
            if (i < n / 2) {
                ++cur_j;
            } else {
                --cur_j;
            }
            k = cur_j;
            if (i >= n % 2) {
                ++k;
            }
        } else {
            if (i < n / 2 - 1) {
                ++cur_j;
            } else if (i >= n / 2){
                --cur_j;
            }
            k = cur_j;
            if (i >= n / 2 - 1) {
                ++k;
            }
        }
        unique_indexes.clear();

        if (flag_1) {
            if (first_prev_pos.second < n) {
                second_prev_pos = std::make_pair(first_prev_pos.first, first_prev_pos.second - 1); 
            } else {
                second_prev_pos = std::make_pair(first_prev_pos.first - 1, first_prev_pos.second); 
            }

            cur_count_non_nul = rand() % std::min(i - second_prev_pos.first + 1, k - (i - second_prev_pos.first));

            count_not_zero -= cur_count_non_nul;
            index = 0;
            special_flag = (cur_count_non_nul == 0);
            while (unique_indexes.size() < cur_count_non_nul || special_flag) {
                index = rand() % (std::max(cur_count_non_nul, static_cast<uint32_t>(1)) + 1);
                unique_indexes.insert(index);
                special_flag = false;
            }

            sign = rand() % 2;
            for (std::set<uint32_t>::const_iterator iter = unique_indexes.cbegin(); iter != unique_indexes.cend(); ++iter) {
                if ((sign == 0 || i == second_prev_pos.first) && n - 1 > second_prev_pos.second) {
                    columns_arrays[second_prev_pos.first - *iter].push_back(second_prev_pos.second + *iter);
                } else {
                    columns_arrays[second_prev_pos.first + *iter].push_back(second_prev_pos.second  - *iter);
                }
            }
        } else {
            if (k != 0 && count_not_zero > 0) {
                cur_count_non_nul = rand() % std::min(k + 1, count_not_zero);
            } else {
                cur_count_non_nul = 0;
            }
            cur_count_non_nul /= (rand() % (i + 1) + 1);
            count_not_zero -= cur_count_non_nul;
            while (unique_indexes.size() < cur_count_non_nul) {
                uint32_t index = rand() % k; 
                unique_indexes.insert(index);
            }
            for (std::set<uint32_t>::const_iterator iter = unique_indexes.cbegin(); iter != unique_indexes.cend(); ++iter) {
                columns_arrays[i - *iter].push_back(i + *iter + 1);
            }
        }
    }
    columns_arrays.push_back({n-1});
    count_not_zero = 0;
    for (uint32_t i : std::ranges::views::iota(static_cast<uint32_t>(0), n)) {
        count_not_zero += columns_arrays[i].size();
    }
    new_file << n << ' ' << n << ' ' << count_not_zero << '\n';

    for (uint32_t i : std::ranges::views::iota(static_cast<uint32_t>(0), n)) {
        for (uint32_t j : columns_arrays[i]) {
            cpp_dec_float_100 val = generate_value(abs_min, abs_max);
            new_file << j + 1 << ' ' << i + 1 << ' ' << val << '\n';
        }
    }
    return "file.txt";
}


CSRMatrix CSRMatrix::get_transposed_matrix(const CSRMatrix& matrix) {
    std::vector<uint32_t> count_in_rows_vec;
    std::vector<uint32_t> columns_indexes_vec;
    std::vector<cpp_dec_float_100> values_vec;
    count_in_rows_vec.reserve(matrix.first_count);
    columns_indexes_vec.reserve(matrix.second_count);
    values_vec.reserve(matrix.second_count);
    
    uint32_t cur_count = 0;
    uint32_t* cur_pos = new uint32_t[matrix.first_count];

    for (uint32_t i : std::ranges::views::iota(static_cast<uint32_t>(0), matrix.first_count)) {
        if (i == 0) {
            cur_pos[i] = 0;
        } else {
            cur_pos[i] = matrix.count_in_rows[i - 1];
        }
    }

    for (uint32_t i : std::ranges::views::iota(static_cast<uint32_t>(0), matrix.first_count)) {
        for (uint32_t j : std::ranges::views::iota(static_cast<uint32_t>(0), matrix.first_count)) {
            if (matrix.columns_indexes[cur_pos[j]] == i && cur_pos[j] < matrix.count_in_rows[j]) {
                ++cur_count;
                columns_indexes_vec.push_back(j);
                values_vec.push_back(matrix.values[cur_pos[j]]);
                ++cur_pos[j];
            }
        }
        if (count_in_rows_vec.empty()) {
            count_in_rows_vec.push_back(cur_count);
        } else {
            count_in_rows_vec.push_back(cur_count + count_in_rows_vec.back());
        }
        cur_count = 0;
    }

    delete[] cur_pos;
    CSRMatrix transposed_matrix(matrix.first_count, matrix.second_count, std::move(count_in_rows_vec), std::move(columns_indexes_vec), std::move(values_vec));
    return transposed_matrix;
}


CSRMatrix operator*(CSRMatrix first_matrix, const CSRMatrix& second_matrix) {
    return first_matrix *= second_matrix;
}


CSRMatrix& CSRMatrix::operator=(const CSRMatrix& other) {
    first_count = other.first_count;
    second_count = other.second_count;
    count_in_rows = other.count_in_rows;
    columns_indexes = other.columns_indexes;
    values = other.values;
    return *this;
}


CSRMatrix& CSRMatrix::operator=(CSRMatrix&& other)  {
    first_count = other.first_count;
    second_count = other.second_count;
    count_in_rows = std::move(other.count_in_rows);
    columns_indexes = std::move(other.columns_indexes);
    values = std::move(other.values);
    return *this;
}


CSRMatrix& CSRMatrix::operator*=(const CSRMatrix& second_matrix) {
    std::vector<uint32_t> count_in_rows_vec;
    std::vector<uint32_t> columns_indexes_vec;
    std::vector<cpp_dec_float_100> values_vec;
    count_in_rows_vec.reserve(second_matrix.get_first_count());
    columns_indexes_vec.reserve(second_matrix.get_second_count());
    values_vec.reserve(second_matrix.get_second_count());
    CSRMatrix transposed_second_matrix(CSRMatrix::get_transposed_matrix(second_matrix));

    uint32_t prev_first_start_pos, first_start_pos, first_end_pos, second_start_pos, second_end_pos, cur_count = 0;
    cpp_dec_float_100 cur_answer = 0;
    #pragma omp parallel for
    for (uint32_t i : std::ranges::views::iota(static_cast<uint32_t>(0), second_matrix.first_count)) {
        if (i == 0) {
            first_start_pos = 0;
            first_end_pos = this->count_in_rows[0];
        } else {
            first_start_pos = this->count_in_rows[i - 1];
            first_end_pos = this->count_in_rows[i];
        }
        prev_first_start_pos = first_start_pos;
        #pragma omp parallel for
        for (uint32_t j : std::ranges::views::iota(static_cast<uint32_t>(0), second_matrix.first_count)) {
            if (j == 0) {
                second_start_pos = 0;
                second_end_pos = transposed_second_matrix.count_in_rows[0];
            } else {
                second_start_pos = transposed_second_matrix.count_in_rows[j - 1];
                second_end_pos = transposed_second_matrix.count_in_rows[j];
            }
            while (second_start_pos < second_end_pos && first_start_pos < first_end_pos) {
                if (this->columns_indexes[first_start_pos] == transposed_second_matrix.columns_indexes[second_start_pos]) {
                    cur_answer += this->values[first_start_pos] * transposed_second_matrix.values[second_start_pos];
                    ++first_start_pos;
                    ++second_start_pos;
                } else if (this->columns_indexes[first_start_pos] < transposed_second_matrix.columns_indexes[second_start_pos]) {
                    ++first_start_pos;
                } else {
                    ++second_start_pos;
                }
            }
            if (cur_answer != 0 && fabs(cur_answer) > 0.0000000001) {
                columns_indexes_vec.push_back(j);
                values_vec.push_back(cur_answer);
                ++cur_count;
            }
            first_start_pos = prev_first_start_pos;
            cur_answer = 0; 
        }
        count_in_rows_vec.push_back(cur_count);
    }

    *this = CSRMatrix(this->first_count, columns_indexes_vec.size(), std::move(count_in_rows_vec), std::move(columns_indexes_vec), std::move(values_vec));
    return *this;
}


std::vector<cpp_dec_float_100> operator*(const CSRMatrix& A, const std::vector<cpp_dec_float_100>& vec) {
    std::vector<cpp_dec_float_100> result;

    std::pair<uint32_t, uint32_t> start_and_end_pos_index;
    #pragma omp parallel for
    for (uint32_t i : std::ranges::views::iota(static_cast<uint32_t>(0), A.first_count)) {
        if (i == 0) {
            start_and_end_pos_index = std::make_pair(0, A.count_in_rows[0]);
        } else {
            start_and_end_pos_index = std::make_pair(A.count_in_rows[i - 1], A.count_in_rows[i]);
        }
        cpp_dec_float_100 cur_sum = 0;
        for (uint32_t j : std::ranges::views::iota(start_and_end_pos_index.first, start_and_end_pos_index.second)) {
            cur_sum += A.values[j] * vec[A.columns_indexes[j]];
        }
        
        result.push_back(cur_sum);
    }
    return result;
}


CSRMatrix operator+(CSRMatrix first_matrix, const CSRMatrix& second_matrix) {
    return first_matrix += second_matrix;
}


CSRMatrix& CSRMatrix::operator+=(const CSRMatrix& second_matrix) {
    std::vector<uint32_t> count_in_rows_vec;
    std::vector<uint32_t> columns_indexes_vec;
    std::vector<cpp_dec_float_100> values_vec;
    count_in_rows_vec.reserve(second_matrix.get_first_count());
    columns_indexes_vec.reserve(second_matrix.get_second_count());
    values_vec.reserve(second_matrix.get_second_count());

    uint32_t first_start_pos, first_end_pos, second_start_pos, second_end_pos, cur_count = 0;
    for (uint32_t i : std::ranges::views::iota(static_cast<uint32_t>(0), second_matrix.first_count)) {
        if (i == 0) {
            first_start_pos = 0;
            first_end_pos = this->count_in_rows[0];
            second_start_pos = 0;
            second_end_pos = second_matrix.count_in_rows[0];
        } else {
            first_start_pos = this->count_in_rows[i - 1];
            first_end_pos = this->count_in_rows[i];
            second_start_pos = second_matrix.count_in_rows[i - 1];
            second_end_pos = second_matrix.count_in_rows[i];
        }

        while (second_start_pos < second_end_pos && first_start_pos < first_end_pos) {
            if (this->columns_indexes[first_start_pos] == second_matrix.columns_indexes[second_start_pos]) {
                columns_indexes_vec.push_back(this->columns_indexes[first_start_pos]);
                values_vec.push_back(this->values[first_start_pos] + second_matrix.values[second_start_pos]);
                ++first_start_pos;
                ++second_start_pos;
            } else if (this->columns_indexes[first_start_pos] < second_matrix.columns_indexes[second_start_pos]) {
                columns_indexes_vec.push_back(this->columns_indexes[first_start_pos]);
                values_vec.push_back(this->values[first_start_pos]);
                ++first_start_pos;
            } else {
                columns_indexes_vec.push_back(second_matrix.columns_indexes[second_start_pos]);
                values_vec.push_back(second_matrix.values[second_start_pos]);
                ++second_start_pos;
            }
            ++cur_count;
        } 
        while (second_start_pos == second_end_pos && first_start_pos < first_end_pos) {
            columns_indexes_vec.push_back(this->columns_indexes[first_start_pos]);
            values_vec.push_back(this->values[first_start_pos]);
            ++first_start_pos;
            ++cur_count;
        }
        while (second_start_pos < second_end_pos && first_start_pos == first_end_pos) {
            columns_indexes_vec.push_back(second_matrix.columns_indexes[second_start_pos]);
            values_vec.push_back(second_matrix.values[second_start_pos]);
            ++second_start_pos;
            ++cur_count;
        }
        
        count_in_rows_vec.push_back(cur_count);
        cur_count = 0; 
    }

    *this = CSRMatrix(this->first_count, columns_indexes_vec.size(), std::move(count_in_rows_vec), std::move(columns_indexes_vec), std::move(values_vec));
    return *this;
}


CSRMatrix CSRMatrix::operator-() const{
    CSRMatrix answer(*this);
    for (uint32_t i : std::ranges::views::iota(static_cast<uint32_t>(0), this->second_count)) {
        answer.values[i] *= -1;
    }
    return answer;
}


CSRMatrix operator-(CSRMatrix first_matrix, const CSRMatrix& second_matrix) {
    return first_matrix -= second_matrix;
}


CSRMatrix& CSRMatrix::operator-=(const CSRMatrix& second_matrix) {
    CSRMatrix new_second_matrix = -second_matrix;
    return *this += new_second_matrix;
}


bool CSRMatrix::is_equal(const CSRMatrix& first_matrix, const CSRMatrix& second_matrix) {
    bool answer = true;
    cpp_dec_float_100 error_rate = 0.000001;
    if (first_matrix.first_count == second_matrix.first_count && first_matrix.second_count == second_matrix.second_count) {
        for (uint32_t i = 0; i < first_matrix.first_count && answer; ++i) {
            if (first_matrix.count_in_rows[i] != second_matrix.count_in_rows[i]) {
                answer = false;
            }
        }
        for (uint32_t i = 0; i < first_matrix.second_count && answer; ++i) {
            if (first_matrix.columns_indexes[i] != second_matrix.columns_indexes[i]) {
                answer = false;
            }
        }
        for (uint32_t i = 0; i < first_matrix.second_count; ++i) {
            if (first_matrix.values[i] - second_matrix.values[i] > error_rate) {
                answer = false;
                std::cout << fabs(first_matrix.values[i] - second_matrix.values[i]) << '\n';
            }
        }
    } else {
        answer = false;
        if (answer == false)
            std::cout << "Fatal Pizdec:  " << first_matrix.second_count << "  " << second_matrix.second_count;
    }
    return answer;
}


std::vector<cpp_dec_float_100> CSRMatrix::generate_b(const std::vector<cpp_dec_float_100>& values, uint32_t first_count) {
    std::srand(static_cast<unsigned int>(std::time(0)));
    std::vector<cpp_dec_float_100> b;
    b.reserve(first_count);

    uint64_t abs_min = converted_to<uint64_t, cpp_dec_float_100>(abs(values.front())), abs_max = abs_min;
    for (uint32_t i : std::ranges::views::iota(static_cast<uint32_t>(0), values.size())) {
        if (fabs(values[i]) > abs_max) {
            abs_max = converted_to<uint64_t, cpp_dec_float_100>(abs(values[i]));
        }
        if (fabs(values[i]) < abs_min) {
            abs_min = converted_to<uint64_t, cpp_dec_float_100>(abs(values[i]));
        }
    }

    for (uint32_t i : std::ranges::views::iota(static_cast<uint32_t>(0), first_count)) {
        uint8_t random_number = 0 + std::rand() % (3 + 1);
        
        if (random_number == 0) {
            b.push_back(0);
        } else {
            cpp_dec_float_100 cur_intrger_part = abs_min + std::rand() % (abs_max - abs_min + 1);

            random_number = 0 + std::rand() % (1 + 1);
            if (random_number == 0) {
                b.push_back(cur_intrger_part);
            } else {
                b.push_back(-cur_intrger_part);
            }
            random_number = 0 + std::rand() % (1 + 1);
            if (random_number == 0) {
                cpp_dec_float_100 cur_fractional_part = abs_min + std::rand() % (abs_max - abs_min + 1);
                cur_fractional_part = cur_fractional_part / pow(10, cur_fractional_part.str().length());
                b.back() += cur_fractional_part;
            }
        }
    }

    return b;
}