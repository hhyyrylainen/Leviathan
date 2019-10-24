#!/usr/bin/env ruby

# Setup script for Leviathan
# Downloads the assets and dependencies and then builds and installs them
# NOTE: sudo is required on linux (unless skipping package manager installs)

# RubySetupSystem Bootstrap
if !File.exist? 'RubySetupSystem/RubySetupSystem.rb'
  puts 'Initializing RubySetupSystem'
  system 'git submodule init && git submodule update'

  if $CHILD_STATUS.exitstatus != 0
    abort('Failed to initialize or update git submodules. ' \
          'Please make sure git is in path and ' \
          'you have an ssh key setup for your github account')
  end
else
  # Make sure RubySetupSystem is up to date
  # This may make debugging RubySetupSystem harder so feel free to comment out
  system 'git submodule update'
end

require 'fileutils'

require_relative 'RubySetupSystem/RubyCommon.rb'

def checkRunFolder(suggested)
  doxyFile = File.join(suggested, 'LeviathanDoxy.in')

  onError('Not ran from Leviathan base directory!') unless File.exist?(doxyFile)

  thirdPartyFolder = File.join suggested, 'ThirdParty'

  FileUtils.mkdir_p thirdPartyFolder
  FileUtils.mkdir_p File.join suggested, 'build', 'ThirdParty'

  thirdPartyFolder
end

def projectFolder(baseDir)
  File.expand_path File.join(baseDir, '../')
end

def parseExtraArgs; end

require_relative 'RubySetupSystem/RubySetupSystem.rb'

# If false won't get breakpad
# TODO: fix
GetBreakpad = false

require_relative 'LeviathanLibraries.rb'

# All the objects
installer = Installer.new(
  $leviathanLibList
)

if GetBreakpad

  breakpad = Breakpad.new(
    version: 'master',
    installPath: THIRD_PARTY_INSTALL,
    noInstallSudo: true
  )

  installer.addLibrary breakpad

end

# register us for the dependencies runs
installer.registerSelfAsLibrary $leviathanSelfLib

installer.run

onError "'leviathan' folder is missing" unless File.exist? ProjectDir

Dir.chdir(ProjectDir) do
  info 'Pulling assets with Git LFS'

  runOpen3Checked 'git', 'lfs', 'pull'

  FileUtils.mkdir_p 'build'

  # Symlink the js libraries and images from bin to make local previewing of the GUI easier
  if OS.windows?
    info 'Creating junctions for assets to be referenced from gui ' \
         'html without running cmake every time'
    # runSystemSafe "cmd", "/c", "mklink", "/J",
    #               convertPathToWindows(File.join(ProjectDir, "Textures")),
    #               convertPathToWindows(File.join(ProjectDir, "Assets", "Textures"))
    runSystemSafe 'cmd', '/c', 'mklink', '/J',
                  convertPathToWindows(File.join(ProjectDir, 'Fonts')),
                  convertPathToWindows(File.join(ProjectDir, 'Assets', 'Fonts'))
    runSystemSafe 'cmd', '/c', 'mklink', '/J',
                  convertPathToWindows(File.join(ProjectDir, 'JSVendor')),
                  convertPathToWindows(File.join(ProjectDir, 'Assets',
                                                 'JSVendor'))
  else
    # if !File.exists? File.join(ProjectDir, "Textures")
    #   FileUtils.ln_sf File.join(ProjectDir, "Assets", "Textures"),
    #                   File.join(ProjectDir, "Textures")
    # end

    unless File.exist? File.join(ProjectDir, 'Fonts')
      FileUtils.ln_sf File.join(ProjectDir, 'Assets', 'Fonts'),
                      File.join(ProjectDir, 'Fonts')
    end

    unless File.exist? File.join(ProjectDir, 'JSVendor')
      FileUtils.ln_sf File.join(ProjectDir, 'Assets', 'JSVendor'),
                      File.join(ProjectDir, 'JSVendor')
    end
  end
end

success 'Leviathan folder and assets are good to go'

info 'Compiling Leviathan'

# Build directory is made earlier

Dir.chdir(File.join(ProjectDir, 'build')) do
  unless runCMakeConfigure [
    # This is done automatically
    # "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON",
  ]
    onError 'Failed to configure Leviathan'
  end

  onError 'Failed to compile Leviathan' unless TC.runCompiler
end

success 'Done compiling Leviathan'

success 'All done.'

if OS.linux?

  info "To compile again just run 'make' in ./build"
  puts 'You may need to run this setup again from time to time'

else

  info 'Open build/Leviathan.sln and start coding'

end

exit 0
