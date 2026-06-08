# MyDB Progress Log-1                           

## Stage 1: REPL

### Goal
Create an interactive database shell.

### Features Implemented
- `mydb>` prompt
- Infinite command loop
- Input handling

### Example

```text
mydb> select
Executed.
```

---

## Stage 2: Meta Commands

### Features Implemented
- `.exit`
- `.help`

### Key Concept
Meta commands control the database shell itself rather than table data.

### Code

```cpp
bool metacommand(const std::string& input) {
    return !input.empty() && input[0] == '.';
}
```

---

## Stage 3: Statement Preparation

### Features Implemented
- `insert`
- `select`
- Syntax validation

### Statement Structure

```cpp
struct Statement {
    StatementType type;
    Row row_to_insert;
};
```

### Example

Input:

```text
insert 1 mark mark@email.com
```

Parsed Into:

```text
Statement {
    type = INSERT,
    row = {1, "mark", "mark@email.com"}
}
```

---

## Stage 4: Row Serialization

### Goal
Convert rows into raw bytes.

### Layout

| Field | Offset | Size |
|---------|---------|---------|
| id | 0 | 4 |
| username | 4 | 32 |
| email | 36 | 255 |

### Functions

```cpp
serialize_row(...)
deserialize_row(...)
```

### Data Flow

```text
Row -> Raw Bytes -> Row
```

---

## Current Status

### Completed

- [x] REPL
- [x] Meta Commands
- [x] Statement Parsing
- [x] Insert
- [x] Select
- [x] Row Struct
- [x] Fixed Row Layout
- [x] Serialization
- [x] Deserialization

### Next

- [ ] Page-based Storage
- [ ] Pager
- [ ] Database File
- [ ] Cursor
- [ ] B-Tree Nodes
- [ ] B-Tree Search
- [ ] B-Tree Insert


# MyDB Progress Log-2
- [x] Page-based Storage
### Goal
Move table storage from `std::vector<Row>` to fixed-size memory pages.

### Features Implemented
- Added `PAGE_SIZE`, `ROWS_PER_PAGE`, `TABLE_MAX_PAGES`, and calculated `TABLE_MAX_ROWS`
- Replaced `vector<Row>` with `num_rows` and `void* pages[TABLE_MAX_PAGES]`
- Added `row_slot()` to locate the correct byte position for a row
- Insert now serializes rows into page memory
- Select now deserializes rows from page memory
- Added page cleanup on `.exit`

### Key Concept
Rows are no longer stored as C++ objects directly. They are converted into raw bytes and placed into fixed-size pages.

### Data Flow

```text
insert command
-> Row
-> serialize_row()
-> row_slot()
-> page memory

select command
-> row_slot()
-> deserialize_row()
-> Row
-> print

### Next

- [ ] Pager
- [ ] Database File
- [ ] Cursor
- [ ] B-Tree Nodes
- [ ] B-Tree Search
- [ ] B-Tree Insert

# MyDB Progress Log-3 