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

	if (status == 0)
		sprintf '%.0f', median(times)
	else
		"ERR"
	end
end

def run_command (command)
	stdout,stderr,status = Open3.capture3(command)
	output = stdout
	if (stderr.length != 0)
		output = output + "\nERROR:\n" + stderr
	end
	return output
end


@files_H = Hash.new("")
@files_H["76baeeb"] = "vbdb/linux/"

@variant_config_H = Hash.new("")
@variant_config_H["76baeeb"] = "-D CONFIG_X86_32 -D CONFIG_NUMA -D CONFIG_PCI"

@llbmc_args_H = Hash.new("")
@llbmc_args_H["76baeeb"] = "-ignore-missing-function-bodies --no-max-loop-iterations-checks"

def folder (folder, file, ext)
	"#{folder}/#{@files_H[file]}#{file}/#{file}#{ext}"
end

def source (file)
	folder("source", file, ".c")
end

def target (file)
	folder("target", file, ".c")
end

def variant (file)
	folder("variant", file, ".c")
end

def oracle (file)
	folder("oracle", file, ".c")
end

def variantBC (file)
	folder("variant", file, ".bc")
end

def oracleBC (file)
	folder("oracle", file, ".bc")
end

puts "\n"*20
puts "Run test"

if (ARGV[0] != nil)
	if (@files_H.keys.include?(ARGV[0]))
		file = ARGV[0]
		puts
		puts "--------------------------------------------------------------"
		puts "  TESTING " + file
		puts
		puts "  source size: " + run_command("stat --printf=\"%s\" #{source(file)}") + "B"
		puts "  oracle size: " + run_command("stat --printf=\"%s\" #{oracle(file)}") + "B"
		puts
		run_command("mkdir -p variant/#{@files_H[file]}#{file}")
		puts "  " + command = "clang -E #{@variant_config_H[file]} -o #{variant(file)} #{source(file)}"
		puts "\n" + run_command(command)
		puts
		run_command("mkdir -p target/#{@files_H[file]}#{file}")
		puts "  " + command = "java -jar reconfigurator.jar -source #{source(file)} -target #{target(file)} -oracle #{oracle(file)}"
		puts "\n" + run_command(command)
		puts
		puts "--------------------------------------------------------------"
		puts "  FRAMA-C"
		puts
		puts "  " + command = "frama-c -val -quiet #{variant(file)}"
		puts "\n" + run_command(command)
		puts
		puts "  " + command = "frama-c -val -quiet #{oracle(file)}"
		puts "\n" + run_command(command)
		puts
		puts "--------------------------------------------------------------"
		puts "  CLANG"
		puts
		puts "  " + command = "clang -c -g -emit-llvm -Wall -o #{variantBC(file)} #{variant(file)}"
		puts "\n" + run_command(command)
		puts
		puts "  " + command = "clang -c -g -emit-llvm -Wall -o #{oracleBC(file)} #{oracle(file)}"
		puts "\n" + run_command(command)
		puts
		puts "--------------------------------------------------------------"
		puts "  LLBMC"
		puts
		puts "  " + command = "llbmc #{@llbmc_args_H[file]} #{variantBC(file)}"
		puts "\n" + run_command(command)
		puts
		puts "  " + command = "llbmc #{@llbmc_args_H[file]} #{oracleBC(file)}"
		puts "\n" + run_command(command)
	else
		puts "file not found"
	end
else
	id = 0
	puts " ID  | HASH    | file size (B)   | frama-c (ms)    | clang (ms)      | llbmc (ms)      |"
	puts "     |         | source | oracle | var    | oracle | var    | oracle | var    | oracle |"
	puts "----------------------------------------------------------------------------------------"
	for file in @files_H.keys
		id = id + 1
		print id.to_s.rjust(4, ' ') + " |"
		print file.rjust(8, ' ') + " |"
		
		run_command("mkdir -p variant/#{@files_H[file]}#{file}")
		run_command("clang -E #{@variant_config_H[file]} -o #{variant(file)} #{source(file)}")

		run_command("mkdir -p target/#{@files_H[file]}#{file}")
		run_command("java -jar reconfigurator.jar -source #{source(file)} -target #{target(file)} -oracle #{oracle(file)}")

		print run_command("stat --printf=\"%s\" #{source(file)}").rjust(7, ' ') + " |"
		print run_command("stat --printf=\"%s\" #{oracle(file)}").rjust(7, ' ') + " |"

		print command_median("frama-c -val -quiet #{variant(file)}").rjust(7, ' ') + " |"
		print command_median("frama-c -val -quiet #{oracle(file)}").rjust(7, ' ') + " |"
		
		print command_median("clang -c -g -emit-llvm -Wall -o #{variantBC(file)} #{variant(file)}").rjust(7, ' ') + " |"
		print command_median("clang -c -g -emit-llvm -Wall -o #{oracleBC(file)} #{oracle(file)}").rjust(7, ' ') + " |"

		print command_median("llbmc #{@llbmc_args_H[file]} #{variantBC(file)}").rjust(7, ' ') + " |"
		print command_median("llbmc #{@llbmc_args_H[file]} #{oracleBC(file)}").rjust(7, ' ') + " |"

		puts
	end
end

# for file in Dir.glob("./simple/*.c").sort
# 	puts file

# 	name = File.basename(file, ".c")
# 	# Dir.mkdir name
# 	# Dir.mkdir name + "/variant-err"
# 	# Dir.mkdir name + "/source-err"
# 	# Dir.mkdir name + "/target-err"
# 	# system "cp simple/#{name}.c #{name}/source-err/"
# 	# system "cp simple-target/#{name}.c #{name}/target-err/"

# 	# command = "clang -E -include varconfig.h #{name}/source-err/#{name}.c -o #{name}/variant-err/#{name}.c"
# 	# stdout,stderr,status = Open3.capture3(command)
# 	# puts ":> " + command
# 	# if (status != 0)
# 	# 	puts ":> ERROR"
# 	# end

# 	command_median("frama variant: ", "frama-c -val #{name}/variant-err/#{name}.c")
	
# 	command_median("frama reconfd: ", "frama-c -val #{name}/target-err/#{name}.c")

	
# 	command_median("clang variant: ", "clang -c -g -emit-llvm -Wall -include varconfig.h #{name}/source-err/#{name}.c -o #{name}/variant-err/#{name}.bc")

# 	command_median("clang reconfd: ", "clang -c -g -emit-llvm -Wall -include recconfig.h #{name}/target-err/#{name}.c -o #{name}/target-err/#{name}.bc")

	

# 	llbmc_H = Hash.new("llbmc -ignore-missing-function-bodies --no-max-loop-iterations-checks [&NAME]/variant-err/[&NAME].bc")
# 	llbmc_H["0988c4c"] = "llbmc --max-loop-iterations=100 0988c4c/variant-err/0988c4c.bc"
# 	llbmc_H["0dc77b6"] = "llbmc --max-loop-iterations=100 0dc77b6/variant-err/0dc77b6.bc"
# 	llbmc_H["0f8f809"] = "llbmc --max-loop-iterations=100 0f8f809/variant-err/0f8f809.bc"
# 	llbmc_H["1c17e4d"] = "llbmc --ignore-missing-function-bodies --max-loop-iterations=100 1c17e4d/variant-err/1c17e4d.bc"
# 	llbmc_H["1f758a4"] = "llbmc --max-loop-iterations=100 1f758a4/variant-err/1f758a4.bc"
# 	llbmc_H["208d898"] = "llbmc -ignore-missing-function-bodies --no-max-loop-iterations-checks 208d898/variant-err/208d898.bc"
# 	llbmc_H["218ad12"] = "llbmc -leak-check --max-loop-iterations=100 218ad12/variant-err/218ad12.bc"
# 	llbmc_H["221ac32"] = "llbmc -ignore-missing-function-bodies --no-max-loop-iterations-checks 221ac32/variant-err/221ac32.bc"
# 	llbmc_H["30e0532"] = "llbmc -ignore-missing-function-bodies --no-max-loop-iterations-checks 30e0532/variant-err/30e0532.bc"
# 	llbmc_H["36855dc"] = "llbmc -ignore-missing-function-bodies --no-max-loop-iterations-checks 36855dc/variant-err/36855dc.bc"
# 	llbmc_H["472a474"] = "llbmc -ignore-missing-function-bodies --no-max-loop-iterations-checks 472a474/variant-err/472a474.bc"
# 	llbmc_H["60e233a"] = "llbmc -ignore-missing-function-bodies --no-max-loop-iterations-checks 60e233a/variant-err/60e233a.bc"
# 	llbmc_H["6252547"] = "llbmc -ignore-missing-function-bodies --no-max-loop-iterations-checks 6252547/variant-err/6252547.bc"
# 	llbmc_H["63878ac"] = "llbmc -no-custom-assertions -ignore-missing-function-bodies --no-max-loop-iterations-checks 63878ac/variant-err/63878ac.bc"
# 	llbmc_H["657e964"] = "llbmc -ignore-missing-function-bodies --no-max-loop-iterations-checks 657e964/variant-err/657e964.bc"
# 	llbmc_H["76baeeb"] = "llbmc -ignore-missing-function-bodies --no-max-loop-iterations-checks 76baeeb/variant-err/76baeeb.bc"
# 	llbmc_H["7acf6cd"] = "llbmc -ignore-missing-function-bodies --no-max-loop-iterations-checks 7acf6cd/variant-err/7acf6cd.bc"
# 	llbmc_H["8c82962"] = "llbmc -ignore-missing-function-bodies --no-max-loop-iterations-checks 8c82962/variant-err/8c82962.bc"
# 	llbmc_H["91ea820"] = "llbmc -ignore-missing-function-bodies --no-max-loop-iterations-checks 91ea820/variant-err/91ea820.bc"
# 	llbmc_H["ae249b5"] = "llbmc -ignore-missing-function-bodies --no-max-loop-iterations-checks ae249b5/variant-err/ae249b5.bc"
# 	llbmc_H["eb91f1d"] = "llbmc -ignore-missing-function-bodies --no-max-loop-iterations-checks eb91f1d/variant-err/eb91f1d.bc"
# 	llbmc_H["f3d83e2"] = "llbmc -ignore-missing-function-bodies --no-max-loop-iterations-checks f3d83e2/variant-err/f3d83e2.bc"
# 	llbmc_H["f7ab9b4"] = "llbmc -ignore-missing-function-bodies --no-max-loop-iterations-checks f7ab9b4/variant-err/f7ab9b4.bc"



# 	command_median("llbmc variant: ", llbmc_H[name].gsub("[&NAME]", name))

	

# 	llbmc_H = Hash.new("llbmc -ignore-missing-function-bodies --no-max-loop-iterations-checks [&NAME]/target-err/[&NAME].bc")
# 	llbmc_H["0988c4c"] = "llbmc --max-loop-iterations=100 0988c4c/target-err/0988c4c.bc"
# 	llbmc_H["0dc77b6"] = "llbmc --max-loop-iterations=100 0dc77b6/target-err/0dc77b6.bc"
# 	llbmc_H["0f8f809"] = "llbmc --max-loop-iterations=100 0f8f809/target-err/0f8f809.bc"
# 	llbmc_H["1c17e4d"] = "llbmc --ignore-missing-function-bodies --max-loop-iterations=100 1c17e4d/target-err/1c17e4d.bc"
# 	llbmc_H["1f758a4"] = "llbmc --max-loop-iterations=100 1f758a4/target-err/1f758a4.bc"
# 	llbmc_H["208d898"] = "llbmc -ignore-missing-function-bodies --no-max-loop-iterations-checks 208d898/target-err/208d898.bc"
# 	llbmc_H["218ad12"] = "llbmc -leak-check --max-loop-iterations=100 218ad12/target-err/218ad12.bc"
# 	llbmc_H["221ac32"] = "llbmc -ignore-missing-function-bodies --no-max-loop-iterations-checks 221ac32/target-err/221ac32.bc"
# 	llbmc_H["30e0532"] = "llbmc -ignore-missing-function-bodies --no-max-loop-iterations-checks 30e0532/target-err/30e0532.bc"
# 	llbmc_H["36855dc"] = "llbmc -ignore-missing-function-bodies --no-max-loop-iterations-checks 36855dc/target-err/36855dc.bc"
# 	llbmc_H["472a474"] = "llbmc -ignore-missing-function-bodies --no-max-loop-iterations-checks 472a474/target-err/472a474.bc"
# 	llbmc_H["60e233a"] = "llbmc -ignore-missing-function-bodies --no-max-loop-iterations-checks 60e233a/target-err/60e233a.bc"
# 	llbmc_H["6252547"] = "llbmc -ignore-missing-function-bodies --no-max-loop-iterations-checks 6252547/target-err/6252547.bc"
# 	llbmc_H["63878ac"] = "llbmc -no-custom-assertions -ignore-missing-function-bodies --no-max-loop-iterations-checks 63878ac/target-err/63878ac.bc"
# 	llbmc_H["657e964"] = "llbmc -ignore-missing-function-bodies --no-max-loop-iterations-checks 657e964/target-err/657e964.bc"
# 	llbmc_H["76baeeb"] = "llbmc -ignore-missing-function-bodies --no-max-loop-iterations-checks 76baeeb/target-err/76baeeb.bc"
# 	llbmc_H["7acf6cd"] = "llbmc -ignore-missing-function-bodies --no-max-loop-iterations-checks 7acf6cd/target-err/7acf6cd.bc"
# 	llbmc_H["8c82962"] = "llbmc -ignore-missing-function-bodies --no-max-loop-iterations-checks 8c82962/target-err/8c82962.bc"
# 	llbmc_H["91ea820"] = "llbmc -ignore-missing-function-bodies --no-max-loop-iterations-checks 91ea820/target-err/91ea820.bc"
# 	llbmc_H["ae249b5"] = "llbmc -ignore-missing-function-bodies --no-max-loop-iterations-checks ae249b5/target-err/ae249b5.bc"
# 	llbmc_H["eb91f1d"] = "llbmc -ignore-missing-function-bodies --no-max-loop-iterations-checks eb91f1d/target-err/eb91f1d.bc"
# 	llbmc_H["f7ab9b4"] = "llbmc -ignore-missing-function-bodies --no-max-loop-iterations-checks f7ab9b4/target-err/f7ab9b4.bc"

# 	command_median("llbmc reconfd: ", llbmc_H[name].gsub("[&NAME]", name))

# 	puts
# end

puts "End test"