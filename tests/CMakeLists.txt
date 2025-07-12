find_package(Catch2 2.13.10 REQUIRED)

add_executable(test_HelloWorldApp
        test_HelloWorldApp.cpp
)
target_link_libraries(test_HelloWorldApp
        PRIVATE
        Catch2::Catch2
)

add_test(NAME HelloWorldAppTests COMMAND test_HelloWorldApp)
