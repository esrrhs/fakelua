#pragma once

namespace fakelua {

// Shared by transient JIT workspaces and the default filesystem cache.
std::string GetTmpDirectory();

std::string GenerateTmpFilename(const std::string &head, const std::string &tail);

}// namespace fakelua
