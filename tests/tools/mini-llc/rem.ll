@format = private global [4 x i8] c"%d\0A\00"

declare i32 @printf(ptr, ...)

define i32 @urem_1(i32 %0) noinline {
1:
  %2 = urem i32 %0, 1
  ret i32 %2
}

define i32 @urem_2(i32 %0) noinline {
1:
  %2 = urem i32 %0, 2
  ret i32 %2
}

define i32 @urem_3(i32 %0) noinline {
1:
  %2 = urem i32 %0, 3
  ret i32 %2
}

define i32 @urem_4(i32 %0) noinline {
1:
  %2 = urem i32 %0, 4
  ret i32 %2
}

define i32 @urem_100(i32 %0) noinline {
1:
  %2 = urem i32 %0, 100
  ret i32 %2
}

define i32 @urem_255(i32 %0) noinline {
1:
  %2 = urem i32 %0, 255
  ret i32 %2
}

define i32 @urem_256(i32 %0) noinline {
1:
  %2 = urem i32 %0, 256
  ret i32 %2
}

define i32 @urem_4294967295(i32 %0) noinline {
1:
  %2 = urem i32 %0, 4294967295
  ret i32 %2
}

define i32 @srem_1(i32 %0) noinline {
1:
  %2 = srem i32 %0, 1
  ret i32 %2
}

define i32 @srem_2(i32 %0) noinline {
1:
  %2 = srem i32 %0, 2
  ret i32 %2
}

define i32 @srem_3(i32 %0) noinline {
1:
  %2 = srem i32 %0, 3
  ret i32 %2
}

define i32 @srem_4(i32 %0) noinline {
1:
  %2 = srem i32 %0, 4
  ret i32 %2
}

define i32 @srem_100(i32 %0) noinline {
1:
  %2 = srem i32 %0, 100
  ret i32 %2
}

define i32 @srem_255(i32 %0) noinline {
1:
  %2 = srem i32 %0, 255
  ret i32 %2
}

define i32 @srem_256(i32 %0) noinline {
1:
  %2 = srem i32 %0, 256
  ret i32 %2
}

define i32 @srem_2147483647(i32 %0) noinline {
1:
  %2 = srem i32 %0, 2147483647
  ret i32 %2
}

define i32 @srem_n1(i32 %0) noinline {
1:
  %2 = srem i32 %0, -1
  ret i32 %2
}

define i32 @srem_n2(i32 %0) noinline {
1:
  %2 = srem i32 %0, -2
  ret i32 %2
}

define i32 @srem_n3(i32 %0) noinline {
1:
  %2 = srem i32 %0, -3
  ret i32 %2
}

define i32 @srem_n4(i32 %0) noinline {
1:
  %2 = srem i32 %0, -4
  ret i32 %2
}

define i32 @srem_n100(i32 %0) noinline {
1:
  %2 = srem i32 %0, -100
  ret i32 %2
}

define i32 @srem_n255(i32 %0) noinline {
1:
  %2 = srem i32 %0, -255
  ret i32 %2
}

define i32 @srem_n256(i32 %0) noinline {
1:
  %2 = srem i32 %0, -256
  ret i32 %2
}

define i32 @srem_n2147483648(i32 %0) noinline {
1:
  %2 = srem i32 %0, -2147483648
  ret i32 %2
}

define i32 @main() {
0:
  %1 = call i32 @urem_1(i32 12345)
  %2 = call i32 @printf(ptr @format, i32 %1)
  %3 = call i32 @urem_2(i32 12345)
  %4 = call i32 @printf(ptr @format, i32 %3)
  %5 = call i32 @urem_3(i32 12345)
  %6 = call i32 @printf(ptr @format, i32 %5)
  %7 = call i32 @urem_4(i32 12345)
  %8 = call i32 @printf(ptr @format, i32 %7)
  %9 = call i32 @urem_100(i32 12345)
  %10 = call i32 @printf(ptr @format, i32 %9)
  %11 = call i32 @urem_255(i32 12345)
  %12 = call i32 @printf(ptr @format, i32 %11)
  %13 = call i32 @urem_256(i32 12345)
  %14 = call i32 @printf(ptr @format, i32 %13)
  %15 = call i32 @urem_4294967295(i32 12345)
  %16 = call i32 @printf(ptr @format, i32 %15)
  %17 = call i32 @srem_1(i32 12345)
  %18 = call i32 @printf(ptr @format, i32 %17)
  %19 = call i32 @srem_2(i32 12345)
  %20 = call i32 @printf(ptr @format, i32 %19)
  %21 = call i32 @srem_3(i32 12345)
  %22 = call i32 @printf(ptr @format, i32 %21)
  %23 = call i32 @srem_4(i32 12345)
  %24 = call i32 @printf(ptr @format, i32 %23)
  %25 = call i32 @srem_100(i32 12345)
  %26 = call i32 @printf(ptr @format, i32 %25)
  %27 = call i32 @srem_255(i32 12345)
  %28 = call i32 @printf(ptr @format, i32 %27)
  %29 = call i32 @srem_256(i32 12345)
  %30 = call i32 @printf(ptr @format, i32 %29)
  %31 = call i32 @srem_2147483647(i32 12345)
  %32 = call i32 @printf(ptr @format, i32 %31)
  %33 = call i32 @srem_n1(i32 12345)
  %34 = call i32 @printf(ptr @format, i32 %33)
  %35 = call i32 @srem_n2(i32 12345)
  %36 = call i32 @printf(ptr @format, i32 %35)
  %37 = call i32 @srem_n3(i32 12345)
  %38 = call i32 @printf(ptr @format, i32 %37)
  %39 = call i32 @srem_n4(i32 12345)
  %40 = call i32 @printf(ptr @format, i32 %39)
  %41 = call i32 @srem_n100(i32 12345)
  %42 = call i32 @printf(ptr @format, i32 %41)
  %43 = call i32 @srem_n255(i32 12345)
  %44 = call i32 @printf(ptr @format, i32 %43)
  %45 = call i32 @srem_n256(i32 12345)
  %46 = call i32 @printf(ptr @format, i32 %45)
  %47 = call i32 @srem_n2147483648(i32 12345)
  %48 = call i32 @printf(ptr @format, i32 %47)
  ret i32 0
}
