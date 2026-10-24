// src/tools/workspace.cpp — YOU IMPLEMENT (spec §4.4).
//
// Workspace::resolve: a purely lexical check, steps 1–6 of the spec.
// Do not touch the filesystem here (no exists(), canonical(), ...).
// The constructor, root() and check_symlinks() are provided in
// workspace_provided.cpp.

#include "tools/workspace.h"

#include <string>

namespace fs = std::filesystem;

bool Workspace::resolve(std::string_view user_path, fs::path* out,
                        std::string* reason) const {
    try {                                                                                                               // try to ensure that absolutely nothing throws
        if (user_path.empty()) {                                                                                        // Step 1: if `user_path` is empty, then reject with reason.
            *reason = "empty path";
            return false;
        }

        if (user_path.find('\0') != std::string_view::npos) {                                                           // Step 2: if `user_path` contains a `'\0'` byte, then reject with reason
            *reason = "path contains a NUL byte";
            return false;
        }

        const fs::path p{std::string(user_path)};                                                                       // Step 3: build `std::filesystem::path p{std::string(user_path)}`. If `p.is_absolute()` or `p.has_root_path()`, then reject
        if (p.is_absolute() || p.has_root_path()) {                                                                     // only workspace-relative paths are allowed, even if an absolute path happens to point inside root.
            *reason = "absolute paths are not allowed; use a path relative to the workspace";
            return false;
        }

        const fs::path n = p.lexically_normal();                                                                        // Step 4: compute `n = p.lexically_normal()`, which removes `.` components and duplicate slashes, and folds `x/..` pairs

        if (*n.begin() == "..") {                                                                                       // Step 5: if the **first component** of `n` is `..` (check `*n.begin() == ".."`), then reject
            *reason = "path escapes the workspace";
            return false;
        }

        if (n == ".") {                                                                                                 // Step 6: if `n` is `.`, set `*out = root()`. Otherwise set `*out = root() / n`
            *out = root();
        } else {
            *out = root() / n;
        }
        return true;                                                                                                    // returns true if the above steps were all completed
    } catch (...) {                                                                                                     // catches anything that would normally throw
        *reason = "invalid path";                                                                                       // reject with reason and return false
        return false;
    }
}
