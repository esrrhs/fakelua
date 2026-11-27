#pragma once

#include <openssl/err.h>
#include <openssl/ssl.h>
#include <openssl/x509v3.h>
#include <string>

#if defined(_WIN32)
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#endif

namespace fakelua::tls {

inline SSL_CTX *MakeClientContext(bool verify, const std::string &ca) {
    SSL_CTX *ctx = SSL_CTX_new(TLS_client_method());
    if (!ctx) return nullptr;
    SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3);
    if (verify) {
        SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, nullptr);
        if (!ca.empty()) {
            if (SSL_CTX_load_verify_locations(ctx, ca.c_str(), nullptr) != 1) {
                SSL_CTX_free(ctx);
                return nullptr;
            }
        } else {
            if (SSL_CTX_set_default_verify_paths(ctx) != 1) {
                SSL_CTX_free(ctx);
                return nullptr;
            }
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

inline bool IsIpLiteral(const std::string &host) {
    if (host.empty()) return false;
    unsigned char buf[16]{};
    return inet_pton(AF_INET, host.c_str(), buf) == 1 || inet_pton(AF_INET6, host.c_str(), buf) == 1;
}

inline bool SetSniHostname(SSL *ssl, const std::string &host) {
    if (!ssl) return false;
    const std::string clean = HostnameWithoutPort(host);
    if (clean.empty() || IsIpLiteral(clean)) return true;
    return SSL_set_tlsext_host_name(ssl, clean.c_str()) == 1;
}

inline bool ConfigurePeerVerification(SSL *ssl, const std::string &host, bool verify) {
    if (!ssl) return false;
    const std::string clean = HostnameWithoutPort(host);
    if (!SetSniHostname(ssl, clean)) return false;
    if (!verify) return true;
    if (clean.empty()) return false;
    if (IsIpLiteral(clean)) {
        X509_VERIFY_PARAM *param = SSL_get0_param(ssl);
        return param != nullptr && X509_VERIFY_PARAM_set1_ip_asc(param, clean.c_str()) == 1;
    }
    SSL_set_hostflags(ssl, X509_CHECK_FLAG_NO_PARTIAL_WILDCARDS);
    return SSL_set1_host(ssl, clean.c_str()) == 1;
}

}// namespace fakelua::tls
