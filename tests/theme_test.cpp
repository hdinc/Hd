// tests for themes with an app's color table: built-in themes, colors by key
// and theme files. no imgui context is needed.

#include <Hd/Theme.h>

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>

static int failures = 0;

#define CHECK(cond)                                                              \
    do {                                                                         \
        if (!(cond)) {                                                           \
            std::printf("%s:%d: CHECK(%s) failed\n", __FILE__, __LINE__, #cond); \
            failures++;                                                          \
        }                                                                        \
    } while (0)

static bool same(ImVec4 a, ImVec4 b)
{
    return Hd::toHex(a) == Hd::toHex(b);
}

static const ImVec4 red { 1, 0, 0, 1 }, green { 0, 1, 0, 1 }, blue { 0, 0, 1, 1 }, grey { 0.5f, 0.5f, 0.5f, 1 };

static void test_hex()
{
    ImVec4 c;
    CHECK(Hd::parseHex("#15171a", c) && Hd::toHex(c) == "#15171aff");
    CHECK(Hd::parseHex("#ff8c1a80", c) && Hd::toHex(c) == "#ff8c1a80");
    CHECK(!Hd::parseHex("15171a", c));
    CHECK(!Hd::parseHex("#15171", c));
    CHECK(!Hd::parseHex("#zz171a", c));
}

static void test_table()
{
    // before the app's table: no colors of its own, fallbacks by key
    CHECK(Hd::Theme::current().colors.empty());
    CHECK(same(Hd::Theme::current().color("error", grey), grey));

    Hd::Theme::setColorTable({
        { "error", "Error", "Messages", "errors", red, green, blue },
        { "accent", "Accent", "Canvas", "highlights", green, blue, red },
    });
    CHECK(Hd::Theme::colorTable().size() == 2);
    // the current theme takes them from its base
    CHECK(Hd::Theme::current().colors.size() == 2 && same(Hd::Theme::current()[0], red));
    CHECK(same(Hd::Theme::current().color("accent", grey), green));
    CHECK(same(Hd::Theme::current().color("nope", grey), grey));
    CHECK(same(Hd::Theme::current()[7], {}));

    auto dark = Hd::Theme::makeBuiltin("dark");
    auto light = Hd::Theme::makeBuiltin("light");
    auto black = Hd::Theme::makeBuiltin("black");
    CHECK(same(dark[0], red) && same(light[0], green) && same(black[0], blue));
    CHECK(dark.builtin() && dark.name == "Dark" && light.name == "Light" && black.name == "Black");
    CHECK(Hd::Theme::makeBuiltin("nope").base == "dark");
    CHECK(Hd::Theme::builtinThemes().size() == 3);
    CHECK(dark.key() == "dark" && black.key() == "black");
    // imgui's colors: its light style, black windows on black
    ImGuiStyle s;
    ImGui::StyleColorsLight(&s);
    CHECK(same(light.ui[ImGuiCol_WindowBg], s.Colors[ImGuiCol_WindowBg]));
    CHECK(Hd::toHex(black.ui[ImGuiCol_WindowBg]) == "#000000ff");
    CHECK(same(black.ui[ImGuiCol_Button], dark.ui[ImGuiCol_Button]));

    Hd::Theme::setCurrent(light);
    CHECK(Hd::Theme::current().name == "Light");
}

static void test_files(const std::filesystem::path& dir)
{
    std::filesystem::remove_all(dir);

    auto t = Hd::Theme::makeBuiltin("light");
    t.name = "Mine";
    t.file = dir / "Mine.json";
    t.colors[1] = { 0.1f, 0.2f, 0.3f, 0.4f };
    t.ui[ImGuiCol_Button] = { 1, 0, 1, 1 };
    std::string error;
    CHECK(t.save(error));
    Hd::Theme back;
    CHECK(Hd::Theme::load(t.file, back, error));
    CHECK(back.name == "Mine" && back.base == "light" && !back.builtin() && back.key() == "Mine");
    CHECK(back.colors.size() == 2 && same(back.colors[0], t.colors[0]) && same(back.colors[1], t.colors[1]));
    CHECK(same(back.ui[ImGuiCol_Button], t.ui[ImGuiCol_Button]));

    // missing and invalid colors come from the base, unknown keys are ignored
    {
        std::ofstream out(dir / "partial.json");
        out << R"({"base": "black", "colors": {"accent": "#010203", "error": "red", "nonsense": "#ffffff"},
                  "interface": {"Text": "#ff0000ff", "Nope": "#000000"}})";
    }
    Hd::Theme p;
    CHECK(Hd::Theme::load(dir / "partial.json", p, error));
    CHECK(p.name == "partial" && p.base == "black");
    CHECK(Hd::toHex(p[1]) == "#010203ff" && same(p[0], blue));
    CHECK(Hd::toHex(p.ui[ImGuiCol_Text]) == "#ff0000ff");

    CHECK(!Hd::Theme::load(dir / "missing.json", p, error) && !error.empty());
    {
        std::ofstream out(dir / "broken.json");
        out << "{ not json";
    }
    CHECK(!Hd::Theme::load(dir / "broken.json", p, error));

    std::filesystem::remove_all(dir);
}

static void test_config()
{
    Hd::setConfigName("hd-theme-test");
    CHECK(Hd::configDir().filename() == "hd-theme-test");
    CHECK(Hd::Theme::dir() == Hd::configDir() / "themes");
    // not a built-in one and no such file: Dark
    Hd::Theme t;
    std::string error;
    CHECK(!Hd::Theme::find("no such theme here", t, error) && t.name == "Dark");
    CHECK(Hd::Theme::find("light", t, error) && t.name == "Light");
    CHECK(Hd::Theme::find("", t, error) && t.name == "Dark");
}

int main(int argc, char** argv)
{
    auto dir = std::filesystem::path(argc > 1 ? argv[1] : ".") / "theme_test_dir";

    test_hex();
    test_table();
    test_files(dir);
    test_config();

    if (failures) {
        std::printf("%d check(s) failed\n", failures);
        return 1;
    }
    std::printf("all tests passed\n");
    return 0;
}
