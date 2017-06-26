#!/usr/bin/env ruby
# coding: utf-8
# Setup script for Leviathan
# Downloads the assets and dependencies and then builds and installs them
# NOTE: sudo is required on linux

# TODO: update the things
# Ogre required libs: rapidjson-devel
# compile flags: OGRE_CONFIG_THREADS=2 (background resources) OGRE_CONFIG_THREAD_PROVIDER=std

# RubySetupSystem Bootstrap
system "git submodule init && git submodule update"

if $?.exitstatus != 0
  abort("Failed to initialize or update git submodules. " +
        "Please make sure git is in path and " +
        "you have an ssh key setup for your github account")
end

require 'fileutils'

require_relative 'RubySetupSystem/RubyCommon.rb'

def checkRunFolder(suggested)

    doxyFile = File.join(suggested, "LeviathanDoxy.in")

    onError("Not ran from Leviathan base directory!") if not File.exist?(doxyFile)

    thirdPartyFolder = suggested, "ThirdParty"

    FileUtils.mkdir_p thirdPartyFolder
    FileUtils.mkdir_p File.join suggested, "build", "ThirdParty"
    
    thirdPartyFolder
    
end

def projectFolder(baseDir)

  File.expand_path File.join(baseDir, "../")
  
end

def parseExtraArgs

end

require_relative 'RubySetupSystem/RubySetupSystem.rb'

# If false won't get breakpad
GetBreakpad = true

# Doesn't get the resources for samples into leviathan/bin if set to false
FetchAssets = true


# Setup dependencies settings
THIRD_PARTY_INSTALL = File.join(ProjectDir, "build", "ThirdParty")

newton = Newton.new(
  version: "master",
  installPath: THIRD_PARTY_INSTALL,
  noInstallSudo: true
)

angelscript = AngelScript.new(
  version: "2.31.2",
  installPath: THIRD_PARTY_INSTALL,
  noInstallSudo: true
)

sfml = SFML.new(
  version: "2.4.x",
  installPath: THIRD_PARTY_INSTALL,
  noInstallSudo: true
)

ogre = Ogre.new(
  version: "2.4.x",
  installPath: THIRD_PARTY_INSTALL,
  noInstallSudo: true
)

cegui = CEGUI.new(
  # TODO: once my pull request is accepted put it here
  version: "master",
  installPath: THIRD_PARTY_INSTALL,
  extraSearchPath: THIRD_PARTY_INSTALL,
  noInstallSudo: true
)

# All the objects
installer = Installer.new(
  [newton, angelscript, sfml, ogre, cegui]
)

if GetBreakpad

  breakpad = Breakpad.new(
    version: "master",
    installPath: THIRD_PARTY_INSTALL,
    noInstallSudo: true
  )
  
  installer.addLibrary 

end

installer.run

if not File.exist? ProjectDir

  onError "'leviathan' folder is missing"

end

Dir.chdir(ProjectDir) do
  
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

Dir.chdir(File.join(ProjectDir, "build")) do

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
  Dir.chdir(ProjectDir) do
    systemChecked "RubySetupSystem/RunCodeIndexing.sh"
  end
  
  success "All done."
  info "To compile run 'make' in ./build"
  
else
  
  success "All done."
  info "Open build/Leviathan.sln and start coding"
  
end

exit 0

