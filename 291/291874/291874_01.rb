def I(ary, n)
  a = [1]
  (0..n - 1).each{|i| a << -(0..i).inject(0){|s, j| s + ary[1 + i - j] * a[j]}}
  a
end

ary =
[1,1,1,1,2,3,4,6,9,14,21,31,47,71,107,161,243,367,
 553,834,1258,1898,2863,4318,6514,9827,14824,22361,
 33732,50886,76762,115796,174680,263509,397508,
 599647,904579,1364576,2058489,3105269,4684359,
 7066449,10659877,16080632,24257950,36593598,
 55202165,83273553,125619799,189499952]
p I(ary, 49)

# a =
# [1,1,0,0,1,0,0,1,0,1,1,0,2,1,1,3,2,3,4,4,6,7,8,11,
#  13,16,20,24,31,37,46,58,70,88,108,133,167,204,252,
#  315,386,479,594,731,909,1122,1386,1720,2124,2628,
#  3254,4022,4980,6160,7618,9432,11665,14433,17860,
#  22093,27341,33824,41847,51785,64065,79267]
# p a.map{|i| -i}