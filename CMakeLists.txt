cmake_minimum_required(VERSION 3.16)
project(VisualAutomata LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)

find_package(Threads REQUIRED)

find_package(Qt6 COMPONENTS Core Widgets Gui Xml Test QUIET)
if(Qt6_FOUND)
    set(VA_QT Qt6)
else()
    find_package(Qt5 COMPONENTS Core Widgets Gui Xml Test QUIET)
    if(Qt5_FOUND)
        set(VA_QT Qt5)
    else()
        message(FATAL_ERROR
            "Qt 6 or Qt 5 is required (Core, Widgets, Gui, Xml, and Test).")
    endif()
endif()

add_library(va_core STATIC
    src/model/State.cpp
    src/model/Transition.cpp
    src/model/Automaton.cpp
    src/model/DFA.cpp
    src/model/NFA.cpp
    src/controller/AutomatonFactory.cpp
    src/simulation/DFAStrategy.cpp
    src/simulation/NFAStrategy.cpp
    src/simulation/ParallelTester.cpp
    src/adapters/JsonAdapter.cpp
    src/adapters/JffAdapter.cpp
)
target_include_directories(va_core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(va_core PUBLIC
    ${VA_QT}::Core
    ${VA_QT}::Xml
    Threads::Threads
)

add_library(va_view STATIC
    src/view/AutomatonScene.cpp
    src/view/AutomatonScene.h
    src/view/SimulationPanel.cpp
    src/view/SimulationPanel.h
    src/view/StateItem.cpp
    src/view/StateItem.h
    src/view/TransitionItem.cpp
    src/view/TransitionItem.h
    src/view/MainWindow.cpp
    src/view/MainWindow.h
)
target_include_directories(va_view PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(va_view PUBLIC
    va_core
    ${VA_QT}::Widgets
    ${VA_QT}::Gui
)

add_executable(VisualAutomata src/controller/main.cpp)
target_link_libraries(VisualAutomata PRIVATE va_view)

enable_testing()

function(add_va_test test_name test_source)
    add_executable(${test_name} ${test_source})
    target_link_libraries(${test_name} PRIVATE va_core ${VA_QT}::Test)
    add_test(NAME ${test_name} COMMAND ${test_name})
endfunction()

add_va_test(test_automaton tests/test_automaton.cpp)
add_va_test(test_dfa tests/test_dfa.cpp)
add_va_test(test_nfa tests/test_nfa.cpp)
add_va_test(test_strategies tests/test_strategies.cpp)
add_va_test(test_adapters tests/test_adapters.cpp)
add_va_test(test_factory tests/test_factory.cpp)

add_executable(test_view tests/test_view.cpp)
target_link_libraries(test_view PRIVATE va_view ${VA_QT}::Test)
add_test(NAME test_view COMMAND test_view)
set_tests_properties(test_view PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
