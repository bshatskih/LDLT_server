#include <iostream>
#include <vector>
#include <asio.hpp>
#include "CSRMatrix.h"
#include "LDLT_matrices.h"

using asio::ip::tcp;

class tcp_connection : public std::enable_shared_from_this<tcp_connection>
{
public:
  typedef std::shared_ptr<tcp_connection> pointer;

  static pointer create(asio::io_context &io_context)
  {
    return pointer(new tcp_connection(io_context));
  }

  tcp::socket &socket()
  {
    return socket_;
  }

  template <typename T>
  void send_array(const std::vector<T> &array, tcp::socket &socket)
  {
    uint32_t size = static_cast<uint32_t>(array.size());
    asio::write(socket, asio::buffer(&size, sizeof(size)));
    asio::write(socket, asio::buffer(array.data(), size * sizeof(T)));
  }

  void start()
  {
    auto matrix = CSRMatrix::get_from_socket(socket_);
    LDLT_matrices ldlt(matrix);
    auto result = ldlt.solution();
    auto b = ldlt.get_b();
    send_array(result, socket_);
    send_array(b, socket_);
    std::cout << "Result sended!" << std::endl;
  }

private:
  tcp_connection(asio::io_context &io_context)
      : socket_(io_context)
  {
    id=num;
    num++;
  }

  tcp::socket socket_;
  static int num;
  int id;
};

int tcp_connection::num = 0;

#define PORT 9451

class tcp_server
{
public:
  tcp_server(asio::io_context &io_context)
      : io_context_(io_context),
        acceptor_(io_context, tcp::endpoint(tcp::v4(), PORT))
  {
    std::cout << "Server started at port " << PORT << std::endl;;
    start_accept();
  }

private:
  void start_accept()
  {
    tcp_connection::pointer new_connection = tcp_connection::create(io_context_);

    acceptor_.async_accept(new_connection->socket(),
                           [this, new_connection](std::error_code ec)
                           {
                             if (!ec)
                             {
                               new_connection->start();
                             }
                             else
                             {
                               std::cerr << "Error accepting connection: " << ec.message() << std::endl;
                             }
                             start_accept();
                           });
  }

  asio::io_context &io_context_;
  tcp::acceptor acceptor_;
};



int main()
{
  try
  {
    asio::io_context io_context;
    tcp_server server(io_context);
    io_context.run();
  }
  catch (std::exception &e)
  {
    std::cerr << "Exception: " << e.what() << std::endl;
  }

  return 0;
}
