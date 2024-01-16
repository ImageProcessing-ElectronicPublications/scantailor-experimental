PROJECT("Application")

INCLUDE_DIRECTORIES("${CMAKE_CURRENT_BINARY_DIR}")
INCLUDE_DIRECTORIES("${toplevel_ui_sources_BINARY_DIR}")

# crash_reporter is included unconditionally to collect translation sources from there.
ADD_SUBDIRECTORY(fix_orientation)
ADD_SUBDIRECTORY(page_split)
ADD_SUBDIRECTORY(deskew)
ADD_SUBDIRECTORY(select_content)
ADD_SUBDIRECTORY(page_layout)
ADD_SUBDIRECTORY(output)
