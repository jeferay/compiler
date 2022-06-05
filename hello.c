int main() {
  int a = 1, b = a + 2;
  {
    b = a + b;
    const int a = 2;
    b = a + b;
    {
      b = a + b;
      int a = b + b;
      {
        int a = 3;
        b = b + a;
        {
          b = b + a;
          {
            int a = 4;
            b = b + a;
            {
              b = b + a;
              {
                int b = 1;
              }
              {{{{b = b + a;}}}}
              {
                return b;
              }
              return 0;
              b = b + a;
              {
                int b = 2;
                return b;
              }
            }
          }
        }
      }
    }
    int b = 1;
  }
}