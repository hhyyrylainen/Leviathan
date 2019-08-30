# Libraries used by leviathan
require 'os'

require_relative 'RubySetupSystem/Libraries/SetupBullet.rb'
require_relative 'RubySetupSystem/Libraries/SetupAngelScript.rb'
require_relative 'RubySetupSystem/Libraries/SetupSFML.rb'
require_relative 'RubySetupSystem/Libraries/SetupFFMPEG.rb'
require_relative 'RubySetupSystem/Libraries/SetupcAudio.rb'
require_relative 'RubySetupSystem/Libraries/SetupCEF.rb'
require_relative 'RubySetupSystem/Libraries/SetupBreakpad.rb'
require_relative 'RubySetupSystem/Libraries/SetupBSFramework.rb'

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

$bullet = Bullet.new(
  version: "2.87",
  installPath: THIRD_PARTY_INSTALL,
  disableGraphicalBenchmark: true,
  disableCPUDemos: true,
  disableGLUT: true,
  disableDemos: true,
  noInstallSudo: true,
  # Only way to properly get -fPIC on linux (without hacking with CXX_FLAGS)
  # And bullet doesn't support shared libs on Windows
  shared: if OS.linux? then true else false end
)

$angelscript = AngelScript.new(
  version: 2547,
  installPath: THIRD_PARTY_INSTALL,
  noInstallSudo: true
)

$sfml = SFML.new(
  version: "2.5.x",
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

$bsf = BSFramework.new(
  version: "f6441617945b2359b8b6c9b73ad09ac3b06e6a90",
  installPath: THIRD_PARTY_INSTALL,
  noInstallSudo: true,
  renderAPI: if OS.windows? then "DirectX 11" else "OpenGL" end,
  buildAllRenderAPI: true,
  physicsModule: "Null",
  audioModule: "Null",
  extraLibSearch: "lib/",
)

$caudio = CAudio.new(
  version: "master",
  epoch: 2,
  installPath: THIRD_PARTY_INSTALL,
  noInstallSudo: true,
  noTutorials: true
)

$cef = CEF.new(
  installPath: THIRD_PARTY_INSTALL,
  noInstallSudo: true,
  version: "75.1.4+g4210896+chromium-75.0.3770.100"
)

$breakpad = Breakpad.new(
  installPath: THIRD_PARTY_INSTALL,
  noInstallSudo: true
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
    version: "master",
    epoch: 3    
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
  [$bullet, $angelscript, $sfml, $cef, $ffmpeg]

if true
  $leviathanLibList += [$breakpad]
end

if OS.windows?
  $leviathanLibList += [$sdl, $openalsoft]
end

# ogre deps $zlib, $freeimage, $freetype,

$leviathanLibList += [$caudio, $bsf]




