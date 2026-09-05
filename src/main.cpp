#include "Common.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"
#include "Evaluator.hpp"
#include <fstream>
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc > 1) {
        std::string arg1 = argv[1];
        if (arg1 == "--version" || arg1 == "-v" || arg1 == "-V") {
            std::cout << "BY# (BYSharp) version 1.0.0\n";
            std::cout << "Architecture: 64-bit | Standard: ASCII Keyboard & Mandatory Binary Text\n";
            return 0;
        }
        if (arg1 == "--help" || arg1 == "-h") {
            std::cout << "BY# (BYSharp) Interpreter\n";
            std::cout << "Usage:\n";
            std::cout << "  bys <file.by#>      Run script file\n";
            std::cout << "  by# <file.by#>      Run script file (alias)\n";
            std::cout << "  bys -e \"<code>\"     Execute inline BY# code\n";
            std::cout << "  bys                 Launch interactive REPL\n";
            std::cout << "  bys --version       Display version information\n";
            std::cout << "  bys --help          Display this help message\n";
            return 0;
        }
        if (arg1 == "-e") {
            if (argc < 3) {
                std::cerr << "Error: -e expects code string argument\n";
                return 1;
            }
            std::string source = argv[2];
            try {
                Lexer lexer(source);
                auto tokens = lexer.tokenize();
                Parser parser(std::move(tokens));
                auto program = parser.parseProgram();
                Evaluator evaluator;
                evaluator.executeProgram(program);
            } catch (const SyntaxError& e) {
                std::cerr << e.what();
                if (e.line > 0) {
                    std::cerr << " at line " << e.line << ", column " << e.column;
                }
                std::cerr << "\n";
                return 1;
            } catch (const RuntimeError& e) {
                std::cerr << e.what() << "\n";
                return 1;
            } catch (const std::exception& e) {
                std::cerr << "Error: " << e.what() << "\n";
                return 1;
            }
            return 0;
        }

        std::string filePath = argv[1];
        std::ifstream file(filePath);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open file '" << filePath << "'\n";
            return 1;
        }
        std::string source((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();

        try {
            Lexer lexer(source);
            auto tokens = lexer.tokenize();
            Parser parser(std::move(tokens));
            auto program = parser.parseProgram();
            Evaluator evaluator;
            evaluator.executeProgram(program);
        } catch (const SyntaxError& e) {
            std::cerr << e.what();
            if (e.line > 0) {
                std::cerr << " at line " << e.line << ", column " << e.column;
            }
            std::cerr << "\n";
            return 1;
        } catch (const RuntimeError& e) {
            std::cerr << e.what() << "\n";
            return 1;
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << "\n";
            return 1;
        }
        return 0;
    }

    std::cout << "========================================\n";
    std::cout << "  BY# Interactive Shell (v1.0.0)\n";
    std::cout << "  Standard: ASCII Keyboard & Binary Text\n";
    std::cout << "  Type 'exit' to quit.\n";
    std::cout << "========================================\n";

    auto globalEnv = std::make_shared<Environment>();
    Evaluator evaluator(globalEnv);

    std::string lineBuffer;
    int openBrackets = 0;

    while (true) {
        if (openBrackets == 0) {
            std::cout << "BY#> ";
        } else {
            std::cout << "... ";
        }
        std::cout.flush();

        std::string line;
        if (!std::getline(std::cin, line)) {
            break;
        }

        if (openBrackets == 0 && (line == "exit" || line == "exit;" || line == "quit")) {
            break;
        }

        lineBuffer += line + "\n";

        for (size_t i = 0; i < line.size(); ++i) {
            if (line[i] == '[' && (i + 1 >= line.size() || line[i + 1] != ':')) {
                openBrackets++;
            } else if (line[i] == ']' && (i == 0 || line[i - 1] != ':')) {
                if (openBrackets > 0) openBrackets--;
            }
        }

        if (openBrackets > 0) {
            continue;
        }

        std::string currentCode = lineBuffer;
        lineBuffer.clear();

        if (currentCode.find_first_not_of(" \t\r\n") == std::string::npos) {
            continue;
        }

        try {
            Lexer lexer(currentCode);
            auto tokens = lexer.tokenize();
            Parser parser(std::move(tokens));
            auto program = parser.parseProgram();
            evaluator.executeProgram(program);
        } catch (const SyntaxError& e) {
            if (currentCode.find(';') == std::string::npos) {
                try {
                    Lexer retryLexer(currentCode + ";");
                    auto retryTokens = retryLexer.tokenize();
                    Parser retryParser(std::move(retryTokens));
                    auto retryProgram = retryParser.parseProgram();
                    evaluator.executeProgram(retryProgram);
                    continue;
                } catch (...) {}
            }
            std::cerr << e.what();
            if (e.line > 0) {
                std::cerr << " at line " << e.line << ", column " << e.column;
            }
            std::cerr << "\n";
        } catch (const RuntimeError& e) {
            std::cerr << e.what() << "\n";
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << "\n";
        }
    }

    return 0;
}
