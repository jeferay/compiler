int main() {
  int a = 1, sum = 0;
  {
    a = a + 2;
    int b = a + 3;
    b = b + 4;
    sum = sum + a + b;
    {
      b = b + 5;
      int c = b + 6;
      a = a + c;
      sum = sum + a + b + c;
      {
        b = b + a;
        int a = c + 7;
        a = a + 8;
        sum = sum + a + b + c;
        {
          b = b + a;
          int b = c + 9;
          a = a + 10;
          const int a = 11;
          b = b + 12;
          sum = sum + a + b + c;
          {
            c = c + b;
            int c = b + 13;
            c = c + a;
            sum = sum + a + b + c;
          }
          sum = sum - c;
        }
        sum = sum - b;
      }
      sum = sum - a;
    }
  }
  return sum % 77;
}

%entry:
  @a_0 = alloc i32
  store 1, @a_0
  @sum_0 = alloc i32
  store 0, @sum_0
  %0 = load @a_0
  %1 = add %0, 2
  store %1, @a_0
  @b_0 = alloc i32
  %2 = load @a_0
  %3 = add %2, 3
  store %3, @b_0
  %4 = load @b_0
  %5 = add %4, 4
  store %5, @b_0
  %6 = load @sum_0
  %7 = load @a_0
  %8 = add %6, %7
  %9 = load @b_0
  %10 = add %8, %9
  store %10, @sum_0
  %11 = load @b_0
  %12 = add %11, 5
  store %12, @b_0
  @c_0 = alloc i32
  %13 = load @b_0
  %14 = add %13, 6
  store %14, @c_0

  %15 = load @a_0
  %16 = load @c_0
  %17 = add %15, %16
  store %17, @a_0

  %18 = load @sum_0
  %19 = load @a_0
  %20 = add %18, %19
  %21 = load @b_0
  %22 = add %20, %21
  %23 = load @c_0
  %24 = add %22, %23
  store %24, @sum_0

  %25 = load @b_0
  %26 = load @a_0
  %27 = add %25, %26
  store %27, @b_0
  @a_1 = alloc i32
  %28 = load @c_0
  %29 = add %28, 7
  store %29, @a_1

  %30 = load @a_1
  %31 = add %30, 8
  store %31, @a_1
  %32 = load @sum_0
  %33 = load @a_1
  %34 = add %32, %33
  %35 = load @b_0
  %36 = add %34, %35
  %37 = load @c_0
  %38 = add %36, %37
  store %38, @sum_0

  %39 = load @b_0
  %40 = load @a_1
  %41 = add %39, %40
  store %41, @b_0

  @b_1 = alloc i32
  %42 = load @c_0
  %43 = add %42, 9
  store %43, @b_1
  %44 = load @a_1
  %45 = add %44, 10
  store %45, @a_1
  %46 = load @b_1
  %47 = add %46, 12
  store %47, @b_1

  %48 = load @sum_0
  %49 = add %48, 11
  %50 = load @b_1
  %51 = add %49, %50
  %52 = load @c_0
  %53 = add %51, %52
  store %53, @sum_0

  %54 = load @c_0
  %55 = load @b_1
  %56 = add %54, %55
  store %56, @c_0
  @c_1 = alloc i32
  %57 = load @b_1
  %58 = add %57, 13
  store %58, @c_1
  
  %59 = load @c_1
  %60 = add %59, 11
  store %60, @c_1
  %61 = load @sum_0
  %62 = add %61, 11
  %63 = load @b_1
  %64 = add %62, %63
  %65 = load @c_1
  %66 = add %64, %65
  store %66, @sum_0
  %67 = load @sum_0
  %68 = load @c_0
  %69 = sub %67, %68
  store %69, @sum_0
  %70 = load @sum_0
  %71 = load @b_0
  %72 = sub %70, %71
  store %72, @sum_0
  %73 = load @sum_0
  %74 = load @a_0
  %75 = sub %73, %74
  store %75, @sum_0
  %76 = load @sum_0
  %77 = mod %76, 77
  ret %77
}