set(SOURCE_FILES
  basic_rules.cpp
  and_rule.cpp
  or_rule.cpp
  callback.cpp
  discard.cpp
  catch_main.cpp
  catch.hpp
  )


add_executable(unittests ${SOURCE_FILES})
target_link_libraries(unittests PUBLIC cppeg)
