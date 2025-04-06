#include <iostream>
#include <vector>
#include <asio.hpp>
#include <CSRMatrix.h>

using asio::ip::tcp;

void send_array(const std::vector<double> &array, tcp::socket &socket)
{
  uint32_t size = static_cast<uint32_t>(array.size());
  asio::write(socket, asio::buffer(&size, sizeof(size)));
  asio::write(socket, asio::buffer(array.data(), size * sizeof(double)));
}

template <typename T>
std::vector<T> read_array(tcp::socket &socket)
{
  uint32_t size;
  asio::read(socket, asio::buffer(&size, sizeof(size)));
  std::vector<T> result(size);
  asio::read(socket, asio::buffer(result.data(), size * sizeof(T)));
  return result;
}

template<typename T>
void print_array(const std::string &name, const std::vector<T> &vec){
  std::cout << name << " = ";
  for(auto &v: vec){
    std::cout << v << " ";
  }
  std::cout << std::endl;
}


void show_error(const std::vector<cpp_dec_float_100>& Ax, const std::vector<cpp_dec_float_100>& b) {
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


int main(int argc, char *argv[])
{
  try
  {
    std::string filename = "../../client/tests/test_29.txt";
    CSRMatrix matrix(filename);
    std::cout << matrix;

    asio::io_context io_context;

    tcp::resolver resolver(io_context);
    tcp::resolver::results_type endpoints = resolver.resolve("localhost", "9451");

    tcp::socket socket(io_context);
    asio::connect(socket, endpoints);

  
    matrix.send_by_socket(socket);
    auto result = read_array<cpp_dec_float_100>(socket);
    auto b = read_array<cpp_dec_float_100>(socket);

    print_array("x", result);
    print_array("b", b);

    auto Ax = matrix * result;

    show_error(Ax, b);
    
    std::cout << "Finished" << std::endl;
  }
  catch (std::exception &e)
  {
    std::cerr << "Exception: " << e.what() << std::endl;
  }

  return 0;
}