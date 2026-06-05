#include<string>
#include<iostream>
#include<sstream>
#include<cstdint>
#include<cstring>

const uint32_t COLUMN_USERNAME_SIZE=32;
const uint32_t COLUMN_EMAIL_SIZE=255;
const uint32_t ID_SIZE=sizeof(uint32_t);
const uint32_t USERNAME_SIZE = COLUMN_USERNAME_SIZE;
const uint32_t EMAIL_SIZE = COLUMN_EMAIL_SIZE;
const uint32_t ID_OFFSET = 0;
const uint32_t USERNAME_OFFSET = ID_OFFSET + ID_SIZE;
const uint32_t EMAIL_OFFSET = USERNAME_OFFSET + USERNAME_SIZE;
const uint32_t ROW_SIZE = ID_SIZE + USERNAME_SIZE + EMAIL_SIZE;
const uint32_t PAGE_SIZE=4096;
const uint32_t ROWS_PER_PAGE=PAGE_SIZE/ROW_SIZE;
const uint32_t TABLE_MAX_PAGES=100;
const uint32_t TABLE_MAX_ROWS=ROWS_PER_PAGE*TABLE_MAX_PAGES;


struct Row {
    uint32_t id;
    char username[COLUMN_USERNAME_SIZE+1];
    char email[COLUMN_EMAIL_SIZE+1];
};

struct Table {
    uint32_t num_rows=0;
    void* pages[TABLE_MAX_PAGES]={};
};

enum StatementType { STATEMENT_INSERT, STATEMENT_SELECT };
enum MetaCommandResult { META_EXIT, META_SUCCESS, META_UNRECOGNIZED };
enum PrepareResult { PREPARE_SUCCESS, PREPARE_UNRECOGNISED_STATE,PREPARE_SYNTAX_ERROR };
enum ExecuteResult { EXECUTE_SUCCESS,EXECUTE_TABLE_FULL };

struct Statement {
    StatementType type;
    Row row_to_insert;
};

bool metacommand(const std::string& input);
MetaCommandResult meta_command(const std::string& input);
PrepareResult prepare_statement(const std::string& input, Statement& statement);
ExecuteResult execute_statement(const Statement& statement, Table& table);
void serialize_row(const Row& source,void* destination);
void deserialize_row(const void* source,Row& destination);
void* row_slot(Table& table,uint32_t row_num);

int main() {
    std::string input;
    Table table;
    Statement stmt;
    while (true) {
        std::cout << "mydb> ";
        std::getline(std::cin, input);
        if (metacommand(input)) {
            if (meta_command(input) == META_EXIT) break;
            continue;
        }
        PrepareResult prepare_result = prepare_statement(input, stmt);
        if (prepare_result == PREPARE_UNRECOGNISED_STATE) {
            std::cout << "Unrecognised keyword at start of '" << input << "'.\n";
            continue;
        }
        if (prepare_result == PREPARE_SYNTAX_ERROR) {
            continue;
        }
        ExecuteResult execute_result=execute_statement(stmt,table);
        if(execute_result==EXECUTE_TABLE_FULL){
            std::cout<<"Error: Table full .\n";
            continue;
        }
        std::cout << "Executed.\n";
    }
    return 0;
}

bool metacommand(const std::string& input) {
    return !input.empty() && input[0] == '.';
}

MetaCommandResult meta_command(const std::string& input) {
    if (input == ".help") {
        std::cout << "Available commands:\n  .exit  Exit mydb\n  .help  Show this help message\n";
        return META_SUCCESS;
    } else if (input == ".exit") {
        return META_EXIT;
    } else {
        std::cout << "Unknown meta command: " << input << "\n";
        return META_UNRECOGNIZED;
    }
}

PrepareResult prepare_statement(const std::string& input, Statement& statement) {
    if (input.rfind("insert", 0) == 0) {
        statement.type = STATEMENT_INSERT;
        std::istringstream stream(input);
        std::string keyword;
        std::string username;
        std::string email;
        if(!(stream >> keyword >> statement.row_to_insert.id
               >> username
               >> email)){
                std::cout << "Syntax error. Usage: insert <id> <username> <email>\n";
                return PREPARE_SYNTAX_ERROR;
               }
        if(username.size()>COLUMN_USERNAME_SIZE || email.size()>COLUMN_EMAIL_SIZE){
            std::cout<<"String is too long.\n";
            return PREPARE_SYNTAX_ERROR;
        } 
        std::strncpy(statement.row_to_insert.username,username.c_str(),COLUMN_USERNAME_SIZE);
        statement.row_to_insert.username[COLUMN_USERNAME_SIZE]='\0';
        std::strncpy(statement.row_to_insert.email, email.c_str(), COLUMN_EMAIL_SIZE);
        statement.row_to_insert.email[COLUMN_EMAIL_SIZE] = '\0';      
        return PREPARE_SUCCESS;
    } else if (input.rfind("select", 0) == 0) {
        statement.type = STATEMENT_SELECT;
        return PREPARE_SUCCESS;
    } else {
        return PREPARE_UNRECOGNISED_STATE;
    }
}

ExecuteResult execute_statement(const Statement& statement, Table& table) {
    if (statement.type == STATEMENT_INSERT) {
        if(table.num_rows>=TABLE_MAX_ROWS){
            return EXECUTE_TABLE_FULL;
        }
        serialize_row(statement.row_to_insert, row_slot(table, table.num_rows));
        table.num_rows++;
        std::cout << "Inserted row: " << statement.row_to_insert.id
          << " " << statement.row_to_insert.username
          << " " << statement.row_to_insert.email << "\n";
    } 
    else if (statement.type == STATEMENT_SELECT) {
        Row row;
        for (uint32_t i = 0; i < table.num_rows; i++) {
            deserialize_row(row_slot(table, i), row);
            std::cout << row.id << " | " << row.username << " | " << row.email << "\n";
        }
    }
    return EXECUTE_SUCCESS;
}

void serialize_row(const Row& source,void* destination){
    std::memcpy(static_cast<char*>(destination)+ID_OFFSET,&source.id,ID_SIZE);
    std::memcpy(static_cast<char*>(destination)+USERNAME_OFFSET,source.username,USERNAME_SIZE);
    std::memcpy(static_cast<char*>(destination)+EMAIL_OFFSET,source.email,EMAIL_SIZE);
}

void deserialize_row(const void* source,Row& destination){
    std::memcpy(&destination.id,static_cast<const char*>(source)+ID_OFFSET,ID_SIZE);
    std::memcpy(destination.username,static_cast<const char*>(source)+USERNAME_OFFSET,USERNAME_SIZE);
    destination.username[COLUMN_USERNAME_SIZE]='\0';
    std::memcpy(destination.email,static_cast<const char*>(source)+EMAIL_OFFSET,EMAIL_SIZE);
    destination.email[COLUMN_EMAIL_SIZE]='\0';

}

void* row_slot(Table& table,uint32_t row_num){
    uint32_t page_num = row_num / ROWS_PER_PAGE;
    void* page = table.pages[page_num];

    if (page == nullptr) {
        page = operator new(PAGE_SIZE);
        table.pages[page_num] = page;
    }

    uint32_t row_offset = row_num % ROWS_PER_PAGE;
    uint32_t byte_offset = row_offset * ROW_SIZE;

    return static_cast<char*>(page) + byte_offset;
}