#!/bin/sh

# $1 -> symbols name
# $2 -> resource_file
# $3 -> output_file

if [[ $# -ne  3 ]]
then
    echo "wrong arguments"
    exit
fi

cat <<EOF > "$3"

.text

.global __binary_$1_start
.global __binary_$1_end
.global __binary_$1_size

.align 4
__binary_$1_start:
    .incbin "$2"
__binary_$1_end:
__binary_$1_size:
    .int __binary_$1_end - __binary_$1_start

EOF
