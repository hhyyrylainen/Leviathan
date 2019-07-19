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
