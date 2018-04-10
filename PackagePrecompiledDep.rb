# Package a leviathan dependency as precompiled binary for current platform
# To package all run: `ruby PackagePrecompiledDep.rb "newton dynamics"  angelscript sfml ffmpeg zlib freeimage sdl freetype ogre cef "openal soft" caudio`
PrecompiledInstallFolder = "build/ThirdParty"

require_relative 'RubySetupSystem/CreatePrecompiled.rb'

require_relative 'LeviathanLibraries.rb'


def getDependencyObjectByName(name)

  # Could make sure this is automatically up to date
  $leviathanLibList.each{|lib|
    
    if name.casecmp(lib.Name).zero? || name.casecmp(lib.FolderName).zero?
      return lib
    end
  }
  nil
end

runPackager()






