#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <iostream>
#include <string>
#include <thread>

using namespace boost::asio;
using ip::udp;
using namespace std; // for std::bind and std::placeholders

// UDP Channel class
class UDPChannel
{
public:
    UDPChannel(io_service &ioService, uint16_t port)
        : socket_(ioService, udp::endpoint(udp::v4(), port)) {}

    void startReceive(function<void(const string &)> callback)
    {
        callback_ = callback;
        receiveNext();
    }

    void send(const string &message, const udp::endpoint &remoteEndpoint)
    {
        socket_.async_send_to(
            buffer(message + "\n", message.length() + 1), remoteEndpoint,
            [](const boost::system::error_code &error, std::size_t bytesTransferred)
            {
                if (error)
                {
                    std::cerr << "Send error: " << error.message() << std::endl;
                }
            });
    }

    void send(const char *message, size_t length, const udp::endpoint &remoteEndpoint)
    {
        socket_.async_send_to(
            buffer(message, length), remoteEndpoint,
            [](const boost::system::error_code &error, std::size_t bytesTransferred)
            {
                if (error)
                {
                    std::cerr << "Send error: " << error.message() << std::endl;
                }
            });
    }

private:
    void receiveNext()
    {
        socket_.async_receive_from(
            buffer(temp_buffer_), remoteEndpoint_,
            std::bind(&UDPChannel::handleReceive, this, std::placeholders::_1, std::placeholders::_2));
    }

    void handleReceive(const boost::system::error_code &error, std::size_t bytesTransferred)
    {
        if (!error)
        {
            string message(temp_buffer_.data(), bytesTransferred);
            callback_(message);
            receiveNext(); // Start receiving the next message
        }
        else
        {
            std::cerr << "Receive error: " << error.message() << std::endl;
        }
    }

    udp::socket socket_;
    udp::endpoint remoteEndpoint_;
    array<char, 4096> temp_buffer_;
    function<void(const string &)> callback_;
};

// UDP Publisher
class UDPPublisher
{
public:
    UDPPublisher(io_service &ioService, uint16_t localPort, const udp::endpoint &remoteEndpoint)
        : channel_(ioService, localPort)
    {
        remoteEndpoint_ = remoteEndpoint;
    }

    void publish(const string &message)
    {
        channel_.send(message, remoteEndpoint_);
    }

    void publish(const char *message, size_t length)
    {
        channel_.send(message, length, remoteEndpoint_);
    }

private:
    UDPChannel channel_;
    udp::endpoint remoteEndpoint_;
};

// UDP Receiver
class UDPReceiver
{
public:
    UDPReceiver(io_service &ioService, uint16_t port)
        : channel_(std::make_unique<UDPChannel>(ioService, port)) {}

    void startReceiving()
    {
        channel_->startReceive(handleMessage);
    }

private:
    static void handleMessage(const string &message)
    {
        std::cout << "Received message: " << message << std::endl;
    }

    std::unique_ptr<UDPChannel> channel_;
};

int main()
{
    io_service ioService;

    // Create a publisher and a receiver
    UDPPublisher publisher(ioService, 0, udp::endpoint(ip::address::from_string("127.0.0.1"), 8000));
    UDPReceiver receiver(ioService, 8000);

    // Start receiving messages
    receiver.startReceiving();

    // Test sending and receiving messages
    publisher.publish("Hello");
    publisher.publish("World");
    publisher.publish("Test123", 7);

    // Run the io_service in a separate thread
    std::thread ioThread([&ioService]()
                         { ioService.run(); });

    // Wait for the user to press Enter
    std::cout << "Press Enter to exit..." << std::endl;
    std::cin.get();

    // Stop the io_service and wait for the thread to finish
    ioService.stop();
    ioThread.join();

    return 0;
}

// g++ -std=c++20 -o udp_example udpchannel.cpp -lboost_system