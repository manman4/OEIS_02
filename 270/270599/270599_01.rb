def f(n)
  n - 1 + n % 2
end
def partition(n, min, max)
  return [[]] if n == 0
  [f(max), f(n)].min.step(min, -2).flat_map{|i| partition(n - i, min, i).map{|rest| [i, *rest]}}
end
def A270599(n)
  ary = [1]
  (2..n).each{|m|
    cnt = 0
    partition(m, 2, m).each{|ary|
      cnt += 1 if ary.inject(0){|s, i| s + 1 / i.to_r} == 1
    }
    ary << cnt
  }
  ary
end
cnt = 1
A270599(50).each{|i|
  print cnt
  print ' '
  p i
  cnt += 1
}
