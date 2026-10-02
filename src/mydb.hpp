#ifndef MYDB_HPP
#define MYDB_HPP

#include <cstdint>
#include <fstream>
#include <string>

inline constexpr uint32_t COLUMN_USERNAME_SIZE = 32;
inline constexpr uint32_t COLUMN_EMAIL_SIZE = 255;
inline constexpr uint32_t ID_SIZE = sizeof(uint32_t);
inline constexpr uint32_t USERNAME_SIZE = COLUMN_USERNAME_SIZE;
inline constexpr uint32_t EMAIL_SIZE = COLUMN_EMAIL_SIZE;
inline constexpr uint32_t ID_OFFSET = 0;
inline constexpr uint32_t USERNAME_OFFSET = ID_OFFSET + ID_SIZE;
inline constexpr uint32_t EMAIL_OFFSET = USERNAME_OFFSET + USERNAME_SIZE;
inline constexpr uint32_t ROW_SIZE = ID_SIZE + USERNAME_SIZE + EMAIL_SIZE;
inline constexpr uint32_t PAGE_SIZE = 4096;
inline constexpr uint32_t ROWS_PER_PAGE = PAGE_SIZE / ROW_SIZE;
inline constexpr uint32_t TABLE_MAX_PAGES = 100;
inline constexpr uint32_t TABLE_MAX_ROWS = ROWS_PER_PAGE * TABLE_MAX_PAGES;

enum StatementType { STATEMENT_INSERT, STATEMENT_SELECT };
enum MetaCommandResult { META_EXIT, META_SUCCESS, META_UNRECOGNIZED };
enum PrepareResult {
    PREPARE_SUCCESS,
    PREPARE_UNRECOGNISED_STATEMENT,
    PREPARE_SYNTAX_ERROR
};
enum ExecuteResult { EXECUTE_SUCCESS, EXECUTE_TABLE_FULL };

struct Pager {
    std::fstream file;
    uint32_t file_length;
    void* pages[TABLE_MAX_PAGES] = {};
};

struct Row {
    uint32_t id;
    char username[COLUMN_USERNAME_SIZE + 1];
    char email[COLUMN_EMAIL_SIZE + 1];
};

struct Table {
    uint32_t num_rows = 0;
    Pager* pager;
};

struct Cursor {
    Table* table;
    uint32_t row_num;
    bool end_of_table;
};

inline constexpr uint32_t BTREE_MIN_DEGREE = 4;
inline constexpr uint32_t BTREE_MAX_KEYS = (2 * BTREE_MIN_DEGREE) - 1;

struct BTreeNode {
    bool is_leaf = true;
    uint32_t num_keys = 0;
    uint32_t keys[BTREE_MAX_KEYS] = {};
    BTreeNode* children[BTREE_MAX_KEYS + 1] = {};
};

struct BTree {
    BTreeNode* root = nullptr;
};

struct Statement {
    StatementType type;
    Row row_to_insert;
};

// REPL and statement preparation
bool metacommand(const std::string& input);
MetaCommandResult meta_command(const std::string& input);
PrepareResult prepare_statement(const std::string& input, Statement& statement);

// Statement execution and row storage
ExecuteResult execute_statement(const Statement& statement, Table& table);
void serialize_row(const Row& source, void* destination);
void deserialize_row(const void* source, Row& destination);
void* row_slot(Table& table, uint32_t row_num);

// Pager and database lifetime
Pager* pager_open(const std::string& filename);
Table* db_open(const std::string& filename);
void* get_page(Pager* pager, uint32_t page_num);
void pager_flush(Pager* pager, uint32_t page_num, uint32_t size);
void db_close(Table* table);

// Cursor navigation
Cursor table_start(Table* table);
Cursor table_end(Table* table);
void* cursor_value(const Cursor& cursor);
void cursor_advance(Cursor& cursor);

// B-tree storage and lookup
BTree* btree_create();
void btree_destroy(BTree* tree);
bool btree_contains(const BTree* tree, uint32_t key);
void btree_insert(BTree* tree, uint32_t key);

#endif
