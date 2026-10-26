# writes the file IN as the c array NAME and its size NAME_size to OUT
file(READ ${IN} hex HEX)
file(SIZE ${IN} size)
string(REGEX REPLACE "(..)" "0x\\1," bytes "${hex}")
string(REGEX REPLACE "((0x..,){32})" "\\1\n" bytes "${bytes}")
file(WRITE ${OUT} "const char ${NAME}[] = {\n${bytes}0};\nconst long ${NAME}_size = ${size};\n")
