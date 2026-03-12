#include <api.h>
#include <app_utils.h>

typedef struct {
    int addr1;
    int addr2;
    int has_addr1;
    int has_addr2;
    char cmd;
} EditorCommand;

static void print_char(char c) {
    char out[2];
    out[0] = c;
    out[1] = '\0';
    stdout_write(out);
}

static int str_eq(const char* a, const char* b) {
    while (*a && *b) {
        if (*a != *b) return 0;
        a++;
        b++;
    }
    return *a == '\0' && *b == '\0';
}

static int bounded_strlen(const char* s, int max_len) {
    int len = 0;
    while (len < max_len && s[len] != '\0') len++;
    return len;
}

static void move_bytes(char* dst, const char* src, int count) {
    if (count <= 0 || dst == src) return;

    if (dst < src) {
        for (int i = 0; i < count; i++) dst[i] = src[i];
        return;
    }

    for (int i = count - 1; i >= 0; i--) dst[i] = src[i];
}

static int get_line_count(const char* buffer, int length) {
    int count = 0;
    for (int i = 0; i < length; i++) {
        if (buffer[i] == '\n') count++;
    }
    if (length > 0 && buffer[length - 1] != '\n') count++;
    return count;
}

static int find_line_start(const char* buffer, int length, int line_no) {
    if (line_no <= 1) return 0;

    int current = 1;
    for (int i = 0; i < length; i++) {
        if (buffer[i] == '\n') {
            current++;
            if (current == line_no) return i + 1;
        }
    }
    return -1;
}

static int find_line_end(const char* buffer, int length, int line_no) {
    int start = find_line_start(buffer, length, line_no);
    if (start < 0) return -1;

    for (int i = start; i < length; i++) {
        if (buffer[i] == '\n') return i + 1;
    }
    return length;
}

static int parse_number(const char* text, int* index, int* value) {
    int found = 0;
    int result = 0;

    while (text[*index] >= '0' && text[*index] <= '9') {
        found = 1;
        result = result * 10 + (text[*index] - '0');
        (*index)++;
    }

    if (!found) return 0;
    *value = result;
    return 1;
}

static int parse_command(const char* text, EditorCommand* out) {
    int idx = 0;

    out->addr1 = 0;
    out->addr2 = 0;
    out->has_addr1 = 0;
    out->has_addr2 = 0;
    out->cmd = '\0';

    if (parse_number(text, &idx, &out->addr1)) {
        out->has_addr1 = 1;
        if (text[idx] == ',') {
            idx++;
            if (!parse_number(text, &idx, &out->addr2)) return 0;
            out->has_addr2 = 1;
        }
    }

    if (text[idx] == '\0') return 0;
    out->cmd = text[idx++];
    return text[idx] == '\0';
}

static void print_number(int value) {
    char buf[16];
    app_u32_to_str((uint32_t)value, buf);
    stdout_write(buf);
    stdout_write("\n");
}

static void print_line_number(int value) {
    char buf[16];
    app_u32_to_str((uint32_t)value, buf);
    stdout_write(buf);
    stdout_write("\t");
}

static void read_line(char* out, int max_len) {
    int idx = 0;

    while (1) {
        char c = stdin_read();
        if (c == 0) continue;

        if (c == '\n') {
            out[idx] = '\0';
            stdout_write("\n");
            return;
        }

        if (c == '\b') {
            if (idx > 0) {
                idx--;
                stdout_write("\b \b");
            }
            continue;
        }

        if (idx < max_len - 1) {
            out[idx++] = c;
            print_char(c);
        }
    }
}

static int insert_line_at(char* buffer, int* length, int max_len, int offset, const char* line) {
    int line_len = app_strlen(line);
    int insert_len = line_len + 1;

    if (*length + insert_len >= max_len) return 0;

    move_bytes(buffer + offset + insert_len, buffer + offset, *length - offset + 1);
    for (int i = 0; i < line_len; i++) buffer[offset + i] = line[i];
    buffer[offset + line_len] = '\n';
    *length += insert_len;
    buffer[*length] = '\0';
    return insert_len;
}

static int insert_mode(char* buffer, int* length, int max_len, int offset) {
    char line[128];

    while (1) {
        read_line(line, sizeof(line));
        if (str_eq(line, ".")) return 1;

        {
            int inserted = insert_line_at(buffer, length, max_len, offset, line);
            if (!inserted) {
                stdout_write("?\n");
                return 0;
            }
            offset += inserted;
        }
    }
}

static int print_range(const char* buffer, int length, int start_line, int end_line, int numbered) {
    int total = get_line_count(buffer, length);

    if (total == 0 || start_line < 1 || end_line < start_line || end_line > total) return 0;

    for (int line = start_line; line <= end_line; line++) {
        int start = find_line_start(buffer, length, line);
        int end = find_line_end(buffer, length, line);

        if (start < 0 || end < start) return 0;
        if (numbered) print_line_number(line);
        for (int i = start; i < end; i++) print_char(buffer[i]);
    }
    return 1;
}

static int delete_line(char* buffer, int* length, int line_no) {
    int total = get_line_count(buffer, *length);
    int start;
    int end;

    if (line_no < 1 || line_no > total) return 0;

    start = find_line_start(buffer, *length, line_no);
    end = find_line_end(buffer, *length, line_no);
    if (start < 0 || end < start) return 0;

    move_bytes(buffer + start, buffer + end, *length - end + 1);
    *length -= (end - start);
    buffer[*length] = '\0';
    return 1;
}

void main() {
    char path[64];
    char content[2048];
    char line[128];
    int content_len = 0;

    app_copy_args(path, sizeof(path));
    content[0] = '\0';
    content[sizeof(content) - 1] = '\0';

    if (path[0] != '\0' && read_file(path, content)) {
        content[sizeof(content) - 1] = '\0';
        content_len = bounded_strlen(content, sizeof(content) - 1);
        print_number(content_len);
    }

    while (1) {
        EditorCommand command;
        int line_count;

        read_line(line, sizeof(line));

        if (!parse_command(line, &command)) {
            if (line[0] != '\0') stdout_write("?\n");
            continue;
        }

        line_count = get_line_count(content, content_len);

        if (command.cmd == 'a') {
            int offset = content_len;

            if (command.has_addr1) {
                if (command.addr1 < 0 || command.addr1 > line_count) {
                    stdout_write("?\n");
                    continue;
                }
                if (command.addr1 > 0) {
                    offset = find_line_end(content, content_len, command.addr1);
                    if (offset < 0) {
                        stdout_write("?\n");
                        continue;
                    }
                } else {
                    offset = 0;
                }
            }

            insert_mode(content, &content_len, sizeof(content), offset);
        }
        else if (command.cmd == 'i') {
            int target = 1;
            int offset = 0;

            if (command.has_addr1) target = command.addr1;
            if (line_count == 0) target = 1;

            if (target < 1 || target > line_count + 1) {
                stdout_write("?\n");
                continue;
            }

            if (line_count > 0 && target <= line_count) {
                offset = find_line_start(content, content_len, target);
                if (offset < 0) {
                    stdout_write("?\n");
                    continue;
                }
            } else {
                offset = content_len;
            }

            insert_mode(content, &content_len, sizeof(content), offset);
        }
        else if (command.cmd == 'p') {
            int start = command.has_addr1 ? command.addr1 : 1;
            int end = command.has_addr2 ? command.addr2 : (command.has_addr1 ? command.addr1 : line_count);

            if (!print_range(content, content_len, start, end, 0)) stdout_write("?\n");
        }
        else if (command.cmd == 'n') {
            int start = command.has_addr1 ? command.addr1 : 1;
            int end = command.has_addr2 ? command.addr2 : (command.has_addr1 ? command.addr1 : line_count);

            if (!print_range(content, content_len, start, end, 1)) stdout_write("?\n");
        }
        else if (command.cmd == 'd') {
            int target = command.has_addr1 ? command.addr1 : 0;

            if (!command.has_addr1 || command.has_addr2 || !delete_line(content, &content_len, target)) {
                stdout_write("?\n");
            }
        }
        else if (command.cmd == 'w') {
            if (path[0] == '\0') {
                stdout_write("?\n");
                continue;
            }
            if (!write_file(path, content)) {
                stdout_write("?\n");
                continue;
            }
            print_number(content_len);
        }
        else if (command.cmd == 'q') {
            return;
        }
        else {
            stdout_write("?\n");
        }
    }
}
