if [[ ! -d out ]]; then
  mkdir out
fi

extra_files=""

if [ $1 == "01" ]; then
  extra_files+=" deps/hashmap.c/hashmap.c"
fi

clang day$1/main.c $extra_files \
  -g \
  -Ideps \
  -Ideps/CTypes \
  -fsanitize=address \
  -o out/day$1
