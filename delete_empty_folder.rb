require 'fileutils'

def delete_empty_folders(path)
  Dir.foreach(path) do |entry|
    next if entry == '.' || entry == '..'

    entry_path = File.join(path, entry)
    if File.directory?(entry_path)
      delete_empty_folders(entry_path) # 再帰的にサブディレクトリを検索
      if Dir.entries(entry_path).size == 2 # エントリは . と .. のみ
        puts "削除: #{entry_path}"
        FileUtils.rm_rf(entry_path)
      end
    end
  end
end

# 削除を実行するディレクトリのパスを（慎重に）指定
directory_to_clean = './20X'
delete_empty_folders(directory_to_clean)