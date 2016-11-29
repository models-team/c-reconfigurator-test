require 'open3'

def time_diff_milli(start, finish)
   #((finish - start) * 1000.0).round(2)
   (finish - start) * 1000
end

def median(array)
  sorted = array.sort
  len = sorted.length
  (sorted[(len - 1) / 2] + sorted[len / 2]) / 2.0
end

def command_median (command)
	status = 0
	times = Array.new 
	(1..50).each do |n|
		t1 = Time.now
		stdout,stderr,status = Open3.capture3(command)
		t2 = Time.now
		msecs = time_diff_milli t1, t2
		times << msecs
	end

	sprintf '%.0f', median(times)
end

def run_command (command)
	stdout,stderr,status = Open3.capture3(command)
	output = stdout
	if (stderr.length != 0)
		output = output + "\nERROR:\n" + stderr
	end
	return output
end

puts "\n"*20
puts "Run real test"

run_command("rm -rf variant")
run_command("rm -rf target")

run_command("mkdir -p variant/libssh/0a4ea19")
puts " ===> " + command = "clang-3.5 -E -I source/libssh/0a4ea19 -o variant/libssh/0a4ea19/pki.c source/libssh/0a4ea19/pki.c"
puts "\n" + run_command(command)
puts
puts " ===> " + command =
	"java -Xms2048m -Xmx10240m -Xss128m -jar reconfigurator.jar" +
	" -source /home/alex/reconfigurator/c-reconfigurator-test/source/libssh/0a4ea19/pki.c" +
	" -target /home/alex/reconfigurator/c-reconfigurator-test/target/libssh/0a4ea19/pki.c" +
	" -oracle /home/alex/reconfigurator/c-reconfigurator-test/oracle/libssh/0a4ea19/pki.c" +
	" -hdFile /home/alex/reconfigurator/c-reconfigurator-test/source/libssh/0a4ea19/parsefix.h" +
	" -hdFile /home/alex/reconfigurator/c-reconfigurator-test/source/libssh/0a4ea19/libssh/libcrypto.h" +
	" -hdFile /home/alex/reconfigurator/c-reconfigurator-test/source/libssh/0a4ea19/libssh/libgcrypt.h" +
	" -hdFile /home/alex/reconfigurator/c-reconfigurator-test/source/libssh/0a4ea19/libssh/libssh.h" +
	" -hdFile /home/alex/reconfigurator/c-reconfigurator-test/source/libssh/0a4ea19/libssh/pki.h" +
	" -hdFile /home/alex/reconfigurator/c-reconfigurator-test/source/libssh/0a4ea19/libssh/pki_priv.h" +
	" -hdFile /home/alex/reconfigurator/c-reconfigurator-test/source/libssh/0a4ea19/libssh/priv.h" +
	" -undef __CYGWIN__" +
	" -undef _WIN32" +
	" -undef __SUNPRO_C"
puts "\n" + run_command(command)
puts
puts " ===> " + command = "clang-3.5 -c -g -emit-llvm -Wall -I source/libssh/0a4ea19/ -o target/libssh/0a4ea19/pki.bc target/libssh/0a4ea19/pki.c"
puts "\n" + run_command(command)
puts




# if (ARGV[0] != nil)
# 	if (@files_H.keys.include?(ARGV[0]))
# 		file = ARGV[0]
# 		puts
# 		puts "--------------------------------------------------------------"
# 		puts "  TESTING " + file
# 		puts
# 		puts "--------------------------------------------------------------"
# 		puts "  C-RECONFIGURATOR"
# 		puts
# 		run_command("mkdir -p variant/#{@files_H[file]}#{file}")
# 		puts " ===> " + command = "clang-3.5 -E#{@variant_config_H[file]} -I source/#{@files_H[file]}#{file} -o #{variant(file)} #{source(file)}"
# 		puts "\n" + run_command(command)
# 		puts
# 		run_command("mkdir -p target/#{@files_H[file]}#{file}")
# 		puts " ===> " + command = "java -Xms2048m -Xmx10240m -Xss128m -jar reconfigurator.jar -source #{Dir.pwd}/#{source(file)} -target #{Dir.pwd}/#{target(file)} -oracle #{Dir.pwd}/#{oracle(file)} -include #{Dir.pwd}/source/#{@files_H[file]}#{file}"
# 		puts "\n" + run_command(command)
# 		puts
# 		puts "--------------------------------------------------------------"
# 		puts "  SIZES"
# 		puts
# 		puts "  source size: " + run_command("stat --printf=\"%s\" #{source(file)}") + "B"
# 		puts "  target size: " + run_command("stat --printf=\"%s\" #{target(file)}") + "B"
# 		puts
# 		puts "--------------------------------------------------------------"
# 		puts "  FRAMA-C"
# 		puts
# 		puts " ===> " + command = "frama-c -val -quiet #{variant(file)}"
# 		puts "\n" + run_command(command)
# 		puts
# 		puts " ===> " + command = "frama-c -val -quiet #{target(file)}"
# 		puts "\n" + run_command(command)
# 		puts
# 		puts "--------------------------------------------------------------"
# 		puts "  CLANG"
# 		puts
# 		puts " ===> " + command = "clang-3.5 -c -g -emit-llvm -Wall -o #{variantBC(file)} #{variant(file)}"
# 		puts "\n" + run_command(command)
# 		puts
# 		puts " ===> " + command = "clang-3.5 -c -g -emit-llvm -Wall -o #{targetBC(file)} #{target(file)}"
# 		puts "\n" + run_command(command)
# 		puts
# 		puts "--------------------------------------------------------------"
# 		puts "  LLBMC"
# 		puts
# 		puts " ===> " + command = "llbmc #{@llbmc_args_H[file]} #{variantBC(file)}"
# 		puts "\n" + run_command(command)
# 		puts
# 		puts " ===> " + command = "llbmc #{@llbmc_args_H[file]} #{targetBC(file)}"
# 		puts "\n" + run_command(command)
# 	else
# 		puts "file not found"
# 	end
# else
# 	id = 0
# 	puts " ID  | HASH    | file size (B)   | frama-c (ms)    | clang (ms)      | llbmc (ms)      |"
# 	puts "     |         | source | target | var    | target | var    | target | var    | target |"
# 	puts "----------------------------------------------------------------------------------------"
# 	for file in @files_H.keys
# 		id = id + 1
# 		print id.to_s.rjust(4, ' ') + " |"
# 		print file.rjust(8, ' ') + " |"
		
# 		run_command("mkdir -p variant/#{@files_H[file]}#{file}")
# 		run_command("clang-3.5 -E#{@variant_config_H[file]} -I source/#{@files_H[file]}#{file} -o #{variant(file)} #{source(file)}")

# 		run_command("mkdir -p target/#{@files_H[file]}#{file}")
# 		run_command("java -Xms2048m -Xmx10240m -Xss128m -jar reconfigurator.jar -source #{Dir.pwd}/#{source(file)} -target #{Dir.pwd}/#{target(file)} -oracle #{Dir.pwd}/#{oracle(file)} -I source/#{@files_H[file]}#{file}")

# 		print run_command("stat --printf=\"%s\" #{source(file)}").rjust(7, ' ') + " |"
# 		print run_command("stat --printf=\"%s\" #{target(file)}").rjust(7, ' ') + " |"

# 		print command_median("frama-c -val -quiet #{variant(file)}").rjust(7, ' ') + " |"
# 		print command_median("frama-c -val -quiet #{target(file)}").rjust(7, ' ') + " |"
		
# 		print command_median("clang-3.5 -c -g -emit-llvm -Wall -o #{variantBC(file)} #{variant(file)}").rjust(7, ' ') + " |"
# 		print command_median("clang-3.5 -c -g -emit-llvm -Wall -o #{targetBC(file)} #{target(file)}").rjust(7, ' ') + " |"

# 		print command_median("llbmc #{@llbmc_args_H[file]} #{variantBC(file)}").rjust(7, ' ') + " |"
# 		print command_median("llbmc #{@llbmc_args_H[file]} #{targetBC(file)}").rjust(7, ' ') + " |"

# 		puts
# 	end
# end

puts "End test"