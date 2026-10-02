#include "mydb.hpp"

#include <cstdlib>
#include <cstring>
#include <iostream>

ExecuteResult execute_statement(const Statement& statement, Table& table) {
    if (statement.type == STATEMENT_INSERT) {
        // Check for duplicate id
        {
            Row existing;
            Cursor scan = table_start(&table);
            while (!scan.end_of_table) {
                deserialize_row(cursor_value(scan), existing);
                if (existing.id == statement.row_to_insert.id) {
                    std::cout << "Error: Duplicate id " << statement.row_to_insert.id << ".\n";
                    return EXECUTE_SUCCESS;
                }
                cursor_advance(scan);
            }
        }

        if (table.num_rows >= TABLE_MAX_ROWS) {
            return EXECUTE_TABLE_FULL;
        }

        Cursor cursor = table_end(&table);
        serialize_row(statement.row_to_insert, cursor_value(cursor));
        table.num_rows++;
        std::cout << "Inserted row: " << statement.row_to_insert.id << " "
                  << statement.row_to_insert.username << " "
                  << statement.row_to_insert.email << "\n";
    } else if (statement.type == STATEMENT_SELECT) {
        Row row;
        Cursor cursor = table_start(&table);
        while (!cursor.end_of_table) {
            deserialize_row(cursor_value(cursor), row);
            std::cout << row.id << " | " << row.username << " | " << row.email << "\n";
            cursor_advance(cursor);
        }
    }

    return EXECUTE_SUCCESS;
}

void serialize_row(const Row& source, void* destination) {
    std::memcpy(static_cast<char*>(destination) + ID_OFFSET, &source.id, ID_SIZE);
    std::memcpy(static_cast<char*>(destination) + USERNAME_OFFSET, source.username, USERNAME_SIZE);
    std::memcpy(static_cast<char*>(destination) + EMAIL_OFFSET, source.email, EMAIL_SIZE);
}

void deserialize_row(const void* source, Row& destination) {
    std::memcpy(&destination.id, static_cast<const char*>(source) + ID_OFFSET, ID_SIZE);
    std::memcpy(destination.username, static_cast<const char*>(source) + USERNAME_OFFSET, USERNAME_SIZE);
    destination.username[COLUMN_USERNAME_SIZE] = '\0';
    std::memcpy(destination.email, static_cast<const char*>(source) + EMAIL_OFFSET, EMAIL_SIZE);
    destination.email[COLUMN_EMAIL_SIZE] = '\0';
}

void* row_slot(Table& table, uint32_t row_num) {
    uint32_t page_num = row_num / ROWS_PER_PAGE;
    void* page = get_page(table.pager, page_num);
    uint32_t row_offset = row_num % ROWS_PER_PAGE;
    uint32_t byte_offset = row_offset * ROW_SIZE;
    return static_cast<char*>(page) + byte_offset;
}

Pager* pager_open(const std::string& filename) {
    Pager* pager = new Pager();
    pager->file.open(filename, std::ios::in | std::ios::out | std::ios::binary);
    if (!pager->file.is_open()) {
        pager->file.open(filename, std::ios::out | std::ios::binary);
        pager->file.close();
        pager->file.open(filename, std::ios::in | std::ios::out | std::ios::binary);
    }

    pager->file.seekg(0, std::ios::end);
    pager->file_length = static_cast<uint32_t>(pager->file.tellg());
    return pager;
}

Table* db_open(const std::string& filename) {
    Table* table = new Table();
    table->pager = pager_open(filename);
    table->num_rows = table->pager->file_length / ROW_SIZE;
    return table;
}

void* get_page(Pager* pager, uint32_t page_num) {
    if (page_num >= TABLE_MAX_PAGES) {
        std::cout << "Tried to fetch page number out of bounds.\n";
        std::exit(1);
    }

    if (pager->pages[page_num] == nullptr) {
        void* page = operator new(PAGE_SIZE);
        std::memset(page, 0, PAGE_SIZE);

        uint32_t num_pages = pager->file_length / PAGE_SIZE;
        if (pager->file_length % PAGE_SIZE != 0) {
            num_pages++;
        }

        if (page_num < num_pages) {
            pager->file.seekg(page_num * PAGE_SIZE, std::ios::beg);
            pager->file.read(static_cast<char*>(page), PAGE_SIZE);
        }

        pager->pages[page_num] = page;
    }

    return pager->pages[page_num];
}

void pager_flush(Pager* pager, uint32_t page_num, uint32_t size) {
    if (pager->pages[page_num] == nullptr) {
        return;
    }

    pager->file.seekp(page_num * PAGE_SIZE, std::ios::beg);
    pager->file.write(static_cast<char*>(pager->pages[page_num]), size);
    pager->file.flush();
}

void db_close(Table* table) {
    Pager* pager = table->pager;
    uint32_t num_full_pages = table->num_rows / ROWS_PER_PAGE;

    for (uint32_t i = 0; i < num_full_pages; i++) {
        if (pager->pages[i] != nullptr) {
            pager_flush(pager, i, PAGE_SIZE);
        }
    }

    uint32_t num_additional_rows = table->num_rows % ROWS_PER_PAGE;
    if (num_additional_rows > 0) {
        uint32_t page_num = num_full_pages;
        if (pager->pages[page_num] != nullptr) {
            pager_flush(pager, page_num, num_additional_rows * ROW_SIZE);
        }
    }

    for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++) {
        if (pager->pages[i] != nullptr) {
            operator delete(pager->pages[i]);
        }
    }

    if (pager->file.is_open()) {
        pager->file.close();
    }

    delete pager;
    delete table;
}

Cursor table_start(Table* table) {
    return Cursor{table, 0, table->num_rows == 0};
}

Cursor table_end(Table* table) {
    return Cursor{table, table->num_rows, true};
}

void* cursor_value(const Cursor& cursor) {
    return row_slot(*cursor.table, cursor.row_num);
}

void cursor_advance(Cursor& cursor) {
    cursor.row_num++;
    cursor.end_of_table = cursor.row_num >= cursor.table->num_rows;
}
