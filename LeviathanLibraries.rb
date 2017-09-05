# Libraries used by leviathan

require_relative 'RubySetupSystem/Libraries/SetupNewton.rb'
require_relative 'RubySetupSystem/Libraries/SetupAngelScript.rb'
require_relative 'RubySetupSystem/Libraries/SetupSFML.rb'
require_relative 'RubySetupSystem/Libraries/SetupOgre.rb'
require_relative 'RubySetupSystem/Libraries/SetupCEGUI.rb'
require_relative 'RubySetupSystem/Libraries/SetupFFMPEG.rb'

require_relative 'RubySetupSystem/Libraries/SetupLeviathan.rb'

# Setup dependencies settings
THIRD_PARTY_INSTALL = File.join(ProjectDir, "build", "ThirdParty")

$newton = Newton.new(
  version: "7c5970ccda537dea134e0443d702ef9f5ce81a38",
  installPath: THIRD_PARTY_INSTALL,
  noInstallSudo: true,
  disableDemos: true
)

$angelscript = AngelScript.new(
  version: "2.31.2",
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

$ogre = Ogre.new(
  version: "v2-1",
  installPath: THIRD_PARTY_INSTALL,
  noInstallSudo: true
)

$cegui = CEGUI.new(
  version: "0555c53769264bd9bfd61a094a7dafe59bb2192c",
  installPath: THIRD_PARTY_INSTALL,
  # Find Ogre in our search path
  extraOptions: ["-DOGRE_HOME=#{THIRD_PARTY_INSTALL}"],
  noInstallSudo: true
)

$leviathanSelfLib = Leviathan.new({})
