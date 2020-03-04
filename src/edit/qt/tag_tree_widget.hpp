// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__EDIT__QT__TAG_TREE_WIDGET_HPP
#define INVADER__EDIT__QT__TAG_TREE_WIDGET_HPP

#include <QTreeWidget>
#include <filesystem>
#include <invader/file/file.hpp>

#include <invader/hek/class_int.hpp>

namespace Invader::EditQt {
    class TagTreeWindow;

    class TagTreeWidget : public QTreeWidget {
    public:
        /**
         * Instantiate a TagTreeWidget
         * @param parent           parent widget
         * @param parent_window    parent window
         * @param classes          optional array of classes to filter in
         * @param show_directories show all directories even if empty
         */
        TagTreeWidget(QWidget *widget, TagTreeWindow *parent_window, const std::optional<std::vector<HEK::TagClassInt>> &classes = std::nullopt, const std::optional<std::vector<std::size_t>> &tags_directories = std::nullopt, bool show_directories = false);

        /**
         * Set the filter, limiting the view to those classes and directories that contain the given classes
         * @param classes an optional array of classes; if none is given, then the filter is cleared
         */
        void set_filter(const std::optional<std::vector<HEK::TagClassInt>> &classes = std::nullopt, const std::optional<std::vector<std::size_t>> &tags_directories = std::nullopt);

        /**
         * Get the total tags found
         * @return total tags found
         */
        std::size_t get_total_tags();

        /**
         * Get the tag file
         * @return pointer to tag file if one is selected or nullptr
         */
        const File::TagFile *get_selected_tag() const noexcept;

        /**
         * Sort elements in the tree
         */
        void resort_elements();
    private:
        std::size_t total_tags = 0;
        std::optional<std::vector<HEK::TagClassInt>> filter;
        std::optional<std::vector<std::size_t>> tag_arrays_to_show;
        bool show_directories;
        void refresh_view(TagTreeWindow *window);
    };
}

#endif