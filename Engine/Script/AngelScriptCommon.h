void ScriptMessageCallback(const asSMessageInfo *msg, void *param){

	if(msg->type == asMSGTYPE_WARNING){
        
		Logger::Get()->Write(string("[SCRIPT] [WARNING] ")+msg->section+" ("+Convert::ToString(msg->row)+
            ", "+Convert::ToString(msg->col)+") : "+msg->message);
        
	} else if(msg->type == asMSGTYPE_INFORMATION){

		Logger::Get()->Write(string("[SCRIPT] [INFO] ")+msg->section+" ("+Convert::ToString(msg->row)+", "+
            Convert::ToString(msg->col)+") : "+msg->message);
        
	} else {

		Logger::Get()->Write(string("[SCRIPT] [ERROR] ")+msg->section+" ("+Convert::ToString(msg->row)+
            ", "+Convert::ToString(msg->col)+") : "+msg->message);
	}
	
	return;
}









