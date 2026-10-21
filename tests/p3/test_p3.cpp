// tests/p3/test_p3.cpp — YOUR P3 TEST SUITE.
//
// The five tests below are worked examples. Keep them, and add your own
// until you have AT LEAST 15 (spec §6 lists what must be covered).
// Name every test <area>_<what>: reg_ for the registry, ws_ for the
// workspace, calc_ for the calculator.
//
// Build & run:   cmake --build build && ./build/test_p3
// One area:      ./build/test_p3 reg_        (names starting with "reg_")
// One test:      ./build/test_p3 =reg_add_and_find

#include "../mini_test.h"
#include "../test_helpers.h"     // TempDir, write_text, args1, ...

#include "tools/calculator.h"
#include "tools/tool_registry.h"
#include "tools/workspace.h"

#include <cstdio>
#include <memory>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace {

// A tool whose name you choose — use it to fill the registry in tests.
class NamedTool : public Tool {
public:
    explicit NamedTool(std::string n) : n_(std::move(n)), schema_(JsonValue::object()) {}
    std::string name() const override { return n_; }
    std::string description() const override { return "test tool"; }
    const JsonValue& schema() const override { return schema_; }
    ToolResult invoke(const JsonValue&, ToolContext&) override { return ToolResult::success(n_); }
private:
    std::string n_;
    JsonValue   schema_;   // an empty JSON object: {}
};

// Tokenize with the provided tokenizer, then run YOUR calc_evaluate.
bool eval(const std::string& expr, double* value, std::string* error) {
    std::vector<CalcToken> tokens;
    if (!calc_tokenize(expr, &tokens, error)){
        return false;
    }
    return calc_evaluate(tokens, value, error);
}

}  // namespace

// ---------------------------------------------------------------- examples

TEST(reg_add_and_find) {
    ToolRegistry r;
    auto t = std::make_unique<NamedTool>("tool_1");
    Tool* raw = t.get();                        // remember the address before moving
    REQUIRE(r.add(std::move(t)));
    CHECK(r.size() == 1);
    CHECK(r.find("tool_1") == raw);              // same object, not a copy
    CHECK(r.find("tool_2") == nullptr);
}

TEST(reg_duplicate_keeps_original) {
    ToolRegistry r;
    auto first = std::make_unique<NamedTool>("tool_1");
    Tool* original = first.get();
    REQUIRE(r.add(std::move(first)));
    CHECK(!r.add(std::make_unique<NamedTool>("tool_1")));
    CHECK(r.find("tool_1") == original);
    CHECK(r.size() == 1);
}

TEST(ws_rejects_parent_escape) {
    TempDir d;
    Workspace ws(d.path());
    fs::path out;
    std::string reason;
    REQUIRE(ws.resolve("notes/todo.txt", &out, &reason));   // sanity: accepts *something*
    CHECK(out == ws.root() / "notes" / "todo.txt");
    CHECK(!ws.resolve("../etc/passwd", &out, &reason));
    CHECK(!reason.empty());
}

TEST(calc_precedence) {
    double v = 0;
    std::string err;
    REQUIRE(eval("2 + 3 * 4", &v, &err));
    CHECK(v == 14);
}

TEST(calc_division_by_zero) {
    double v = 0;
    std::string err;
    CHECK(!eval("1 / 0", &v, &err));
    CHECK(err == "division by zero");
}

// ------------------------------------------------------------- your tests
//
// Still to write (spec §6 has the full list). Suggested names:
//   reg_null_rejected, reg_for_each_visits_each_once
//   reg_10000_tools               REQUIRED: print bucket_count / load_factor /
//                                 max_probe_length, check the bound
//   reg_alternative_hash          REQUIRED: longest chain of a weak hash
//   ws_<case>                     REQUIRED: every row of the table in spec §4.4
//   calc_<case>                   left-to-right, parentheses, negatives

TEST(reg_null_rejected) {
    ToolRegistry r;
    CHECK(!r.add(nullptr));                     
    CHECK(r.size() == 0);
    REQUIRE(r.add(std::make_unique<NamedTool>("tool_1")));
    CHECK(!r.add(nullptr));                     
    CHECK(r.size() == 1);
}

TEST(reg_find_absent_name) {
    ToolRegistry r;
    CHECK(r.find("tool_1") == nullptr);          
    CHECK(r.size() == 0);
    REQUIRE(r.add(std::make_unique<NamedTool>("tool_1")));
    CHECK(r.find("tool_2") == nullptr);          
    CHECK(r.size() == 1);                       
}

TEST(reg_for_each_visits_each_once) {
    ToolRegistry r;
    const std::vector<std::string> names = {"tool_1", "tool_2", "tool_3", "tool_4"};
    for (const auto& n : names) {
        REQUIRE(r.add(std::make_unique<NamedTool>(n)));
    }

    std::vector<std::string> visited;
    r.for_each([&](const Tool& t) { visited.push_back(t.name()); });

    CHECK(visited.size() == names.size());     
    for (const auto& n : names) {
        int seen = 0;
        for (const auto& v : visited) {
            if (v == n){
                ++seen;
            }
        }
        CHECK(seen == 1);                      
    }
}

TEST(reg_10000_tools) {
    ToolRegistry r;
    Tool* first = nullptr;
    bool load_ok = true;
    for (int i = 0; i < 10000; ++i) {
        auto t = std::make_unique<NamedTool>("tool_" + std::to_string(i));
        if (i == 0){
            first = t.get();  
        }
        REQUIRE(r.add(std::move(t)));
        if (r.load_factor() > 1.0){
            load_ok = false;
        }
    }
    CHECK(load_ok);                             
    CHECK(r.size() == 10000);

    bool all_found = true;
    for (int i = 0; i < 10000; ++i) {
        if (r.find("tool_" + std::to_string(i)) == nullptr){
            all_found = false;
        }
    }
    CHECK(all_found);                           
    CHECK(r.find("tool_0") == first);           
    CHECK(r.max_probe_length() <= 64);

    std::printf("reg_10000_tools: bucket_count=%zu load_factor=%f max_probe_length=%zu\n",
                r.bucket_count(), r.load_factor(), r.max_probe_length());
}

TEST(reg_alternative_hash) {
    ToolRegistry r;
    for (int i = 0; i < 10000; ++i) {
        REQUIRE(r.add(std::make_unique<NamedTool>("tool_" + std::to_string(i))));
    }

    std::vector<int> counts(r.bucket_count(), 0);
    int weak_longest = 0;
    for (int i = 0; i < 10000; ++i) {
        const std::string s = "tool_" + std::to_string(i);
        const std::size_t h = static_cast<std::size_t>(s[0]) + s.size();
        const int c = ++counts[h % counts.size()];
        if (c > weak_longest) weak_longest = c;
    }

    std::printf("reg_alternative_hash: bucket_count=%zu  real max_probe_length=%zu  "
                "weak hash (s[0] + s.size()) longest chain=%d\n",
                r.bucket_count(), r.max_probe_length(), weak_longest);
    CHECK(r.max_probe_length() < static_cast<std::size_t>(weak_longest));
}

TEST(ws_normalizes_dot_and_slashes) {
    TempDir d;
    Workspace ws(d.path());
    fs::path out;
    std::string reason;
    REQUIRE(ws.resolve("./notes//todo.txt", &out, &reason));
    CHECK(out == ws.root() / "notes" / "todo.txt");
}

TEST(ws_dot_and_dotdot_pairs) {
    TempDir d;
    Workspace ws(d.path());
    fs::path out;
    std::string reason;
    REQUIRE(ws.resolve(".", &out, &reason));
    CHECK(out == ws.root());                    
    REQUIRE(ws.resolve("a/..", &out, &reason));
    CHECK(out == ws.root());                    
    REQUIRE(ws.resolve("a/../b", &out, &reason));
    CHECK(out == ws.root() / "b");
}

TEST(ws_accepts_nonexistent_path) {
    TempDir d;
    Workspace ws(d.path());
    fs::path out;
    std::string reason;
    REQUIRE(ws.resolve("new/dir/file.txt", &out, &reason));
    CHECK(out == ws.root() / "new" / "dir" / "file.txt");
    CHECK(!fs::exists(ws.root() / "new"));
}

TEST(ws_accepts_dots_in_name) {
    TempDir d;
    Workspace ws(d.path());
    fs::path out;
    std::string reason;
    REQUIRE(ws.resolve("my..notes.txt", &out, &reason));
    CHECK(out == ws.root() / "my..notes.txt"); 
}

TEST(ws_rejects_empty) {
    TempDir d;
    Workspace ws(d.path());
    fs::path out;
    std::string reason;
    CHECK(!ws.resolve("", &out, &reason));
    CHECK(reason == "empty path");
}

TEST(ws_rejects_nul_byte) {
    TempDir d;
    Workspace ws(d.path());
    fs::path out;
    std::string reason;
    const std::string with_nul = std::string("notes") + '\0' + "/x";
    CHECK(!ws.resolve(with_nul, &out, &reason));
    CHECK(!reason.empty());
}

TEST(ws_rejects_absolute) {
    TempDir d;
    Workspace ws(d.path());
    fs::path out;
    std::string reason;
    CHECK(!ws.resolve("/etc/passwd", &out, &reason));
    CHECK(!reason.empty());
}

TEST(ws_rejects_dotdot) {
    TempDir d;
    Workspace ws(d.path());
    fs::path out;
    std::string reason;
    CHECK(!ws.resolve("..", &out, &reason));
    CHECK(!reason.empty());
    reason.clear();
    CHECK(!ws.resolve("a/../../b", &out, &reason));
    CHECK(!reason.empty());
}

TEST(ws_rejects_escape_that_returns_inside) {
    TempDir d;
    fs::create_directories(d.path() / "ws");
    Workspace ws(d.path() / "ws");
    REQUIRE(ws.root().filename() == "ws");
    fs::path out;
    std::string reason;
    CHECK(!ws.resolve("notes/../../ws/notes/todo.txt", &out, &reason));
    CHECK(!reason.empty());
}

TEST(calc_left_to_right) {
    double v = 0;
    std::string err;
    REQUIRE(eval("10 - 4 - 3", &v, &err));
    CHECK(v == 3);
    REQUIRE(eval("100 / 10 / 5", &v, &err));
    CHECK(v == 2);
}

TEST(calc_parentheses) {
    double v = 0;
    std::string err;
    REQUIRE(eval("(2 + 3) * 4", &v, &err));
    CHECK(v == 20);
    REQUIRE(eval("2 * (3 + 4) - 5", &v, &err));
    CHECK(v == 9);
}

TEST(calc_negative_numbers) {
    double v = 0;
    std::string err;
    REQUIRE(eval("-3 * -3", &v, &err));
    CHECK(v == 9);
}

TEST(calc_fractional_result) {
    double v = 0;
    std::string err;
    REQUIRE(eval("7 / 2", &v, &err));
    CHECK(v == 3.5);
}

TEST(calc_division_by_zero_in_parentheses) {
    double v = 0;
    std::string err;
    CHECK(!eval("1 / (2 - 2)", &v, &err));
    CHECK(err == "division by zero");
}

TEST(calc_custom_expressions) {
    double v = 0;
    std::string err;
    REQUIRE(eval("2 * 3 + 4 * 5", &v, &err));
    CHECK(v == 26);
    REQUIRE(eval("8 / 2 * 4", &v, &err));
    CHECK(v == 16);
    REQUIRE(eval("10 - (4 - 3)", &v, &err));
    CHECK(v == 9);
}

int main(int argc, char** argv) { return mt::run_all(argc, argv); }
