# Package a leviathan dependency as precompiled binary for current platform
# To package all run: `ruby PackagePrecompiledDep.rb newton  angelscript sfml ffmpeg zlib freeimage sdl freetype ogre cegui`
PrecompiledInstallFolder = "build/ThirdParty"

require_relative 'RubySetupSystem/CreatePrecompiled.rb'

require_relative 'LeviathanLibraries.rb'


def getDependencyObjectByName(name)

  # Could make sure this is automatically up to date
  case name
  when "newton"
    return $newton
  when "angelscript"
    return $angelscript
  when "sfml"
    return $sfml
  when "ffmpeg"
    return $ffmpeg
  when "ogre"
    return $ogre
  when "cegui"
    return $cegui
  # These are Windows only
  when "zlib"
    return $zlib
  when "freeimage"
    return $freeimage
  when "sdl"
    return $sdl
  when "freetype"
    return $freetype
  end
end

runPackager()






