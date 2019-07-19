# Common classes for generating C++ code with cmake
require 'fileutils'

# Sanitizes a type for use as a name
# Removes <>'s and other stuff
def sanitizeName(name)
  name.gsub(/<|>/i, "")
end

def genComparisonExpression(values)

  values.map{|v| v[0] + " == " + v[1]}.join(" && ")
  
end

require_relative 'Variable'
require_relative 'ConstructorInfo'
require_relative 'OutputClass'
require_relative 'SerializeClass'
require_relative 'GameWorld'


class Generator
  def initialize(outputFile, separateFiles: false, bareOutput: false)

    @Separated = separateFiles
    
    @OutputFile = outputFile
    @OutputObjs = Array.new
    @Namespace = nil
    @Includes = []
    @ImplIncludes = []
    @BareOutput = bareOutput
    @ExportType = "DLLEXPORT"
  end

  def add(obj)

    @OutputObjs.push obj
    
  end

  def useNamespace(namespace="Leviathan")
    @Namespace = namespace
  end

  def useExportMacro(newtype)
    @ExportType = newtype
  end

  def addInclude(include)
    @Includes.push include
  end
  def addImplInclude(include)
    @ImplIncludes.push include
  end  

  def prepareFile(file)
    FileUtils.mkdir_p File.dirname(file)
    FileUtils.rm_f file
  end

  def outputFile(file, options)

    File.open(file, 'w') do |file|
      
      file.puts "// Automatically Generated File Do not edit! //" unless @BareOutput
      file.puts "//" unless @BareOutput

      if options.include?(:header) and not @BareOutput
        file.puts "#pragma once"
      end

      @Includes.each{|i|
        file.puts "#include \"#{i}\""
      }

      @OutputObjs.each do |obj|

        if obj.respond_to? :getExtraIncludes
          obj.getExtraIncludes .each{|i|
            file.puts "#include \"#{i}\""
          }
        end
      end      

      if options.include?(:impl)
        @ImplIncludes.each{|i|
          file.puts "#include \"#{i}\""
        }        
      end
      
      file.puts ""

      if options.include?(:includeHeader)
        file.puts "#include \"#{options[:includeHeader]}\""
      end

      if @Namespace
        file.puts "namespace #{@Namespace}{"
      end

      @OutputObjs.each do |obj|

        if obj.respond_to? :setExport
          obj.setExport @ExportType
        end
        obj.toText(file, options)

      end

      if @Namespace
        file.puts "}"
      end      

      # Prevent editing
      FileUtils.chmod 'a-w', file
    end
  end

  def run

    if @Separated

      prepareFile @OutputFile + ".h"
      prepareFile @OutputFile + ".cpp"
      
      outputFile(@OutputFile + ".h", {header: true})
      outputFile(@OutputFile + ".cpp", {impl: true, includeHeader: @OutputFile + ".h"})
    else
      
      prepareFile @OutputFile
      outputFile(@OutputFile, {both: true, header: true, impl: true})
    end
  end
end

class OutputText

  def initialize(text)
    @Text = text
  end

  def toText(f, opts)
    f.puts @Text
  end
  
end
