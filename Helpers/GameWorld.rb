class GameWorldClass < OutputClass

  attr_accessor :WorldType

  def initialize(name, componentTypes: [], systems: [], systemspreticksetup: nil,
                 framesystemrun: "", networking: true, perworlddata: [])

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

    @PerWorldData = perworlddata

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
      @Members.push(Variable.new("_" + s.Name, s.Type))
    }

    @Members.concat @PerWorldData

    if @Members.empty?
      # Needed to call genMemberConstructor which is always needed
      @Members.push Variable.new("dummy", "int")
    end
  end

  # Default includes
  def getExtraIncludes
    return ["Script/ScriptConversionHelpers.h", "boost/range/adaptor/map.hpp"]
  end

  def genMemberConstructor(f, opts)

    f.write "#{export}#{qualifier opts}#{@Name}(const " +
            "std::shared_ptr<Leviathan::PhysicsMaterialManager>& physicsMaterials, " +
            "int worldid#{default opts, '-1'})"
    
    if opts.include?(:impl)
      f.puts " : #{@BaseClass}(#{@WorldType},\n physicsMaterials, worldid) "
      genPerWorldConstructors f, opts
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
      genPerWorldConstructors f, opts
      f.puts "{}"
    else
      f.puts ";"
    end    

  end

  def genPerWorldConstructors(f, opts)
    @PerWorldData.each{|s|
      f.puts ", " + s.Name + "(*this)"
    }
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

      @PerWorldData.each{|s|
        f.puts s.Name + ".OnClear();"
      }
      
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
          f.puts "_#{s.Name}.Clear();"
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

      if opts.include?(:header)
        f.write "#{export}inline uint64_t #{qualifier opts}GetComponentCount_#{c.type}() " +
                "const"

        f.puts "{"
        f.puts "    return Component#{c.type}.GetIndexSize();"
        f.puts "}"
      end

      if opts.include?(:header)
        f.write "#{export}inline #{c.type}* #{qualifier opts}GetComponentPtrByIndex_" +
                "#{c.type}(uint64_t index)"

        f.puts "{"
        f.puts "    return Component#{c.type}.GetAtIndex(index);"
        f.puts "}"
      end

      if opts.include?(:header)
        f.puts "//! \\note This creates a new array object on each call"
      end
      f.write "#{export}CScriptArray* #{qualifier opts}GetComponentIndexWrapper_#{c.type}()"
      if opts.include?(:impl)
        f.puts "{"
        f.puts "    const auto& index = Component#{c.type}.GetIndex();"
        f.puts "    asIScriptContext* ctx = asGetActiveContext();"

        f.puts "    asIScriptEngine* engine = ctx ? ctx->GetEngine() : " +
               "Leviathan::ScriptExecutor::Get()->GetASEngine();"

        f.puts "    return ConvertIteratorToASArray((index | " +
               "boost::adaptors::map_keys).begin(),"
        f.puts "          (index | boost::adaptors::map_keys).end(), " +
               %{engine, "array<ObjectID>");}
        f.puts "}"
      else
        f.puts ";"
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

        f.puts "#{export}#{s.Type}& Get#{s.Name}(){ return _#{s.Name}; }"
      }

      f.puts ""
    end

    # Gets for per world data objects
    if opts.include?(:header)
      f.puts "// Per world data object gets"
      @PerWorldData.each{|s|

        f.puts "#{export}#{s.Type}& Get#{s.Type}(){ return #{s.Name}; }"
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

            f.puts "    receiver << " + p.formatMemberSerializer(c.type.downcase + "->") + ";"
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
        
        f.puts "_#{s.Name}.Run(*this" +
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
        
        f.puts "_#{s.Name}.Run(*this" +
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
          
          f.puts "    _#{s.Name}.CreateNodes("
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
          
          f.puts "    _#{s.Name}.DestroyNodes("
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
          f.puts "_#{s.Name}.Init(#{s.Init.map(&:formatForArgumentList).join(', ')});"
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
          f.puts "_#{s.Name}.Release(#{s.Release.map(&:formatForArgumentList).join(', ')});"
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
            f.puts p.formatDeserializer("data", target: p.Name.downcase)
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

      f.puts ""

      f.write "#{export}void #{qualifier opts}_CreateStatesFromUpdateMessage(" +
              "ObjectID id, int32_t ticknumber, sf::Packet& data, int32_t referencetick, " +
              "int decodedtype)" +
              "#{override opts}"

      if opts.include?(:impl)
        f.puts "{"

        f.puts "while(true){"

        f.puts "if(decodedtype == -1){"
        f.puts "    // Type not decoded yet"
        f.puts "    uint16_t tmpType;"
        f.puts "    data >> tmpType;"
        f.puts "    decodedtype = tmpType;"
        f.puts "}"
        f.puts "if(!data){"
        f.puts %{    // Ended, there is no entry count in the message}
        f.puts "    return;"
        f.puts "}"
        
        f.puts ""

        f.puts "switch(decodedtype){"

        @ComponentTypes.each{|c|

          if !c.StateType
            next
          end

          f.puts "case static_cast<int>(#{c.type}::TYPE):"
          f.puts "{"
          f.puts "const auto& #{c.type.downcase} = Component#{c.type}.Find(id);"
          f.puts "if(#{c.type.downcase}){"
          # This is needed for the interpolation system to be able to
          # know that it should interpolate stuff
          f.puts "     #{c.type.downcase}->StateMarked = true;"
          f.puts "} else {"
          f.puts %{    LOG_ERROR("GameWorld: received states for not created Component, "}
          f.puts %{        "can't mark states as active. And the states may now be } +
                 %{kept forever");}
          f.puts "}"
          f.puts ""
          f.puts "#{c.type}States.DeserializeState(id, ticknumber, data, referencetick);"
          f.puts "decodedtype = -1;"
          f.puts "continue;"
          f.puts "}"
        }

        f.puts "default:"
        f.puts "return #{@BaseClass}::_CreateStatesFromUpdateMessage(id, ticknumber, data, " +
               "referencetick, decodedtype);"      
        
        f.puts "}"
        
        f.puts "}"
        f.puts "}"

      else
        f.puts ";"
      end

      f.puts ""

      f.write "#{export}void #{qualifier opts}_ApplyLocalControlUpdateMessage(" +
              "ObjectID id, int32_t ticknumber, sf::Packet& data, int32_t referencetick, " +
              "int decodedtype)" +
              "#{override opts}"

      if opts.include?(:impl)
        f.puts "{"

        f.puts "while(true){"

        f.puts "if(decodedtype == -1){"
        f.puts "    // Type not decoded yet"
        f.puts "    uint16_t tmpType;"
        f.puts "    data >> tmpType;"
        f.puts "    decodedtype = tmpType;"
        f.puts "}"
        f.puts "if(!data){"
        f.puts %{    // Ended, there is no entry count in the message}
        f.puts "    return;"
        f.puts "}"
        
        f.puts ""

        f.puts "switch(decodedtype){"

        @ComponentTypes.each{|c|

          if !c.StateType
            next
          end

          f.puts "case static_cast<int>(#{c.type}::TYPE):"
          f.puts "{"
          f.puts "const auto& #{c.type.downcase} = Component#{c.type}.Find(id);"
          f.puts "if(#{c.type.downcase}){"
          # This is will mark the component if this is a newer state
          # update than the latest. In the future it would be nice to
          # just be able to echo the update message to other clients
          f.puts "    #{c.type}States.DeserializeAndApplyState(id, *#{c.type.downcase}, " +
                 "ticknumber, data, referencetick);"
          f.puts "} else {"
          f.puts %{    LOG_ERROR("GameWorld: received local control states for not created , "}
          f.puts %{        "Component, this is the client's fault");}
          f.puts "}"
          f.puts ""

          f.puts "decodedtype = -1;"
          f.puts "continue;"
          f.puts "}"
        }

        f.puts "default:"
        f.puts "return #{@BaseClass}::_ApplyLocalControlUpdateMessage(id, ticknumber, data, " +
               "referencetick, decodedtype);"      
        
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
    str += %{if(engine->RegisterObjectMethod(classname, "array<ObjectID>@ GetComponentIndex_} +
           %{#{c.type}()", \n} +
           %{asMETHOD(WorldType, GetComponentIndexWrapper_#{c.type}), asCALL_THISCALL) } +
           %{< 0)\n} +
           "{\nANGELSCRIPT_REGISTERFAIL;\n}\n"
    
    str += %{if(engine->RegisterObjectMethod(classname, "uint64 GetComponentCount_#{c.type}} +
           %{()", \n} +
           %{asMETHOD(WorldType, GetComponentCount_#{c.type}), asCALL_THISCALL) < 0)\n} +
           "{\nANGELSCRIPT_REGISTERFAIL;\n}\n"
    str += %{if(engine->RegisterObjectMethod(classname, "#{c.type}@ GetComponentByIndex_} +
           %{#{c.type}(uint64 index)", \n} +
           %{asMETHOD(WorldType, GetComponentPtrByIndex_#{c.type}), asCALL_THISCALL) < 0)\n} +
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
        str += %{if(engine->RegisterObjectMethod(classname, "#{s.Type}@ Get#{s.Name}()",\n} +
               %{asMETHOD(WorldType, Get#{s.Name}), asCALL_THISCALL) < 0)\n} +
               "{\nANGELSCRIPT_REGISTERFAIL;\n}\n"
      end
    }

    @PerWorldData.each{|s|
      if s.AngelScriptUseInstead && s.AngelScriptUseInstead == ""
        next
      end

      useType = s.Type + "@"

      if s.AngelScriptUseInstead
        useType = s.AngelScriptUseInstead
      end

      str += %{if(engine->RegisterObjectMethod(classname, "#{useType} Get#{s.Type}()",\n} +
               %{asMETHOD(WorldType, Get#{s.Type}), asCALL_THISCALL) < 0)\n} +
               "{\nANGELSCRIPT_REGISTERFAIL;\n}\n"
    }
    
    str
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
              :VisibleToScripts, :Name

  # Leave nodeComponens empty if not using combined nodes
  def initialize(type, nodeComponents=[], runtick: nil, runrender: nil, init: nil, 
                 release: nil, nostate: nil, visibletoscripts: false)
    @Type = type
    @Name = sanitizeName(type)
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
