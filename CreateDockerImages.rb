#!/usr/bin/env ruby
# Creates docker images for building Leviathan inside them
require_relative 'RubySetupSystem/DockerImageCreator'

require_relative 'LeviathanLibraries'

runDockerCreate($leviathanLibList, $leviathanSelfLib) 



