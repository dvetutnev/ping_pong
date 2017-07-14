#include <iostream>
#include <sstream>
#include <algorithm>

#include <docopt.h>
#include <uvw/loop.hpp>
#include <uvw/udp.hpp>
#include <uvw/timer.hpp>
#include <date.h>
#include <tz.h>

enum class Mode { Responder, Requester };
std::ostream& operator<< (std::ostream&, Mode);

struct ProgramOptions
{
    Mode mode;
    std::string bind_ip;
    unsigned short bind_port;
    std::string response;
    std::string remote_ip;
    unsigned short remote_port;
    std::string request;
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
        std::cout << get_time_pretty() << " Send request <" << options.request << ">" << std::endl;
        auto data = std::make_unique<char[]>( options.request.size() );
        std::copy_n( options.request.c_str(), options.request.size(), data.get() );
        socket->send( options.remote_ip, options.remote_port, std::move(data), static_cast<unsigned int>(options.request.size()) );
    };

    auto on_data_requester = [](const auto& event, const auto&)
    {
        std::cout << get_time_pretty() << " Received from " << event.sender.ip << ":" << event.sender.port << ", data: " << std::string{event.data.get(), event.length} << std::endl;
    };

    auto on_data_responder = [&options](const auto& event, auto& soc)
    {
        std::cout << get_time_pretty() << " Received from " << event.sender.ip << ":" << event.sender.port << ", data: " << std::string{event.data.get(), event.length} << std::endl;
        std::cout << get_time_pretty() << " Send respond <" << options.response << ">" << std::endl;
        auto data = std::make_unique<char[]>( options.response.size() );
        std::copy_n( options.response.c_str(), options.response.size(), data.get() );
        soc.send( event.sender.ip, event.sender.port, std::move(data), static_cast<unsigned int>(options.response.size()) );
    };



    socket->bind(options.bind_ip, options.bind_port);

    switch (options.mode)
    {
    case Mode::Requester:
        socket->on<uvw::UDPDataEvent>(on_data_requester);
        socket->recv();
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
         ping_pong responder [--bind_ip <bind ip>] --bind_port <bind port> [--response <pong>]
         ping_pong requester [--bind_ip <bind ip>] --bind_port <bind port> --remote_ip <remote ip> --remote_port <remote port> [--request <ping>] [--repeat <seconds>]
         ping_pong (-h | --help)

        Options:
         -h --help                      Show this message
         --bind_ip <bind ip>            Address for pending data [default: 0.0.0.0]
         --bind_port <bind port>        Number port for pending data
         --response <pong>              Response message [default: pong]
         --remote_ip <remote ip>        Address for send request
         --remote_port <remote port>    Number port for send request
         --request <ping>               Request message [default: ping]
         --repeat <seconds>             Repeat time, in seconds [default: 5]
)";

const ProgramOptions parse_program_options(int argc, char* argv[])
{
    auto opt = docopt::docopt(usage, {argv + 1, argv + argc} );

    Mode mode = ( opt["responder"].asBool() ) ? Mode::Responder : Mode::Requester;

    long bind_port = opt["--bind_port"].asLong();
    std::string response = (mode == Mode::Responder) ? opt["--response"].asString() : "";

    std::string remote_ip = (opt["--remote_ip"]) ? opt["--remote_ip"].asString() : "";
    long remote_port = (opt["--remote_port"]) ? opt["--remote_port"].asLong() : 0;
    std::string request = (mode == Mode::Requester) ? opt["--request"].asString() : "";
    long repeat = (mode == Mode::Requester) ? opt["--repeat"].asLong() : 0;

    using ushort = unsigned short;
    return ProgramOptions{mode,
                opt["--bind_ip"].asString(), static_cast<ushort>(bind_port), response,
                std::move(remote_ip), static_cast<ushort>(remote_port), request, static_cast<ushort>(repeat)
    };
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
    os << "Response message: " << po.response << std::endl;
    os << "Remote IP: " << po.remote_ip << std::endl;
    os << "Remote port: " << po.remote_port << std::endl;
    os << "Request message: " << po.request << std::endl;
    os << "Repeat time: " << po.repeat << std::endl;
    return os;
}

std::string get_time_pretty()
{
    using namespace date;
    using namespace std::chrono;
    auto now = make_zoned( current_zone(), system_clock::now() );
    std::ostringstream ss;
    ss << "[ " << now << " ]";
    return ss.str();
}
