#!/usr/bin/env ruby
# coding: utf-8
# Setup script for Leviathan
# Downloads the assets and dependencies and then builds and installs them
# NOTE: sudo is required on linux

# TODO: update the things
# Ogre required libs: rapidjson-devel
# compile flags: OGRE_CONFIG_THREADS=2 (background resources) OGRE_CONFIG_THREAD_PROVIDER=std

require 'fileutils'

require_relative 'Helpers/RubyCommon.rb'

def checkRunFolder(suggested)

    doxyFile = File.join(suggested, "LeviathanDoxy.in")

    onError("Not ran from Leviathan base directory!") if not File.exist?(doxyFile)

    return File.expand_path("..", suggested)
    
end

def projectFolder(baseDir)

  return File.join baseDir, "leviathan"
  
end

require_relative 'Helpers/RubySetupSystem.rb'

# If false won't get breakpad
GetBreakpad = true

# Doesn't get the resources for samples into leviathan/bin if set to false
FetchAssets = true


# All the objects
installer = Installer.new(Array[Newton.new, AngelScript.new, SFML.new, Ogre.new,
                                CEGUIDependencies.new, CEGUI.new])

if GetBreakpad

  installer.addLibrary Breakpad.new

end

installer.run

# Library windows hackery
# TODO: move this into installer.run
if not OnlyMainProject

  if BuildPlatform == "windows"

    onError "todo: update this stuff"

    Dir.chdir(File.join(CurrentDir, "leviathan")) do
      
      # Make sure Ogre home is set
      ENV["OGRE_HOME"] = "#{CurrentDir}/../ogre/build/sdk"
      ENV["OIS_HOME"] = "#{CurrentDir}/../ogre/build/sdk"
      
      info "Moving all the libraries to leviathan/Windows/ThirdParty"
      FileUtils.mkdir_p "Windows/ThirdParty"
      FileUtils.mkdir_p "Windows/ThirdParty/lib"
      FileUtils.mkdir_p "Windows/ThirdParty/bin"
      FileUtils.mkdir_p "Windows/ThirdParty/include"
      
      Dir.chdir("Windows/ThirdParty/include") do
        FileUtils.copy_entry "#{CurrentDir}/../cAudio/build/Install/include/cAudio", "cAudio"
        FileUtils.copy_entry "#{CurrentDir}/../cegui-dependencies/src/glm-0.9.4.5/glm", "glm"
        FileUtils.copy_entry "#{CurrentDir}/../cegui/cegui/include/CEGUI", "CEGUI"
        FileUtils.cp_r(Dir.glob("#{CurrentDir}/../cegui/build/cegui/include/CEGUI/*"), "CEGUI")
        FileUtils.cp_r(Dir.glob("#{CurrentDir}/../ogre/build/sdk/include/*"), "./")
        FileUtils.copy_entry "#{CurrentDir}/../SFML/include/SFML", "SFML"
      end
      
      copyStuff ".lib", "Windows/ThirdParty/lib"
      copyStuff ".dll", "Windows/ThirdParty/bin"
      
      runGlobberAndCopy(Globber.new(["sfml-network.lib", "sfml-system.lib"], "#{CurrentDir}/../SFML/build"), 
                        "Windows/ThirdParty/lib")
      runGlobberAndCopy(Globber.new(["sfml-network-2.dll", "sfml-system-2.dll"], "#{CurrentDir}/../SFML/build"), 
                        "Windows/ThirdParty/bin")
      
      # Debug versions
      runGlobberAndCopy(Globber.new(["sfml-network-d.lib", "sfml-system-d.lib"], "#{CurrentDir}/../SFML/build"), 
                        "Windows/ThirdParty/lib")
      runGlobberAndCopy(Globber.new(["sfml-network-d-2.dll", "sfml-system-d-2.dll"], "#{CurrentDir}/../SFML/build"), 
                        "Windows/ThirdParty/bin")
      
      
      runGlobberAndCopy(Globber.new(["pcre.dll", "freetype.dll"], "#{CurrentDir}/../cegui"), 
                        "Windows/ThirdParty/bin")
      
      # TODO: configure this
      runGlobberAndCopy(Globber.new(["boost_chrono-vc140-mt-1_60.dll", 
                                     "boost_system-vc140-mt-1_60.dll", "boost_filesystem-vc140-mt-1_60.dll"], "#{CurrentDir}/../boost/stage"), 
                        "Windows/ThirdParty/bin")
      # And debug versions
      runGlobberAndCopy(Globber.new(["boost_chrono-vc140-mt-gd-1_60.dll", 
                                     "boost_system-vc140-mt-gd-1_60.dll", "boost_filesystem-vc140-mt-gd-1_60.dll"], "#{CurrentDir}/../boost/stage"), 
                        "Windows/ThirdParty/bin")
    end
  end
  
end

if not File.exist? File.join(CurrentDir, "leviathan")

  onError "'leviathan' folder is missing"

end

Dir.chdir(File.join(CurrentDir, "leviathan")) do
  
  # Assets svn
  if FetchAssets and not SkipPullUpdates
    
    info "Checking out assets svn"

    FileUtils.mkdir_p "bin"

    Dir.chdir("bin") do

      # Check is it an svn repository
      system "svn info"
      
      if $?.exitstatus > 0
        info "Creating a working copy for assets svn"

        systemChecked "svn co https://subversion.assembla.com/svn/leviathan-assets/trunk ."
        
      end

      # Update to latest version (don't force in case there are local changes)
      system "svn update"

      success "Updated Assets"
    end

    success "Assets are good to go"
  end
  
  FileUtils.mkdir_p "build"
  
end

success "Leviathan folder and assets are good to go"

info "Compiling Leviathan"

# Build directory is made earlier

Dir.chdir(File.join(CurrentDir, "leviathan", "build")) do

  runCMakeConfigure "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DUSE_BREAKPAD=OFF"

  
  if $?.exitstatus > 0
    onError "Failed to configure Leviathan"
  end

  runCompiler CompileThreads
  onError "Failed to compile Leviathan " if $?.exitstatus > 0
  
end

success "Done compiling Leviathan"

if BuildPlatform == "linux"
  
  info "Indexing with cscope"
  system "./RunCodeIndexing.sh"
  
  success "All done."
  info "To compile run 'make' in ./build"
  
else
  
  success "All done."
  info "Open build/Leviathan.sln and start coding"
  
end

exit 0





