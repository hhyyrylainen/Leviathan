#!/bin/ruby
# Setup script for Leviathan
# Downloads the assets and dependencies and then builds and installs them
require 'fileutils'
require 'colorize'
require 'etc'


require_relative 'Helpers/CommonCode'

# Setup code

CMakeBuildType = "RelWithDebInfo"
CompileThreads = Etc.nprocessors

# If set to true will install CEGUI editor
InstallCEED = false

# If set to false won't install libs that need sudo
DoSudoInstalls = true

# If false won't get breakpad
GetBreakpad = true

# Doesn't get the resources for samples into leviathan/bin if set to false
FetchAssets = true

# Check that the directory is correct
info "Running in dir '#{CurrentDir}'"

# Check for correct folder
verifyIsMainFolder

puts "Using #{CompileThreads} threads to compile, configuration: #{CMakeBuildType}"

# Path helper
# For breakpad depot tools
class PathModifier
  def initialize(newpathentry)
    @OldPath = ENV["PATH"]

    abort "Failed to get env path" if @OldPath == nil

    if BuildPlatform == "linux"
      
      newpath = newpathentry + ":" + @OldPath
      
      info "Setting path to: #{newpath}"
      ENV["PATH"] = newpath
      
    else

      newpath = @OldPath + ";" + newpathentry
      
      info "Setting path to: #{newpath}"
      ENV["PATH"] = newpath
      
    end
  end

  def Restore()
    info "Restored old path"
    ENV["PATH"] = @OldPath
  end
end

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
    includefolder = File.join(CurrentDir, "Newton", "include")
    
    FileUtils.mkdir_p libfolder
    FileUtils.mkdir_p binfolder
    FileUtils.mkdir_p includefolder

    FileUtils.cp File.join(@Folder, "coreLibrary_300/source/newton", "Newton.h"), includefolder
    
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

    return true if not DoSudoInstalls
    
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

class Breakpad < BaseDep
  def initialize
    super("Google Breakpad", "breakpad")
    @DepotFolder = File.join(CurrentDir, "..", "depot_tools")
    @CreatedNewFolder = false
  end

  def RequiresClone
    if File.exist?(@DepotFolder) and File.exist?(@Folder)
      return false
    end
    
    true
  end
  
  def DoClone

    # Depot tools
    system "git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git"
    return false if $?.exitstatus > 0

    if not File.exist?(@Folder)
      
      FileUtils.mkdir_p @Folder
      @CreatedNewFolder = true
      
    end
    
    true
  end

  def DoUpdate

    # Update depot tools
    Dir.chdir(@DepotFolder) do
      system "git checkout master"
      system "git pull origin master"
    end

    if $?.exitstatus > 0
      return false
    end

    if not @CreatedNewFolder
      Dir.chdir(@Folder) do
        # The first source subdir is the git repository
        Dir.chdir("src") do
          system "git checkout master"
          system "git pull origin master"
          system "gclient sync"
        end
      end
    end
    
    true
  end

  def DoSetup

    # Setup depot tools
    if BuildPlatform == "windows"
      abort "Verify breakpad installation on windows: " +
            "https://chromium.googlesource.com/breakpad/breakpad/"

      Dir.chdir(@DepotFolder) do
        system "gclient.exe"
      end
      onError "Initial gclient dependency install run on windows failed" if $?.exitstatus > 0
    end

    if not @CreatedNewFolder
      return true
    end
    

    # Bring the depot tools to path
    pathedit = PathModifier.new(@DepotFolder)

    # Get source for breakpad
    Dir.chdir(@Folder) do

      system "fetch breakpad"

      if $?.exitstatus > 0
        pathedit.Restore
        onError "fetch breakpad failed"
      end
      
      Dir.chdir("src") do

        # Configure script
        system "./configure"

        if $?.exitstatus > 0
          pathedit.Restore
          onError "configure breakpad failed" 
        end
      end
    end

    pathedit.Restore
    true
  end
  
  def DoCompile

    # Bring the depot tools to path
    pathedit = PathModifier.new(@DepotFolder)

    # Build breakpad
    Dir.chdir(File.join(@Folder, "src")) do

      system "make -j #{CompileThreads}"
      
      if $?.exitstatus > 0
        pathedit.Restore
        onError "breakpad build failed" 
      end
    end

    
    pathedit.Restore
    true
  end
  
  def DoInstall

    # Create target folders
    FileUtils.mkdir_p File.join(CurrentDir, "Breakpad", "lib")
    FileUtils.mkdir_p File.join(CurrentDir, "Breakpad", "bin")

    breakpadincludelink = File.join(CurrentDir, "Breakpad", "include")
    
    # Need to delete old file before creating a new symlink
    File.delete(breakpadincludelink) if File.exist?(breakpadincludelink)
    FileUtils.ln_s File.join(@Folder, "src/src"), breakpadincludelink
    
    if BuildPlatform == "windows"

      abort "TODO: Breakpad install"
    end

    FileUtils.cp File.join(@Folder, "src/src/client/linux", "libbreakpad_client.a"),
                 File.join(CurrentDir, "Breakpad", "lib")

    FileUtils.cp File.join(@Folder, "src/src/tools/linux/dump_syms", "dump_syms"),
                 File.join(CurrentDir, "Breakpad", "bin")

    FileUtils.cp File.join(@Folder, "src/src/processor", "minidump_stackwalk"),
                 File.join(CurrentDir, "Breakpad", "bin")
    
    true
  end
end

class Ogre < BaseDep
  def initialize
    super("Ogre", "ogre")
  end

  def DoClone

    system "hg clone https://bitbucket.org/sinbad/ogre"
    $?.exitstatus == 0
  end

  def DoUpdate
    system "hg pull"
    system "hg update v2-0"
    $?.exitstatus == 0
  end

  def DoSetup

    FileUtils.mkdir_p "build"

    Dir.chdir("build") do
      system "cmake .. -DCMAKE_BUILD_TYPE=#{CMakeBuildType} " +
             "-DOGRE_BUILD_RENDERSYSTEM_GL3PLUS=ON -DOGRE_BUILD_COMPONENT_OVERLAY=OFF " +
             "-DOGRE_BUILD_COMPONENT_PAGING=OFF -DOGRE_BUILD_COMPONENT_PROPERTY=OFF " +
             "-DOGRE_BUILD_COMPONENT_TERRAIN=OFF -DOGRE_BUILD_COMPONENT_VOLUME=OFF "+
             "-DOGRE_BUILD_PLUGIN_BSP=OFF -DOGRE_BUILD_PLUGIN_CG=OFF " +
             "-DOGRE_BUILD_PLUGIN_OCTREE=OFF -DOGRE_BUILD_PLUGIN_PCZ=OFF -DOGRE_BUILD_SAMPLES=OFF"
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

    return true if not DoSudoInstalls
    
    Dir.chdir("build") do
      system "sudo make install"
    end
    $?.exitstatus == 0
  end
end

# Depends on Ogre to be installed
class CEGUI < BaseDep
  def initialize
    super("CEGUI", "cegui")
  end

  def DoClone

    system "hg clone https://bitbucket.org/cegui/cegui"
    $?.exitstatus == 0
  end

  def DoUpdate
    system "hg pull"
    system "hg update default"
    $?.exitstatus == 0
  end

  def DoSetup

    FileUtils.mkdir_p "build"

    if InstallCEED
      python = "ON"
    else
      python = "OFF"
    end

    Dir.chdir("build") do
      # Use UTF-8 strings with CEGUI (string class 1)
      system "cmake .. -DCMAKE_BUILD_TYPE=#{CMakeBuildType} -DCEGUI_STRING_CLASS=1 " +
             "-DCEGUI_BUILD_APPLICATION_TEMPLATES=OFF -DCEGUI_BUILD_PYTHON_MODULES=#{python} " +
             "-DCEGUI_SAMPLES_ENABLED=ON"
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

    return true if not DoSudoInstalls
    
    Dir.chdir("build") do
      system "sudo make install"
    end
    $?.exitstatus == 0
  end
end

class SFML < BaseDep
  def initialize
    super("SFML", "SFML")
  end

  def DoClone
    system "git clone https://github.com/SFML/SFML.git"
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
      system "cmake .. -DCMAKE_BUILD_TYPE=#{CMakeBuildType}"
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

    return true if not DoSudoInstalls
    
    Dir.chdir("build") do
      system "sudo make install"
    end
    $?.exitstatus == 0
  end

  def LinuxPackages
    if Linux == "Fedora"
      return Array["xcb-util-image-devel", "systemd-devel", "libjpeg-devel", "libvorbis-devel",
                   "flac-devel"]
    else
      onError "LinuxPackages not done for this linux system"
    end
  end
end


##### Actual body ####

# Assets svn
if FetchAssets
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
    system "svn update"

    success "Updated Assets"
  end
end

# All the objects
depobjects = Array[Newton.new, AngelScript.new, CAudio.new, SFML.new, Ogre.new, CEGUI.new]

if GetBreakpad
  # Add this last as this does some environment variable trickery
  depobjects.push Breakpad.new
end


info "Retrieving dependencies"

depobjects.each do |x|

  x.Retrieve
  
end

success "Successfully retrieved all dependencies. Beginning compile"

info "Configuring dependencies"

depobjects.each do |x|

  x.Setup
  x.Compile
  x.Install
  
end

info "Dependencies done, configuring Leviathan"

FileUtils.mkdir_p "build"

Dir.chdir("build") do

  system "cmake .. -DCMAKE_BUILD_TYPE=#{CMakeBuildType} -DCREATE_SDK=ON " +
         "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DUSE_BREAKPAD=OFF"
  
end

if $?.exitstatus > 0
  onError "Failed to configure Leviathan. Are you using a broken version, or did a dependency fail "+
          "to install?"
end
  
# Create a symbolic link for build database
builddblink = "compile_commands.json"
File.delete(builddblink) if File.exist?(builddblink)
FileUtils.ln_s File.join("build", "compile_commands.json"), builddblink

if BuildPlatform == "linux"
  
  info "Indexing with cscope"
  system "./RunCodeIndexing.sh"
  
end

success "All done."
info "To compile run 'make' in ./build"
exit 0



