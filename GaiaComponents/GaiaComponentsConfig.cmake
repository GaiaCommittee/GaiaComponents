cmake_minimum_required(3.10)

find_path(GaiaComponents_INCLUDE_DIRS "GaiaComponents")
find_library(GaiaComponents_LIBS "GaiaComponents")

if (GaiaComponents_INCLUDE_DIRS AND GaiaComponents_LIBS)
    set(GaiaComponents_FOUND TRUE)
endif()