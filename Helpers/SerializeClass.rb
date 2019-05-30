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

    self.addDeserializeArg "#{name}* referencestate"
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
