macro(MacroAppend CurList ToAppend)
    set(${CurList} "${${CurList}};${ToAppend}")
endmacro()

function(FuncAppend CurList ToAppend)
    set(${CurList} "${${CurList}};${ToAppend}" PARENT_SCOPE)
endfunction()

set(Letters "Alpha;Beta")
MacroAppend(Letters "Gamma")
message("Letters contains: ${Letters}")

foreach(Letter IN LISTS Letters)
    message("Letter: ${Letter}")
endforeach()

FuncAppend(Letters "Delta")
message("Letters contains: ${Letters}")

foreach(Letter IN LISTS Letters)
    message("Letter: ${Letter}")
endforeach()

include(other_file.cmake)

OtherFileFunc("Variable yes")