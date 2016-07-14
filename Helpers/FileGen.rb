# Common classes for generating C++ code with cmake
require 'fileutils'


class Generator
  def initialize(outputFile)

    @outputFile = outputFile
    @outputObjs = Array.new
  end

  def add(obj)

    @outputObjs.push obj
    
  end

  def run

    FileUtils.mkdir_p File.dirname(@outputFile)

    File.open(@outputFile, 'w') do |file|
      
      file.write "// Automatically Generated File Do not edit! //\n"
      file.write "//\n"
      file.write "\n"

      @outputObjs.each do |obj|

        file.write obj.toText

      end
    end
  end
end

class OutputClass

  def initialize(name)

    @name = name
    @members = Array.new
    @baseClass = ""
    @baseConstructor = ""
    @cargs = Array.new
  end

  def toText

    "class #{@name} " +
      if not @baseClass.empty?
        ": public #{@baseClass} {\n"
      else
        "{\n"
      end +
      genBody +
      "\n" +
      "};\n\n"
  end
  
  def genBody
    "public:\n" +
      if not @members.empty?
        # A constructor
        genMemberConstructor
      else
        ""
      end +
      genMethods +
      "\n" +
      #Members
      genMembers
  end

  def genMethods
    "\n"
  end

  def genMemberConstructor
    str = "#{@name}("

    if not @cargs.empty?

      str += @cargs.join(", ") + ", "
      
    end
    
    str += @members.map{|a|
      if a[:default].nil?
        if a[:move].nil?
          "const #{a[:type]} &#{a[:name].downcase}"
        else
          # Move constructor
          "#{a[:type]}&& #{a[:name].downcase}"
        end
      else
        "const #{a[:type]} &#{a[:name].downcase} = #{a[:default]}"
      end
    }.join(", \n")

    # Initializer list
    str += ") :\n"

    # Base constructor
    if not @baseConstructor.empty?
      str += "#{@baseClass}(#{@baseConstructor}),\n"
    end
    
    str += @members.map{|a|
      if a[:move].nil?
        "#{a[:name]}(#{a[:name].downcase})"
      else
        # Move constructor
        "#{a[:name]}(std::move(#{a[:name].downcase}))"
      end
        
    }.join(", ")
    
    str += "\n{}\n"
    str
  end
  
  def genMembers
    
    @members.map { |a|
      if a[:default].nil?
        "#{a[:type]} #{a[:name]};"
      else
        "#{a[:type]} #{a[:name]} = #{a[:default]};"
      end
    }.join("\n")
  end
  
  def base(baseName)
    @baseClass = baseName
  end

  def baseConstructor(arguments)
    @baseConstructor = arguments
  end

  def addMember(arguments = {})
    abort "addMember not hash" if not arguments.class == Hash
    # If default is an empty string change it to be "\"\""
    if not arguments[:default].nil?
        if arguments[:default].empty?
            arguments[:default] = "\"\""
        end
    end
    
    @members.push arguments
  end

  # Adds an extra argument to the constructor
  def constructorMember(definition)
    @cargs.push definition
  end
  
end

class SFMLSerializeClass < OutputClass
  
  def initialize(name)
    
    super name
    @deserializeArgs = []
    @deserializeBase = ""
  end

  def genMethods
    "\n" +
      genSerializer + "\n" +
      genSFMLConstructor + "\n"
  end

  def genToPacket
    "packet << " +
      @members.map { |a|
      if a[:as].nil?
        "#{a[:name]}"
      else
        "static_cast<#{a[:as]}>(#{a[:name]})"
      end
    }.join(" << ")
  end

  def genSerializer
    "void AddDataToPacket(sf::Packet &packet) const{\n"+
      genToPacket +
      ";\n}\n"
  end

  def addDeserializeArg(arg)
    
    @deserializeArgs.push arg
    
  end
  
  def deserializeBase(arg)
    @deserializeBase = arg
  end
  
  def genSFMLConstructor
    
    tmpargs = @cargs + @deserializeArgs
    tmpbaseconstructor = @baseConstructor 
    
    if not @deserializeBase.empty?
      tmpbaseconstructor += ", " + @deserializeBase
    end
  
    "#{@name}(" +
      if not tmpargs.empty?
        tmpargs.join(", ") + ", "
      else
        ""
      end +
      "sf::Packet &packet) " +
      # Base constructor
      if not tmpbaseconstructor.empty?
        ": #{@baseClass}(#{tmpbaseconstructor}) {\n"
      else
        "{\n"
      end +
      genDeserializer +
      # Error check
      "if(!packet)\n    //Error loading\n" +
      "    throw InvalidArgument(\"Invalid packet format for: '#{@name}'\");" +
      "\n}\n"
  end

  def genDeserializer

    str = ""
    
    nonAsIfThings = false

    @members.each do |a|

      if a[:as].nil?

        nonAsIfThings = true
        next
      end

      tempName = "temp_#{a[:name]}"
      str += "#{a[:as]} #{tempName};\n"
      str += "packet >> #{tempName};\n"
      str += "#{a[:name]} = static_cast<#{a[:type]}>(#{tempName});\n"
      
    end
    
    if nonAsIfThings
      str += "packet >> " + @members.select{ |a| a[:as].nil? }.map { |a| "#{a[:name]}" }.
                            join(" >> ") + ";\n"
    end
    str
  end
end

class ResponseClass < SFMLSerializeClass

  def genSerializer
    "void _SerializeCustom(sf::Packet &packet) const override{\n"+
      genToPacket +
      ";\n}\n"
  end
  
end
