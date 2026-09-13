#pragma once

#include <openssl/err.h>
#include <openssl/ssl.h>
#include <string>

namespace fakelua::tls {

inline SSL_CTX *MakeClientContext(bool verify, const std::string &ca) {
    SSL_CTX *ctx = SSL_CTX_new(TLS_client_method());
    if (!ctx) return nullptr;
    SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3);
    if (verify) {
        SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, nullptr);
        if (!ca.empty()) {
            SSL_CTX_load_verify_locations(ctx, ca.c_str(), nullptr);
        } else {
            SSL_CTX_set_default_verify_paths(ctx);
        }
    } else {
        SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, nullptr);
    }
    return ctx;
}

inline SSL_CTX *MakeServerContext(const std::string &cert, const std::string &key) {
    SSL_CTX *ctx = SSL_CTX_new(TLS_server_method());
    if (!ctx) return nullptr;
    SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3);
    if (SSL_CTX_use_certificate_chain_file(ctx, cert.c_str()) != 1) {
        SSL_CTX_free(ctx);
        return nullptr;
    }
    if (SSL_CTX_use_PrivateKey_file(ctx, key.c_str(), SSL_FILETYPE_PEM) != 1) {
        SSL_CTX_free(ctx);
        return nullptr;
    }
    return ctx;
}

inline bool SetSniHostname(SSL *ssl, const std::string &host) {
    if (!ssl || host.empty()) return true;
    return SSL_set_tlsext_host_name(ssl, host.c_str()) == 1;
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

}// namespace fakelua::tls
