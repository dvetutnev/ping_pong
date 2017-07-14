#include <iostream>
#include <iomanip>
#include <ctime>
#include <algorithm>

#include <docopt.h>
#include <uvw.hpp>

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

int main(int argc, char* argv[])
{
    const auto options = parse_program_options(argc, argv);
    std::cout << std::endl << options << std::endl;

    auto loop = uvw::Loop::getDefault();
    auto timer = loop->resource<uvw::TimerHandle>();
    auto socket = loop->resource<uvw::UDPHandle>();

    auto error_handler = [](auto& err, auto&)
    {
        std::cout << "libuv error! name: " << err.name() << " what: " << err.what() << std::endl;
        std::abort();
    };
    timer->on<uvw::ErrorEvent>(error_handler);
    socket->on<uvw::ErrorEvent>(error_handler);

    if (options.mode == Mode::Requester)
    {
        socket->bind(options.bind_ip, options.bind_port);

        auto timer_handler = [socket, &options](auto&, auto&)
        {
            std::time_t t =  std::time(NULL);
            std::tm tm    = *std::localtime(&t);
            std::cout << std::put_time(&tm, "[ %H:%M:%S ] Send ping") << std::endl;

            const char message[] = "ping";
            auto data = std::make_unique<char[]>( sizeof(message) );
            std::copy_n( message, sizeof(message), data.get() );

            socket->send( options.remote_ip, options.remote_port, std::move(data), sizeof(message) );
        };
        timer->on<uvw::TimerEvent>(timer_handler);
        timer->start(std::chrono::seconds{options.repeat}, std::chrono::seconds{options.repeat});
    }

    loop->run();

    timer->clear();
    timer->close();
    socket->clear();
    socket->close();

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
