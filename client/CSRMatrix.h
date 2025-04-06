#pragma once
#include <boost/multiprecision/cpp_dec_float.hpp>
#include <asio.hpp>
#include <iostream>
#include <ranges>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <random>
#include <ctime>
#include <iterator>
#include <algorithm>
#include <set>
#include <cstdlib>
#include <cmath>
#include <map>
#include <vector>

using namespace boost::multiprecision;

using asio::ip::tcp;

class CSRMatrix {
    private:
    uint32_t first_count;
    uint32_t second_count;
    std::vector<uint32_t> count_in_rows;
    std::vector<uint32_t> columns_indexes; 
    std::vector<cpp_dec_float_100> values;

    public:

    CSRMatrix() = default;
    CSRMatrix(const CSRMatrix&);
    CSRMatrix(const std::vector<std::vector<cpp_dec_float_100>>&);
    CSRMatrix(CSRMatrix&&);
    CSRMatrix(uint32_t, uint32_t, std::vector<uint32_t>&&, std::vector<uint32_t>&&, std::vector<cpp_dec_float_100>&&);
    CSRMatrix(std::string&);

    uint32_t get_count_of_columns(const std::vector<uint32_t>, uint32_t) const;
    uint32_t get_first_count() const;
    uint32_t get_second_count() const;
    const std::vector<uint32_t>& get_count_in_rows() const;
    const std::vector<uint32_t>& get_columns_indexes() const;
    const std::vector<cpp_dec_float_100>& get_values() const;

    void send_by_socket(tcp::socket &socket);
    static CSRMatrix get_from_socket(tcp::socket &socket);

    
    CSRMatrix& operator=(const CSRMatrix&);
    CSRMatrix& operator=(CSRMatrix&&);
    CSRMatrix& operator*=(const CSRMatrix&);
    CSRMatrix& operator+=(const CSRMatrix&);
    CSRMatrix& operator-=(const CSRMatrix&);
    CSRMatrix operator-() const;
    
    friend CSRMatrix operator*(CSRMatrix, const CSRMatrix&);
    friend std::vector<cpp_dec_float_100> operator*(const CSRMatrix&, const std::vector<cpp_dec_float_100>&);
    friend CSRMatrix operator+(CSRMatrix, const CSRMatrix&);
    friend CSRMatrix operator-(CSRMatrix, const CSRMatrix&);
    friend std::istream& operator>>(std::istream& is_buffer, CSRMatrix& other);
    friend std::ostream& operator<<(std::ostream& os_buffer, const CSRMatrix& other);

    void save_matrix();
    static void save_vector(const std::vector<cpp_dec_float_100>&);
    static std::vector<cpp_dec_float_100> generate_b(const std::vector<cpp_dec_float_100>&, uint32_t);
    static CSRMatrix get_transposed_matrix(const CSRMatrix&);
    static std::string generate_matrix(uint32_t, uint32_t, uint64_t, uint64_t);
    static bool is_equal(const CSRMatrix&, const CSRMatrix&);
};