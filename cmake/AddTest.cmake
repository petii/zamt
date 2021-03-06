
function(SetTestMode target_name)
  target_compile_definitions(${target_name} PRIVATE ZAMT_TEST)
  target_compile_definitions(${target_name} PRIVATE TEST)
endfunction(SetTestMode)

function(GetLibForTests used_modules)
  set(lib_for_tests testlib)
  foreach(libname ${used_modules})
    string(CONCAT lib_for_tests ${lib_for_tests} _ ${libname})
  endforeach(libname)
  list(FIND compiled_libraries ${lib_for_tests} lib_found)
  if(lib_found EQUAL -1)
    AddLib(${lib_for_tests} "${used_modules}")
    SetTestMode(${lib_for_tests})
    list(APPEND compiled_libraries ${lib_for_tests})
  endif()
  set(lib_for_tests ${lib_for_tests} PARENT_SCOPE)
  set(compiled_libraries ${compiled_libraries} PARENT_SCOPE)
endfunction(GetLibForTests)

function(AddTest test_name test_module other_modules test_sources)
  set(used_modules ${test_module} ${other_nodules})
  GetLibForTests("${used_modules}")
  unset(cpp_sources)
  foreach(cpp ${test_sources})
    set(cpp_sources ${cpp_sources} ${test_module}/test/${cpp})
  endforeach(cpp)
  source_group(${test_module} FILES ${cpp_sources})
  add_executable(${test_name} ${cpp_sources})
  CollectSources("${used_modules}")
  SetupTarget(${test_name} "${used_modules}")
  SetTestMode(${test_name})
  target_include_directories(${test_name} SYSTEM PRIVATE ${collected_includes})
  target_link_libraries(${test_name} ${lib_for_tests})
  LinkTarget(${test_name} "${collected_libs}")
  add_test(NAME ${test_name} COMMAND ${test_name})
  set(compiled_libraries ${compiled_libraries} PARENT_SCOPE)
endfunction(AddTest)

function(AddAllTests tested_modules)
  unset(compiled_libraries)
  foreach(mod ${tested_modules})
    include(${mod}/tests.cmake)
  endforeach(mod)
endfunction(AddAllTests)
