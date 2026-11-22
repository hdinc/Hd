#pragma once

#include <string>
#include <vector>

struct GLFWwindow;

// the few things a program needs from the operating system that the rest of
// Hd doesn't do (configDir is in Theme.h, fonts in Fonts.h, an ImGui file
// dialog in FileDialog.h). Paths are UTF-8 everywhere.
namespace Hd::Platform {

// the command line's arguments after the program's name, in UTF-8
// (Windows' argv isn't)
std::vector<std::string> args(int argc, char** argv);

struct FileFilter {
    std::string name; // "PDF documents"
    std::string patterns; // "*.pdf;*.PDF"
};
// asks for files to open; empty when cancelled or when no dialog is
// available (Linux needs zenity or kdialog)
std::vector<std::string> openFilesDialog(GLFWwindow* parent, const std::string& title, const std::vector<FileFilter>& filters, bool multiple = true);
// opens a URI (http, https, mailto, ...) with the system's handler. It
// doesn't check what the URI is: callers decide what is safe to open.
bool openUri(const std::string& uri);
// absolute and normal, for comparing paths
std::string normalPath(const std::string& path);

// the system is set to a dark look (Windows' app mode, GNOME's color-scheme,
// a ":dark" GTK theme)
bool prefersDarkTheme();
// Windows: the window's title bar in dark colours (Windows 10 2004 and
// later); elsewhere nothing happens
void setDarkTitleBar(GLFWwindow* window, bool dark);
// the path of the system's interface font (Segoe UI, or fontconfig's
// sans-serif: Noto Sans, DejaVu Sans), with weight selected by bold.
// Empty when none is found. Fonts.h
// has the rest: every installed font, the default monospace one.
std::string systemUiFont(bool bold = false);

}
