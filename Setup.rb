#!/usr/bin/env ruby
# coding: utf-8
# Setup script for Leviathan
# Downloads the assets and dependencies and then builds and installs them
# NOTE: sudo is required on linux (unless skipping package manager installs)

# TODO: update the things
# Ogre required libs: rapidjson-devel
# compile flags: OGRE_CONFIG_THREADS=2 (background resources) OGRE_CONFIG_THREAD_PROVIDER=std

# RubySetupSystem Bootstrap
if not File.exists? "RubySetupSystem/RubySetupSystem.rb"
  puts "Initializing RubySetupSystem"
  system "git submodule init && git submodule update"

  if $?.exitstatus != 0
    abort("Failed to initialize or update git submodules. " +
          "Please make sure git is in path and " +
          "you have an ssh key setup for your github account")
  end
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
require_relative 'RubySetupSystem/Libraries/SetupNewton.rb'
require_relative 'RubySetupSystem/Libraries/SetupAngelScript.rb'
require_relative 'RubySetupSystem/Libraries/SetupSFML.rb'
require_relative 'RubySetupSystem/Libraries/SetupOgre.rb'
require_relative 'RubySetupSystem/Libraries/SetupCEGUI.rb'
require_relative 'RubySetupSystem/Libraries/SetupFFMPEG.rb'

# If false won't get breakpad
# TODO: fix
GetBreakpad = false

# Doesn't get the resources for samples into leviathan/bin if set to false
FetchAssets = true


# Setup dependencies settings
THIRD_PARTY_INSTALL = File.join(ProjectDir, "build", "ThirdParty")

newton = Newton.new(
  version: "7c5970ccda537dea134e0443d702ef9f5ce81a38",
  installPath: THIRD_PARTY_INSTALL,
  noInstallSudo: true,
  disableDemos: true
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

ffmpeg = FFMPEG.new(
  version: "release/3.3",
  installPath: THIRD_PARTY_INSTALL,
  noInstallSudo: true,
  enablePIC: true,
  buildShared: true,
  enableSmall: true,
  # noStrip: true,
  extraOptions: [
    "--disable-postproc", "--disable-avdevice",
    "--disable-avfilter",
    if !OS.windows? then 
      "--enable-rpath"
    else
      ""
    end,
    
    "--disable-network",

    # Can't be bothered to check which specific things we need so some of these disables
    # are disabled
    #"--disable-everything",
    #"--disable-demuxers",
    "--disable-encoders",
    "--disable-decoders",
    "--disable-hwaccels",
    "--disable-muxers",
    #"--disable-parsers",
    #"--disable-protocols",
    "--disable-indevs",
    "--disable-outdevs",
    "--disable-filters",

    # Wanted things
    # These aren't enough so all the demuxers protocols and parsers are enabled
    "--enable-decoder=aac", "--enable-decoder=mpeg4", "--enable-decoder=h264",
    "--enable-parser=h264", "--enable-parser=aac", "--enable-parser=mpeg4video",
    "--enable-demuxer=h264", "--enable-demuxer=aac", "--enable-demuxer=m4v",

    
    # Disable all the external libraries
    "--disable-bzlib", "--disable-iconv",
    "--disable-libxcb",
    "--disable-lzma", "--disable-sdl2", "--disable-xlib", "--disable-zlib",
    "--disable-audiotoolbox", "--disable-cuda", "--disable-cuvid",
    "--disable-nvenc", "--disable-vaapi", "--disable-vdpau",
    "--disable-videotoolbox"
  ].flatten
)

ogre = Ogre.new(
  version: "v2-1",
  installPath: THIRD_PARTY_INSTALL,
  noInstallSudo: true
)

cegui = CEGUI.new(
  version: "75906a4cdfc2",
  installPath: THIRD_PARTY_INSTALL,
  # Find Ogre in our search path
  extraOptions: ["-DOGRE_HOME=#{THIRD_PARTY_INSTALL}"],
  noInstallSudo: true
)

# All the objects
installer = Installer.new(
  [newton, angelscript, sfml, ffmpeg, ogre, cegui]
)

if GetBreakpad

  breakpad = Breakpad.new(
    version: "master",
    installPath: THIRD_PARTY_INSTALL,
    noInstallSudo: true
  )
  
  installer.addLibrary breakpad

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

        runOpen3Checked("svn", "co",
                        "https://subversion.assembla.com/svn/leviathan-assets/trunk", ".")
        
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

  if !runCMakeConfigure [
       "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON",
       "-DOGRE_HOME=#{THIRD_PARTY_INSTALL}",
       "-DUSE_BREAKPAD=OFF"
     ]
    onError "Failed to configure Leviathan"
  end
  
  if !runCompiler CompileThreads
    onError "Failed to compile Leviathan"
  end
  
end

success "Done compiling Leviathan"

if OS.linux?
  
  info "Indexing with cscope"
  Dir.chdir(ProjectDir) do
    runOpen3Checked File.join(ProjectDir, "RubySetupSystem/RunCodeIndexing.rb")
  end
  
  success "All done."
  info "To compile again just run 'make' in ./build"
  puts "You may need to run this setup again from time to time"
  
else
  
  success "All done."
  info "Open build/Leviathan.sln and start coding"
  
end

exit 0

