include(FetchContent)

if(TARGET asmjit)
    message(STATUS "asmjit already supplied as subdependency!")
else()
    set(FETCHCONTENT_QUIET OFF)

    set(ASMJIT_STATIC TRUE)

    FetchContent_MakeAvailable(asmjit)
endif()