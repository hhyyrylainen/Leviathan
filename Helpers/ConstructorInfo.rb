class ConstructorInfo
  attr_reader :Parameters, :UseDataStruct, :NoAngelScript, :BaseParameters

  def initialize(parameters, usedatastruct: false, memberinitializers: nil,
                 noangelscript: false, baseparameters: nil)
    @Parameters = parameters
    @UseDataStruct = usedatastruct
    @MemberInitializers = memberinitializers
    @NoAngelScript = noangelscript
    @BaseParameters = baseparameters
  end

  def formatMemberInitializers
    return '' unless @MemberInitializers

    ' : ' + @MemberInitializers.map { |i| i[0] + '(' + i[1] + ')' }.join(', ')
  end

  # Formats the parameters of this function for a parameter list
  def formatParameters(opts, leadingcomma: true)
    if @Parameters.empty? || @Parameters.reject(&:NonMethodParam).empty?
      ''
    else
      (leadingcomma ? ', ' : '') + @Parameters.reject(&:NonMethodParam).map do |p|
        p.formatForParams opts
      end.join(', ')
    end
  end

  # Formats the types of parameters of this function for a parameter list
  def formatParameterTypes(leadingcomma: true)
    if @Parameters.empty? || @Parameters.reject(&:NonMethodParam).empty?
      ''
    else
      (leadingcomma ? ', ' : '') + @Parameters.reject(&:NonMethodParam).map(&:formatType).join(', ')
    end
  end

  #
  # AngelScript binding versions
  #
  def formatParametersAngelScript(leadingcomma: true)
    if @Parameters.empty? || @Parameters.reject(&:NonMethodParam).empty?
      ''
    else
      (leadingcomma ? ', ' : '') + @Parameters.reject(&:NonMethodParam).map(&:formatForParamsAngelScript).join(', ')
    end
  end

  def formatNamesForForward(leadingcomma: true)
    if @Parameters.empty? || @Parameters.reject(&:NonMethodParam).empty?
      ''
    else
      (leadingcomma ? ', ' : '') + @Parameters.reject(&:NonMethodParam)
                                   .map(&:formatForArgumentList).join(', ')
    end
  end

  def formatNames(componentclass)
    if @Parameters.empty?
      if @UseDataStruct
        ", #{componentclass}::Data{}"
      else
        ''
      end
    else
      if @UseDataStruct
        ", #{componentclass}::Data{" + @Parameters.map(&:formatForArgumentList).join(', ') +
          '}'
      else
        ', ' + @Parameters.map(&:formatForArgumentList).join(', ')
      end
    end
  end

  # Returns true if this needs a wrapper for scripts
  def needs_wrapper
    @Parameters.reject(&:NonMethodParam).any?(&:needs_wrapper)
  end

  def format_wrapped_parameters(opts, leading_comma: true)
    if @Parameters.empty? || @Parameters.reject(&:NonMethodParam).empty?
      ''
    else
      (leading_comma ? ', ' : '') + @Parameters.reject(&:NonMethodParam).map do |p|
        p.format_for_wrapped_parameters opts
      end.join(', ')
    end
  end

  def format_wrapped_parameter_types(leading_comma: true)
    if @Parameters.empty? || @Parameters.reject(&:NonMethodParam).empty?
      ''
    else
      (leading_comma ? ', ' : '') + @Parameters.reject(&:NonMethodParam)
                                    .map(&:format_type_wrapped).join(', ')
    end
  end

  def format_parameters_for_angelscript_wrapped(leading_comma: true)
    if @Parameters.empty? || @Parameters.reject(&:NonMethodParam).empty?
      ''
    else
      (leading_comma ? ', ' : '') + @Parameters.reject(&:NonMethodParam)
                                    .map(&:format_for_params_angelscript_wrapped).join(', ')
    end
  end

  def format_wrapped_names(componentclass)
    if @Parameters.empty?
      if @UseDataStruct
        ", #{componentclass}::Data{}"
      else
        ''
      end
    else
      if @UseDataStruct
        ", #{componentclass}::Data{" +
          @Parameters.map(&:format_for_wrapped_argument_list).join(', ') + '}'
      else
        ', ' + @Parameters.map(&:format_for_wrapped_argument_list).join(', ')
      end
    end
  end
end
