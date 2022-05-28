fun @main(): i32 {
%entry:
  @a = alloc i32
  store 1, @a
  %0 = load @a
  ret %0
}

