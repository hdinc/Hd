// the system's fonts: the CJK fallback fonts merged into the interface font.
// skipped (77) on a system without any.

#include <Hd/Fonts.h>

#include <imgui.h>

#include <stb/stb_truetype.h>

#include <cstdio>
#include <string>
#include <vector>

static int failures = 0;
#define CHECK(cond) do { if (!(cond)) { std::printf("%s:%d: CHECK(%s) failed\n", __FILE__, __LINE__, #cond); failures++; } } while (0)

namespace {

bool hasGlyph(const std::vector<unsigned char>& data, int index, int codepoint)
{
    stbtt_fontinfo info;
    int offset = stbtt_GetFontOffsetForIndex(data.data(), index);
    return offset >= 0 && stbtt_InitFont(&info, data.data(), offset) && stbtt_FindGlyphIndex(&info, codepoint);
}

}

int main()
{
    auto fonts = Hd::fallbackFonts();
    // the same list every time, without reading the fonts again
    CHECK(Hd::fallbackFonts() == fonts);
    for (auto& f : fonts)
        std::printf("fallback: %s\n", Hd::toString(f).c_str());
#ifdef _WIN32
    // every Windows 10/11 has Microsoft YaHei and Malgun Gothic
    CHECK(fonts.size() >= 2);
#endif
    if (fonts.empty()) {
        std::printf("no CJK fonts on this system, skipped\n");
        return 77;
    }

    // together they cover Chinese, Japanese kana and Hangul, and each adds
    // something: no font is there twice
    const int letters[] = {0x6771, 0x4EAC, 0x3042, 0x30A2, 0xAC00};
    std::vector<std::vector<unsigned char>> data;
    for (auto& f : fonts) {
        std::string error;
        data.push_back(Hd::readFont(f, error));
        CHECK(!data.back().empty());
    }
    for (int c : letters) {
        bool covered = false;
        for (size_t k = 0; k < fonts.size(); k++)
            if (!data[k].empty() && hasGlyph(data[k], fonts[k].index, c)) covered = true;
        if (!covered) std::printf("U+%04X not covered\n", c);
        CHECK(covered);
    }
    for (size_t a = 0; a < fonts.size(); a++)
        for (size_t b = a + 1; b < fonts.size(); b++)
            CHECK(!(fonts[a] == fonts[b]));

    // merged into imgui's fonts: the built-in interface font and the
    // monospace one draw CJK letters, from the fallback fonts
    ImGui::CreateContext();
    auto& io = ImGui::GetIO();
    io.BackendFlags |= ImGuiBackendFlags_RendererHasTextures;
    std::vector<std::string> errors;
    Hd::loadFonts({}, {}, {}, errors);
    CHECK(errors.empty());
    for (ImFont* font : {io.Fonts->Fonts[0], Hd::monoFont()}) {
        ImFontBaked* baked = font->GetFontBaked(20.0f);
        CHECK(baked->FindGlyphNoFallback('A') != nullptr);
        for (int c : letters)
            CHECK(baked->FindGlyphNoFallback((ImWchar)c) != nullptr);
    }
    // without fallback fonts the built-in font has none
    Hd::loadFonts({}, {}, {}, errors, true, false);
    CHECK(io.Fonts->Fonts[0]->GetFontBaked(20.0f)->FindGlyphNoFallback(0x6771) == nullptr);
    ImGui::DestroyContext();

    std::printf(failures ? "%d failed\n" : "ok\n", failures);
    return failures ? 1 : 0;
}
