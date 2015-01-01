void ScriptMessageCallback(const asSMessageInfo *msg, void *param){

	if(msg->type == asMSGTYPE_WARNING){
        
		Logger::Get()->Warning(string("[SCRIPT] ")+msg->section+" ("+Convert::ToString(msg->row)+
            ", "+Convert::ToString(msg->col)+") : "+msg->message);
        
	} else if(msg->type == asMSGTYPE_INFORMATION){

		Logger::Get()->Info(string("[SCRIPT] ")+msg->section+" ("+Convert::ToString(msg->row)+", "+
            Convert::ToString(msg->col)+") : "+msg->message);
        
	} else {

		Logger::Get()->Error(string("[SCRIPT] ")+msg->section+" ("+Convert::ToString(msg->row)+
            ", "+Convert::ToString(msg->col)+") : "+msg->message);
	}
	
	return;
}









