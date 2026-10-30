// tests for the terminal emulator (Vt100). no test framework, a
// failing check prints the location and the test exits with 1.

#include <Hd/Vt100.h>

#include <cstdio>
#include <string>

using Hd::Vt100;

static int failures = 0;

#define CHECK(cond)                                                              \
    do {                                                                         \
        if (!(cond)) {                                                           \
            std::printf("%s:%d: CHECK(%s) failed\n", __FILE__, __LINE__, #cond); \
            failures++;                                                          \
        }                                                                        \
    } while (0)

// a screen row without trailing spaces
static std::string row(const Vt100& t, int r) { return t.text(t.historySize() + size_t(r), t.historySize() + size_t(r)); }

static void test_text()
{
    Vt100 t(10, 3);
    t.feed("ab\r\ncd");
    CHECK(row(t, 0) == "ab" && row(t, 1) == "cd");
    CHECK(t.cursorRow() == 1 && t.cursorCol() == 2);
    // backspace and tab
    t.feed("\bX\tY");
    CHECK(row(t, 1) == "cX      Y");
    // the deferred wrap: the 10th character stays on the row, the 11th wraps
    t.feed("\r\n0123456789");
    CHECK(row(t, 2) == "0123456789" && t.cursorRow() == 2 && t.cursorCol() == 9);
    t.feed("A");
    // the screen scrolled, its top row went into the history
    CHECK(t.historySize() == 1 && t.text(0, 0) == "ab");
    CHECK(row(t, 1) == "0123456789" && row(t, 2) == "A");
    // utf-8
    t.feed("\xc3\xa9\xe2\x82\xac");
    CHECK(t.at(2, 1).ch == U'é' && t.at(2, 2).ch == U'€');
    CHECK(row(t, 2) == "A\xc3\xa9\xe2\x82\xac");
}

static void test_csi()
{
    Vt100 t(10, 4);
    t.feed("hello\x1b[2;3Hx");
    CHECK(row(t, 1) == "  x");
    t.feed("\x1b[H\x1b[2J");
    CHECK(row(t, 0).empty() && row(t, 1).empty() && t.cursorRow() == 0 && t.cursorCol() == 0);
    t.feed("abcdef\x1b[3D\x1b[K");
    CHECK(row(t, 0) == "abc");
    t.feed("\x1b[1G\x1b[2P");
    CHECK(row(t, 0) == "c");
    t.feed("\x1b[2@");
    CHECK(row(t, 0) == "  c");
    t.feed("\x1b[5C\x1b[A\x1b[B\x1b[B");
    CHECK(t.cursorRow() == 2 && t.cursorCol() == 5);
    // colors and attributes
    t.feed("\x1b[1;31mR\x1b[0m\x1b[38;5;196mS\x1b[48;2;1;2;3mT\x1b[7mU");
    CHECK(t.at(2, 5).fg == Vt100::palette(1) && (t.at(2, 5).attrs & Vt100::bold));
    CHECK(t.at(2, 6).fg == Vt100::palette(196) && t.at(2, 6).attrs == 0);
    CHECK(t.at(2, 7).bg == (0x01000000u | 0x010203u));
    CHECK(t.at(2, 8).attrs & Vt100::inverse);
    // cursor position report
    t.feed("\x1b[6n");
    CHECK(t.takeReply() == "\x1b[3;10R");
    CHECK(t.takeReply().empty());
    // an osc title is skipped
    t.feed("\x1b]0;title\x07\r\nok");
    CHECK(row(t, 3) == "ok");
}

static void test_scroll_region()
{
    Vt100 t(5, 5);
    t.feed("1\r\n2\r\n3\r\n4\r\n5");
    // rows 2..4 scroll, 1 and 5 stay
    t.feed("\x1b[2;4r\x1b[4;1H\nX");
    CHECK(row(t, 0) == "1" && row(t, 1) == "3" && row(t, 2) == "4" && row(t, 3) == "X" && row(t, 4) == "5");
    // no history from a region that isn't the whole screen
    CHECK(t.historySize() == 0);
    // reverse index at the region's top
    t.feed("\x1b[2;1H\x1bM");
    CHECK(row(t, 1).empty() && row(t, 2) == "3");
    // insert and delete lines
    t.feed("\x1b[r\x1b[1;1H\x1b[L");
    CHECK(row(t, 0).empty() && row(t, 1) == "1");
    t.feed("\x1b[M");
    CHECK(row(t, 0) == "1");
}

static void test_alternate_screen()
{
    Vt100 t(6, 3);
    t.feed("main");
    t.feed("\x1b[?1049h\x1b[Hvi");
    CHECK(t.alternateScreen() && row(t, 0) == "vi");
    t.feed("\x1b[?1049l");
    CHECK(!t.alternateScreen() && row(t, 0) == "main" && t.cursorCol() == 4);
    t.feed("\x1b[?1h");
    CHECK(t.applicationCursor());
    t.feed("\x1b[?25l");
    CHECK(!t.cursorVisible());
    t.feed("\x1b" "c");
    CHECK(t.cursorVisible() && !t.applicationCursor() && row(t, 0).empty());
}

static void test_resize_and_copy()
{
    Vt100 t(8, 3);
    t.feed("one\r\ntwo\r\nthree");
    t.resize(4, 2);
    CHECK(t.historySize() == 1 && t.text(0, 0) == "one");
    CHECK(row(t, 0) == "two" && row(t, 1) == "thre");
    CHECK(t.cursorRow() == 1 && t.cursorCol() == 3);
    CHECK(t.text(0, 2) == "one\ntwo\nthre");
    CHECK(t.text(1, 1, 2, 2) == "wo\nthr");
}

int main()
{
    test_text();
    test_csi();
    test_scroll_region();
    test_alternate_screen();
    test_resize_and_copy();
    if (failures) std::printf("%d failures\n", failures);
    return failures ? 1 : 0;
}
