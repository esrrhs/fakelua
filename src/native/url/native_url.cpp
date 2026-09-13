#include "native/url/native_url.h"

#include "native/native_common.h"
#include "native/table/native_table.h"
#include "var/var.h"

#include <boost/url.hpp>

#include <string>
#include <string_view>

namespace fakelua::url {

namespace urls = boost::urls;

static std::string CVarToString(CVar v) {
    return inter::FakeluaToNativeString(nullptr, v);
}

static CVar TableGetStr(State *s, CVar tbl, const char *key) {
    return table::TableHelper::GetTableStrId(s, tbl, key);
}

static void SetStr(State *s, CVar tbl, const char *key, std::string_view val) {
    table::TableHelper::SetTableStrId(s, tbl, key, inter::NativeToFakeluaString(s, std::string(val)));
}

static std::string EncodeView(std::string_view s) {
    return urls::encode(s, urls::unreserved_chars);
}

static std::string DecodeView(std::string_view s) {
    auto pct = urls::make_pct_string_view(s);
    if (pct.has_error()) {
        ThrowFakeluaException(std::format("url.decode: {}", pct.error().message()));
    }
    urls::encoding_opts opt;
    opt.space_as_plus = true;
    return pct->decode(opt);
}

static CVar ParamsToTable(State *s, urls::params_view params) {
    CVar tbl = table::TableHelper::CreateTable(s);
    for (auto p: params) {
        std::string key(p.key);
        if (!p.has_value) {
            SetStr(s, tbl, key.c_str(), "");
            continue;
        }
        SetStr(s, tbl, key.c_str(), p.value);
    }
    return tbl;
}

static CVar UrlParse(State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "url.parse", "url string expected");
    CVar a0 = inter::GetNativeArg(s, args, n, 0);
    CheckStringArg(a0, 1, "url.parse");
    std::string str = CVarToString(a0);

    auto parsed = urls::parse_uri_reference(str);
    if (parsed.has_error()) {
        ThrowFakeluaException(std::format("url.parse: {}", parsed.error().message()));
    }
    urls::url_view u = *parsed;

    CVar tbl = table::TableHelper::CreateTable(s);
    if (u.has_scheme()) SetStr(s, tbl, "scheme", u.scheme());
    if (u.has_userinfo()) SetStr(s, tbl, "user", u.user());
    if (u.has_password()) SetStr(s, tbl, "password", u.password());
    if (u.has_authority()) SetStr(s, tbl, "host", u.host());
    if (u.has_port()) {
        table::TableHelper::SetTableStrId(s, tbl, "port", inter::NativeToFakeluaInt(s, static_cast<int64_t>(u.port_number())));
        SetStr(s, tbl, "port_str", u.port());
    }
    SetStr(s, tbl, "path", u.path());
    if (u.has_query()) {
        SetStr(s, tbl, "query", u.query());
        table::TableHelper::SetTableStrId(s, tbl, "params", ParamsToTable(s, u.params()));
    }
    if (u.has_fragment()) SetStr(s, tbl, "fragment", u.fragment());
    SetStr(s, tbl, "href", u.buffer());
    return tbl;
}

static CVar UrlFormat(State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "url.format", "table expected");
    CVar a0 = inter::GetNativeArg(s, args, n, 0);
    if (a0.type_ != static_cast<int>(VarType::Table) || !a0.data_.t) {
        ThrowBadArgument(1, "url.format", "table expected");
    }

    urls::url u;
    CVar scheme = TableGetStr(s, a0, "scheme");
    if (scheme.type_ != static_cast<int>(VarType::Nil)) {
        u.set_scheme(CVarToString(scheme));
    }
    CVar user = TableGetStr(s, a0, "user");
    if (user.type_ != static_cast<int>(VarType::Nil)) {
        u.set_user(CVarToString(user));
    }
    CVar password = TableGetStr(s, a0, "password");
    if (password.type_ != static_cast<int>(VarType::Nil)) {
        u.set_password(CVarToString(password));
    }
    CVar host = TableGetStr(s, a0, "host");
    if (host.type_ != static_cast<int>(VarType::Nil)) {
        u.set_host(CVarToString(host));
    }
    CVar port = TableGetStr(s, a0, "port");
    if (port.type_ != static_cast<int>(VarType::Nil)) {
        if (port.type_ == static_cast<int>(VarType::Int) || port.type_ == static_cast<int>(VarType::Float)) {
            u.set_port_number(static_cast<std::uint16_t>(inter::CVarToInteger(port, 0)));
        } else {
            u.set_port(CVarToString(port));
        }
    }
    CVar path = TableGetStr(s, a0, "path");
    if (path.type_ != static_cast<int>(VarType::Nil)) {
        u.set_path(CVarToString(path));
    }
    CVar query = TableGetStr(s, a0, "query");
    if (query.type_ != static_cast<int>(VarType::Nil)) {
        u.set_query(CVarToString(query));
    } else {
        CVar params = TableGetStr(s, a0, "params");
        if (params.type_ == static_cast<int>(VarType::Table) && params.data_.t) {
            auto kvs = table::TableHelper::CollectKVPairs(params);
            auto ref = u.params();
            for (auto &kv: kvs) {
                ref.append({CVarToString(kv.key), CVarToString(kv.val)});
            }
        }
    }
    CVar fragment = TableGetStr(s, a0, "fragment");
    if (fragment.type_ != static_cast<int>(VarType::Nil)) {
        u.set_fragment(CVarToString(fragment));
    }

    return inter::NativeToFakeluaString(s, std::string(u.buffer()));
}

static CVar UrlEncode(State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "url.encode", "string expected");
    CVar a0 = inter::GetNativeArg(s, args, n, 0);
    CheckStringArg(a0, 1, "url.encode");
    return inter::NativeToFakeluaString(s, EncodeView(CVarToString(a0)));
}

static CVar UrlDecode(State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "url.decode", "string expected");
    CVar a0 = inter::GetNativeArg(s, args, n, 0);
    CheckStringArg(a0, 1, "url.decode");
    try {
        return inter::NativeToFakeluaString(s, DecodeView(CVarToString(a0)));
    } catch (const std::exception &e) {
        ThrowFakeluaException(std::format("url.decode: {}", e.what()));
    }
}

static CVar UrlEncodeQuery(State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "url.encode_query", "table expected");
    CVar a0 = inter::GetNativeArg(s, args, n, 0);
    if (a0.type_ != static_cast<int>(VarType::Table) || !a0.data_.t) {
        ThrowBadArgument(1, "url.encode_query", "table expected");
    }
    urls::url u;
    auto ref = u.params();
    auto kvs = table::TableHelper::CollectKVPairs(a0);
    for (auto &kv: kvs) {
        ref.append({CVarToString(kv.key), CVarToString(kv.val)});
    }
    return inter::NativeToFakeluaString(s, std::string(u.encoded_query()));
}

static CVar UrlDecodeQuery(State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "url.decode_query", "string expected");
    CVar a0 = inter::GetNativeArg(s, args, n, 0);
    CheckStringArg(a0, 1, "url.decode_query");
    std::string str = CVarToString(a0);
    auto parsed = urls::parse_query(str);
    if (parsed.has_error()) {
        ThrowFakeluaException(std::format("url.decode_query: {}", parsed.error().message()));
    }
    urls::url u;
    u.set_encoded_query(str);
    return ParamsToTable(s, u.params());
}

void RegisterUrlLibraryApi(State *s) {
    if (!s) return;
    RegisterNativeFunction(s, "url.parse", 1, false, UrlParse);
    RegisterNativeFunction(s, "url.format", 1, false, UrlFormat);
    RegisterNativeFunction(s, "url.encode", 1, false, UrlEncode);
    RegisterNativeFunction(s, "url.decode", 1, false, UrlDecode);
    RegisterNativeFunction(s, "url.encode_query", 1, false, UrlEncodeQuery);
    RegisterNativeFunction(s, "url.decode_query", 1, false, UrlDecodeQuery);
}

}// namespace fakelua::url
