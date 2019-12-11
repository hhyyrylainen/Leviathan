#!/usr/bin/env ruby
# Package a leviathan dependency as precompiled binary for current platform
# To package all run: `ruby PackagePrecompiledDep.rb --all`
PRECOMPILED_INSTALL_FOLDER = 'build/ThirdParty'.freeze

def dependency_object_by_name(name)
  $leviathanLibList.each do |lib|
    return lib if name.casecmp(lib.Name).zero? || name.casecmp(lib.FolderName).zero?
  end
  nil
end

def all_dependencies
  $leviathanLibList
end

require_relative 'RubySetupSystem/CreatePrecompiled.rb'

require_relative 'LeviathanLibraries.rb'

run_packager
