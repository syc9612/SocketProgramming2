#include <iostream>
#include <string>
#include <unordered_map>
#include <memory>
#include <boost/bind/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/array.hpp>

using boost::asio::ip::tcp;

class tcp_connection : public boost::enable_shared_from_this<tcp_connection>{
    private:
    
    std::shared_ptr<tcp::socket> sock_ptr_;
    std::string client_name_;
    std::unordered_map<std::string, std::shared_ptr<tcp::socket>> name_to_socket_;

    tcp_connection(boost::asio::io_context& io_context){
        sock_ptr_ = std::make_shared<tcp::socket>(io_context);
    }

    void handle_receive(const boost::system::error_code& e, std::shared_ptr<boost::array<char, 1024>> buf_ptr){
        if (!e)
        {
            std::stringstream s(buf_ptr->data());
            std::string cmd;
            s >> cmd;
            
            if (cmd == "init") // initial message from client
            {
                s >> client_name_;

                if (name_to_socket_.find(client_name_) != name_to_socket_.end()) // name already exists
                {
                    sock_ptr_->async_send(boost::asio::buffer("error: duplicate name\n"),
                                            boost::bind(&tcp_connection::handle_send, shared_from_this()));
                    sock_ptr_->close();
                }
                else
                {
                    name_to_socket_[client_name_] = sock_ptr_;
                }
            }
            else if (cmd == "send")
            {
                std::string receiver_name, content;
                s >> receiver_name;
                getline(s, content);
                
                if (receiver_name.empty() == true || content.empty() == true) // incorrect format
                {
                    sock_ptr_->async_send(boost::asio::buffer("error: <name> <message>\n"),
                                                              boost::bind(&tcp_connection::handle_send, shared_from_this()));
                }
                else
                {
                    if (name_to_socket_.find(receiver_name) == name_to_socket_.end()) // receiver not found
                    {
                        sock_ptr_->async_send(boost::asio::buffer("error: name not found\n"),
                                              boost::bind(&tcp_connection::handle_send, shared_from_this()));
                    }
                    else
                    {
                        std::string message = client_name_ + ":" + content + "\n";
                        name_to_socket_[receiver_name]->async_send(boost::asio::buffer(message),
                                                                   boost::bind(&tcp_connection::handle_send, shared_from_this()));
                    }
                }
            }
        }

        start();
    }

    void handle_send()
    {
    }

    public:
    typedef boost::shared_ptr<tcp_connection> pointer;
    static pointer create(boost::asio::io_context& io_context){
        return pointer(new tcp_connection(io_context));
    }
    tcp::socket& socket(){
        return *sock_ptr_;
    }
    void start(){
        auto buf_ptr = std::make_shared<boost::array<char,1024>>();
        sock_ptr_->async_receive(boost::asio::buffer(*buf_ptr),
                                boost::bind(&tcp_connection::handle_receive, shared_from_this(), boost::asio::placeholders::error, buf_ptr));
    }
};

class tcp_server{
    private:
    boost::asio::io_context& io_context_;
    boost::asio::ip::tcp::acceptor acceptor_;

    void start_accept(){
        tcp_connection::pointer new_connection = tcp_connection::create(io_context_);
        acceptor_.async_accept(new_connection->socket(), boost::bind(&tcp_server::handle_accept, this, new_connection, boost::asio::placeholders::error));
    }
    void handle_accept(tcp_connection::pointer new_connection, const boost::system::error_code& e){
        if(!e){
            new_connection->start();
        }
        start_accept();
    }
    public:
        tcp_server(boost::asio::io_context& io_context): io_context_(io_context), acceptor_(io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 8080)){
            start_accept();
        }
};

int main(){
    try{
        boost::asio::io_context io_context;
        tcp_server server(io_context);
        io_context.run();
    }
    catch(std::exception& e){
        std::cerr << e.what() << std::endl;
    }
    return 0;
}