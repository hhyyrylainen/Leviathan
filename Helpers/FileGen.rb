# Common classes for generating C++ code with cmake
require 'fileutils'


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

class OutputClass

  def initialize(name, members: [], constructors: nil, nodllexport: false, methods: nil,
                 copyconstructors: false, copyoperators: false)

    @Name = name
    @Members = members
    @BaseClass = ""
    @BaseConstructor = ""
    @CArgs = []
    # Additional constructors
    @Constructors = constructors
    @NoDLLExport = nodllexport
    @Methods = methods
    @CopyConstructors = copyconstructors
    @CopyOperators = copyoperators
  end

  def setExport(exportmacro)
    @ExportMacro = exportmacro
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

    if @CopyConstructors

      genCopyConstructors f, opts
      f.puts ""
    end

    if @CopyOperators
      genCopyOperators f, opts
      f.puts ""
    end
    
    genMethods f, opts
    f.puts ""
    genFromMethodList f, opts

    if opts.include?(:header)
      genMembers f, opts
    end
  end

  def genMethods(f, opts)
    f.puts ""
  end

  def genFromMethodList(f, opts)
    if !@Methods
      return
    end

    @Methods.each{|m|

      f.write "#{export}#{m.ReturnType} #{qualifier opts}#{m.Name}(" +
              m.formatParameters(opts) + ")"
      
      if opts.include?(:impl)
        f.puts "{"
        f.puts m.Body
        f.puts "}"
      else
        f.puts ";"
      end
    }

    f.puts ""
  end

  def genMemberConstructor(f, opts)
    str = "#{export}#{qualifier opts}#{@Name}("

    if not @CArgs.empty?

      str += @CArgs.join(", ") + ", "
      
    end
    
    str += @Members.map{|a|

      a.formatForParams opts
    }.join(", ")

    if opts.include?(:impl)
      # Initializer list
      str += ") :\n"

      # Base constructor
      if not @BaseConstructor.empty?
        str += "#{@BaseClass}(#{@BaseConstructor}),\n"
      end
      
      str += @Members.map{|a|

        a.formatInitializer
        
      }.join(", ")

      str += "\n{}\n"
      
    else
      str += ");\n"
    end

    f.write str

    if @Constructors
      f.puts "// Extra constructors"

      @Constructors.each{|constructor|

        f.write "#{export}#{qualifier opts}#{@Name}("

        f.write constructor.formatParameters(opts, leadingcomma: false)

        if opts.include?(:impl)
          f.write ")"

          memberInitializers = constructor.formatMemberInitializers

          # Base constructor
          if constructor.BaseParameters
            if not memberInitializers.empty?
              f.write " : #{@BaseClass}(#{constructor.BaseParameters}),\n"
              memberInitializers.sub! " : ", ""
            else
              f.write " : #{@BaseClass}(#{constructor.BaseParameters})\n"
            end
          else
            if not @BaseConstructor.empty?
              if not memberInitializers.empty?
                f.write " : #{@BaseClass}(#{@BaseConstructor}),\n"
                memberInitializers.sub! " : ", ""
              else
                f.write " : #{@BaseClass}(#{@BaseConstructor})\n"
              end
            end
          end
          
          f.puts memberInitializers

          f.puts "{"

          f.puts "}"
        else
          f.puts ");"
        end
        
      }
      
    end
  end

  def genCopyConstructors(f, opts)
    # copy
    f.write "#{export}#{qualifier opts}#{@Name}(const #{@Name}& other) noexcept"

    if opts.include?(:impl)
      f.puts " :"

      # Base
      if @BaseClass
        f.write "#{@BaseClass}(other)"

        if not @Members.empty?
          f.write ", "
        end
      end
      
      f.puts @Members.map{|a|

        a.formatCopy
        
      }.join(", ")

      f.puts "{}"
      
    else
      f.puts ";"
    end

    # move
    f.write "#{export}#{qualifier opts}#{@Name}(#{@Name}&& other) noexcept"

    if opts.include?(:impl)
      f.puts " :"

      # Base
      if @BaseClass
        f.write "#{@BaseClass}(std::move(other))"

        if not @Members.empty?
          f.write ", "
        end
      end
      
      f.puts @Members.map{|a|

        a.formatMove
        
      }.join(", ")

      f.puts "{}"
      
    else
      f.puts ";"
    end
  end

  def genCopyOperators(f, opts)
    # copy
    f.write "#{export}#{@Name}& #{qualifier opts}operator=(const #{@Name}& other) noexcept"

    if opts.include?(:impl)

      f.puts "{"
      
      # Base
      if @BaseClass
        f.puts "#{@BaseClass}::operator=(other);"
      end
      
      @Members.each{|a|

        f.puts a.Name + " = other.#{a.Name};"
      }

      f.puts "return *this;"
      f.puts "}"
      
    else
      f.puts ";"
    end

    # move
    f.write "#{export}#{@Name}& #{qualifier opts}operator=(#{@Name}&& other) noexcept"

    if opts.include?(:impl)

      f.puts "{"      

      # Base
      if @BaseClass
        f.puts "#{@BaseClass}::operator=(std::move(other));"
      end
      
      @Members.each{|a|

        f.puts a.Name + " = std::move(other.#{a.Name});"
      }

      f.puts "return *this;"
      f.puts "}"
      
    else
      f.puts ";"
    end
  end
  
  def genMembers(f, opts)
    
    f.puts @Members.map { |a|

      a.formatDefinition
    }.join("\n")
  end
  
  def base(baseName)
    @BaseClass = baseName
  end

  def baseConstructor(arguments)
    @BaseConstructor = arguments
  end

  def addMember(arguments = {})
    raise "addMember not Variable" if not arguments.class == Variable
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
      " override"
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

  # Returns value if :header set
  def default(opts, value)
    if opts.include?(:header)
      " = #{value}"
    else
      ""
    end
  end

  # Returns #{export}if not disabled
  def export
    if @NoDLLExport
      ""
    else
      if !@ExportMacro.nil?
        "#{@ExportMacro} "
      else
        ""
      end
    end
  end
end

class SFMLSerializeClass < OutputClass
  
  def initialize(name, **basekeywords)
    
    super name, **basekeywords
    @deserializeArgs = []
    @deserializeBase = ""
  end

  def genMethods(f, opts)
    f.puts ""

    f.puts "// Packet constructor and serializer //"
    
    genSerializer f, opts
    genSFMLConstructor f, opts
  end

  def genToPacket
    "packet << " +
      @Members.map { |a|
      a.formatSerializer
     
    }.join(" << ")
  end

  def genSerializer(f, opts)

    f.write "#{export}void #{qualifier opts}AddDataToPacket(sf::Packet &packet) const"
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
    
    f.write "#{export}#{qualifier opts}#{@Name}(" +
            if not tmpargs.empty?
              tmpargs.join(", ") + ", "
            else
              ""
            end +
            "sf::Packet &packet)"

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
      f.puts "\nif(!packet)\n    //Error loading\n" +
             "    throw Leviathan::InvalidArgument(\"Invalid packet format for: " +
             "'#{@Name}'\");" +
             "\n}"
    else
      f.puts ";"
    end
  end

  def genDeserializer

    str = ""
    
    insideBracketExpression = false

    @Members.each do |a|

      str += a.formatDeserializer "packet"
      
    end
    
    if insideBracketExpression
          
      str += ";\n"
    end

    str
  end
end

class ResponseClass < SFMLSerializeClass

  def genSerializer(f, opts)
    f.write "#{export}void #{qualifier opts}_SerializeCustom(sf::Packet &packet) " +
            "const#{override opts}"

    if opts.include?(:impl)
      f.puts "{\n" + genToPacket + ";\n}"
    else
      f.puts ";"
    end
  end
  
end


class ComponentState < SFMLSerializeClass

  def initialize(name, members: [], constructors: nil, methods: nil, statebits: nil,
                 copyconstructors: false, copyoperators: false)

    # members = [Variable.new("TickNumber", "int", noRef: true)] + members
    
    super name, members: members, constructors: constructors, methods: methods,
          copyconstructors: copyconstructors, copyoperators: copyoperators

    @BaseClass = "BaseComponentState"
    @StateBits = statebits

    @Type = "COMPONENT_TYPE::#{name.sub 'State', ''}"
    @BaseConstructor = "-1, #{@Type}"
  end

  def genSerializer(f, opts)

    f.write "#{export}void #{qualifier opts}AddDataToPacket(sf::Packet &packet, " +
            "BaseComponentState* olderstate) const#{override opts}"
    if opts.include?(:impl)
      f.puts "{\n"
      f.puts "packet << static_cast<uint16_t>(#{@Type});"
      f.puts genToPacket + ";"
      f.puts "}"
    else
      f.puts ";"
    end
  end

  def genMethods(f, opts)

    super f, opts
    
    f.puts ""

    f.write "#{export}bool #{qualifier opts}FillMissingData(BaseComponentState& otherstate)" +
            "#{override opts}"
    if opts.include?(:impl)
      f.puts "{"

      if @StateBits

        puts "statebits handling not implemented"
        exit 2 
      else
        f.puts "// This has no state subdivision //"
        f.puts "return true;"
      end
      
      f.puts "}"
    else
      f.puts ";"
    end
  end
  
end


class GameWorldClass < OutputClass

  attr_accessor :WorldType

  def initialize(name, componentTypes: [], systems: [], systemspreticksetup: nil,
                 framesystemrun: "", networking: true)

    super name

    @NetworkingEnabled = networking
    
    @BaseClass = "GameWorld"
    @WorldType = "-1 /* unset, won't work over the network */"

    @FrameSystemRun = framesystemrun
    @SystemsPreTickSetup = systemspreticksetup

    @ComponentTypes = componentTypes
    @Systems = systems
    # Used for a few specific things that need to be generated in this case
    @HasSendable = false

    @ComponentTypes.each{|c|
      @Members.push(Variable.new("Component" + c.type, "Leviathan::ComponentHolder<" + c.type +
                                                       ">"))

      if c.StateType
        @Members.push(Variable.new(c.type + "States", "Leviathan::StateHolder<" + c.type +
                                                      "State>"))
      end

      if c.type == "Sendable"
        @HasSendable = true
      end

      if c.type == "Received"
        @HasReceived = true
      end
    }

    @Systems.each{|s|
      @Members.push(Variable.new("_" + s.Type, s.Type))
    }
    
  end

  def genMemberConstructor(f, opts)

    f.write "#{export}#{qualifier opts}#{@Name}(const " +
            "std::shared_ptr<Leviathan::PhysicsMaterialManager>& physicsMaterials, " +
            "int worldid#{default opts, '-1'})"
    
    if opts.include?(:impl)
      f.puts " : #{@BaseClass}(#{@WorldType},\n physicsMaterials, worldid) "
      f.puts "{}"
    else
      f.puts ";"
    end

    # Child class type overwrite constructor
    f.write "#{export}#{qualifier opts}#{@Name}(int32_t typeoverride, const " +
            "std::shared_ptr<Leviathan::PhysicsMaterialManager>& physicsMaterials, " +
            "int worldid#{default opts, '-1'})"
    
    if opts.include?(:impl)
      f.puts " : #{@BaseClass}(typeoverride, physicsMaterials, worldid) "
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

    f.write "#{export}void #{qualifier opts}_ResetOrReleaseComponents()#{override opts}"

    if opts.include?(:impl)
      f.puts "{"

      f.puts @BaseClass + "::_ResetOrReleaseComponents();"
      
      f.puts "// Reset all component holders //"
      @ComponentTypes.each{|c|
        if c.Release
          f.puts "Component#{c.type}.ReleaseAllAndClear(" +
                 if c.Release.length > 0
                   c.Release.join(", ") else
                   ""
                 end +");"
        else
          f.puts "Component#{c.type}.Clear();"
        end
      }
      f.puts "}"
    else
      f.puts ";"
    end

    f.write "#{export}void #{qualifier opts}_ResetSystems()#{override opts}"

    if opts.include?(:impl)
      f.puts "{"
      f.puts @BaseClass + "::_ResetSystems();"
      f.puts "// Reset all system nodes //"
      @Systems.each{|s|

        if !s.NodeComponents.empty? and !s.NoState
          f.puts "_#{s.Type}.Clear();"
        end
      }
      f.puts "}"
    else
      f.puts ";"
    end

    firstLoop = true
    stateHolderCommentPrinted = false

    f.puts "// Component types (#{@ComponentTypes.length}) //"

    @ComponentTypes.each{|c|

      if firstLoop and opts.include?(:header)
        f.puts "//! \\brief Returns a reference to a component of wanted type"
        f.puts "//! \\exception NotFound when the specified entity doesn't have a component of"
        f.puts "//! the wanted type"
        f.puts "//! \\note This is the recommended way to get components. \n"
        f.puts "//! AngelScript uses the Ptr variant but with this name to not throw " +
               "exceptions to the scripts"
      end
      
      f.write "#{export}#{c.type}& #{qualifier opts}GetComponent_#{c.type}(ObjectID id)"
      if opts.include?(:impl)
        f.puts "{"
        f.puts "auto component = Component#{c.type}.Find(id);"
        f.puts "if(!component)"
        f.puts "    throw Leviathan::NotFound(\"Component for entity with id was not found\");"
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
      
      f.write "#{export}bool #{qualifier opts}RemoveComponent_#{c.type}(ObjectID id)"
      if opts.include?(:impl)
        f.puts "{"

        if c.Release
          f.puts "    const bool destroyed = Component#{c.type}.ReleaseIfExists(id, true" +
                 if c.Release.length > 0
                   ", " + c.Release.join(", ") else
                   ""
                 end +");"
        else
          f.puts "    const bool destroyed = Component#{c.type}.DestroyIfExists(id, true);"
        end
        f.puts "    //_OnComponentDestroyed(id, #{c.type}::TYPE());"
        f.puts "    return destroyed;"
        
        f.puts "}"
      else
        f.puts ";"
      end

      if firstLoop and opts.include?(:header)
        f.puts "//! \\brief Creates a new component for entity"
        f.puts "//! \\exception Exception if the component failed to init or it already exists"
      end

      c.constructors.each{|a|

        f.write "#{export}#{c.type}& #{qualifier opts}Create_#{c.type}(ObjectID id" +
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

      if opts.include?(:header)
        if c.StateType
          
          if !stateHolderCommentPrinted and opts.include?(:header)
            f.puts "//! \\brief Returns state holder for component type"
            stateHolderCommentPrinted = true
          end

          f.puts "#{export}StateHolder<#{c.type}State>& GetStatesFor_#{c.type}(){"
          
          f.puts "    return #{c.type}States;"
          f.puts "}"
        end
      end

      if firstLoop and opts.include?(:header)
        f.puts "//! \\brief Returns a pointer to entity's component if it has one of this type"
        f.puts "//! \\returns nullptr if not found"
        f.puts "//! \\note This is not the recommended way. Use GetComponent_ instead"
      end
      
      f.write "#{export}#{c.type}* #{qualifier opts}GetComponentPtr_#{c.type}(ObjectID id)"
      if opts.include?(:impl)
        f.puts "{"
        f.puts "return Component#{c.type}.Find(id);"
        f.puts "}"
      else
        f.puts ";"
      end      

      if opts.include?(:header)
        f.write "#{export}inline const auto& #{qualifier opts}GetComponentIndex_#{c.type}()"

        f.puts "{"
        f.puts "    return Component#{c.type}.GetIndex();"
        f.puts "}"
      end
      
      f.puts ""
      firstLoop = false
    }
    

    f.write "#{export}void #{qualifier opts}DestroyAllIn(ObjectID id)#{override opts}"

    if opts.include?(:impl)
      f.puts "{"
      f.puts @BaseClass + "::DestroyAllIn(id);"
      @ComponentTypes.each{|c|
        if c.Release
          f.puts "Component#{c.type}.ReleaseIfExists(id, true" +
                 if c.Release.length > 0
                   ", " + c.Release.join(", ") else
                   ""
                 end +");"
        else
          f.puts "Component#{c.type}.DestroyIfExists(id, true);"
        end
      }
      f.puts "}"
    else
      f.puts ";"
    end

    f.write "#{export}std::tuple<void*, bool> #{qualifier opts}GetComponent(" +
            "ObjectID id, Leviathan::COMPONENT_TYPE type)#{override opts}"

    if opts.include?(:impl)
      f.puts "{"
      f.puts "switch(static_cast<uint16_t>(type)){"
      
      @ComponentTypes.each{|c|
        f.puts "case static_cast<uint16_t>(#{c.type}::TYPE):"
        f.puts "{"
        f.puts "return std::make_tuple(Component#{c.type}.Find(id), true);"
        f.puts "}"
      }

      f.puts "default:"
      
      f.puts "return " + @BaseClass + "::GetComponent(id, type);"
      f.puts "}"
      f.puts "}"
    else
      f.puts ";"
    end

    f.write "#{export}std::tuple<void*, Leviathan::ComponentTypeInfo, bool> " +
            "#{qualifier opts}GetComponentWithType(" +
            "ObjectID id, Leviathan::COMPONENT_TYPE type)#{override opts}"

    if opts.include?(:impl)
      f.puts "{"
      f.puts "switch(static_cast<uint16_t>(type)){"
      
      @ComponentTypes.each{|c|
        f.puts "case static_cast<uint16_t>(#{c.type}::TYPE):"
        f.puts "{"
        f.puts "auto* ptr = Component#{c.type}.Find(id);"
        f.puts "if(!ptr)"
        f.puts "    return std::make_tuple(nullptr, " +
               "Leviathan::ComponentTypeInfo(-1, -1), true);"
        f.puts "return std::make_tuple(ptr, "
        f.puts "    Leviathan::ComponentTypeInfo(static_cast<uint16_t>(#{c.type}::TYPE), " 
        f.puts "        Leviathan::AngelScriptTypeIDResolver<#{c.type}>::Get("
        f.puts "        Leviathan::GetCurrentGlobalScriptExecutor())), true);"
        f.puts "}"
      }

      f.puts "default:"
      
      f.puts "return " + @BaseClass + "::GetComponentWithType(id, type);"
      f.puts "}"
      f.puts "}"
    else
      f.puts ";"
    end


    f.write "#{export}bool #{qualifier opts}GetRemovedFor(" +
            "Leviathan::COMPONENT_TYPE type, std::vector<std::tuple<void*, ObjectID>>& " +
            "result)#{override opts}"

    if opts.include?(:impl)
      f.puts "{"
      f.puts "switch(static_cast<uint16_t>(type)){"
      
      @ComponentTypes.each{|c|
        f.puts "case static_cast<uint16_t>(#{c.type}::TYPE):"
        f.puts "{"
        f.puts "auto& vec = Component#{c.type}.GetRemoved();"
        f.puts "result.insert(std::end(result), std::begin(vec), std::end(vec));"
        f.puts "return true;"
        f.puts "}"
      }

      f.puts "default:"
      f.puts "return #{@BaseClass}::GetRemovedFor(type, result);"
      f.puts "}"
      f.puts "}"
    else
      f.puts ";"
    end

    f.write "#{export}bool #{qualifier opts}GetAddedFor(" +
            "Leviathan::COMPONENT_TYPE type, std::vector<std::tuple<void*, ObjectID," +
            "Leviathan::ComponentTypeInfo>>& result)#{override opts}"

    if opts.include?(:impl)
      f.puts "{"
      f.puts "switch(static_cast<uint16_t>(type)){"
      
      @ComponentTypes.each{|c|
        f.puts "case static_cast<uint16_t>(#{c.type}::TYPE):"
        f.puts "{"
        f.puts "auto& vec = Component#{c.type}.GetAdded();"
        f.puts "result.reserve(result.size() + vec.size());"
        f.puts "for(const auto& res : vec){"
        f.puts "    result.push_back(std::make_tuple(std::get<0>(res), std::get<1>(res), "
        f.puts "        Leviathan::ComponentTypeInfo(static_cast<uint16_t>(#{c.type}::TYPE), " 
        f.puts "        Leviathan::AngelScriptTypeIDResolver<#{c.type}>::Get("
        f.puts "        Leviathan::GetCurrentGlobalScriptExecutor()))));"
        f.puts "}"
        f.puts "return true;"
        f.puts "}"
      }

      f.puts "default:"
      f.puts "return #{@BaseClass}::GetAddedFor(type, result);"
      f.puts "}"
      f.puts "}"
    else
      f.puts ";"
    end
    

    # Gets for systems
    if opts.include?(:header)
      f.puts "// System gets"
      @Systems.each{|s|

        f.puts "#{export}#{s.Type}& Get#{s.Type}(){ return _#{s.Type}; }"
      }

      f.puts ""
    end
    
    if opts.include?(:header)
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

template<class TComponent>
Leviathan::StateHolder<typename TComponent::StateT>& GetStatesFor(){

    std::tuple<void*, bool> stateHolder = GetStatesFor(TComponent::TYPE);

    if(!std::get<1>(stateHolder))
        throw InvalidArgument("Unrecognized component type as template parameter for "
            "state holder");

    void* ptr = std::get<0>(stateHolder);
    
    return *static_cast<Leviathan::StateHolder<typename TComponent::StateT>*>(ptr);
}
END
    end

    f.write "#{export}std::tuple<void*, bool> #{qualifier opts}GetStatesFor(" +
            "Leviathan::COMPONENT_TYPE type)#{override opts}"

    if opts.include?(:impl)
      f.puts "{"
      f.puts "switch(static_cast<uint16_t>(type)){"
      
      @ComponentTypes.each{|c|

        if !c.StateType
          next
        end
        
        f.puts "case static_cast<uint16_t>(#{c.type}::TYPE):"
        f.puts "{"
        f.puts "return std::make_tuple(&#{c.type}States, true);"
        f.puts "}"
      }

      f.puts "default:"      
      f.puts "return " + @BaseClass + "::GetStatesFor(type);"
      f.puts "}"
      f.puts "}"
    else
      f.puts ";"
    end

    if @NetworkingEnabled
      
      f.write "#{export}#{virtual opts} void #{qualifier opts}CaptureEntityState(" +
              "ObjectID id, Leviathan::EntityState& curstate) " +
              "const #{override opts}"

      if opts.include?(:impl)
        f.puts "{"

        @ComponentTypes.each{|c|

          if !c.StateType
            next
          end

          f.puts ""
          f.puts "const auto& #{c.type.downcase} = Component#{c.type}.Find(id);"
          f.puts "if(#{c.type.downcase})"
          f.puts "    curstate.Append(std::make_unique<#{c.type}State>(#{c.type}States." +
                 "CreateStateForSending("
          f.puts "        *#{c.type.downcase}, GetTickNumber())));"
        }

        f.puts ""
        f.puts "#{@BaseClass}::CaptureEntityState(id, curstate);"
        f.puts "}"
      else
        f.puts ";"
      end
      
      f.write "#{export}#{virtual opts} uint32_t #{qualifier opts}CaptureEntityStaticState(" +
              "ObjectID id, sf::Packet& receiver) " +
              "const #{override opts}"

      if opts.include?(:impl)
        f.puts "{"

        f.puts "uint32_t addedComponentCount = 0;"
        
        @ComponentTypes.each{|c|

          # Skip types like Sendable and Received
          if c.NoSynchronize
            next
          end

          f.puts ""
          f.puts "const auto& #{c.type.downcase} = Component#{c.type}.Find(id);"
          f.puts "if(#{c.type.downcase}){"
          f.puts "    ++addedComponentCount;"
          f.puts "    receiver << static_cast<uint16_t>(#{c.type}::TYPE);"

          c.constructors[0].Parameters.each{|p|

            if p.NonMethodParam or p.NonSerializeParam
              next
            end

            f.puts "    receiver << #{c.type.downcase}->#{p.MemberAccess};"
          }
          
          f.puts "}"
        }

        f.puts ""
        f.puts "return addedComponentCount + " +
               "#{@BaseClass}::CaptureEntityStaticState(id, receiver);"
        f.puts "}"
      else
        f.puts ";"
      end

      f.puts ""

    end

    f.write "#{export}void #{qualifier opts}RunFrameRenderSystems(int tick, " +
            "int timeintick)#{override opts}"

    if opts.include?(:impl)
      f.puts "{"
      f.puts @BaseClass + "::RunFrameRenderSystems(tick, timeintick);"
      f.puts ""
      f.puts @FrameSystemRun
      f.puts ""
      
      renderSystems = @Systems.select{|s| !s.RunRender.nil?}.sort_by {|x| x.RunRender[:group]}

      outGroup = nil
      
      renderSystems.each{|s|

        if outGroup != s.RunRender[:group]
          outGroup = s.RunRender[:group]
          f.puts "// Begin of group #{s.RunRender[:group]} //"
        end
        
        f.puts "_#{s.Type}.Run(*this" +
               formatEntitySystemParameters(s.RunRender) + ");"
      }
      
      f.puts "}"
    else
      f.puts ";"
    end

    if opts.include?(:header)
      f.puts "REFERENCE_HANDLE_UNCOUNTED_TYPE(#{@Name});"
      f.puts ""
    end

    if opts.include?(:header)
      f.puts "protected:"
    end

    f.write "#{export}void #{qualifier opts}_RunTickSystems()#{override opts}"

    if opts.include?(:impl)
      f.puts "{"
      f.puts @BaseClass + "::_RunTickSystems();"

      if @SystemsPreTickSetup
        f.puts @SystemsPreTickSetup
        f.puts ""
      end

      tickSystems = @Systems.select{|s| !s.RunTick.nil?}.sort_by {|x| x.RunTick[:group]}

      outGroup = nil
      
      tickSystems.each{|s|

        if outGroup != s.RunTick[:group]
          outGroup = s.RunTick[:group]
          f.puts "// Begin of group #{s.RunTick[:group]} //"
        end
        
        f.puts "_#{s.Type}.Run(*this" +
               formatEntitySystemParameters(s.RunTick) + ");"
      }
      f.puts "}"
    else
      f.puts ";"
    end

    f.write "#{export}void #{qualifier opts}HandleAddedAndDeleted()#{override opts}"

    if opts.include?(:impl)
      f.puts "{"
      f.puts @BaseClass + "::HandleAddedAndDeleted();"
      f.puts ""
      
      @ComponentTypes.each{|c|
        f.puts "const auto& added#{c.type} = Component#{c.type}.GetAdded();"
      }

      addedComment = false

      # Added types from parent world types
      alreadySet = {}
      
      @Systems.each{|s|
        
        if s.NodeComponents.empty?
          next
        end

        s.NodeComponents.each{|c|
          found = false

          @ComponentTypes.each{|doesMatch|
            if doesMatch.type == c
              found = true
              break
            end
          }

          if !found
            found = alreadySet.include? c
          end

          if !found
            alreadySet[c] = true
              
            if !addedComment
              addedComment = true
              f.puts ""
              f.puts "// Component types of parent type"
            end

            f.puts "const auto& added#{c} = Component#{c}.GetAdded();"
            f.puts "const auto& removed#{c} = Component#{c}.GetRemoved();"
          end
        }
      }

      f.puts ""
      f.puts ""
      f.puts "// Added"
      
      @Systems.each{|s|
        
        if !s.NodeComponents.empty?

          f.write "if(" + s.NodeComponents.map{|c| "!added" + c + ".empty()" }.join(" || ")
          f.puts "){"
          
          f.puts "    _#{s.Type}.CreateNodes("
          f.puts "        " + (s.NodeComponents.map{|c| "added" + c }.join(", ")) + ","
          f.puts "        " + (s.NodeComponents.map{|c| "Component" + c }.join(", ")) + ");"
          f.puts "}"
        end
      }

      f.puts "// Removed"
      @ComponentTypes.each{|c|
        f.puts "const auto& removed#{c.type} = Component#{c.type}.GetRemoved();"
      }

      @Systems.each{|s|
        
        if !s.NodeComponents.empty?

          f.write "if(" + s.NodeComponents.map{|c| "!removed" + c + ".empty()" }.join(" || ")
          f.puts "){"
          
          f.puts "    _#{s.Type}.DestroyNodes("
          f.puts "        " + (s.NodeComponents.map{|c| "removed" + c }.join(", ")) + ");"
          f.puts "}"
        end
      }      

      f.puts "}"
    else
      f.puts ";"
    end

    f.write "#{export}void #{qualifier opts}ClearAddedAndRemoved()#{override opts}"

    if opts.include?(:impl)
      f.puts "{"
      f.puts @BaseClass + "::ClearAddedAndRemoved();"
      f.puts ""

      @ComponentTypes.each{|c|
        f.puts "Component#{c.type}.ClearAdded();"
        f.puts "Component#{c.type}.ClearRemoved();"
      }

      f.puts "}"
    else
      f.puts ";"
    end

    f.write "#{export}void #{qualifier opts}_DoSystemsInit()#{override opts}"

    if opts.include?(:impl)
      f.puts "{"
      f.puts @BaseClass + "::_DoSystemsInit();"
      f.puts "// Call Init on all systems that need it //"
      @Systems.each{|s|

        if s.Init
          f.puts "_#{s.Type}.Init(#{s.Init.map(&:formatForArgumentList).join(', ')});"
        end
      }
      f.puts "}"
    else
      f.puts ";"
    end

    f.write "#{export}void #{qualifier opts}_DoSystemsRelease()#{override opts}"

    if opts.include?(:impl)
      f.puts "{"
      f.puts @BaseClass + "::_DoSystemsRelease();"
      f.puts "// Call Release on all systems that need it //"
      @Systems.each{|s|

        if s.Release
          f.puts "_#{s.Type}.Release(#{s.Release.map(&:formatForArgumentList).join(', ')});"
        end
      }
      f.puts "}"
    else
      f.puts ";"
    end

    if @HasSendable
      f.write "#{export}void #{qualifier opts}_CreateSendableComponentForEntity(" +
              "ObjectID id)#{override opts}"

      if opts.include?(:impl)
        f.puts "{"
        f.puts "Create_Sendable(id);"
        f.puts "}"
      else
        f.puts ";"
      end      
    end

    if @HasReceived
      f.write "#{export}void #{qualifier opts}_CreateReceivedComponentForEntity(" +
              "ObjectID id)#{override opts}"

      if opts.include?(:impl)
        f.puts "{"
        f.puts "Create_Received(id);"
        f.puts "}"
      else
        f.puts ";"
      end      
    end


    if @NetworkingEnabled
      f.write "#{export}void #{qualifier opts}_CreateComponentsFromCreationMessage(" +
              "ObjectID id, sf::Packet& data, int entriesleft, int decodedtype)" +
              "#{override opts}"

      if opts.include?(:impl)
        f.puts "{"

        f.puts "while(entriesleft > 0){"

        f.puts "if(decodedtype == -1){"
        f.puts "    // Type not decoded yet"
        f.puts "    uint16_t tmpType;"
        f.puts "    data >> tmpType;"
        f.puts "    decodedtype = tmpType;"
        f.puts "}"
        f.puts "if(!data){"
        f.puts %{    LOG_ERROR("GameWorld: entity decode: packet data ended too soon");}
        f.puts "    return;"
        f.puts "}"
        
        f.puts ""

        f.puts "switch(decodedtype){"

        counter = 0

        @ComponentTypes.each{|c|

          # Skip types like Sendable and Received
          if c.NoSynchronize
            next
          end

          f.puts "case static_cast<int>(#{c.type}::TYPE):"
          f.puts "{"

          # parameter decoding
          c.constructors[0].Parameters.each{|p|

            if p.NonMethodParam or p.NonSerializeParam
              next
            end

            f.puts "#{p.Type} #{p.Name.downcase};"
            f.puts "data >> #{p.Name.downcase};"
          }

          f.puts "if(!data){"
          f.puts %{    LOG_ERROR("GameWorld: entity decode: packet data ended } +
                 %{too soon (no component parameters)");}
          f.puts "    return;"
          f.puts "}"

          # Figuring out where to grab this data is hard...
          # should make the ECS used by Leviathan to be more pure to have this data
          # available easier
          c.constructors[0].Parameters.each{|p|

            if p.NonMethodParam or !p.NonSerializeParam
              next
            end

            helper = "helper#{counter += 1}"

            errorhelper = <<-END
if(!#{helper}){
LOG_ERROR("GameWorld: entity decode: magic deserialize on type '#{p.Type}' failed,"
"canceling entity creation");
throw InvalidArgument("can't find related required deserialize resources");
}
END

            case p.Type
            when "Ogre::SceneNode*"
              f.puts "auto #{helper} = GetComponentPtr_RenderNode(id);"
              f.puts errorhelper
              helperProperty = helper + "->Node";
            when "Ogre::Item*"
              # This won't be good enough once there can be multiple different types of these
              f.puts "auto #{helper} = GetComponentPtr_Model(id);"
              f.puts errorhelper
              helperProperty = helper + "->GraphicalObject";
            when "Position"
              f.puts "auto #{helper} = GetComponentPtr_Position(id);"
              f.puts errorhelper
              helperProperty = "*" + helper;
            else
              abort("Can't do magic deserialize on Variable: #{p.Type}")
            end

            f.puts "#{p.Type}& #{p.formatForArgumentList} = #{helperProperty};"  
          }        
          
          f.puts "Create_#{c.type}(id" + c.constructors[0].formatNamesForForward + ");"
          f.puts "--entriesleft;"
          f.puts "decodedtype = -1;"
          f.puts "continue;"

          f.puts "}"
        }

        f.puts "default:"
        f.puts "return #{@BaseClass}::_CreateComponentsFromCreationMessage(id, data, " +
               "entriesleft, decodedtype);"      
        
        f.puts "}"
        
        f.puts "}"
        f.puts "}"

      else
        f.puts ";"
      end
    end
    
    # f.puts "public:"
    
  end

  def genComponentBinding(c)
    str = ""

    opts = {header: true}

    str += %{if(engine->RegisterObjectMethod(classname, "#{c.type}@ GetComponent_#{c.type}} +
           %{(ObjectID id)", \n} +
           %{asMETHOD(WorldType, GetComponentPtr_#{c.type}), asCALL_THISCALL) < 0)\n} +
           "{\nANGELSCRIPT_REGISTERFAIL;\n}\n"
    str += %{if(engine->RegisterObjectMethod(classname, "#{c.type}@ } +
           %{RemoveComponent_#{c.type}(ObjectID id)", \n} +
           %{asMETHOD(WorldType, GetComponent_#{c.type}), asCALL_THISCALL) < 0)\n} +
           "{\nANGELSCRIPT_REGISTERFAIL;\n}\n\n"    

    c.constructors.each{|a|

      if a.NoAngelScript
        next
      end
    
      str += %{if(engine->RegisterObjectMethod(classname, "#{c.type}@ Create_#{c.type}} +
             %{(ObjectID id#{a.formatParametersAngelScript()})", \n} +
             %{asMETHODPR(WorldType, Create_#{c.type}, \n} +
             %{    (ObjectID id#{a.formatParameterTypes()}), #{c.type}&), \n} +
             %{asCALL_THISCALL) < 0)\n} +
             "{\nANGELSCRIPT_REGISTERFAIL;\n}\n\n"
    }

    str
  end
  
  def genAngelScriptBindings

    str = "\n"

    @ComponentTypes.each{|c|
      str += genComponentBinding c
    }

    str += "\n"

    @Systems.each{|s|
      if s.VisibleToScripts
        str += %{if(engine->RegisterObjectMethod(classname, "#{s.Type}@ Get#{s.Type}()",\n} +
               %{asMETHOD(WorldType, Get#{s.Type}), asCALL_THISCALL) < 0)\n} +
               "{\nANGELSCRIPT_REGISTERFAIL;\n}\n"
      end
    }
    
    str
  end
end

class ConstructorInfo
  attr_reader :Parameters, :UseDataStruct, :NoAngelScript, :BaseParameters

  def initialize(parameters, usedatastruct: false, memberinitializers: nil,
                 noangelscript: false, baseparameters: nil)
    @Parameters = parameters
    @UseDataStruct = usedatastruct
    @MemberInitializers = memberinitializers
    @NoAngelScript= noangelscript
    @BaseParameters = baseparameters
  end

  def formatMemberInitializers

    if !@MemberInitializers
      return ""
    end

    " : " + @MemberInitializers.map{|i| i[0] + "(" + i[1] + ")"}.join(", ")
  end

  # Formats the parameters of this function for a parameter list
  def formatParameters(opts, leadingcomma: true)
    if @Parameters.empty? or @Parameters.select{|p| !p.NonMethodParam}.empty?
      ""
    else
      (if leadingcomma then ", " else "" end) + @Parameters.select{|p| !p.NonMethodParam}.map{
        |p| p.formatForParams opts
      }.join(", ")
    end
  end

  # Formats the types of parameters of this function for a parameter list
  def formatParameterTypes(leadingcomma: true)
    if @Parameters.empty? or @Parameters.select{|p| !p.NonMethodParam}.empty?
      ""
    else
      (if leadingcomma then ", " else "" end) + @Parameters.select{|p| !p.NonMethodParam}.map{
        |p| p.formatType
      }.join(", ")
    end
  end

  #
  # AngelScript binding versions
  #
  def formatParametersAngelScript(leadingcomma: true)
    if @Parameters.empty? or @Parameters.select{|p| !p.NonMethodParam}.empty?
      ""
    else
      (if leadingcomma then ", " else "" end) + @Parameters.select{|p| !p.NonMethodParam}.map{
        |p| p.formatForParamsAngelScript
      }.join(", ")
    end
  end

  def formatNamesForForward(leadingcomma: true)
    if @Parameters.empty? or @Parameters.select{|p| !p.NonMethodParam}.empty?
      ""
    else
      (if leadingcomma then ", " else "" end) + @Parameters.select{|p| !p.NonMethodParam}.
                                                  map(&:formatForArgumentList).join(", ")
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
        ", #{componentclass}::Data{" + @Parameters.map(&:formatForArgumentList).join(", ") +
          "}"
      else
        ", " + @Parameters.map(&:formatForArgumentList).join(", ")
      end
    end
  end
  
end

class Variable
  attr_reader :Name, :Type, :Default, :NonMethodParam, :AngelScriptRef, :AngelScriptUseInstead,
              :MemberAccess, :NonSerializeParam, :NoConst

  def initialize(name, type, default: nil, noRef: false, noConst: false, nonMethodParam: false,
                 move: false, serializeas: nil,
                 angelScriptRef: "in", angelScriptUseInstead: nil, memberaccess: nil,
                 nonserializeparam: false)

    @Name = name
    @Type = type
    @Default = default
    # This is for accessing this parameter from the finished object, instead of the normal
    # way around
    @MemberAccess = memberaccess
    @NonSerializeParam = nonserializeparam

    if !@Default.nil?
      if @Default == true
        @Default = "true"
      elsif @Default == false
        @Default = "false"
      end

      # Empty string is output as two quotes
      if @Default.is_a? String and @Default == ""

        @Default = %{""}
        
      end
    end
    
    @NoRef = noRef
    @NoConst = noConst
    @NonMethodParam = nonMethodParam
    @Move = move
    @SerializeAs = serializeas
    @AngelScriptRef = angelScriptRef
    @AngelScriptUseInstead = angelScriptUseInstead
  end

  def formatDefinition()
    "#{@Type} #{@Name}#{formatDefault(header: true)};"
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

  # Formats full definition for use in parameter list
  def formatForParams(opts)
    if @NoRef
      "#{@Type} #{@Name.downcase}#{formatDefault opts}"
    elsif @Move
      "#{@Type}&& #{@Name.downcase}#{formatDefault opts}"
    else
      if @NoConst
        "#{@Type} &#{@Name.downcase}#{formatDefault opts}"
      else
        "const #{@Type} &#{@Name.downcase}#{formatDefault opts}"
      end
    end
  end

  # Formats only the type. Useful for use in templates
  def formatType()
    if @NoRef
      "#{@Type}"
    elsif @Move
      "#{@Type}&&"
    else
      if @NoConst
        "#{@Type}&"
      else
        "const #{@Type}&"
      end
    end    
  end


  # Formatting for angelscript bindings
  def TypeAS
    # Standard translations that reduce typing
    case @Type
    when "std::string"
      "string"
    when "uint8_t"
      "uint8"
    else
      @Type.sub('*', '@')
    end
  end
  
  # Formats full definition for use in parameter list
  def formatForParamsAngelScript()

    if(@AngelScriptUseInstead)
      return @AngelScriptUseInstead.formatForParamsAngelScript()
    end
    
    opts = {header: true}
    if @NoRef
      "#{self.TypeAS} #{@Name.downcase}#{formatDefault opts}"
    elsif @Move
      "#{self.TypeAS}&& #{@Name.downcase}#{formatDefault opts}"
    else
      if @NoConst
        "#{self.TypeAS} &#{@AngelScriptRef} #{@Name.downcase}#{formatDefault opts}"
      else
        "const #{self.TypeAS} &#{@AngelScriptRef} #{@Name.downcase}#{formatDefault opts}"
      end
    end
  end  

  def formatForArgumentList()
    if !@NonMethodParam
      "#{@Name.downcase}"
    else
      "#{@Name}"
    end
  end

  def formatInitializer()
    if @Move
      # Move constructor
      "#{@Name}(std::move(#{@Name.downcase}))" 
    else
      "#{@Name}(#{@Name.downcase})" 
    end
  end

  def formatSerializer()
    if @SerializeAs.nil?
      "#{@Name}"
    else
      "static_cast<#{@SerializeAs}>(#{@Name})"
    end
  end

  def formatCopy()
    "#{@Name}(other.#{@Name})" 
  end

  def formatMove()
    "#{@Name}(std::move(other.#{@Name}))" 
  end  

  def formatDeserializer(packetname)
    if @SerializeAs.nil?
      "#{packetname} >> #{@Name};"
    else
      tempName = "temp_" + @Name
      str = "#{@SerializeAs} #{tempName};\n"
      str += "#{packetname} >> #{tempName};\n"
      str += "#{@Name} = static_cast<#{@Type}>(#{tempName});\n"
      str
    end
    
  end
end

# For easily adding methods to classes
class GeneratedMethod

  attr_reader :Name, :ReturnType, :Body

  def initialize(name, type, parameters, body: "")
    @Name = name
    @ReturnType = type
    @Parameters = parameters
    @Body = body
  end

  def formatParameters(opts)
    if !@Parameters
      ""
    else
      @Parameters.map{|v| v.formatForParams opts}.join(", ")
    end
  end
end

# Components for adding to gameworld
class EntityComponent
  
  attr_reader :type, :constructors, :StateType, :Release, :CustomStaticSerializer,
              :CustomStaticLoader, :NoSynchronize
  
  def initialize(type, constructors=[ConstructorInfo.new], statetype: nil, releaseparams: nil,
                 customstaticserializer: nil, customstaticloader: nil, nosynchronize: false)
    @type = type
    @constructors = constructors
    @StateType = statetype
    @Release = releaseparams
    @CustomStaticSerializer = customstaticserializer
    @CustomStaticLoader = customstaticloader
    @NoSynchronize = nosynchronize
  end

end

class EntitySystem
  attr_reader :Type, :NodeComponents, :RunTick, :RunRender, :Init, :Release, :NoState,
              :VisibleToScripts

  # Leave nodeComponens empty if not using combined nodes
  def initialize(type, nodeComponents=[], runtick: nil, runrender: nil, init: nil, 
                 release: nil, nostate: nil, visibletoscripts: false)
    @Type = type
    @NodeComponents = nodeComponents
    @RunTick = runtick
    @RunRender = runrender
    @Init = init
    @Release = release
    # If NoState is true then this doesn't hold nodes and .Clear() isn't called on this
    @NoState = nostate
    @VisibleToScripts = visibletoscripts

    if @Init
      raise "wrong type" unless @Init.is_a? Array
    end
    if @Release
      raise "wrong type" unless @Release.is_a? Array
    end
  end
end

def formatEntitySystemParameters(params)
  if params.include?(:parameters) and !params[:parameters].empty? then
    ", " + params[:parameters].join(", ")
  else "" end 
end

def genComparisonExpression(values)

  values.map{|v| v[0] + " == " + v[1]}.join(" && ")
  
end


