#include "mydb.hpp"

#include <cstring>
#include <iostream>
#include <sstream>

bool metacommand(const std::string& input) {
    return !input.empty() && input[0] == '.';
}

MetaCommandResult meta_command(const std::string& input) {
    if (input == ".help") {
        std::cout << "Available commands:\n  .exit  Exit mydb\n  .help  Show this help message\n";
        return META_SUCCESS;
    }
    if (input == ".exit") {
        return META_EXIT;
    }

    std::cout << "Unknown meta command: " << input << "\n";
    return META_UNRECOGNIZED;
}

PrepareResult prepare_statement(const std::string& input, Statement& statement) {
    if (input.rfind("insert", 0) == 0) {
        statement.type = STATEMENT_INSERT;

        std::istringstream stream(input);
        std::string keyword;
        std::string username;
        std::string email;
        if (!(stream >> keyword >> statement.row_to_insert.id >> username >> email)) {
            std::cout << "Syntax error. Usage: insert <id> <username> <email>\n";
            return PREPARE_SYNTAX_ERROR;
        }
        if (username.size() > COLUMN_USERNAME_SIZE || email.size() > COLUMN_EMAIL_SIZE) {
            std::cout << "String is too long.\n";
            return PREPARE_SYNTAX_ERROR;
        }

        std::strncpy(statement.row_to_insert.username, username.c_str(), COLUMN_USERNAME_SIZE);
        statement.row_to_insert.username[COLUMN_USERNAME_SIZE] = '\0';
        std::strncpy(statement.row_to_insert.email, email.c_str(), COLUMN_EMAIL_SIZE);
        statement.row_to_insert.email[COLUMN_EMAIL_SIZE] = '\0';
        return PREPARE_SUCCESS;
    }

    if (input.rfind("select", 0) == 0) {
        statement.type = STATEMENT_SELECT;
        return PREPARE_SUCCESS;
    }

    return PREPARE_UNRECOGNISED_STATEMENT;
}
