# Package a leviathan dependency as precompiled binary for current platform
# To package all run: `ruby PackagePrecompiledDep.rb --all`
PrecompiledInstallFolder = "build/ThirdParty"

def getDependencyObjectByName(name)

  # Could make sure this is automatically up to date
  $leviathanLibList.each{|lib|
    
    if name.casecmp(lib.Name).zero? || name.casecmp(lib.FolderName).zero?
      return lib
    end
  }
  nil
end

def getAllDependencies
  $leviathanLibList
end

require_relative 'RubySetupSystem/CreatePrecompiled.rb'

require_relative 'LeviathanLibraries.rb'

runPackager()
