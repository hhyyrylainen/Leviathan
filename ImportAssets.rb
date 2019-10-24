#!/usr/bin/env ruby
# This script imports engine assets to BSF format. The imported files are placed in Assets
# NOTE: you don't have to run this if you didn't change the assets.
# Already imported versions are included
require 'fileutils'
require 'open3'

require_relative 'RubySetupSystem/RubyCommon'

editor = 'build/bin/LeviathanEditor'

editor += '.exe' if OS.windows?

unless File.exist? editor

  onError 'LeviathanEditor is not compiled! Or this script is ran from the wrong directory'
end

editor = File.realpath editor

info 'Importing assets.'

Open3.popen3(editor, '--import', File.realpath('Assets/'),
             File.realpath('Assets/'),
             chdir: File.realpath(File.dirname(editor))) { |_stdin, stdout, stderr, wait_thr|

  out_thread = Thread.new {
    stdout.each { |line|
      puts line
    }
  }

  err_thread = Thread.new {
    stderr.each { |line|
      puts line.red
    }
  }

  exit_status = wait_thr.value
  out_thread.join
  err_thread.join

  onError 'Failed to run import with LeviathanEditor' if exit_status != 0
}

info "NOTE: import failures aren't reported as exit codes so this script " \
     "can't currently detect failures"
success 'Done importing'
