#include "mydb.hpp"

#include <iostream>
#include <string>

int main() {
    std::string input;
    Table* table = db_open("mydb.db");
    Statement statement;

    while (true) {
        std::cout << "mydb> ";
        std::getline(std::cin, input);

        if (metacommand(input)) {
            if (meta_command(input) == META_EXIT) {
                db_close(table);
                break;
            }
            continue;
        }

        PrepareResult prepare_result = prepare_statement(input, statement);
        if (prepare_result == PREPARE_UNRECOGNISED_STATEMENT) {
            std::cout << "Unrecognised keyword at start of '" << input << "'.\n";
            continue;
        }
        if (prepare_result == PREPARE_SYNTAX_ERROR) {
            continue;
        }

        ExecuteResult execute_result = execute_statement(statement, *table);
        if (execute_result == EXECUTE_TABLE_FULL) {
            std::cout << "Error: Table full.\n";
            continue;
        }

        std::cout << "Executed.\n";
    }

    return 0;
}
