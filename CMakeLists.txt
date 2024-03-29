cmake_minimum_required(VERSION 3.0)

project(Spotless)

set(REACTION_SOURCES
ReAction/Listbrowser.cpp
ReAction/MainWindow.cpp
ReAction/Requester.cpp
ReAction/Speedbar.cpp
ReAction/Progress.cpp
ReAction/Layout.cpp
ReAction/Widget.cpp
ReAction/Screen.cpp
ReAction/Button.cpp
ReAction/Panel.cpp
ReAction/Menubar.cpp
ReAction/String.cpp
ReAction/Checkbox.cpp
ReAction/Config.cpp
)

set(SIMPLEDEBUG_SOURCES
SimpleDebug/Binary.cpp
SimpleDebug/Breaks.cpp
SimpleDebug/Handle.cpp
SimpleDebug/LowLevel.cpp
SimpleDebug/Pipe.cpp
SimpleDebug/Process.cpp
SimpleDebug/Stacktracer.cpp
SimpleDebug/Strings.cpp
SimpleDebug/Symbols.cpp
SimpleDebug/TextFile.cpp
SimpleDebug/Tracer.cpp
)

set(SPOTLESS_SOURCES main.cpp version.c Spotless/Spotless.cpp Spotless/Code.cpp)
set(SOURCES ${SPOTLESS_SOURCES} ${REACTION_SOURCES} ${SIMPLEDEBUG_SOURCES})

add_executable(Spotless ${SOURCES})
target_link_libraries(Spotless -lunix -lauto)

add_executable(ReActionTests ReAction/Tests/test.cpp ${REACTION_SOURCES})
target_link_libraries(ReActionTests -lauto)

add_executable(ConfigTest ReAction/Tests/config_test.cpp ReAction/Config.cpp)
target_link_libraries(ConfigTest -lauto)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mcrt=newlib -athread=native -gstabs")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mcrt=newlib -gstabs -mstrict-align -O0")