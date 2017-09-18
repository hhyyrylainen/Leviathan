# Common classes for generating C++ code with cmake
require 'fileutils'


class Generator
  def initialize(outputFile, separateFiles=false)

    @Separated = separateFiles
    
    @OutputFile = outputFile
    @OutputObjs = Array.new
    @Namespace = nil
    @Includes = []
  end

  def add(obj)

    @OutputObjs.push obj
    
  end

  def useNamespace(namespace="Leviathan")
    @Namespace = namespace
  end

  def addInclude(include)
    @Includes.push include
  end

  def prepareFile(file)
    FileUtils.mkdir_p File.dirname(file)
    FileUtils.rm_f file
  end

  def outputFile(file, options)

    File.open(file, 'w') do |file|
      
      file.puts "// Automatically Generated File Do not edit! //"
      file.puts "//"

      if options.include?(:header)
        file.puts "#pragma once"
      end

      @Includes.each{|i|
        file.puts "#include \"#{i}\""
      }
      
      file.puts ""

      if options.include?(:includeHeader)
        file.puts "#include \"#{options[:includeHeader]}\""
      end

      if @Namespace
        file.puts "namespace #{@Namespace}{"
      end

      @OutputObjs.each do |obj|

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

class OutputClass

  def initialize(name)

    @Name = name
    @Members = []
    @BaseClass = ""
    @BaseConstructor = ""
    @CArgs = []
  end

  def toText(f, opts)

    if opts.include?(:header)
      f.write "class #{@Name} " +
              if not @BaseClass.empty?
                ": public #{@BaseClass} {\n"
              else
                "{\n"
              end
      
      genBody f, opts

      f.puts ""
      f.puts "};\n\n"
    else
      genBody f, opts
    end
  end
  
  def genBody(f, opts)

    if opts.include?(:header)
      f.puts "public:"
    end
    
    if not @Members.empty?
      # A constructor
      genMemberConstructor f, opts
    end
    
    genMethods f, opts
    f.puts ""

    if opts.include?(:header)
      genMembers f, opts
    end
  end

  def genMethods(f, opts)
    f.puts ""
  end

  def genMemberConstructor(f, opts)
    str = "#{qualifier opts}#{@Name}("

    if not @CArgs.empty?

      str += @CArgs.join(", ") + ", "
      
    end
    
    str += @Members.map{|a|
      if a[:default].nil? or (opts.include?(:imp) and !opts.include?(:both))
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

    if opts.include?(:impl)
      # Initializer list
      str += ") :\n"

      # Base constructor
      if not @BaseConstructor.empty?
        str += "#{@BaseClass}(#{@BaseConstructor}),\n"
      end
      
      str += @Members.map{|a|
        if a[:move].nil?
          "#{a[:name]}(#{a[:name].downcase})"
        else
          # Move constructor
          "#{a[:name]}(std::move(#{a[:name].downcase}))"
        end
        
      }.join(", ")

      str += "\n{}\n"
      
    else
      str += ");\n"
    end

    f.write str
  end
  
  def genMembers(f, opts)
    
    f.puts @Members.map { |a|
      if a[:default].nil?
        "#{a[:type]} #{a[:name]};"
      else
        "#{a[:type]} #{a[:name]} = #{a[:default]};"
      end
    }.join("\n")
  end
  
  def base(baseName)
    @BaseClass = baseName
  end

  def baseConstructor(arguments)
    @BaseConstructor = arguments
  end

  def addMember(arguments = {})
    abort "addMember not hash" if not arguments.class == Hash
    # If default is an empty string change it to be "\"\""
    if not arguments[:default].nil?
        if arguments[:default].empty?
            arguments[:default] = "\"\""
        end
    end
    
    @Members.push arguments
  end

  # Adds an extra argument to the constructor
  def constructorMember(definition)
    @CArgs.push definition
  end

  # Returns string with the class name:: if needed
  def qualifier(opts)
    if !opts.include?(:header)
      "#{@Name}::"
    else
      ""
    end
  end

  # Returns override if :header set
  def override(opts)
    if opts.include?(:header)
      "override"
    else
      ""
    end
  end

  # Returns virtual if :header set
  def virtual(opts)
    if opts.include?(:header)
      "virtual"
    else
      ""
    end
  end  
  
end

class SFMLSerializeClass < OutputClass
  
  def initialize(name)
    
    super name
    @deserializeArgs = []
    @deserializeBase = ""
  end

  def genMethods(f, opts)
    f.puts ""
    
    genSerializer f, opts
    genSFMLConstructor f, opts
  end

  def genToPacket
    "packet << " +
      @Members.map { |a|
      if a[:as].nil?
        "#{a[:name]}"
      else
        "static_cast<#{a[:as]}>(#{a[:name]})"
      end
    }.join(" << ")
  end

  def genSerializer(f, opts)

    f.write "void AddDataToPacket(sf::Packet &packet) const"
    if opts.include?(:impl)
      f.puts "{\n" + genToPacket + ";\n}"
    else
      f.puts ";"
    end
  end

  def addDeserializeArg(arg)
    
    @deserializeArgs.push arg
    
  end
  
  def deserializeBase(arg)
    @deserializeBase = arg
  end
  
  def genSFMLConstructor(f, opts)
    
    tmpargs = @CArgs + @deserializeArgs
    tmpbaseconstructor = @BaseConstructor 
    
    if not @deserializeBase.empty?
      tmpbaseconstructor += ", " + @deserializeBase
    end

    f.write "#{@Name}(" +
            if not tmpargs.empty?
              tmpargs.join(", ") + ", "
            else
              ""
            end +
            "sf::Packet &packet) "

    if opts.include?(:impl)
    
      # Base constructor
      f.puts(
        if(not tmpbaseconstructor.empty?)
          ": #{@BaseClass}(#{tmpbaseconstructor}) {"
        else
          "{"
        end
      )
      
      f.write genDeserializer
      
      # Error check
      f.puts "if(!packet)\n    //Error loading\n" +
             "    throw InvalidArgument(\"Invalid packet format for: '#{@Name}'\");" +
             "\n}"
    else
      f.puts ";"
    end
  end

  def genDeserializer

    str = ""
    
    insideBracketExpression = false

    @Members.each do |a|

      if a[:as].nil?
        if not insideBracketExpression
        
          str += "packet"
          insideBracketExpression = true
          
        end
        
        str += " >> " + a[:name]
        
      else
        
        if insideBracketExpression
          
          str += ";\n"
          insideBracketExpression = false
        end
      
        tempName = "temp_#{a[:name]}"
        str += "#{a[:as]} #{tempName};\n"
        str += "packet >> #{tempName};\n"
        str += "#{a[:name]} = static_cast<#{a[:type]}>(#{tempName});\n"
      end


      
    end
    
    if insideBracketExpression
          
      str += ";\n"
    end

    str
  end
end

class ResponseClass < SFMLSerializeClass

  def genSerializer(f, opts)
    f.write "void _SerializeCustom(sf::Packet &packet) const #{override opts}"

    if opts.include?(:impl)
      f.puts "{\n" + genToPacket + ";\n}"
    else
      f.puts ";"
    end
  end
  
end


class GameWorldClass < OutputClass

  def initialize(name, componentTypes: [], systems: [], tickrunmethod: "")

    super name

    @BaseClass = "GameWorld"

    @TickRunMethod = tickrunmethod

    @ComponentTypes = componentTypes
    @Systems = systems

    @ComponentTypes.each{|c|
      @Members.push({type: "ComponentHolder<" + c.type + ">", name: "Component" + c.type})
    }

    @Systems.each{|s|
      @Members.push({type: s.type, name: "_" + s.type})
    }
  end

  def genMemberConstructor(f, opts)

    f.write "#{qualifier opts}#{@Name}()"
    
    if opts.include?(:impl)
      f.puts "{}"
    else
      f.puts ";"
    end

  end

  def genMembers(f, opts)

    f.puts "protected:"
    super f, opts
    
  end

  def genMethods(f, opts)

    f.write "DLLEXPORT void #{qualifier opts}_ResetComponents() #{override opts}"

    if opts.include?(:impl)
      f.puts "{"
      f.puts "// Reset all component holders //"
      @ComponentTypes.each{|c|
        f.puts "Component#{c.type}.Clear();"
      }
      f.puts "}"
    else
      f.puts ";"
    end

    f.write "DLLEXPORT void #{qualifier opts}_ResetSystems() #{override opts}"

    if opts.include?(:impl)
      f.puts "{"
      f.puts "// Reset all system nodes //"
      @Systems.each{|s|

        if !s.NodeComponents.empty?
          f.puts "_#{s.type}.Clear();"
        end
      }
      f.puts "}"
    else
      f.puts ";"
    end

    firstLoop = true

    f.puts "// Component types (#{@ComponentTypes.length}) //"

    @ComponentTypes.each{|c|

      if firstLoop and opts.include?(:header)
        f.puts "//! \\brief Returns a reference to a component of wanted type"
        f.puts "//! \\exception NotFound when the specified entity doesn't have a component of"
        f.puts "//! the wanted type"
      end
      
      f.write "DLLEXPORT #{c.type}& #{qualifier opts}GetComponent_#{c.type}(ObjectID id)"
      if opts.include?(:impl)
        f.puts "{"
        f.puts "auto component = Component#{c.type}.Find(id);"
        f.puts "if(!component)"
        f.puts "    throw NotFound(\"Component for entity with id was not found\");"
        f.puts ""
        f.puts "return *component;"
       
        f.puts "}"
      else
        f.puts ";"
      end

      if firstLoop and opts.include?(:header)
        f.puts "//! \\brief Destroys a component belonging to an entity"
        f.puts "//! \\return True when destroyed, false if the entity didn't have a component "
        f.puts "//! of this type"
      end
      
      f.write "DLLEXPORT bool #{qualifier opts}RemoveComponent_#{c.type}(ObjectID id)"
      if opts.include?(:impl)
        f.puts "{"

        f.puts "try {"
        f.puts "    Component#{c.type}.Destroy(id, false);"
        f.puts "    //_OnComponentDestroyed(id, #{c.type}::TYPE());"
        f.puts "    return true;"
        f.puts "}"
        f.puts "catch (...) {"
        f.puts "    return false;"
        f.puts "}"
        
        f.puts "}"
      else
        f.puts ";"
      end

      if firstLoop and opts.include?(:header)
        f.puts "//! \\brief Creates a new component for entity"
        f.puts "//! \\exception Exception if the component failed to init or it already exists"
      end

      c.constructors.each{|a|

        f.write "DLLEXPORT #{c.type}& #{qualifier opts}Create_#{c.type}(ObjectID id" +
                a.formatParameters(opts) + ")"

        if opts.include?(:impl)
          f.write "\n"
          f.puts "{"

          f.puts "return *Component#{c.type}.ConstructNew(id" +
                 a.formatNames(c.type) + ");"
          
          f.puts "}"
        else
          f.puts ";"
        end
      }

      


      f.puts ""
      firstLoop = false
    }
    
    @Systems.each{|s|
      
    }


    f.write "DLLEXPORT void #{qualifier opts}DestroyAllIn(ObjectID id) #{override opts}"

    if opts.include?(:impl)
      f.puts "{"
      f.puts @BaseClass + "::DestroyAllIn(id);"
      @ComponentTypes.each{|c|
        f.puts "Component#{c.type}.Destroy(id, false);"
      }
      f.puts "}"
    else
      f.puts ";"
    end

    f.write "DLLEXPORT std::tuple<void*, bool> #{qualifier opts}GetComponent(" +
            "ObjectID id, Leviathan::COMPONENT_TYPE type) #{override opts}"

    if opts.include?(:impl)
      f.puts "{"
      f.puts "const auto baseType = " + @BaseClass + "::GetComponent(id, type);"

      f.puts "if(std::get<1>(baseType))"
      f.puts "    return baseType;"
      f.puts ""

      f.puts "switch(type){"
      
      @ComponentTypes.each{|c|
        f.puts "case #{c.type}::TYPE:"
        f.puts "{"
        f.puts "return std::make_tuple(Component#{c.type}.Find(id), true);"
        f.puts "}"
      }

      f.puts "default:"
      f.puts "return std::make_tuple(nullptr, false);"
      f.puts "}"
      f.puts "}"
    else
      f.puts ";"
    end

    f.puts <<-END
//! Helper for getting component of type. This is much slower than
//! direct lookups with the actual implementation class' GetComponent_Position etc.
//! methods
//! \\exception NotFound if entity has no component of the wanted type
//!
//! This is copied here as a method with the same name would overwrite this otherwise
template<class TComponent>
TComponent& GetComponent(ObjectID id){

    std::tuple<void*, bool> component = GetComponent(id, TComponent::TYPE);

    if(!std::get<1>(component))
        throw Leviathan::InvalidArgument("Unrecognized component type as template parameter");

    void* ptr = std::get<0>(component);

    if(!ptr)
        throw Leviathan::NotFound("Component for entity with id was not found");
    
    return *static_cast<TComponent*>(ptr);
}    
END

    f.write "DLLEXPORT void #{qualifier opts}RunFrameRenderSystems(int tick, " +
            "int timeintick) #{override opts}"

    if opts.include?(:impl)
      f.puts "{"
      f.puts @BaseClass + "::HandleAdded();"
      f.puts ""
      f.puts @TickRunMethod
      
      f.puts "}"
    else
      f.puts ";"
    end

    if opts.include?(:header)
      f.puts "protected:"
    end

    f.write "DLLEXPORT void #{qualifier opts}HandleAdded() #{override opts}"

    if opts.include?(:impl)
      f.puts "{"
      f.puts @BaseClass + "::HandleAdded();"
      f.puts ""
      
      @ComponentTypes.each{|c|
        f.puts "const auto& added#{c.type} = Component#{c.type}.GetAdded();"
      }

      f.puts ""
      @Systems.each{|s|
        
        if !s.NodeComponents.empty?
          f.puts "_#{s.type}.CreateNodes("
          f.puts "    " + (s.NodeComponents.map{|c| "added" + c }.join(", ")) + ","
          f.puts "    " + (s.NodeComponents.map{|c| "Component" + c }.join(", ")) + ");"
          
        end
      }
      

      f.puts "}"
    else
      f.puts ";"
    end

    f.write "DLLEXPORT void #{qualifier opts}ClearAdded() #{override opts}"

    if opts.include?(:impl)
      f.puts "{"
      f.puts @BaseClass + "::ClearAdded();"
      f.puts ""

      @ComponentTypes.each{|c|
        f.puts "Component#{c.type}.ClearAdded();"
      }

      f.puts "}"
    else
      f.puts ";"
    end
    
    # f.puts "public:"
    
  end

end

class ConstructorInfo
  attr_reader :Parameters, :UseDataStruct

  def initialize(parameters, usedatastruct: false)
    @Parameters = parameters
    @UseDataStruct = usedatastruct
  end

  def formatParameters(opts)
    if @Parameters.empty?
      ""
    else
      ", " + @Parameters.select{|p| !p.NonMethodParam}.map{
        |p| p.formatForParams opts
      }.join(", ")
    end
  end

  def formatNames(componentclass)
    if @Parameters.empty?
      if @UseDataStruct
        ", #{componentclass}::Data{}"
      else
        ""
      end
    else
      if @UseDataStruct
        ", #{componentclass}::Data{" + @Parameters.map{|p| p.Name}.join(", ") + "}"
      else
        ", " + @Parameters.map{|p| p.Name}.join(", ")
      end
    end
  end
  
end

class Variable
  attr_reader :Name, :Type, :Default, :NonMethodParam

  def initialize(name, type, default: nil, noRef: false, noConst: false, nonMethodParam: false)

    @Name = name
    @Type = type
    @Default = default

    if !@Default.nil?
      if @Default == true
        @Default = "true"
      elsif @Default == false
        @Default = "false"
      end
    end
    
    @NoRef = noRef
    @NoConst = noConst
    @NonMethodParam = nonMethodParam
  end

  def formatDefault(opts)
    if @Default.nil?
      ""
    else
      if opts.include?(:header)
        " = " + @Default
      else
        "/* = #{@Default} */"
      end
    end
  end

  def formatForParams(opts)
    if @NoRef
      "#{@Type} #{@Name}#{formatDefault opts}"
    else
      if @NoConst
        "#{@Type} &#{@Name}#{formatDefault opts}"
      else
        "const #{@Type} &#{@Name}#{formatDefault opts}"
      end
    end
  end
end

# Components for adding to gameworld
class EntityComponent
  
  attr_reader :type, :constructors
  
  def initialize(type, constructors=[ConstructorInfo.new])
    @type = type
    @constructors = constructors
  end

end

class EntitySystem
  attr_reader :type, :NodeComponents

  # Leave nodeComponens empty if not using combined nodes
  def initialize(type, nodeComponents=[])
    @type = type
    @NodeComponents = nodeComponents
  end
end
