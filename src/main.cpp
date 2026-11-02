#include "lexer.hpp"
#include "node.hpp"
#include "parser.hpp"

#include <cerrno>
#include <filesystem>
#include <fstream>
#include <print>
#include <sstream>

auto main(int argc, char **argv) -> int {
    if (argc != 2) {
        std::println(stderr, "Usage: {} FILE", argv[0]);
        return 1;
    }

    std::filesystem::path file_path = argv[1];
    std::ifstream file_stream = file_path;

    if (!file_stream.is_open() && errno == ENOENT) {
        std::println(stderr, "Error: {} does not exist.", argv[1]);
        return 2;
    }
    else if (!file_stream.is_open() && errno == EACCES) {
        std::println(stderr, "Error: {} does not have reading permission.", argv[1]);
        return 3;
    }
    else if (!file_stream.is_open()) {
        std::println(stderr, "Error: Failed to read {}", argv[1]);
        return 4;
    }

    std::stringstream buffer;
    buffer << file_stream.rdbuf();
    std::string file_content = buffer.str();

    Lexer lexer = Lexer(file_content);
    Parser parser = Parser(lexer);

    Prog::Ptr program = parser.parse();
    if (!program && lexer.has_error) {
        println("{}", lexer.error);

        return 5;
    }
    else if (!program) {
        std::println("{}", parser.error);

        return 6;
    }
    else {
        program->print();
    }

    return 0;
}