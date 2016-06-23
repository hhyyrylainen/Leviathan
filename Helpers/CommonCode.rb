# Common ruby functions for all the ruby scripts
require 'fileutils'
require 'colorize'
require 'os'
# Used by: verifyVSProjectRuntimeLibrary
require 'nokogiri' if OS.windows?
# Required for installs on windows
require 'win32ole' if OS.windows?

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
def error(message)
  puts message.to_s.colorize(:red)
end

def verifyIsMainFolder
  
  onError "pwd invalid output" if not File.directory?(CurrentDir) 

  doxyFile = File.join(CurrentDir, "LeviathanDoxy.in")

  onError("Not ran from Leviathan base directory!") if not File.exist?(doxyFile)

end

# Common variables
CurrentDir = Dir.pwd

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

# Run visual studio environment configure .bat file
def bringVSToPath()
  if not File.exist? "#{ENV[VSToolsEnv]}VsMSBuildCmd.bat"
    onError "VsMSBuildCMD.bat is missing check is VSToolsEnv variable correct in Setup.rb" 
  end
  "call \"#{ENV[VSToolsEnv]}VsMSBuildCmd.bat\""
end

# Makes sure that the wanted value is specified for all targets that match the regex
def verifyVSProjectRuntimeLibrary(projFile, matchRegex, wantedRuntimeLib)
  # Very parameters
  abort "Call verifyVSProjectRuntimeLibrary only on windows!" if not OS.windows?
  onError "Project file: #{projFile} doesn't exist" if not File.exist? projFile
  
  # Load xml with nokogiri
  doc = File.open(projFile) { |f| Nokogiri::XML(f) }
  
  doc.css("Project ItemDefinitionGroup").each do |group|
    if not matchRegex.match group['Condition'] 
      next
    end
    
    info "Checking that project target '#{group['Condition']}' Has RuntimeLibrary of type #{wantedRuntimeLib}"
    
    libType = group.at_css("ClCompile RuntimeLibrary")
    
    if not libType
      warning "Couldn't verify library type. Didn't find RuntimeLibrary node"
      next
    end
    
    if libType.content != wantedRuntimeLib
      
      onError "In file '#{projFile}' target '#{group['Condition']}' "+
        "Has RuntimeLibrary of type #{libType.content} which is not #{wantedRuntimeLib}. "+
        "Please open the visual studio solution in the folder and modify the Runtime Library to be #{wantedRuntimeLib}." +
        "If you don't know how google: 'visual studio set project runtime library'"
    end
  end
  
  success "All targets had correct runtime library types"
end
        
# CMake configure
def runCMakeConfigure(additionalArgs)
    
  if BuildPlatform == "linux"
        
    system "cmake .. -DCMAKE_BUILD_TYPE=#{CMakeBuildType} #{additionalArgs}"
        
  else
        
    system "cmake .. -G \"#{VSVersion}\" #{additionalArgs}"
        
  end
end

# Running make or msbuild
def runCompiler(threads)
    
  if BuildPlatform == "linux"
        
    system "make -j #{threads}"
        
  else
    
    #system "start \"ms\" \"MSBuild.exe\" "
    # Would use this if used project.sln file: /target:ALL_BUILD 
    system "#{bringVSToPath} && MSBuild.exe ALL_BUILD.vcxproj /maxcpucount:#{threads} /p:Configuration=RelWithDebInfo"
        
  end
end

def runWindowsAdmin(cmd)
    shell = WIN32OLE.new('Shell.Application')
    
    shell.ShellExecute("ruby.exe", 
      "\"#{CurrentDir}/Helpers/WinInstall.rb\" " +
      "\"#{cmd.gsub( '"', '\\"')}\"", 
      "#{Dir.pwd}", 'runas')
        
    # TODO: find a proper way to wait here
    info "Please wait while the install script runs and then press any key to continue"
    system "pause"
end

def askToRunAdmin(cmd)
    puts "."
    puts "."
    info "You need to open a new cmd window as administrator and run the following command: "
    info cmd
    info "Sorry, windows is such a pain in the ass"
    system "pause"
end

# Running platform standard cmake install
def runInstall()
    
  if BuildPlatform == "linux"
        
    system "sudo make install"
        
  else
    
    info "Running install script as Administrator"

    # Requires admin privileges
    runWindowsAdmin("#{bringVSToPath} && MSBuild.exe INSTALL.vcxproj /p:Configuration=RelWithDebInfo")

  end
end
