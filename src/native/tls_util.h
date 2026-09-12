#pragma once

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/system/error_code.hpp>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <string>

namespace fakelua::tls {

namespace ssl = boost::asio::ssl;

inline ssl::context MakeClientContext(bool verify, const std::string &ca) {
    ssl::context ctx{ssl::context::tls_client};
    ctx.set_options(ssl::context::default_workarounds | ssl::context::no_sslv2 | ssl::context::no_sslv3);
    if (verify) {
        ctx.set_verify_mode(ssl::verify_peer);
        if (!ca.empty()) {
            ctx.load_verify_file(ca);
        } else {
            ctx.set_default_verify_paths();
        }
    } else {
        ctx.set_verify_mode(ssl::verify_none);
    }
    return ctx;
}

inline ssl::context MakeServerContext(const std::string &cert, const std::string &key) {
    ssl::context ctx{ssl::context::tls_server};
    ctx.set_options(ssl::context::default_workarounds | ssl::context::no_sslv2 | ssl::context::no_sslv3);
    ctx.use_certificate_chain_file(cert);
    ctx.use_private_key_file(key, ssl::context::pem);
    return ctx;
}

template<class Stream>
inline bool SetSniHostname(Stream &stream, const std::string &host, boost::system::error_code &ec) {
    ec.clear();
    if (host.empty()) return true;
    if (!SSL_set_tlsext_host_name(stream.native_handle(), host.c_str())) {
        ec = boost::system::error_code(static_cast<int>(::ERR_get_error()), boost::asio::error::get_ssl_category());
        return false;
    }
    return true;
}

inline std::string HostnameWithoutPort(std::string host) {
    if (!host.empty() && host.front() == '[') {
        auto rb = host.find(']');
        if (rb != std::string::npos) return host.substr(1, rb - 1);
    }
    auto colon = host.rfind(':');
    if (colon != std::string::npos && host.find(':') == colon) {
        return host.substr(0, colon);
    }
    return host;
}

template<class Socket>
inline void CloseTcpSocket(Socket &sock) {
    // Match net TCP (AsioConn::Close): shutdown+close only. socket.cancel()
    // waits on the Windows select reactor that Poll() drives.
    boost::system::error_code ec;
    sock.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
    sock.close(ec);
}

// Close the TCP socket under a TLS stream. Do not call ssl::stream::shutdown()
// here: it waits for close_notify on the same reactor tick() poll()s, on every
// platform. Windows additionally deadlocks the select reactor.
template<class Stream>
inline void CloseTlsStream(Stream &tls) {
    CloseTcpSocket(tls.next_layer());
}

}// namespace fakelua::tls
