cmake_minimum_required (VERSION 3.8)

project (${PROJECT_NAME}_prog)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

list(APPEND CMAKE_PREFIX_PATH "C:/Qt/5.15.0/msvc2019/lib/cmake/Qt5")

find_package(Qt5 COMPONENTS REQUIRED 
  Gui
  Core
  Widgets
  PrintSupport
  SerialPort
  Sql
  Help
) 
find_package(Boost 1.73 REQUIRED)

add_executable (${PROJECT_NAME} 
  main.cpp
  mainwindow.cpp
  mainwindow.h
  mainwindow.ui
  startupdialog.h
  startupdialog.cpp
  startupdialog.ui
  helpdialog.h
  helpdialog.cpp
  helpdialog.ui
  settingsdialog.h
  settingsdialog.cpp
  settingsdialog.ui
  reportdelegate.h 
  reportdelegate.cpp
  serialport.h
  serialport.cpp
  report.h
  report.cpp
  tools.h
  circuitsdelegate.h
  circuitsdelegate.cpp
  sqltools.h
  sqltools.cpp
  winapiprint.h 
  winapiprint.cpp 
  settings.h 
  settings.cpp
  reportdialog.h
  reportdialog.cpp
  reportdialog.ui
  productinputdialog.h
  productinputdialog.cpp
  productinputdialog.ui
  userslistdialog.h
  userslistdialog.cpp
  userslistdialog.ui
  spinboxdelegate.h
  spinboxdelegate.cpp
  ../resources/resources.qrc
 "HelpBrowser.h" "HelpBrowser.cpp")


if(WIN32)
    set_property(TARGET ${PROJECT_NAME} PROPERTY WIN32_EXECUTABLE true)
endif()
target_compile_definitions(${PROJECT_NAME} PUBLIC "$<$<CONFIG:Release>:QT_NO_DEBUG_OUTPUT>")

target_include_directories(${PROJECT_NAME} PRIVATE ${Boost_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME} PRIVATE 
  Qt5::Gui
  Qt5::Core
  Qt5::Widgets 
  Qt5::PrintSupport
  Qt5::SerialPort
  Qt5::Sql
  Qt5::Help
)

get_target_property(_qmake_executable Qt5::qmake IMPORTED_LOCATION)
get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)
find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${_qt_bin_dir}")
find_program(MACDEPLOYQT_EXECUTABLE macdeployqt HINTS "${_qt_bin_dir}")
find_program(QHELPGENERATOR_EXECUTABLE qhelpgenerator HINTS "${_qt_bin_dir}")

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E
        env PATH="${_qt_bin_dir}" "${WINDEPLOYQT_EXECUTABLE}"
            "$<TARGET_FILE:${PROJECT_NAME}>"
    COMMENT "Running windeployqt..."
    COMMAND "${CMAKE_COMMAND}" -E env PATH="${_qt_bin_dir}" "${QHELPGENERATOR_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/help/help.qhcp" COMMENT "Running qhelpgenerator..."
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/help/help.qhc" "${CMAKE_CURRENT_SOURCE_DIR}/help/help.qch" "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
)