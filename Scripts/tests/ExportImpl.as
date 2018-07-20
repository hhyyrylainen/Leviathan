shared enum OPERATION_TYPE{
    None,
    Double
}

shared int PerformOperationFromImported(int value, OPERATION_TYPE op)
{
    switch(op){
    case OPERATION_TYPE::Double:
        return value * value;
    }
    
    return value;    
}
