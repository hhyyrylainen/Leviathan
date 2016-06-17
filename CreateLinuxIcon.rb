#!/bin/ruby
# Creates a .dekstop file for an executable.
# Usage: (Executable) [Icon file]
# If Icon file is not specified uses default leviathan icon
require 'fileutils'
require 'colorize'
require 'optparse'
require 'pathname'

require_relative 'Helpers/CommonCode'

# Abort if not in leviathan/ folder
verifyIsMainFolder

options = {}
OptionParser.new do |opts|
  opts.banner = "Usage: CreateLinuxIcon.rb [options]"

  opts.on("-v", "--[no-]verbose", "Run verbosely") do |v|
    options[:verbose] = v
  end
end.parse!
p options

if ARGV.count < 1
  onError "No file provided"
end

if ARGV.count > 2
  onError "Too many arguments provided"
end

targetExecutable = ARGV[0]

onError "Target file: " + targetExecutable + " doesn't exist" \
                                             if not File.exist? targetExecutable

if ARGV.count > 1
  useIcon = ARGV[1]
  info "Using icon #{useIcon}" if options[:verbose]
else
  useIcon = Path.join(CurrentDir, "LeviathanIcon.bmp")
end

onError "Icon file: " + useIcon + " doesn't exist" if not File.exist? useIcon

targetPath = Pathname.new(targetExecutable)
iconPath = Pathname.new(useIcon)


info "Creating  for #{targetExecutable} in folder #{targetPath.dirname}" if options[:verbose]


