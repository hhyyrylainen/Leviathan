#!/bin/ruby
# Setup script for Leviathan
# Downloads the assets and dependencies and then builds and installs them
# TODO: remove awk usage
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

# If true dependencies won't be updated from remote repositories
SkipPullUpdates = false

# If true will only setup / update dependencies and skip Leviathan
OnlyDependencies = false

# If true skips all dependencies and only tries to configure Leviathan
OnlyLeviathan = false

# If true will create folder OgreBuild with an extra cmake file that includes the dependencies
# and then the Ogre files
WindowsOgreExtraCMake = true

# If true new version of depot tools and breakpad won't be fetched on install
NoBreakpadUpdateOnWindows = false

# On windows visual studio will be automatically opened if required
AutoOpenVS = true

# Visual studio version on windows, required for forced 64 bit builds
VSVersion = "Visual Studio 14 2015 Win64"
VSToolsEnv = "VS140COMNTOOLS"

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
        onError "#{@Name} Failed to Compile. Are you using a broken version? or has the setup process"+
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
  
    if BuildPlatform == "windows"
      
      return File.exist? "packages/projects/visualStudio_2015_dll/build.sln"
    else
      FileUtils.mkdir_p "build"

      Dir.chdir("build") do
    
        runCMakeConfigure "-DNEWTON_DEMOS_SANDBOX=OFF"
        return $?.exitstatus == 0
      end
    end      
  end
  
  def DoCompile
    if BuildPlatform == "windows"
      cmdStr = "#{bringVSToPath} && MSBuild.exe \"packages/projects/visualStudio_2015_dll/build.sln\" " +
        "/maxcpucount:#{CompileThreads} /p:Configuration=release /p:Platform=\"x64\""
      system cmdStr
      return $?.exitstatus == 0
    else
      Dir.chdir("build") do
        
        runCompiler CompileThreads
      
      end
      return $?.exitstatus == 0
    end
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
      
      basePath = "coreLibrary_300/projects/windows/project_vs2015_dll/x64/newton/release"
    
      FileUtils.cp File.join(@Folder, basePath, "newton.dll"), binfolder
      FileUtils.cp File.join(@Folder, basePath, "newton.lib"), libfolder
    end
    true
  end
end

class OpenAL < BaseDep
  def initialize
    super("OpenAL Soft", "openal-soft")
  end

  def DoClone
    onError "Use OpenAL from package manager on linux" if BuildPlatform != "windows"
    system "git clone https://github.com/kcat/openal-soft.git"
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
      
      runCMakeConfigure "-DALSOFT_UTILS=OFF -DALSOFT_EXAMPLES=OFF -DALSOFT_TESTS=OFF"
    end
    
    $?.exitstatus == 0
  end
  
  def DoCompile

    Dir.chdir("build") do
      runCompiler CompileThreads
    end
    $?.exitstatus == 0
  end
  
  def DoInstall
    return true if not DoSudoInstalls
    
    Dir.chdir("build") do
      runInstall
      
      if BuildPlatform == "windows" and not File.exist? "C:/Program Files/OpenAL/include/OpenAL"
        # cAudio needs OpenAL folder in include folder, which doesn't exist. 
        # So we create it here
        askToRunAdmin("mklink /D \"C:/Program Files/OpenAL/include/OpenAL\" " + 
          "\"C:/Program Files/OpenAL/include/AL\"")
      end
    end
    $?.exitstatus == 0
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
      
      if BuildPlatform == "windows"
        # The bundled ones aren't compatible with our compiler setup 
        # -DCAUDIO_DEPENDENCIES_DIR=../Dependencies64
        runCMakeConfigure "-DCAUDIO_BUILD_SAMPLES=OFF -DCAUDIO_DEPENDENCIES_DIR=\"C:/Program Files/OpenAL\" " +
          "-DCMAKE_INSTALL_PREFIX=./Install"
      else
        runCMakeConfigure "-DCAUDIO_BUILD_SAMPLES=OFF"
      end
    end
    
    $?.exitstatus == 0
  end
  
  def DoCompile

    Dir.chdir("build") do
      runCompiler CompileThreads
    end
    $?.exitstatus == 0
  end
  
  def DoInstall

    return true if not DoSudoInstalls
    
    Dir.chdir("build") do
      if BuildPlatform == "windows"
      
        system "#{bringVSToPath} && MSBuild.exe INSTALL.vcxproj /p:Configuration=RelWithDebInfo"
        
        # And then to copy the libs
        
        FileUtils.mkdir_p File.join(CurrentDir, "cAudio")
        FileUtils.mkdir_p File.join(CurrentDir, "cAudio", "lib")
        FileUtils.mkdir_p File.join(CurrentDir, "cAudio", "bin")
        
        FileUtils.cp File.join(@Folder, "build/bin/RelWithDebInfo", "cAudio.dll"),
                 File.join(CurrentDir, "cAudio", "bin")

        FileUtils.cp File.join(@Folder, "build/lib/RelWithDebInfo", "cAudio.lib"),
                 File.join(CurrentDir, "cAudio", "lib")
        
        FileUtils.copy_entry File.join(@Folder, "build/Install/", "include"),
                 File.join(CurrentDir, "cAudio", "include")
        
      else
        runInstall
      end
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
    if BuildPlatform == "linux"
    
      return File.exist? "sdk/angelscript/projects/msvc2015/angelscript.sln"
    else
      return true
    end
  end
  
  def DoCompile

    if BuildPlatform == "linux"
      Dir.chdir("sdk/angelscript/projects/gnuc") do
      
        system "make -j #{CompileThreads}"
      
      end
      $?.exitstatus == 0
    else
      
      info "Verifying that angelscript solution has Runtime Library = MultiThreadedDLL"
      verifyVSProjectRuntimeLibrary "sdk/angelscript/projects/msvc2015/angelscript.vcxproj", 
        %r{Release\|x64}, "MultiThreadedDLL"  
        
      success "AngelScript solution is correctly configured. Compiling"
      
      cmdStr = "#{bringVSToPath} && MSBuild.exe \"sdk/angelscript/projects/msvc2015/angelscript.sln\" " +
        "/maxcpucount:#{CompileThreads} /p:Configuration=Release /p:Platform=\"x64\""
      system cmdStr
      return $?.exitstatus == 0
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
      FileUtils.cp File.join(@Folder, "sdk/angelscript/lib", "angelscript64.lib"), libfolder
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
  
    if BuildPlatform == "windows" and NoBreakpadUpdateOnWindows
      info "Windows: skipping Breakpad update"
      if not File.exist?("src")
        @CreatedNewFolder = true
      end
      return true
    end

    # Update depot tools
    Dir.chdir(@DepotFolder) do
      system "git checkout master"
      system "git pull origin master"
    end

    if $?.exitstatus > 0
      return false
    end

    if not @CreatedNewFolder
    
      if not File.exist?("src")
        # This is set to true if we created an empty folder but we didn't get to the pull stage
        @CreatedNewFolder = true
      else
        Dir.chdir(@Folder) do
          # The first source subdir is the git repository
          Dir.chdir("src") do
            system "git checkout master"
            system "git pull origin master"
            system "gclient sync"
          end
        end
      end
    end
    
    true
  end

  def DoSetup
    
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
        if BuildPlatform == "windows"
          system "src/tools/gyp/gyp.bat src/client/windows/breakpad_client.gyp –no-circular-check"
        else
          system "./configure"
        end
        
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
      
      if BuildPlatform == "linux"
        system "make -j #{CompileThreads}"
      
        if $?.exitstatus > 0
          pathedit.Restore
          onError "breakpad build failed" 
        end
      else
        
        info "Please open the solution at and compile breakpad client in Release and x64. " +
          "Remember to disable treat warnings as errors first: "+
          "#{CurrentDir}/../breakpad/src/src/client/windows/breakpad_client.sln"
        
        system "start #{CurrentDir}/../breakpad/src/src/client/windows/breakpad_client.sln" if AutoOpenVS
        system "pause"
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
    
    if BuildPlatform == "windows"

      askToRunAdmin "mklink /D \"#{breakpadincludelink}\" \"#{File.join(@Folder, "src/src")}\""
      
      FileUtils.copy_entry File.join(@Folder, "src/src/client/windows/Release/lib"),
                   File.join(CurrentDir, "Breakpad", "lib")
                   
                   
                   
      # Might be worth it to have windows symbols dumbed on windows, if the linux dumber can't deal with pdbs
      #FileUtils.cp File.join(@Folder, "src/src/tools/linux/dump_syms", "dump_syms"),
      #             File.join(CurrentDir, "Breakpad", "bin")
                   
    else
    
      # Need to delete old file before creating a new symlink
      File.delete(breakpadincludelink) if File.exist?(breakpadincludelink)
      FileUtils.ln_s File.join(@Folder, "src/src"), breakpadincludelink
    
      FileUtils.cp File.join(@Folder, "src/src/client/linux", "libbreakpad_client.a"),
                   File.join(CurrentDir, "Breakpad", "lib")

      FileUtils.cp File.join(@Folder, "src/src/tools/linux/dump_syms", "dump_syms"),
                   File.join(CurrentDir, "Breakpad", "bin")

      FileUtils.cp File.join(@Folder, "src/src/processor", "minidump_stackwalk"),
                   File.join(CurrentDir, "Breakpad", "bin")
    end
    true
  end
end

class Ogre < BaseDep
  def initialize
    if BuildPlatform == "windows" and WindowsOgreExtraCMake
      super("Ogre", "OgreBuild")
    else
      super("Ogre", "ogre")
    end
  end

  def RequiresClone
    if BuildPlatform == "windows"
      if WindowsOgreExtraCMake
        FileUtils.mkdir_p "OgreBuild"
        return (not File.exist?(@Folder) or not File.exist?(File.join(@Folder, "Dependencies")) or 
          not File.exist?(File.join(@Folder, "ogre")))
      else
        return (not File.exist?(@Folder) or not File.exist?(File.join(@Folder, "Dependencies")))
      end
    else
      return (not File.exist? @Folder)
    end
  end
  
  def DoClone
    if BuildPlatform == "windows"
      
      if WindowsOgreExtraCMake
        Dir.chdir(@Folder) do
          system "hg clone https://bitbucket.org/sinbad/ogre"
          if $?.exitstatus > 0
            return false
          end
      
          system "hg clone https://bitbucket.org/cabalistic/ogredeps Dependencies"
        end
      else
        system "hg clone https://bitbucket.org/sinbad/ogre"
        if $?.exitstatus > 0
          return false
        end
        
        Dir.chdir(@Folder) do

          system "hg clone https://bitbucket.org/cabalistic/ogredeps Dependencies"
        end
      end
      return $?.exitstatus == 0
    else
      system "hg clone https://bitbucket.org/sinbad/ogre"
      return $?.exitstatus == 0
    end
  end

  def DoUpdate
  
    if BuildPlatform == "windows"
        Dir.chdir("Dependencies") do
          system "hg pull"
          system "hg update"
          
          if $?.exitstatus > 0
            return false
          end
        end
        
        if WindowsOgreExtraCMake
          Dir.chdir("ogre") do
            system "hg pull"
            system "hg update v2-0"
          end
          if $?.exitstatus > 0
            return false
          end
          return true
        end
    end
  
    system "hg pull"
    system "hg update v2-0"
    $?.exitstatus == 0
  end

  def DoSetup

    FileUtils.mkdir_p "build"
    
    Dir.chdir("build") do
    
        if BuildPlatform == "windows" and WindowsOgreExtraCMake
          ogreWinCmake = %{
          
            cmake_minimum_required(VERSION 2.8.11)

            # Dependencies first
            add_subdirectory("Dependencies")

            # actual ogre
            add_subdirectory("ogre")
          }
          
           File.open("../CMakeLists.txt", 'w') { |file| file.write(ogreWinCmake) }
        end

      runCMakeConfigure "-DOGRE_BUILD_RENDERSYSTEM_GL3PLUS=ON " +
             "-DOGRE_BUILD_RENDERSYSTEM_D3D9=OFF -DOGRE_BUILD_RENDERSYSTEM_D3D11=OFF "+
             "-DOGRE_BUILD_COMPONENT_OVERLAY=OFF " +
             "-DOGRE_BUILD_COMPONENT_PAGING=OFF -DOGRE_BUILD_COMPONENT_PROPERTY=OFF " +
             "-DOGRE_BUILD_COMPONENT_TERRAIN=OFF -DOGRE_BUILD_COMPONENT_VOLUME=OFF "+
             "-DOGRE_BUILD_PLUGIN_BSP=OFF -DOGRE_BUILD_PLUGIN_CG=OFF " +
             "-DOGRE_BUILD_PLUGIN_OCTREE=OFF -DOGRE_BUILD_PLUGIN_PCZ=OFF -DOGRE_BUILD_SAMPLES=OFF"
    end
    
    $?.exitstatus == 0
  end
  
  def DoCompile

    Dir.chdir("build") do
      runCompiler CompileThreads
    end
    $?.exitstatus == 0
  end
  
  def DoInstall

    return true if not DoSudoInstalls
    Dir.chdir("build") do
    
      if BuildPlatform == "windows"

        system "#{bringVSToPath} && MSBuild.exe INSTALL.vcxproj /p:Configuration=RelWithDebInfo"
        ENV["OGRE_HOME"] = "#{@Folder}/build/ogre/sdk"
        
      else
        runInstall
      end
    end

    $?.exitstatus == 0
  end
end

# Windows only CEGUI dependencies
class CEGUIDependencies < BaseDep
  def initialize
    super("CEGUI Dependencies", "cegui-dependencies")
  end

  def DoClone

    system "hg clone https://bitbucket.org/cegui/cegui-dependencies"
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
      runCMakeConfigure "-DCEGUI_BUILD_PYTHON_MODULES=#{python} "
    end
    
    $?.exitstatus == 0
  end
  
  def DoCompile

    Dir.chdir("build") do
      runCompiler CompileThreads 
    end
    $?.exitstatus == 0
  end
  
  def DoInstall

    FileUtils.copy_entry File.join(@Folder, "build", "dependencies"),
                 File.join(CurrentDir, "../cegui", "dependencies")
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
      runCMakeConfigure "-DCEGUI_STRING_CLASS=1 " +
             "-DCEGUI_BUILD_APPLICATION_TEMPLATES=OFF -DCEGUI_BUILD_PYTHON_MODULES=#{python} " +
             "-DCEGUI_SAMPLES_ENABLED=ON"
    end
    
    $?.exitstatus == 0
  end
  
  def DoCompile

    Dir.chdir("build") do
      runCompiler CompileThreads 
    end
    $?.exitstatus == 0
  end
  
  def DoInstall

    return true if not DoSudoInstalls
    
    Dir.chdir("build") do
      runInstall
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
      runCMakeConfigure ""
    end
    
    $?.exitstatus == 0
  end
  
  def DoCompile

    Dir.chdir("build") do
      runCompiler CompileThreads
    end
    $?.exitstatus == 0
  end
  
  def DoInstall

    return true if not DoSudoInstalls
    
    Dir.chdir("build") do
      runInstall
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
if FetchAssets and not SkipPullUpdates
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
if BuildPlatform == "windows"
  depobjects = Array[Newton.new, AngelScript.new, OpenAL.new, CAudio.new, SFML.new, Ogre.new, 
    CEGUIDependencies.new, CEGUI.new]
else
  depobjects = Array[Newton.new, AngelScript.new, CAudio.new, SFML.new, Ogre.new, CEGUI.new]
end

if GetBreakpad
  # Add this last as this does some environment variable trickery
  depobjects.push Breakpad.new
end


if not SkipPullUpdates and not OnlyLeviathan
  info "Retrieving dependencies"

  depobjects.each do |x|

    x.Retrieve
  
  end

  success "Successfully retrieved all dependencies. Beginning compile"
end

if not OnlyLeviathan

  info "Configuring dependencies"

  depobjects.each do |x|

    x.Setup
    x.Compile
    x.Install
  
  end

  if OnlyDependencies
    success "All done. Skipping Configuring Leviathan"
    exit 0
  end
  info "Dependencies done, configuring Leviathan"

end

if BuildPlatform == "windows"
  # Make sure Ogre home is set
  if WindowsOgreExtraCMake
    ENV["OGRE_HOME"] = "#{CurrentDir}/../OgreBuild/build/ogre/sdk"
  else
    ENV["OGRE_HOME"] = "#{CurrentDir}/../ogre/build/sdk"
  end
end

FileUtils.mkdir_p "build"

Dir.chdir("build") do

  system "cmake .. -DCMAKE_BUILD_TYPE=#{CMakeBuildType} -DCREATE_SDK=ON " +
         "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DUSE_BREAKPAD=OFF"
  
end

if $?.exitstatus > 0
  onError "Failed to configure Leviathan. Are you using a broken version, or did a dependency fail "+
          "to install?"
end
  

if BuildPlatform == "linux"
  # Create a symbolic link for build database
  builddblink = "compile_commands.json"
  File.delete(builddblink) if File.exist?(builddblink)
  FileUtils.ln_s File.join("build", "compile_commands.json"), builddblink
  
  info "Indexing with cscope"
  system "./RunCodeIndexing.sh"
  
  success "All done."
  info "To compile run 'make' in ./build"
  
else
  
  success "All done."
  info "Open build/Leviathan.sln and start coding"
  
end

exit 0

