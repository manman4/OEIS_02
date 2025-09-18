# 読み込むファイル
filename = "b224012.txt"

# 取り出したい番号
targets = []
i = 1
cnt = 1
while i < 449
  targets << i
  i += cnt * 4
  cnt += 1
end
p targets

values = File.readlines(filename, chomp: true).map{|line|
  idx, val = line.split.map(&:to_i)
  [idx, val]
}.to_h

# 欲しい値を抽出
result = targets.map{|t| values[t]}

p result  # => [3, 36, 868, 28541, 1108969, 47735665, 2200064042, 106468976628, 5346274348879, 276427362685199, 14638913051219770, 790954334539356094, 43473056647367595435, 2424900281957535784114, 137007153893707984264455]
