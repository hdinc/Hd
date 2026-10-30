#pragma once

// a terminal emulator for the bytes of a serial line: the vt100/xterm subset
// that linux, busybox and vi use. no drawing here, the terminal window draws
// the cells.

#include <cstdint>
#include <deque>
#include <string>
#include <string_view>
#include <vector>

namespace Hd {

class Vt100 {
public:
    // 0 is the default color, else 0x01RRGGBB
    using Color = uint32_t;
    enum Attr : uint8_t {
        bold = 1,
        dim = 2,
        underline = 4,
        inverse = 8,
        italic = 16
    };
    struct Cell {
        char32_t ch = U' ';
        Color fg = 0;
        Color bg = 0;
        uint8_t attrs = 0;
        bool operator==(const Cell&) const = default;
    };
    using Line = std::vector<Cell>;

    explicit Vt100(int cols = 80, int rows = 24, size_t historyLines = 5000);

    void feed(std::string_view bytes);
    // answers to queries (cursor position, device attributes) for the
    // other side, taken once
    std::string takeReply();

    // keeps the text, cut or padded at the right and bottom
    void resize(int cols, int rows);
    // the state after power on, no history
    void reset();

    int cols() const { return m_cols; }
    int rows() const { return m_rows; }
    // the history (oldest first) and then the screen's rows
    size_t lines() const { return history.size() + screen().size(); }
    const Line& lineAt(size_t k) const { return k < history.size() ? history[k] : screen()[k - history.size()]; }
    size_t historySize() const { return history.size(); }
    // the screen's rows only
    const Cell& at(int row, int col) const { return screen()[size_t(row)][size_t(col)]; }
    int cursorRow() const { return cur_row; }
    int cursorCol() const { return cur_col; }
    bool cursorVisible() const { return show_cursor; }
    // DECCKM: arrows send ESC O A instead of ESC [ A
    bool applicationCursor() const { return app_cursor; }
    bool alternateScreen() const { return alt; }
    // goes up with every change, for redrawing
    uint64_t version() const { return m_version; }

    // the text of lines [first, last] with newlines between them, trailing
    // spaces dropped
    std::string text(size_t first, size_t last) const;
    // part of it: from (first, first_col) to (last, last_col) inclusive
    std::string text(size_t first, int first_col, size_t last, int last_col) const;

    // the rgb of a 256 color index
    static Color palette(int index);

private:
    enum class State {
        ground,
        escape,
        // ESC ( and friends: the next byte is a charset
        charset,
        csi,
        // an OSC or other string, up to BEL or ESC backslash
        string,
        string_escape
    };

    std::vector<Line>& screen() { return alt ? alt_screen : main_screen; }
    const std::vector<Line>& screen() const { return alt ? alt_screen : main_screen; }
    Line blank_line() const;
    Cell blank() const;

    void put(char32_t c);
    void control(uint8_t c);
    void escape(uint8_t c);
    void csi(uint8_t final);
    void sgr();
    void line_feed();
    void reverse_index();
    // rows [top, bottom] of the scroll region move up (n > 0) or down
    void scroll(int n);
    void erase(int row, int from, int to);
    void set_mode(bool on);
    void clamp_cursor();
    int param(size_t k, int def) const;

    int m_cols, m_rows;
    size_t max_history;
    std::vector<Line> main_screen, alt_screen;
    std::deque<Line> history;
    bool alt = false;

    int cur_row = 0, cur_col = 0;
    // the next character goes to the next line first (deferred wrap)
    bool wrap_pending = false;
    Cell pen;
    struct SavedCursor {
        int row = 0, col = 0;
        Cell pen;
    };
    SavedCursor saved, saved_alt;
    int top = 0, bottom = 0;
    bool show_cursor = true, app_cursor = false, autowrap = true, insert_mode = false, origin_mode = false;

    State st = State::ground;
    std::vector<int> params;
    bool param_started = false;
    char private_marker = 0;
    // utf-8 being decoded
    char32_t utf8 = 0;
    int utf8_left = 0;

    std::string reply;
    uint64_t m_version = 0;
};

}
