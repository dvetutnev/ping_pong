#include <iostream>
#include <iomanip>
#include <ctime>
#include <sstream>
#include <algorithm>

#include <docopt.h>
#include <uvw/loop.hpp>
#include <uvw/udp.hpp>
#include <uvw/timer.hpp>

enum class Mode { Responder, Requester };
std::ostream& operator<< (std::ostream&, Mode);

struct ProgramOptions
{
    Mode mode;
    std::string bind_ip;
    unsigned short bind_port;
    std::string remote_ip;
    unsigned short remote_port;
    unsigned short repeat;
};
std::ostream& operator<< (std::ostream&, const ProgramOptions&);
const ProgramOptions parse_program_options(int, char* []);

std::string get_time_pretty();

int main(int argc, char* argv[])
{
    const auto options = parse_program_options(argc, argv);
    std::cout << std::endl << options << std::endl;

    auto loop = uvw::Loop::getDefault();
    auto socket = loop->resource<uvw::UDPHandle>();
    auto timer = loop->resource<uvw::TimerHandle>();

    auto on_error = [](const auto& err, const auto&)
    {
        std::cout << "libuv error! name: " << err.name() << " what: " << err.what() << std::endl;
        std::abort();
    };
    socket->on<uvw::ErrorEvent>(on_error);
    timer->on<uvw::ErrorEvent>(on_error);

    auto on_timer_requester = [socket, &options](const auto&, const auto&)
    {
        const char message[] = "ping";
        std::cout << get_time_pretty() << " Send request <" << message << ">" << std::endl;

        auto data = std::make_unique<char[]>( sizeof(message) );
        std::copy_n( message, sizeof(message), data.get() );
        socket->send( options.remote_ip, options.remote_port, std::move(data), sizeof(message) );
    };

    auto on_data_responder = [](const auto& event, auto& soc)
    {
        std::cout << get_time_pretty() << " Received from " << event.sender.ip << ":" << event.sender.port << ", data: " << std::string{event.data.get(), event.length} << std::endl;

        const char message[] = "pong";
        std::cout << get_time_pretty() << " Send respond <" << message << ">" << std::endl;

        auto data = std::make_unique<char[]>( sizeof(message) );
        std::copy_n( message, sizeof(message), data.get() );
        soc.send( event.sender.ip, event.sender.port, std::move(data), sizeof(message) );
    };



    socket->bind(options.bind_ip, options.bind_port);

    switch (options.mode)
    {
    case Mode::Requester:
        timer->on<uvw::TimerEvent>(on_timer_requester);
        timer->start(std::chrono::milliseconds{42}, std::chrono::seconds{options.repeat});
        break;

    case Mode::Responder:
        socket->on<uvw::UDPDataEvent>(on_data_responder);
        socket->recv();
        break;
    }

    loop->run();

    socket->clear();
    socket->close();
    timer->clear();
    timer->close();

    return 0;
}

static const std::string usage =
R"(Ping-Pong

        Usage:
         ping_pong responder [--bind_ip <bind ip>] --bind_port <bind port>
         ping_pong requester [--bind_ip <bind ip>] --bind_port <bind port> --remote_ip <remote ip> --remote_port <remote port> [--repeat <seconds>]
         ping_pong (-h | --help)

        Options:
         -h --help                      Show this message
         --bind_port <bind port>        Number port for pending data
         --bind_ip <bind ip>            Address for pending data [default: 0.0.0.0]
         --remote_ip <remote ip>        Address for send request
         --remote_port <remote port>    Number port for send request
         --repeat <seconds>             Repeat time, in seconds [default: 5]
)";

const ProgramOptions parse_program_options(int argc, char* argv[])
{
    auto opt = docopt::docopt(usage, {argv + 1, argv + argc} );

    Mode mode = ( opt["responder"].asBool() ) ? Mode::Responder : Mode::Requester;
    long bind_port = opt["--bind_port"].asLong();
    std::string remote_ip = (opt["--remote_ip"]) ? opt["--remote_ip"].asString() : "";
    long remote_port = (opt["--remote_port"]) ? opt["--remote_port"].asLong() : 0;
    long repeat = (mode == Mode::Requester) ? opt["--repeat"].asLong() : 0;

    using ushort = unsigned short;
    return ProgramOptions{mode, opt["--bind_ip"].asString(), static_cast<ushort>(bind_port), std::move(remote_ip), static_cast<ushort>(remote_port), static_cast<ushort>(repeat)};
}

std::ostream& operator<< (std::ostream& os, Mode m)
{
    switch (m)
    {
    case Mode::Responder:
        os << "responder"; break;
    case Mode::Requester:
        os << "requester"; break;
    }
    return os;
}

std::ostream& operator<< (std::ostream& os, const ProgramOptions& po)
{
    os << "Mode: " << po.mode << std::endl;
    os << "Bind IP: " << po.bind_ip << std::endl;
    os << "Bind port: " << po.bind_port << std::endl;
    os << "Remote IP: " << po.remote_ip << std::endl;
    os << "Remote port: " << po.remote_port << std::endl;
    os << "Repeat time: " << po.repeat << std::endl;
    return os;
}

std::string get_time_pretty()
{
    std::time_t t = std::time(NULL);
    std::tm tm = *( std::localtime(&t) );
    std::ostringstream ss;
    ss << std::put_time(&tm, "[ %H:%M:%S ]");
    return ss.str();
}
