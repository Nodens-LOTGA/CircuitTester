cmake_minimum_required (VERSION 3.8)

project (${PROJECT_NAME}_prog)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

list(APPEND CMAKE_PREFIX_PATH "C:/Qt/5.15.0/msvc2019_64/lib/cmake/Qt5")

find_package(Qt5 COMPONENTS REQUIRED 
  Gui
  Core
  Widgets
  PrintSupport
  SerialPort
  Sql
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
  productinputdialog.h
  productinputdialog.cpp
  ../resources/resources.qrc
 )


#set_property(TARGET ${PROJECT_NAME} PROPERTY WIN32_EXECUTABLE true)

target_include_directories(${PROJECT_NAME} PRIVATE ${Boost_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME} PRIVATE 
  Qt5::Gui
  Qt5::Core
  Qt5::Widgets 
  Qt5::PrintSupport
  Qt5::SerialPort
  Qt5::Sql
)