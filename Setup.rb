#!/usr/bin/env ruby
# coding: utf-8
# Setup script for Leviathan
# Downloads the assets and dependencies and then builds and installs them
# NOTE: sudo is required on linux (unless skipping package manager installs)

# RubySetupSystem Bootstrap
if not File.exists? "RubySetupSystem/RubySetupSystem.rb"
  puts "Initializing RubySetupSystem"
  system "git submodule init && git submodule update"

  if $?.exitstatus != 0
    abort("Failed to initialize or update git submodules. " +
          "Please make sure git is in path and " +
          "you have an ssh key setup for your github account")
  end
else
  # Make sure RubySetupSystem is up to date
  # This may make debugging RubySetupSystem harder so feel free to comment out
  system "git submodule update"
end

require 'fileutils'

require_relative 'RubySetupSystem/RubyCommon.rb'

def checkRunFolder(suggested)

    doxyFile = File.join(suggested, "LeviathanDoxy.in")

    onError("Not ran from Leviathan base directory!") if not File.exist?(doxyFile)

    thirdPartyFolder = File.join suggested, "ThirdParty"

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
# TODO: fix
GetBreakpad = false

# Doesn't get the resources for samples into leviathan/bin if set to false
FetchAssets = true

WantedURL = "https://boostslair.com/svn/leviathan_assets"

require_relative 'LeviathanLibraries.rb'

# All the objects
installer = Installer.new(
  $leviathanLibList
)

if GetBreakpad

  breakpad = Breakpad.new(
    version: "master",
    installPath: THIRD_PARTY_INSTALL,
    noInstallSudo: true
  )
  
  installer.addLibrary breakpad

end

# register us for the dependencies runs
installer.registerSelfAsLibrary $leviathanSelfLib

installer.run

if not File.exist? ProjectDir

  onError "'leviathan' folder is missing"

end

Dir.chdir(ProjectDir) do
  
  # Assets svn
  # Always updated for continuous integration stuff
  if FetchAssets # and not SkipPullUpdates
    
    info "Checking out assets svn"

    FileUtils.mkdir_p "bin"

    Dir.chdir("bin") do

      # Check is it an svn repository
      system "svn info"
      
      if $?.exitstatus > 0
        info "Creating a working copy for assets svn"

        runOpen3Checked("svn", "co",
                        WantedURL, ".")
      else
        # Check that url is right
        verifySVNUrl(WantedURL)        
      end

      # Update to latest version (don't force in case there are local changes)
      system "svn update"

      success "Updated Assets"
    end

    success "Assets are good to go"
  end
  
  FileUtils.mkdir_p "build"

  # Symlink the js libraries and images from bin to make local previewing of the GUI easier
  if OS.windows?
    info "Creating junctions for assets to be referenced from gui " +
         "html without running cmake every time"
    runSystemSafe "cmd", "/c", "mklink", "/J",
                  convertPathToWindows(File.join(ProjectDir, "Textures")),
                  convertPathToWindows(File.join(ProjectDir, "bin/Data", "Textures"))
    runSystemSafe "cmd", "/c", "mklink", "/J",
                  convertPathToWindows(File.join(ProjectDir, "Fonts")),
                  convertPathToWindows(File.join(ProjectDir, "bin/Data", "Fonts"))
    runSystemSafe "cmd", "/c", "mklink", "/J",
                  convertPathToWindows(File.join(ProjectDir, "JSVendor")),
                  convertPathToWindows(File.join(ProjectDir, "bin/Data",
                                                 "JSVendor"))  
  else
    if !File.exists? File.join(ProjectDir, "Textures")
      FileUtils.ln_sf File.join(ProjectDir, "bin/Data", "Textures"),
                      File.join(ProjectDir, "Textures")
    end

    if !File.exists? File.join(ProjectDir, "Fonts")
      FileUtils.ln_sf File.join(ProjectDir, "bin/Data", "Fonts"),
                      File.join(ProjectDir, "Fonts")
    end

    if !File.exists? File.join(ProjectDir, "JSVendor")
      FileUtils.ln_sf File.join(ProjectDir, "bin/Data", "JSVendor"),
                      File.join(ProjectDir, "JSVendor")
    end
  end
  
end

success "Leviathan folder and assets are good to go"

info "Compiling Leviathan"

# Build directory is made earlier

Dir.chdir(File.join(ProjectDir, "build")) do

  if !runCMakeConfigure [
       # This is done automatically
       # "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON",
     ]
    onError "Failed to configure Leviathan"
  end
  
  if !TC.runCompiler
    onError "Failed to compile Leviathan"
  end
  
end

success "Done compiling Leviathan"

if OS.linux?
  
  success "All done."
  info "To compile again just run 'make' in ./build"
  puts "You may need to run this setup again from time to time"
  
else
  
  success "All done."
  info "Open build/Leviathan.sln and start coding"
  
end

exit 0

