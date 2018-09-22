# Libraries used by leviathan
require 'os'

require_relative 'RubySetupSystem/Libraries/SetupNewton.rb'
require_relative 'RubySetupSystem/Libraries/SetupAngelScript.rb'
require_relative 'RubySetupSystem/Libraries/SetupSFML.rb'
require_relative 'RubySetupSystem/Libraries/SetupOgre.rb'
require_relative 'RubySetupSystem/Libraries/SetupFFMPEG.rb'
require_relative 'RubySetupSystem/Libraries/SetupcAudio.rb'
require_relative 'RubySetupSystem/Libraries/SetupCEF.rb'

if OS.windows?
  require_relative 'RubySetupSystem/Libraries/SetupFreeType.rb'
  require_relative 'RubySetupSystem/Libraries/SetupZLib.rb'
  require_relative 'RubySetupSystem/Libraries/SetupFreeImage.rb'

  require_relative 'RubySetupSystem/Libraries/SetupSDL.rb'

  require_relative 'RubySetupSystem/Libraries/SetupOpenALSoft.rb'
end

require_relative 'RubySetupSystem/Libraries/SetupLeviathan.rb'

# Setup dependencies settings
THIRD_PARTY_INSTALL = File.join(ProjectDir, "build", "ThirdParty")

$newton = Newton.new(
  version: "6d9be8ccce94845d8738244f5fd9da19c53886ca",
  installPath: THIRD_PARTY_INSTALL,
  noInstallSudo: true,
  disableDemos: true,
  fork: "https://github.com/hhyyrylainen/newton-dynamics.git"
)

$angelscript = AngelScript.new(
  version: 2547,
  installPath: THIRD_PARTY_INSTALL,
  noInstallSudo: true
)

$sfml = SFML.new(
  version: "2.4.x",
  installPath: THIRD_PARTY_INSTALL,
  noInstallSudo: true
)

$ffmpeg = FFMPEG.new(
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
    #"--disable-hwaccels",
    "--disable-muxers",
    #"--disable-parsers",
    #"--disable-protocols",
    "--disable-indevs",
    "--disable-outdevs",
    "--disable-filters",

    # Wanted things
    # These aren't enough so all the demuxers protocols and parsers are enabled
    # "--enable-decoder=aac", "--enable-decoder=mpeg4", "--enable-decoder=h264",
    # "--enable-parser=h264", "--enable-parser=aac", "--enable-parser=mpeg4video",
    # "--enable-demuxer=h264", "--enable-demuxer=aac", "--enable-demuxer=m4v",
    "--enable-decoder=vorbis",
    "--enable-decoder=opus",
    "--enable-decoder=vp9",
    
    # Disable all the external libraries
    "--disable-bzlib", "--disable-iconv",
    "--disable-libxcb",
    "--disable-lzma", "--disable-sdl2", "--disable-xlib", "--disable-zlib",
    "--disable-audiotoolbox", "--disable-cuda", "--disable-cuvid",
    "--disable-nvenc", "--disable-vaapi", "--disable-vdpau",
    "--disable-videotoolbox"
  ].flatten
)

$ogre = Ogre.new(
  version: "v2-1",
  installPath: THIRD_PARTY_INSTALL,
  buildTools: true,
  noInstallSudo: true
)

$caudio = CAudio.new(
  version: "master",
  installPath: THIRD_PARTY_INSTALL,
  noInstallSudo: true,
  noTutorials: true
)

$cef = CEF.new(
  installPath: THIRD_PARTY_INSTALL,
  noInstallSudo: true,
  noTutorials: true
)

$leviathanSelfLib = Leviathan.new({})

if OS.windows?
  $freetype = FreeType.new(
    installPath: THIRD_PARTY_INSTALL,
    noInstallSudo: true
  )

  $zlib = ZLib.new(
    installPath: THIRD_PARTY_INSTALL,
    noInstallSudo: true,
  )

  $freeimage = FreeImage.new(
    installPath: THIRD_PARTY_INSTALL,
    noInstallSudo: true,
    version: "master"
  )

  $sdl = SDL.new(
    installPath: THIRD_PARTY_INSTALL,
    noInstallSudo: true,
    version: "release-2.0.6"
  )

  $openalsoft = OpenALSoft.new(
    installPath: THIRD_PARTY_INSTALL,
    noInstallSudo: true,
    version: "master"
  )
end


$leviathanLibList =
  [$newton, $angelscript, $sfml, $cef, $ffmpeg]

# This if for testing one single dependency compiling
#$leviathanLibList = [$ffmpeg]
#$leviathanLibList = []

# Ogre windows deps and other windows only deps
# sdl is also used by Leviathan directly
if OS.windows?
  $leviathanLibList += [$zlib, $freeimage, $sdl, $freetype, $openalsoft]
end

$leviathanLibList += [$caudio, $ogre]




