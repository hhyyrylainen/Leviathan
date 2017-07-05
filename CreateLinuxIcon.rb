#!/bin/ruby
# Creates a .dekstop file for an executable.
# Usage: (Executable) [Icon file]
# If Icon file is not specified uses default leviathan icon
require 'fileutils'
require 'colorize'
require 'optparse'
require 'pathname'

require_relative 'RubySetupSystem/RubyCommon.rb'

# Abort if not in leviathan/ folder
doxyFile = "LeviathanDoxy.in"

onError("Not ran from Leviathan base directory!") if not File.exist?(doxyFile)

CurrentDir = Dir.pwd

options = {}
OptionParser.new do |opts|
  opts.banner = "Usage: CreateLinuxIcon.rb [options]"

  opts.on("-v", "--[no-]verbose", "Run verbosely") do |v|
    options[:verbose] = v
  end
  opts.on("-n", "--name [NAME]", "Specify name to use in .desktop") do |name|
    options[:name] = name
  end
  opts.on("-p", "--prefix [PREFIX]", "Prefix to add to icon") do |prefix|
    options[:prefix] = prefix
  end
  opts.on("-t", "--terminal", "Specify Terminal in .dekstop") do |term|
    options[:terminal] = term
  end
end.parse!

options[:terminal] = false if not options[:terminal]

if ARGV.count < 1
  onError "No file provided"
end

if ARGV.count > 2
  onError "Too many arguments provided"
end

onError "No name provided" if not options[:name]

targetExecutable = ARGV[0]

onError "Target file: " + targetExecutable + " doesn't exist" \
                                             if not File.exist? targetExecutable

if ARGV.count > 1
  useIcon = ARGV[1]
  info "Using icon #{useIcon}" if options[:verbose]
else
  useIcon = File.join(CurrentDir, "LeviathanIcon.bmp")
end

onError "Icon file: " + useIcon + " doesn't exist" if not File.exist? useIcon

targetPath = Pathname.new(targetExecutable)
iconPath = Pathname.new(useIcon)

if iconPath.extname != ".png"

  info "Icon of extension '#{iconPath.extname}' will be converted to .png" if options[:verbose]
  doConvert = true
end

if options[:verbose]
  info "Creating .desktop for #{targetExecutable} in folder #{targetPath.dirname}"
end

targetDesktop = File.join(targetPath.dirname, "#{targetPath.basename}.desktop")

finalIconFile = File.join(targetPath.dirname,
                          "#{options[:prefix]}#{iconPath.basename(iconPath.extname)}.png")

if not File.exist? finalIconFile

  if doConvert

    if options[:verbose]
      info "Converting icon #{useIcon} -> #{finalIconFile}"
    end
    
    system "convert \"#{useIcon}[0]\" \"#{finalIconFile}\""
    onError "Failed to convert icon file to png" if $?.exitstatus > 0
  else
    FileUtils.cp useIcon, finalIconFile
  end
end

contents = %{
[Desktop Entry]
Encoding=UTF-8
Version=1.0
Type=Application
Terminal=#{options[:terminal].to_s}
Exec=./#{targetPath.basename}
Name=#{options[:name]}
Icon=./#{File.basename(finalIconFile)}
Path=./
}

File.write(targetDesktop, contents)

if options[:verbose]
  success "Created #{targetDesktop}"
end
exit 0

       


