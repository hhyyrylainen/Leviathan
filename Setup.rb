#!/bin/ruby
# Setup script for Leviathan
# Downloads the assets and dependencies and then builds and installs them
require 'fileutils'
require 'colorize'
require 'etc'
require 'os'

# Setup code

CMakeBuildType = "RelWithDebInfo"
CompileThreads = Etc.nprocessors

BuildPlatform = "linux"

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

# Check that the directory is correct
info "Running in dir '#{CurrentDir}'"

# Check for correct folder
onError "pwd invalid output" if not File.directory?(CurrentDir) 

DoxyFile = File.join(CurrentDir, "LeviathanDoxy.in")

onError("Not ran from Leviathan base directory!") if not File.exist?(DoxyFile)

puts "Using #{CompileThreads} threads to compile, configuration: #{CMakeBuildType}"

# Download settings #
class BaseDep
  def initialize(name, foldername)

    @Name = name
    
    @Folder = File.join(CurrentDir, "..", foldername)
    @FolderName = foldername
    
  end

  def RequiresClone
    not File.exist?(@Folder)
  end
  
  def Retrieve
    info "Retrieving #{@Name}"

    Dir.chdir("..") do
      
      if self.RequiresClone
        
        info "Cloning #{@Name} into #{@Folder}"
        
        if not self.DoClone
          onError "Failed to clone repository"
        end
      end
    end

    onError "Retrieve Didn't create a folder for #{@Name} at #{@Folder}" if not File.exist?(@Folder)

    if not self.Update
      # Not fatal
      warning "Failed to update dependency #{@Name}"
    end
    
    success "Successfully retrieved #{@Name}"
  end

  def Update
    Dir.chdir(@Folder) do
      self.DoUpdate
    end
  end

  def Setup
    info "Setting up build files for #{@Name}"
    Dir.chdir(@Folder) do
      if not self.DoSetup
        onError "Setup failed for #{@Name}. Is a dependency missing? or some other cmake error?"
      end
    end
    success "Successfully created project files for #{@Name}"
  end
  
  def Compile
    info "Compiling #{@Name}"
    Dir.chdir(@Folder) do
      if not self.DoCompile
        onError "#{@Name} Failed to Compile. Are you using a broken versio? or has the setup process"+
                " changed between versions"
      end
    end
    success "Successfully compiled #{@Name}"
  end

  def Install
    info "Installing #{@Name}"
    Dir.chdir(@Folder) do
      if not self.DoInstall
        onError "#{@Name} Failed to install. Did you type in your sudo password?"
      end
    end
    success "Successfully installed #{@Name}"
  end
end

class Newton < BaseDep
  def initialize
    super("Newton Dynamics", "newton-dynamics")
  end

  def DoClone
    system "git clone https://github.com/MADEAPPS/newton-dynamics.git"
    $?.exitstatus == 0
  end

  def DoUpdate
    system "git checkout master"
    system "git pull origin master"
    $?.exitstatus == 0
  end

  def DoSetup
    FileUtils.mkdir_p "build"

    Dir.chdir("build") do
      system "cmake .. -DCMAKE_BUILD_TYPE=#{CMakeBuildType} -DNEWTON_DEMOS_SANDBOX=OFF"
    end
    
    $?.exitstatus == 0
  end
  
  def DoCompile

    Dir.chdir("build") do
      system "make -j #{CompileThreads}"
    end
    $?.exitstatus == 0
  end
  
  def DoInstall
    # Copy files to Leviathan folder
    libfolder = File.join(CurrentDir, "Newton", "lib")
    binfolder = File.join(CurrentDir, "Newton", "bin")
    
    FileUtils.mkdir_p libfolder
    FileUtils.mkdir_p binfolder
    
    if BuildPlatform == "linux"

      FileUtils.cp File.join(@Folder, "build/lib", "libNewton.so"), binfolder
      
    else
      abort "TODO: windows copy"
    end
    true
  end
end

class CAudio < BaseDep
  def initialize
    super("cAudio", "cAudio")
  end

  def DoClone
    system "git clone https://github.com/R4stl1n/cAudio.git"
    $?.exitstatus == 0
  end

  def DoUpdate
    system "git checkout master"
    system "git pull origin master"
    $?.exitstatus == 0
  end

  def DoSetup
    FileUtils.mkdir_p "build"

    Dir.chdir("build") do
      system "cmake .. -DCMAKE_BUILD_TYPE=#{CMakeBuildType} -DCAUDIO_BUILD_SAMPLES=OFF"
    end
    
    $?.exitstatus == 0
  end
  
  def DoCompile

    Dir.chdir("build") do
      system "make -j #{CompileThreads}"
    end
    $?.exitstatus == 0
  end
  
  def DoInstall
    Dir.chdir("build") do
      system "sudo make install"
    end
    $?.exitstatus == 0
  end
end

class AngelScript < BaseDep
  def initialize
    super("AngelScript", "angelscript")
    @WantedURL = "http://svn.code.sf.net/p/angelscript/code/tags/2.31.0"

    if @WantedURL[-1, 1] == '/'
      abort "Invalid configuraion in Setup.rb AngelScript tag has an ending '/'. Remove it!"
    end
  end

  def DoClone
    system "svn co #{@WantedURL} angelscript"
    $?.exitstatus == 0
  end

  def DoUpdate

    # Check is tag correct
    currenturl = `svn info | awk '$1 == "URL:" { print $2 }'`.strip!

    if currenturl != @WantedURL
      
      info "Switching AngelScript tag from #{currenturl} to #{@WantedURL}"
      
      system "svn switch #{@WantedURL}"
      onError "Failed to switch svn url" if $?.exitstatus > 0
    end
    
    system "svn update"
    $?.exitstatus == 0
  end

  def DoSetup
    true
  end
  
  def DoCompile

    if BuildPlatform == "linux"
      Dir.chdir("sdk/angelscript/projects/gnuc") do
      
        system "make -j #{CompileThreads}"
      
      end
      $?.exitstatus == 0
    else
      abort "TODO: windows AngelScript compile"
    end
  end
  
  def DoInstall

    # Copy files to Leviathan folder
    FileUtils.mkdir_p File.join(CurrentDir, "AngelScript", "include")
    FileUtils.mkdir_p File.join(CurrentDir, "AngelScript", "add_on")
    
    # First header files and addons
    FileUtils.cp File.join(@Folder, "sdk/angelscript/include", "angelscript.h"),
                 File.join(CurrentDir, "AngelScript", "include")

    addondir = File.join(CurrentDir, "AngelScript", "add_on")


    # All the addons from
    # `ls -m | awk 'BEGIN { RS = ","; ORS = ", "}; NF { print "\""$1"\""};'`
    addonnames = Array[
      "autowrapper", "contextmgr", "datetime", "debugger", "scriptany", "scriptarray",
      "scriptbuilder", "scriptdictionary", "scriptfile", "scriptgrid", "scripthandle",
      "scripthelper", "scriptmath", "scriptstdstring", "serializer", "weakref"
    ]

    addonnames.each do |x|

      FileUtils.copy_entry File.join(@Folder, "sdk/add_on/", x),
                   File.join(addondir, x)
    end

    
    # Then the library
    libfolder = File.join(CurrentDir, "AngelScript", "lib")
    
    FileUtils.mkdir_p libfolder
    
    if BuildPlatform == "linux"

      FileUtils.cp File.join(@Folder, "sdk/angelscript/lib", "libangelscript.a"), libfolder
      
    else
      FileUtils.cp File.join(@Folder, "sdk/angelscript/lib", "angelscript.lib"), libfolder
    end
    true
  end
end

##### Actual body ####

# Assets svn
info "Checking out assets svn"

Dir.chdir("bin") do

  # Check is it an svn repository
  system "svn info"
  
  if $?.exitstatus > 0
    info "Creating a working copy for assets svn"

    system "svn co https://subversion.assembla.com/svn/leviathan-assets/trunk ."
    onError "Failed to clone repository" if $?.exitstatus > 0
    
  end

  # Update to latest version (don't force in case there are local changes)
  #system "svn update"

  success "Updated Assets"
end

# All the objects
depobjects = Array[Newton.new, AngelScript.new, CAudio.new]

info "Retrieving dependencies"

depobjects.each do |x|

  x.Retrieve
  
end

success "Successfully retrieved all dependencies. Beginning compile"

info "Configuring dependencies"

depobjects.each do |x|

  x.Setup
  
end

success "Successfully configured all dependencies. Compiling"

depobjects.each do |x|

  x.Compile
  
end

success "Compilation succeeded"

info "Installing dependencies. Be prepared to type in sudo password"

depobjects.each do |x|

  x.Install
  
end

success "Installing completed"

info "Dependencies done, configuring Leviathan"


success "All done."
info "To compile run 'make' in ./build"
exit 0



