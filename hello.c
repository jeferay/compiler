int main() {
  int a = 0;
  return a;
}
/*
fun @main(): i32 {
%entry:
  %0 = eq 6, 0
  %1 = sub 0, %0
  %2 = sub 0, %1
  ret %2
}

*/

/*
  .text
  .globl main
main:
  # 实现 eq 6, 0 的操作, 并把结果存入 t0
  li    t0, 6
  xor   t0, t0, x0
  seqz  t0, t0
  # 减法
  sub   t1, x0, t0
  # 减法
  sub   t2, x0, t1
  # 设置返回值并返回
  mv    a0, t2
  ret
*/