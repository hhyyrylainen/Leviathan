# Common ruby functions for all the ruby scripts
require 'colorize'
require 'os'

# To get all possible colour values
#puts String.colors

# Error handling
def onError(errordescription)

  puts ("ERROR: " + errordescription).red
  exit 1
end

# Coloured output
def info(message)
  puts message.to_s.colorize(:light_blue)
end
def success(message)
  puts message.to_s.colorize(:light_green)
end
def warning(message)
  puts message.to_s.colorize(:light_yellow)
end

def verifyIsMainFolder
  
  onError "pwd invalid output" if not File.directory?(CurrentDir) 

  doxyFile = File.join(CurrentDir, "LeviathanDoxy.in")

  onError("Not ran from Leviathan base directory!") if not File.exist?(doxyFile)

end

# Common variables
CurrentDir = `pwd`.strip!

# Platform detection, for library suffix
if OS.linux?

  BuildPlatform = "linux"
  
elsif OS.windows?
  
  BuildPlatform = "windows"
  
elsif OS.mac?
  # Shouldn't be any file names that are different
  BuildPlatform = "linux"
else
  abort "Unknown OS type"
end


