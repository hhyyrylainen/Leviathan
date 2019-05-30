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
