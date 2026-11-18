#include "lexer.hpp"
#include "node.hpp"
#include "parser.hpp"
#include "static_analyzer.hpp"

#include <cerrno>
#include <filesystem>
#include <fstream>
#include <print>
#include <sstream>
#include <string>

auto main(int argc, char **argv) -> int {
    std::filesystem::path file_path;
    bool print_lexer_info = false;
    bool print_parser_info = false;
    bool print_ast = false;

    std::string help_message = std::format("Usage: {} <filename> [options...]\n\n"
                                           "Options:\n"
                                           "    -h, --help        Show this message\n"
                                           "    -l, --lexer       Print lexing information\n"
                                           "    -p, --parser      Print parsing information\n"
                                           "    -a, --ast         Print abstract syntax tree\n",
                                           argv[0]);

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        if (arg.length() == 0) {
            std::println(stderr, "Error: Command argument cannot be empty");

            return 1;
        }
        else if (arg == "--help" || arg == "-h") {
            std::print(stderr, "{}", help_message);

            return 0;
        }
        else if (arg == "--lexer" || arg == "-l") {
            print_lexer_info = true;

            continue;
        }
        else if (arg == "--parser" || arg == "-p") {
            print_parser_info = true;

            continue;
        }
        else if (arg == "--ast" || arg == "-a") {
            print_ast = true;

            continue;
        }
        else if (arg[0] == '-') {
            std::println(stderr, "Error: Unknown option {}", arg);

            return 2;
        }
        else if (!file_path.empty()) {
            std::print(stderr, "{}", help_message);

            return 3;
        }

        file_path = arg;
    }

    std::ifstream file_stream = file_path;

    if (!file_stream.is_open() && errno == ENOENT) {
        std::println(stderr, "Error: {} does not exist.", file_path.string());
        return 4;
    }
    else if (!file_stream.is_open() && errno == EACCES) {
        std::println(stderr, "Error: {} does not have reading permission.", file_path.string());
        return 5;
    }
    else if (!file_stream.is_open()) {
        std::println(stderr, "Error: Failed to read {}", file_path.string());
        return 6;
    }

    std::stringstream buffer;
    buffer << file_stream.rdbuf();
    std::string file_content = buffer.str();

    Lexer lexer = Lexer(file_content, print_lexer_info);
    Parser parser = Parser(lexer, print_parser_info);

    if (print_lexer_info && print_parser_info) {
        std::println("===== Lexer and Parser Info =====");
    }
    else if (print_lexer_info) {
        std::println("========== Lexer Info ===========");
    }
    else if (print_parser_info) {
        std::println("========== Parser Info ==========");
    }

    Prog::Ptr program = parser.parse();

    if (print_lexer_info || print_parser_info) {
        std::println("=================================");
    }

    if (!program && lexer.has_error) {
        if (print_lexer_info || print_parser_info) {
            std::println();
        }

        std::print(stderr, "{}", lexer.error);

        return 7;
    }
    else if (!program) {
        if (print_lexer_info || print_parser_info) {
            std::println();
        }

        std::print(stderr, "{}", parser.error);

        return 8;
    }

    if (print_ast) {
        if (print_lexer_info || print_parser_info) {
            std::println();
        }

        std::println("===== Abstract Syntax Tree ======");
        program->print();
        std::println("=================================");
    }

    StaticAnalyzer static_analyzer = StaticAnalyzer();
    static_analyzer.analyze(program);

    if (static_analyzer.has_error) {
        if (print_lexer_info || print_parser_info || print_ast) {
            std::println();
        }

        std::print(stderr, "{}", static_analyzer.error);

        return 9;
    }

    return 0;
}