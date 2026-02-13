/**
 * @file FileDialog.h
 * @brief Open file dialog (multi-select) for editor import.
 */
#ifndef TE_EDITOR_FILE_DIALOG_H
#define TE_EDITOR_FILE_DIALOG_H

#include <string>
#include <vector>

namespace te {
namespace editor {

/**
 * Open a multi-select file dialog. Returns selected file paths (full path).
 * Empty if cancelled or unsupported platform.
 * initialDir: optional starting directory (e.g. import target); null to use shell default.
 */
std::vector<std::string> OpenFileDialogMulti(char const* filterDesc = nullptr,
                                             char const* filterSpec = nullptr,
                                             char const* initialDir = nullptr);

}  // namespace editor
}  // namespace te

#endif
